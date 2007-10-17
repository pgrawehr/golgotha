/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/

#include "pch.h"
#ifdef NETWORK_INCLUDED
#include "net/startup.h"
#include "net/client.h"
#include "net/server.h"

#include "loaders/load.h"

#include "mess_id.h"

#include "time/timedev.h"

#include "app/app.h"

#include "resources.h"

#include "device/device.h"
#include "device/event.h"

#include "file/ram_file.h"
#include "file/get_filename.h"

#include "gui/text_input.h"
#include "gui/text.h"
#include "gui/button.h"
#include "gui/smp_dial.h"

#include "network/login.h"
#include "network/net_find.h"
#include "network/net_prot.h"
#include "network/net_addr.h"
#include "network/net_sock.h"

#include "global_id.h"
#include "g1_object.h"
#include "tick_count.h"
#include "player.h"
#include "team_api.h"
#include "memory/hashtable.h"
#ifdef _WINDOWS
#include <windows.h>
#include <winsock.h>
#endif
//#ifdef __linux
//#include <sys/socket.h>
//#include <netinet/in.h>
//#include <arpa/inet.h>
//#endif

i4_temp_file_class * network_file=0;

static w16 stank_type=0;
i4_event_reaction_class * g1_net_window_class::create_orec(int mess_id)
{
	i4_object_message_event_class * om=new i4_object_message_event_class(this, mess_id);

	return new i4_event_reaction_class(this, om);
}

g1_net_window_class::g1_net_window_class(w16 w, w16 h,
										 i4_graphical_style_class * style,
										 i4_net_protocol * protocol,
										 char * bg_res,
										 int poll_delay,
										 int poll_id)
	: i4_parent_window_class(w,h),
	  style(style),
	  protocol(protocol),
	  poll_delay(poll_delay),
	  poll_id(poll_id)
{
	bg=i4_load_image(i4gets(bg_res));

	i4_object_message_event_class poll(this, poll_id);
	poll_event_id=i4_time_dev.request_event(this, &poll, poll_delay);
}

void g1_net_window_class::receive_event(i4_event * ev)
{
	if (ev->type()==i4_event::OBJECT_MESSAGE)
	{
		CAST_PTR(oev, i4_object_message_event_class, ev);
		if (oev->object==this)
		{
			if (oev->sub_type==(w32)poll_id)
			{
				poll();

				i4_object_message_event_class poll(this, poll_id);
				poll_event_id=i4_time_dev.request_event(this, &poll, poll_delay);
			}
			else
			{
				object_message(oev->sub_type);
			}
		}
		else
		{
			object_message(-1);
			i4_parent_window_class::receive_event(ev);
		}
	}
	else
	{
		i4_parent_window_class::receive_event(ev);
	}
}


void g1_net_window_class::parent_draw(i4_draw_context_class &context)
{
	if (bg)
	{
		local_image->clear(0, context);
		bg->put_image(local_image, 0, 0, context);
	}
}


g1_net_window_class::~g1_net_window_class()
{
	i4_time_dev.cancel_event(poll_event_id);

	if (bg)
	{
		delete bg;
	}
}

g1_startup_window::g1_startup_window(w16 w, w16 h,
									 i4_graphical_style_class * style,
									 i4_net_protocol * protocol)
	: g1_net_window_class(w,h, style, protocol, "net_bg_image", 500, POLL)
{
	serverip=0;
	hostname=new i4_text_input_class(style, i4_string_man.get(0), 190, 200, this);
	add_child(g1_resources.net_hostname_x, g1_resources.net_hostname_y, hostname);

	username=new i4_text_input_class(style, *g1_resources.username, 90, 10, this);
	add_child(g1_resources.net_username_x, g1_resources.net_username_y, username);

	i4_text_window_class * start_game_text=new i4_text_window_class(i4gets("start_server"), style);
	i4_button_class * new_game;
	new_game=new i4_button_class(0,start_game_text, style, create_orec(START_SERVER));
	add_child(g1_resources.net_start_x, g1_resources.net_start_y, new_game);

	i4_text_window_class * quit_text=new i4_text_window_class(i4gets("net_main_menu"), style);
	add_child(new_game->x(), new_game->y()+new_game->height()+1,
			  new i4_button_class(0, quit_text, style, create_orec(QUIT_NET_GAME)));

	i4_net_protocol * p=protocol;
	if (p)
	{
		find=p->create_finder_socket(g1_resources.net_find_port, g1_resources.net_find_port);
	}
	else
	{
		find=0;
	}

	t_buts=0;
	buts=0;
}

void g1_startup_window::free_buts()
{
	for (int i=0; i<t_buts; i++)
	{
		remove_child(buts[i]);
		delete buts[i];
	}

	if (buts)
	{
		i4_free(buts);
	}
	buts=0;
	t_buts=0;
}

void g1_startup_window::set_server_address()
{
	serverip=0;
	i4_query_text_input_class q;
	i4_kernel.send_event(hostname, &q);
	i4_str * setting=q.copy_of_data;
	q.copy_of_data=0;
	char buf[100];
	i4_os_string(*setting,buf,100);
	serverip=inet_addr(buf); //returns ip in network order format
	if (serverip==INADDR_NONE)
	{
		serverip=0;
	}
	if (find)
	{
		find->connect_to(serverip);
	}

	delete setting;
}

void g1_startup_window::grab_uname()
{
	delete g1_resources.username;
	i4_query_text_input_class q;
	i4_kernel.send_event(username, &q);
	g1_resources.username=q.copy_of_data;
	q.copy_of_data=0;
}

void g1_startup_window::object_message(int id)
{
	set_server_address();
	switch (id)
	{
		case START_SERVER:
			{
				grab_uname();

				g1_server=new g1_server_class(g1_resources.net_udp_port, protocol);

				i4_user_message_event_class u(G1_SERVER_MENU);
				i4_kernel.send_event(i4_current_app, &u);
			} break;

		case QUIT_NET_GAME:
			{
				grab_uname();

				i4_user_message_event_class u(G1_MAIN_MENU);
				i4_kernel.send_event(i4_current_app, &u);
			} break;
	}

	if (id>=LAST)
	{
		grab_uname();

		i4_finder_socket::server s;
		if (find)
		{
			find->get_server(id-LAST, s);
		}
		else
		{
			i4_finder_socket::get_localhost(s);
		}

		s.addr->set_port(g1_resources.net_udp_port);
		g1_client=new g1_client_class(s.addr, g1_resources.net_udp_port, protocol);

		i4_user_message_event_class u(G1_CLIENT_JOINED_MENU);
		i4_kernel.send_event(i4_current_app, &u);
	}
}

void g1_startup_window::poll()
{
	if (find)
	{
		if (find->poll())
		{
			free_buts();

			t_buts=find->total_servers();
			buts=(i4_button_class * *)I4_MALLOC(sizeof(i4_button_class *)*t_buts,"but array");

			int y=g1_resources.net_found_y, x=g1_resources.net_found_x1;

			for (int i=0; i<t_buts; i++)
			{
				i4_finder_socket::server s;
				find->get_server(i, s);

				i4_text_window_class * t=new i4_text_window_class(*s.notification_string, style);
				buts[i]=new i4_button_class(0, t, style, create_orec(LAST + i));

				add_child(x,y, buts[i]);
				y+=buts[i]->height()+1;
			}
		}
	}
	//if find is null, this usually means the broadcast class couldn't be initialized
	//because the port was already open, possibly by another instance of golgotha.
	//Let's add "localhost" to the list in this case.
	else
	{
		free_buts();
		i4_finder_socket::server s;
		i4_finder_socket::get_localhost(s);
		buts=(i4_button_class * *)I4_MALLOC(sizeof(i4_button_class *),"but array");
		t_buts=1;
		i4_text_window_class * loc=new i4_text_window_class(*s.notification_string, style);
		buts[0]=new i4_button_class(0,loc,style,create_orec(LAST));
		int y1=g1_resources.net_found_y, x1=g1_resources.net_found_x1;
		add_child(x1,y1,buts[0]);

	}
}

g1_startup_window::~g1_startup_window()
{
	if (find)
	{
		delete find;
	}

	free_buts();
}

g1_server_start_window::g1_server_start_window(w16 w, w16 h,
											   i4_graphical_style_class * style,
											   i4_net_protocol * protocol)
	: g1_net_window_class(w,h, style, protocol, "server_bg_image", 100, POLL)
{
	memset(names,0,sizeof(names));

	i4_text_window_class * start_game_text=new i4_text_window_class(i4gets("start_net_game"),
																	style);

	i4_object_message_event_class * sng_e=new i4_object_message_event_class(this, START_NET_GAME);
	i4_event_reaction_class * sng_r=new i4_event_reaction_class(this, sng_e);
	i4_button_class * new_game=new i4_button_class(0, start_game_text, style, sng_r);
	add_child(480,20, new_game);

	i4_text_window_class * quit_text=new i4_text_window_class(i4gets("net_main_menu"), style);
	i4_object_message_event_class * q_e=new i4_object_message_event_class(this, QUIT_NET_GAME);
	i4_event_reaction_class * q_r=new i4_event_reaction_class(this, q_e);
	i4_button_class * q=new i4_button_class(0, quit_text, style, q_r);
	add_child(new_game->x(), new_game->y()+new_game->height()+1, q);

	if (protocol)
	{
		note=protocol->create_notifier_socket(g1_resources.net_find_port, *g1_resources.username);
	}
	else
	{
		note=0;
	}
}


void g1_server_start_window::object_message(int id)
{
	switch (id)
	{
		case START_NET_GAME:
			{
				i4_user_message_event_class u(G1_START_NEW_GAME);
				i4_kernel.send_event(i4_current_app, &u);
				g1_server->start_game();
			} break;

		case QUIT_NET_GAME:
			{
				if (g1_server)
				{
					delete g1_server;
					g1_server=0;
				}

				i4_user_message_event_class u(G1_MAIN_MENU);
				i4_kernel.send_event(i4_current_app, &u);
			} break;
	}
}

void g1_server_start_window::poll()
{
	if (note)
	{
		note->poll();
	}

	if (g1_server)
	{
		g1_server->poll();
		if (g1_server->list_changed)
		{
			g1_server->list_changed=i4_F;

			int i;
			for (i=0; i<G1_MAX_PLAYERS; i++)
			{
				if (names[i])
				{
					remove_child(names[i]);
					names[i]=0;
				}
			}

			i4_text_window_class * t;
			int x=i4getn("net_joined_x"), y=i4getn("net_joined_y");

			t=new i4_text_window_class(*g1_resources.username, style);
			add_child(x, y, t);
			y+=t->height()+1;
			names[0]=t;

			for (i=1; i<G1_MAX_PLAYERS; i++)
			{
				if (g1_server->clients[i].addr)
				{
					t=new i4_text_window_class(*g1_server->clients[i].username, style);
					add_child(x,y,t);
					y+=t->height()+1;
					names[i]=t;
				}
			}
		}
	}
}

g1_server_start_window::~g1_server_start_window()
{
	if (note)
	{
		delete note;
	}
}



g1_client_wait_window::g1_client_wait_window(w16 w, w16 h,
											 i4_graphical_style_class * style,
											 i4_net_protocol * protocol)
	: g1_net_window_class(w,h, style, protocol, "client_wait_image", 100, POLL)
{
	i4_text_window_class * quit_text=new i4_text_window_class(i4gets("net_cancel"), style);
	i4_object_message_event_class * q_e=new i4_object_message_event_class(this, QUIT_NET_GAME);
	i4_event_reaction_class * q_r=new i4_event_reaction_class(this, q_e);
	i4_button_class * q=new i4_button_class(0, quit_text, style, q_r);

	add_child(i4getn("net_cancel_x"), i4getn("net_cancel_y"), q);

}

void g1_client_wait_window::object_message(int id)
{
	if (id==QUIT_NET_GAME)
	{
		delete g1_client;
		g1_client=0;
		i4_user_message_event_class u(G1_MAIN_MENU);
		i4_kernel.send_event(i4_current_app, &u);
	}
}

void g1_client_wait_window::poll()
{
	if (!g1_client || !g1_client->poll())
	{
		i4_user_message_event_class u(G1_MAIN_MENU);
		i4_kernel.send_event(i4_current_app, &u);
	}
}

g1_client_wait_window::~g1_client_wait_window()
{
//upon closing the wait window, we kill the client?
//very bad idea: Not very sensefull if we close it because the game has started
/*if (g1_client)
   {
   delete g1_client;
   g1_client=0;
   }
 */
}
// CLIENT.CPP
/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/

//This is a common function for server and client
//Todo: Write some functions that handle the state of the entire system
//to keep track of who has the latest version of which object
//such that the server doesn't return packets not needed by the client
//and the client can filter out packets where he has newer data than
//the server. This particularly means that an user-controlled object
//must not get remote-synced..

g1_network_time_manager_class * g1_network_time_man_ptr=0;

g1_network_time_manager_class g1_network_time_man;

i4_bool g1_network_time_manager_class::shouldupdate(w32 id, w32 totimestamp)
{
	net_sync_entry * cur=net_sync.get(id);
	g1_object_class * unit=g1_global_id.get(id);

	if (unit && (unit->id==stank_type) && (unit->player_num==g1_player_man.local_player))
	{
		lasterror=OWNSUPERTANK;    //must newer sync the own stank
		return i4_F;
	}
	if ((!cur) || (cur->ticksynced<totimestamp))
	{
		lasterror=0;
		return i4_T;
	}
	else
	{
		lasterror=OUTOFDATE;
		return i4_F;
	}
};
void g1_network_time_manager_class::updatecomplete(w32 id, w32 attimestamp)
{
	net_sync_entry * nse=new net_sync_entry,* old;

	nse->tickcalced=attimestamp;
	nse->ticksynced=attimestamp;
	old=net_sync.get(id);
	if (old)
	{

		net_sync.replace(id,nse,i4_F);    //adds if it doesn't exist already
		delete old;
	}
	else
	{
		net_sync.insert(id,nse);
	}
};
void g1_network_time_manager_class::deletedobject(w32 id)
{
	net_sync_entry * old=net_sync.remove(id);

	if (old)
	{
		delete old;
	}
	else
	{
		lasterror=OBJDOESNOTEXIST;
	}
};
void process_data_packet(i4_file_class &r,i4_bool server)
{
	r.read_32(); //skip empty field
	w32 ticksent=r.read_32(); //to which tick does this packet belong?
	//here follows: reading out the objects
	g1_realtime_loader_class * rtloader=new g1_realtime_loader_class(&r,i4_F,i4_F);

	w32 objfor=rtloader->read_32();
	w16 typefor=0;
	while (objfor!=0)
	{
		if (objfor==g1_global_id_manager_class::ID_DELETEPROPAGATE)
		{
			//don't delete if object just doesn't exist locally.
			objfor=rtloader->read_32();
			g1_object_class * obdel=g1_global_id.checked_get(objfor);
			if (obdel)
			{
				obdel->set_flag(g1_object_class::THINKING,1); //Perhaps was not thinking locally
				obdel->stop_thinking();
				obdel->unoccupy_location();
				obdel->request_remove();
			}
		}
		else
		{
			typefor=rtloader->read_16();
			g1_object_class * obj=g1_global_id.checked_get(objfor);
			if (!obj)
			{
				//Object seems to be new
				//obj=g1_create_object(typefor);
				if (typefor<=g1_last_object_type)
				{
					obj=g1_object_type_array[typefor]->create_object(typefor,rtloader);
					//the object will be in sync with the load
					obj->occupy_location();
					obj->grab_old();
					i4_warning("Remotelly creating object ID %i as %s.",obj->global_id,g1_object_type_array[typefor]->name());
					if (obj->get_flag(g1_object_class::THINKING))
					{
						//cannot request_think() without resetting this flag.
						obj->set_flag(g1_object_class::THINKING,0);
						obj->request_think();
					}

					g1_player_man.get(obj->player_num)->add_object(obj->global_id);
					if (typefor==g1_get_object_type("stank"))
					{
						// a new enemy supertank. Needs a few special updates
						g1_player_man.get(obj->player_num)->num_stank_lives()--;
						g1_player_man.get(obj->player_num)->calc_upgrade_level();
						g1_player_man.get(obj->player_num)->set_commander(g1_player_piece_class::cast(obj));
						//g1_player_man.get(obj->player_num)->continue_wait=i4_F;
					}
					g1_network_time_man.updatecomplete(obj->global_id,ticksent);
				}
				else
				{
					i4_error("ERROR: Unknown object type requested.");
					delete rtloader;
					return;
				}
				//g1_global_id.assign(objfor,obj);
			}
			else
			{
				if (g1_network_time_man.shouldupdate(obj->global_id,ticksent))
				{
					obj->load(rtloader);
					g1_network_time_man.updatecomplete(obj->global_id,ticksent);
				}
				else
				{
					//i4_warning("Skipping update of object %i, data is outdated.",obj->global_id);
					obj->skipload(rtloader);
				}
			}
			if (server)
			{
				obj->set_flag(g1_object_class::NEEDS_SYNC,1);
			}
			else
			{
				obj->set_flag(g1_object_class::NEEDS_SYNC,0);
			}
		}
		objfor=rtloader->read_32();

	}

	delete rtloader;
}


g1_client_class * g1_client=0;

g1_client_class::g1_client_class(i4_net_address * server_address,
								 int use_port,
								 i4_net_protocol * protocol)
	: server_address(server_address->copy()),
	  use_port(use_port)
{
	listen=0;
	send=0;
	map_name=0;
	num_players=0;
	player_num=0;
	stank_type=g1_get_object_type("stank");
	state=JOIN_START;

	if (protocol)
	{
		listen=protocol->listen(use_port, I4_PACKETED_DATA);
		send=protocol->connect(server_address, I4_PACKETED_DATA);
	}
}

void g1_client_class::send_server(w8 * buf, i4_file_class * fp)
{
	send->write(buf,fp->tell());
}

i4_bool g1_client_class::poll()  // returns false if server is not responding
{
	if (!listen || !send)
	{
		return 0;
	}


	if (state==JOIN_START)
	{
		i4_time_class now;
		if (now.milli_diff(last_responce_time)>40000)  // if it's been 40 secs assume server dead
		{
			i4_message_box("Server not responding","The server is not responding any more. ",MSG_OK);
			return i4_F;
		}


		w8 packet[512];
		i4_ram_file_class r(packet, sizeof(packet));

		r.write_8(G1_PK_I_WANNA_JOIN);
		r.write_16(use_port);
		r.write_counted_str(*g1_resources.username);

		if (!send->write(packet, r.tell()))
		{
			return i4_F;
		}

	}
	if (state==SYNCING) //set from the main loop as soon as the map is loaded
	{
		PACKET(pack,fp); //we just resend this message till we get the matching reply
		fp.write_8(G1_PK_LOADING_DONE);
		send_server(pack,&fp);
	}

	int noloops=0;
	while (listen->ready_to_read() && noloops<10)
	{
		w8 packet[MAX_PACKET_SIZE];
		i4_net_address * a;
		int s=listen->read_from(packet, sizeof(packet), a);
		if (a->equals(server_address))
		{
			i4_ram_file_class r(packet, sizeof(packet));

			w8 new_message=r.read_8();
			switch (new_message)
			{
				case G1_PK_YOU_HAVE_JOINED:
					{
						player_num=(w8)r.read_16();

						if (map_name)
						{
							delete map_name;
						}
						map_name=r.read_counted_str();
						i4_warning("We were accepted by the server to join");
						state=CONNECTING;
						last_responce_time.get();
					} break;
				case G1_PK_WE_ARE_STARTING:
					{
						num_players=r.read_32();
						if (state==CONNECTING)
						{
							i4_warning("Loading game data");
							//don't do this twice.
							//i4_user_message_event_class u(G1_START_NEW_GAME);
							i4_file_open_message_class u(G1_SAVEGAME_LOAD_OK, new i4_str (* map_name));
							i4_kernel.send_event(i4_current_app, &u);
							state=LOADING;
						}
						//li_call("hide_main_menu");
					} break;
				case G1_PK_GAME_START:
					{
						//from now on, it's the main loops task to know that
						//this is a net game
						if (state!=RUNNING)
						{
							int num_ais=r.read_32();
							if (num_ais>G1_MAX_PLAYERS)
							{
								i4_error("ERROR: Local Golgotha version supports fewer players than this game needs. Check your version.");
								num_ais=G1_MAX_PLAYERS;
							}
							i4_str * ainame=0;
							g1_team_api_class * newai=0;
							for (int ais=0; ais<num_ais; ais++)
							{
								//the player the next ai is for
								w32 aifor=r.read_32();
								ainame=r.read_counted_str();
								char buf[100];
								i4_os_string(*ainame,buf,100);
								newai=g1_create_ai(buf,0);
								if (strcmp(buf,"human")==0)
								{
									//this will be the local player
									g1_player_man.local_player=ais;
									i4_warning("We were assigned player number %i.",ais);
								}
								g1_player_man.get(ais)->set_ai(newai);
								newai->init();
								delete ainame;
							}
							g1_global_id.enable_networking(ID_NET_CLIENT);
							state=RUNNING;
							i4_warning("The game is in sync and has started");
						}
						//this message is sent by the server if all clients are ready
					} break;

				case G1_PK_GAME_DATA:
					{
						process_data_packet(r,i4_F);
					} break;
				default:
					{
						if (new_message>=G1_PK_ID_BASE && new_message<=G1_PK_ID_END)
						{
							r.seek(0);
							g1_global_id.receive_packet(&r);
						}
					}
			}
		}
		delete a;
		noloops++;
	}
	return i4_T;
}

int g1_client_class::prepare_command(int command)
{
	g1_global_id.poll();
	switch (state)
	{
		case SYNCING:
		case LOADING:
			{
				state=SYNCING;
				PACKET(sp,r);
				r.write_8(G1_PK_LOADING_DONE);
				i4_warning("We are ready to start.");
				send_server(sp,&r);
			}
			return NE_ERROR_NOTREADY;

		case RUNNING:
			{
				if (!network_file)
				{
					network_file=new i4_temp_file_class(2000,2000);
				}

				switch (command)
				{
					case G1_PK_GAME_DATA:
						{
							network_file->clear();
							network_file->write_8(command);
							network_file->write_32(0); //placeholder for length
							network_file->write_32(g1_tick_counter);

						} break;
					case G1_PK_PROCESS:
						{
							send_server(network_file->get_buffer(),network_file);
							network_file->clear();
							network_file->write_8(G1_PK_GAME_DATA);
							network_file->write_32(0);
							network_file->write_32(g1_tick_counter);
						} break;
				}
			} break;
	}
	if (!g1_global_id.is_ready())
	{
		return NE_ERROR_NOTREADY;
	}
	return NE_ERROR_OK;
}

i4_bool g1_client_class::is_ready()
{
	return g1_global_id.is_ready();
}

g1_client_class::~g1_client_class()
{
	i4_warning("Closing network game");
	delete server_address;

	if (map_name)
	{
		delete map_name;
	}

	if (listen)
	{
		delete listen;
	}

	if (send)
	{
		delete send;
	}
}
// SERVER.CPP
/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/


g1_server_class * g1_server=0;

void g1_server_class::client::cleanup()
{
	if (addr)
	{
		delete addr;
		delete username;
	}
	if (send)
	{
		delete send;
	}
	addr=0;
	send=0;
	flags=0;
	remote_player_num=0;
}

g1_server_class::g1_server_class(int use_port, i4_net_protocol * protocol)
	: protocol(protocol)
{
	map_name=new i4_str(i4gets("tmp_savename"));
	udp_port=0;
	memset(clients, 0, sizeof(clients));
	list_changed=i4_F;
	state=WAITING_FOR_PLAYERS;
	local_player_num=1; //this cannot be changed right now
	stank_type=g1_get_object_type("stank");
	if (protocol)
	{
		udp_port=protocol->listen(use_port, I4_PACKETED_DATA);
		if (!udp_port)
		{
			i4_warning("could not bind to port, server already running?");
		}
	}
}

void g1_server_class::send_to(w8 * buf, i4_file_class * f,int client_id)
{
	if (clients[client_id].addr)
	{

		clients[client_id].send->write(buf,f->tell());
	}
}

void g1_server_class::send_to_all(w8 * buf,i4_file_class * f)
{
	for (int i=0; i<G1_MAX_PLAYERS; i++)
	{
		if (clients[i].addr)
		{
			clients[i].send->write(buf,f->tell());
		}
	}
}

void g1_server_class::start_game()
{

	PACKET(packet,r);
	r.write_8(G1_PK_WE_ARE_STARTING);
	r.write_32(4);
	send_to_all(packet,&r);
	send_to_all(packet,&r);
	state=LOADING;
};


void g1_server_class::send_player_joined(int client_num)
{
	w8 packet[512];

	i4_ram_file_class r(packet, sizeof(packet));

	r.write_8(G1_PK_YOU_HAVE_JOINED);
	r.write_16(client_num+1);
	r.write_counted_str(*map_name);

	clients[client_num].send->write(packet, r.tell());
}

void g1_server_class::process_client_packet(w8 * packet,
											int packet_length,
											int client_num)
{
	clients[client_num].last_data.get();

	i4_ram_file_class r(packet, packet_length);
	w8 msg=r.read_8();
	switch (msg)
	{
		case G1_PK_I_WANNA_JOIN:
			{
				r.read_16(); // skip use port

				delete clients[client_num].username;
				clients[client_num].username=r.read_counted_str();
				send_player_joined(client_num);
			} break;
		case G1_PK_LOADING_DONE:
			{
				clients[client_num].flags|=READY;
				i4_bool ok=i4_T;

				for (int i=0; i<G1_MAX_PLAYERS; i++)
				{
					if (clients[i].addr && !(clients[i].flags&READY))
					{
						ok=i4_F; //at least one is not yet ready;
					}
				}
				if (ok&&state==SYNCING)
				{
					int rem_players,player,k,k2;
					rem_players=0;
					player=1; //zero is always neutral,
					//1 is usually local (server) player
					if (player==local_player_num)
					{
						player++;
					}
					for(k=0; k<G1_MAX_PLAYERS; k++)
					{
						if (clients[k].addr!=0)
						{
							rem_players++;
							clients[k].remote_player_num=player;
							player++;
							if (player==local_player_num)
							{
								player++;
							}
						}
					}
					for (player=0; player<G1_MAX_PLAYERS; player++)
					{
						if (clients[player].addr)
						{
							PACKET(sp,s);
							s.write_8(G1_PK_GAME_START);
							s.write_32(G1_MAX_PLAYERS); //So many ai's are going to follow
							for (k=0; k<G1_MAX_PLAYERS; k++)
							{
								s.write_32(k);
								g1_player_info_class * pl=g1_player_man.get(k);
								i4_bool found=i4_F;
								if (k==0) //the neutral player
								{
									s.write_counted_str(*(pl->get_ai()->ai_name()));
									found=i4_T;
								}
								else if (k==local_player_num)
								{
									s.write_counted_str("remote_player");
									found=i4_T;
								}
								else
								{
									for (k2=0; k2<G1_MAX_PLAYERS; k2++)
									{
										if (clients[k2].addr && clients[k2].remote_player_num==k)
										{
											//this automatically indicates
											//that it is the remote local player
											//(only one player can be human at a time on
											//a particular machine)
											s.write_counted_str("human");
											found=i4_T;
											break;
										}
									}
								}
								if (!found) //this player will stay the same as originally defined on the map
								{
									s.write_counted_str(*(pl->get_ai()->ai_name()));
									found=i4_T;
								}
							}
							send_to(sp,&s,player);
							send_to(sp,&s,player); //its important!
						}
					}
					//finally, set the matching ai's also for the server
					for (player=0; player<G1_MAX_PLAYERS; player++)
					{
						g1_team_api_class * newai=0;
						i4_bool found=i4_F;
						if (player==0)
						{
							newai=g1_create_ai("ai_neutral",0);
							g1_player_man.get(player)->set_ai(newai);
							newai->init();
							found=i4_T;
						}
						else if (player==local_player_num)
						{
							newai=g1_create_ai("human",0);
							g1_player_man.get(player)->set_ai(newai);
							newai->init();
							g1_player_man.local_player=player;
							found=i4_T;
						}
						else
						{
							for (k2=0; k2<G1_MAX_PLAYERS; k2++)
							{
								if (clients[k2].addr && clients[k2].remote_player_num==player)
								{
									//player "player" is a remote player
									newai=g1_create_ai("remote_player");
									g1_player_man.get(player)->set_ai(newai);
									newai->init();
									found=i4_T;
								}
							}
						}
						//in all other cases, we read the correct ai from the disk

					}
					g1_global_id.enable_networking(ID_NET_SERVER);
					state=RUNNING;
					i4_warning("Server: Switched to running state.");
				}
			} break;
		case G1_PK_GAME_DATA:
			{
				process_data_packet(r,i4_T);
				/*
				   r.read_32();//skip empty field
				   w32 ticksent=r.read_32();//to which tick does this packet belong?
				   //here follows: reading out the objects
				   g1_realtime_loader_class *rtloader=new g1_realtime_loader_class(&r,i4_F,i4_F);

				   w32 objfor=rtloader->read_32();
				   w16 typefor=0;
				   while (objfor!=0)
				   	{
				   	if (objfor==g1_global_id_manager_class::ID_DELETEPROPAGATE)
				   		{
				   		//don't delete if object just doesn't exist locally.
				   		objfor=rtloader->read_32();
				   		g1_object_class *obdel=g1_global_id.checked_get(objfor);
				   		if (obdel)
				   			{
				   			obdel->set_flag(g1_object_class::THINKING,1);//Perhaps was not thinking locally
				   			obdel->stop_thinking();
				   			obdel->unoccupy_location();
				   			obdel->request_remove();
				   			}
				   		}
				   	else
				   		{
				   		typefor=rtloader->read_16();
				   		g1_object_class *obj=g1_global_id.checked_get(objfor);
				   		if (!obj)
				   			{
				   			//Object seems to be new
				   			//obj=g1_create_object(typefor);
				   			if (typefor>=0 && typefor<=g1_last_object_type)
				   				{
				   				//The object will just be in sync
				   				obj=g1_object_type_array[typefor]->create_object(typefor,rtloader);
				   				obj->occupy_location();
				   				obj->grab_old();

				   				if (obj->get_flag(g1_object_class::THINKING))
				   					obj->request_think();

				   				g1_player_man.get(obj->player_num)->add_object(obj->global_id);

				   				}
				   			else
				   				{
				   				i4_error("ERROR: Unknown object type requested");
				   				delete rtloader;
				   				return;
				   				}
				   			//g1_global_id.assign(objfor,obj);
				   			}
				   		else
				   			{
				   			obj->load(rtloader);
				   			}
				   		obj->set_flag(g1_object_class::NEEDS_SYNC,1);
				   		}
				   	objfor=rtloader->read_32();
				   	}
				   //don't forget to mark these as to be resent.
				   delete rtloader;
				 */
			} break;
		default:
			{
				if (msg>=G1_PK_ID_BASE && msg<=G1_PK_ID_END)
				{
					r.seek(0);
					g1_global_id.receive_packet(&r);
				}
			}
	}
}


void g1_server_class::poll()
{
	int i,noloops=0;

	list_changed=i4_F;
	while (udp_port && udp_port->ready_to_read() && noloops<10)
	{
		w8 packet[MAX_PACKET_SIZE];
		i4_net_address * a;
		int len=udp_port->read_from(packet, sizeof(packet), a);

		// see if this was from one of our clients

		int found=0, free_spot=-1;

		for (i=0; i<G1_MAX_PLAYERS; i++)
		{
			if (clients[i].addr)
			{
				if (clients[i].addr->equals(a))
				{
					process_client_packet(packet, len, i);
					found=1;
				}
			}
			else
			{
				free_spot=i;
				break;
			}
		}

		if (!found && free_spot!=-1)
		{
			i4_ram_file_class r(packet, len);
			if (r.read_8()==G1_PK_I_WANNA_JOIN)
			{
				clients[free_spot].addr=a->copy();
				clients[free_spot].addr->set_port(r.read_16());
				clients[free_spot].username=r.read_counted_str();

				clients[free_spot].send=protocol->connect(clients[free_spot].addr, I4_PACKETED_DATA);
				clients[free_spot].last_data.get();

				if (clients[free_spot].send)
				{
					send_player_joined(free_spot);
				}
				else
				{
					clients[free_spot].cleanup();
				}

				list_changed=i4_T;
			}
		}

		delete a;
		noloops++;
	}

	if (state==WAITING_FOR_PLAYERS)
	{
		i4_time_class now;
		for (i=0; i<G1_MAX_PLAYERS; i++)
		{
			if (clients[i].addr && now.milli_diff(clients[i].last_data)>50000)
			{
				clients[i].cleanup();
				list_changed=i4_T;
			}

		}
	}
	if (state==LOADING) //has this guy lost a message?
	{
		i4_time_class now;
		for (i=0; i<G1_MAX_PLAYERS; i++)
		{
			if (clients[i].addr && now.milli_diff(clients[i].last_data)>30000)
			{
				PACKET(rp,rpf);
				rpf.write_8(G1_PK_WE_ARE_STARTING);
				rpf.write_32(4);
				send_to(rp,&rpf,i);
			}

		}

	}

}

int g1_server_class::prepare_command(int command)
{
	g1_global_id.poll();
	switch (state)
	{
		case LOADING:
			{
				if (command==G1_PK_GAME_DATA)
				{
					//the server is ready to start
					state=SYNCING;
					return NE_ERROR_NOTREADY;
				}
			} break;
		case SYNCING: //We don't need to resend messages to the server itself...
			{
				i4_bool ok=i4_T;
				for (int i=0; i<G1_MAX_PLAYERS; i++)
				{
					if (clients[i].addr!=0)
					{
						ok=i4_F;
					}
				} //this means we are allone (at the moment, at least)
				if (ok)
				{
					i4_warning("Switching to running state as we seem to be running server-only");
					if (!network_file)
					{
						network_file=new i4_temp_file_class(2000,2000);
					}
					state=RUNNING;
					return NE_ERROR_OK;
				}
				return NE_ERROR_NOTREADY;
			}
		case RUNNING:
			{
				if (!network_file)
				{
					network_file=new i4_temp_file_class(2000,2000);
				}

				switch (command)
				{
					case G1_PK_GAME_DATA:
						{
							network_file->clear();
							network_file->write_8(G1_PK_GAME_DATA);
							network_file->write_32(0); //placeholder for length
							network_file->write_32(g1_tick_counter);

						} break;
					case G1_PK_PROCESS:
						{
							send_to_all(network_file->get_buffer(),network_file);
							network_file->clear();
							network_file->write_8(G1_PK_GAME_DATA);
							network_file->write_32(0);
							network_file->write_32(g1_tick_counter);
						} break;
					default:
						{
							i4_warning("Server: Process_command() encountered unknown instruction");
						}
				}
			} break;
		case QUITING:
			{
				return NE_ERROR_SERVERKILLED;
			}
		default:
			return NE_ERROR_NOTREADY;
	}
	if (!g1_global_id.is_ready())
	{
		return NE_ERROR_NOTREADY;
	}
	return NE_ERROR_OK;
}

bool g1_server_class::is_ready()
{
	return true;
}

g1_server_class::~g1_server_class()
{
	delete map_name;
	state=QUITING;

	for (int i=0; i<G1_MAX_PLAYERS; i++)
	{
		if (clients[i].addr)
		{
			delete clients[i].addr;
			delete clients[i].username;
			delete clients[i].send;
			clients[i].addr=0;
		}
	}
	i4_warning("Server: Shutdown in progress.");
	delete udp_port;
}

/*****************************************************************
   Network utility functions
*****************************************************************/

i4_bool i4_network_active()
{
	if (g1_server||g1_client)
	{
		return i4_T;
	}
	return i4_F;
};


void i4_network_poll()
{
	if (g1_server)
	{
		g1_server->poll();
	}
	if (g1_client)
	{
		g1_client->poll();
	}

}

bool i4_network_mustwait()
{
	if (g1_client)
	{
		if (!g1_client->is_ready())
		{
			return true;
		}
	}
	return false;
}

void i4_network_shutdown()
{
	delete g1_server;
	g1_server=0;
	delete g1_client;
	g1_client=0;
	if (network_file)
	{
		delete network_file;
	}
	network_file=0;
}

int i4_network_prepare_command(int command)
{
	if (g1_server)
	{
		return g1_server->prepare_command(command);
	}
	if (g1_client)
	{
		return g1_client->prepare_command(command);
	}
	return 0;
}
/*!
   	Sends a network packet to the given client.
   	The client number is his local player number (converted to network client ids)
 \param buf A pointer to the buffer to be sent
 \param fp  The file class associated with the buffer (for the length)
 \param send_to The receiver (server only), for server, -1 means to all. Client ignores target.
 */
int i4_network_send(w8 * buf, i4_file_class * fp,int send_to)
{
	if (g1_server)
	{
		if (send_to==-1)
		{
			//send to all
			g1_server->send_to_all(buf,fp);
		}
		else
		{
			for (int i=0; i<G1_MAX_PLAYERS; i++)
			{
				if (g1_server->clients[i].remote_player_num==send_to)
				{
					g1_server->send_to(buf,fp,i);
					return 0;
				}
			}
			i4_warning("Attempted to send a packet to a non-existing client.");

		}
	}
	if (g1_client)
	{
		g1_client->send_server(buf,fp);
	}
	return 0;
}

//This entire file won't be compiled if the network support is unavailable

#endif
