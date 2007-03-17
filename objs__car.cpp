#include "pch.h"

/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/

//this object is used mainly for the traffic-simulation
#include "sound_man.h"
#include "objs/model_id.h"
#include "objs/model_draw.h"
#include "math/pi.h"
#include "math/trig.h"
#include "math/angle.h"
#include "math/random.h"
#include "objs/bullet.h"
#include "resources.h"
#include "saver.h"
#include "map_cell.h"
#include "map.h"
#include "map_man.h"
#include "objs/vehic_sounds.h"
#include "sound/sfx_id.h"
//#include "objs/helicopter.h"
#include "objs/fire.h"
#include "object_definer.h"
#include "objs/path_object.h"
#include "image_man.h"
#include "transport/transport.h"
#include "objs/car.h"
#include "objs/road_object.h"
#include "g1_render.h"
#include "render/r1_clip.h"
#include "player.h"
#include "time/profile.h"
#include "file/ram_file.h"

static g1_model_ref model_ref("engineer_body"),
shadow_ref("engineer_shadow"),
back_wheels_ref("engineer_back_wheels"),
front_wheels_ref("engineer_front_wheels"),
lod_ref("engineer_lod");
static g1_team_icon_ref car_im("bitmaps/radar/car.tga");

i4_profile_class pf_car_start("starting cars");

static i4_3d_vector back_attach, back_offset, front_attach, front_offset;
g1_object_type road_type;
g1_object_type car_type;

i4_isl_list<g2_car_object_class> g2_car_object_list;

void g1_car_init()
{

	back_attach.set(-0.12f,0.0,0.1f);
	model_ref()->get_mount_point("Back Wheels", back_attach);
	back_wheels_ref()->get_mount_point("Back Wheels", back_offset);
	back_offset.reverse();

	front_attach.set(0.11f,0.0,0.1f);
	model_ref()->get_mount_point("Front Wheels", front_attach);
	front_wheels_ref()->get_mount_point("Front Wheels", front_offset);
	front_offset.reverse();
	road_type=g1_get_object_type("road_object");
	car_type=g1_get_object_type("car");
}



g2_car_object_class::g2_car_object_class(g1_object_type id, g1_loader_class *fp)
	: g1_map_piece_class(id,fp)
{
	player_num=g1_player_man.local_player;
	radar_type=G1_RADAR_VEHICLE;
	//radar_image=&car_im;
	//g1_global_id.free(global_id);
	//global_id=0;
	//route_quality=0;
	last_route=0;
	route_len=0;
	start_time=TIME_UNKNOWN;
	arrival_time=TIME_NOTYET;
	linked=i4_F;
	car_id=0;
	start_link=0;
	dest_link=0;
	used_path_len=8;
	act_link_start_time=TIME_NOTYET;
	draw_params.setup(model_ref.id(),shadow_ref.id(), lod_ref.id());

/*		    allocate_mini_objects(3,"Transport car mini objects");

   			back_wheels = &mini_objects[0];
   			back_wheels->defmodeltype = front_wheels_ref.id();
   			back_wheels->position(back_attach);
   			back_wheels->offset = front_offset;

   			front_wheels = &mini_objects[1];
   			front_wheels->defmodeltype = front_wheels_ref.id();
   			front_wheels->position(front_attach);
   			front_wheels->offset = front_offset;
 */

	w16 ver,data_size=0;
	if (fp)
	{
		fp->get_version(ver,data_size);
	}
	else
	{
		ver = 0;
	}

	//need to read/save the info about the way we took
	switch (ver)
	{
		case CAR_DATA_VERSION:
			{
				fp->read_format("444444441",&route_len, &start_time,
								&required_arrival_time,&arrival_time,
								&car_id,&start_link,&dest_link,
								&act_link_start_time,&linked);
				int t=fp->read_32();
				if (t)
				{
					last_route=(g1_id_ref *)I4_MALLOC(sizeof(g1_id_ref)*(t+1),"");
					for (int i=0; i<t; i++)
					{
						if (linked)
						{
							last_route[i].load(fp); //the global id of the node is available
						} //but not the node manager
						else
						{
							//the node manager is available, but not the remap table
							node_id n=fp->read_32();

							g1_road_object_class *rl=(g1_road_object_class *)
													  g1_path_object_class::cast((g1_object_class *)g2_node_man()->get_obj(n));
							last_route[i]=rl->global_id;
						}
					}
					last_route[t].id=0;
				}
			}
			break;

		case 1:
			fp->seek(fp->tell()+ data_size);
			/*
			   fp->read_format("ff", &front_wheels->rotation.y, &front_wheels->rotation.z);
			   front_wheels->rotation.x = 0;
			   front_wheels->grab_old();
			   back_wheels->rotation.x = 0;
			   back_wheels->rotation.y = front_wheels->rotation.y;
			   back_wheels->rotation.z = 0;
			   back_wheels->grab_old();*/
			break;


		default:
			if (fp)
			{
				fp->seek(fp->tell() + data_size);
			}
			/*back_wheels->rotation.x = 0;
			   back_wheels->rotation.y = 0;
			   back_wheels->rotation.z = 0;
			   back_wheels->grab_old();
			   front_wheels->rotation.x = 0;
			   front_wheels->rotation.y = 0;
			   front_wheels->rotation.z = 0;
			   front_wheels->grab_old();*/

			break;
	}

	if (fp)
	{
		fp->end_version(I4_LF);
	}


	set_flag(BLOCKING    |
			 TARGETABLE  |
			 DANGEROUS   |
			 GROUND      |
			 SPECIALTARGETS|
			 EXT_GLOBAL_ID,1);
	g2_car_object_list.insert(*this);
}

void g2_car_object_class::save(g1_saver_class *fp)
{
	g1_map_piece_class::save(fp);
	fp->start_version(CAR_DATA_VERSION);

	//fp->write_format("ff", &front_wheels->rotation.y, &front_wheels->rotation.z);
	//todo: implement this
	fp->write_format("444444441",&route_len, &start_time,
					 &required_arrival_time,&arrival_time,&car_id,&start_link,&dest_link,
					 &act_link_start_time,&linked);
	if (last_route)
	{
		g1_id_ref *r;
		int t=0;
		for (r=last_route; r->id; r++, t++)
		{
			;
		}
		fp->write_32(t);
		for (r=last_route; r->id; r++)
		{
			if (linked)
			{
				r->save(fp); //cannot save this, since on load, the
			}
			//remap info won't be available at this time
			//(the path_to_follow info is not required for cars not on the map)
			//it's ok for linked objects, these have the remap info.
			else
			{
				g1_road_object_class *rs=(g1_road_object_class *)g1_path_object_class::cast(r->get());
				fp->write_32(rs->nid);
			}
		}
	}
	else
	{
		fp->write_32(0);
	}

	fp->end_version();
}


void g2_car_object_class::think()
{
	/*g1_path_object_class *last,*next;
	   last=g1_path_object_class::cast(prev_object.get());//get_my_link()
	   next=g1_path_object_class::cast(next_object.get());
	   if (last&&next)
	   	{
	   	link_id lid=last->link_to(G1_ALLY,next);
	   	speed=g2_link_man()->get_link(lid)->get_freespeed();
	   	//speed*=li_get_float(li_get_value("world_scaling",0),0);
	   	}
	   else
	   	speed=speed+1.0f;//this happens if we are about to go
	   //over crossroads, I think.
	 */
	g2_link *lk=link_on();
	g1_object_class *oldnext=next_path.get();
	if (lk)
	{
		float maxspeed=lk->get_freespeed();
		if (speed<maxspeed)
		{
			speed= speed + (maxspeed/10);
		}
		else
		{
			speed=maxspeed;
		}
	}
	g1_map_piece_class::think();

	if (!alive())
	{
		return;
	}

	i4_float dangle = (theta-ltheta);
	if (dangle>i4_pi())
	{
		dangle -= 2 *i4_pi();
	}
	else if (dangle<-i4_pi())
	{
		dangle += 2 *i4_pi();
	}

	//back_wheels->rotation.y  += speed*0.2f;
	//front_wheels->rotation.y += speed*0.2f;

	//front_wheels->rotation.z = dangle*0.2f;
	if (next_path.get()!=oldnext) //we have advanced over crossroads
	{
		//calculate the time we thought this link would take
		//double mintime;
		//mintime=lk->get_length()/lk->get_freespeed();
		double curtime=(w32) g2_act_man()->daytime;
		double usedtime=curtime-act_link_start_time;
		//if (mintime*1.4<usedtime)//if we need twice as much time than usual
		//	{//we consider it congested
		lk->mark_bad(curtime,0,usedtime);
		//	}
		act_link_start_time=curtime;
	}
	if (!next_path.valid()) //either we are at the end or
	//the road is broken. Currently we assume we are at the end
	{
		if (get_flag(MAP_OCCUPIED))
		{
			unoccupy_location();
		}
		unlink();

		i4_free(path_to_follow); //do not attempt to save these after we
		//left the map
		path_to_follow=0;
		//g1_get_map()->remove_from_think_list(this);//must do this
		stop_thinking();
		//since we are already requested for rethink at this time.
		//NO request_remove() !!!
		g1_player_man.get(player_num)->remove_object(global_id);
		linked=i4_F;
		arrival_time=(w32)g2_act_man()->daytime;
	}
}

i4_str *g2_car_object_class::get_context_string()
{
	char buf[101];
	i4_ram_file_class rf(buf,100);
	rf.printf("%s, id %i, speed %f, path_pos %f, path_len %f", name(),car_id,speed,path_pos,path_len);

	buf[rf.tell()]=0;
	return new i4_str(buf);
}

double g2_car_object_class::calc_util(w32 ttrip,w32 tearly,w32 tlate)
{
	return (double)((double)(-0.4/60)*ttrip)+((double)(-0.25/60)*tearly)+
		   ((double)(-1.5/60)*tlate);
}
w32 g2_car_object_class::calc_new_start_time(w32 olds,w32 reqarr,w32 actarr)
{
	const w32 top=(24*3600/START_TIME_BIN_SIZE);
	//return olds;//testing rerouting first.
	double util[top];
	double utilsum=0;
	w32 best=olds;
	w32 i=0,besti=0;
	w32 testt,ttrip=actarr-olds;
	for(; i<top; i++)
	{
		//what to do with the i?
		testt=i*START_TIME_BIN_SIZE;
		util[i]=calc_util(ttrip,
						  reqarr>testt ? reqarr-testt : 0,
						  reqarr<testt ? testt-reqarr : 0);
		utilsum=utilsum+util[i];
	}
	for (i=0; i<top; i++)
	{
		util[i]=util[i]/utilsum; //normalize to one
	}
	//g2_act_man()->scramble.rnd.set_limits(0,10000);
	//w32 rnd=g2_act_man()->scramble.rnd.rnd();
	double r=i4_float_rand();
	i=0;
	double greatsum=0;
	while (i<top && greatsum<r)
	{
		greatsum+=util[i];
		i++;
	}
	w32 newstarttime=i * START_TIME_BIN_SIZE;
	//w32 sdiff=olds-(newbinstart???)
	if (abs(((sw32)newstarttime)-((sw32)olds))>2*3600)
	{
		if (newstarttime>olds)
		{
			newstarttime=olds+2*3600;
		}
		else
		{
			newstarttime=olds-2*3600;
		}
	}
	return newstarttime;
}

i4_bool g2_car_object_class::start(w32 current_time, link_id for_dest)
{
	//should probably include a try{} catch() pair here if some nodes
	//were removed inexspectedly
	if (linked)
	{
		//already on map?
		return i4_T;
	}
	pf_car_start.start();
	//start_time=current_time;//doesn't make sense with activity start
	//time replaning
	g2_link *lst=g2_link_man()->get_link(start_link);
	dest_link=for_dest;
	node_id stid=lst->get_from();
	g2_link *ldst=g2_link_man()->get_link(dest_link);
	node_id dstid=ldst->get_to();
	//we currently ignore the fact that the link might be full
	//baaaad: Causes the link to fill up with waiting cars and
	//denies other cars entry to this road at the given node.
	//this would simulate the cars leaving their parking space beeing
	//always priorized over those already on the road. Strange rule!
	g1_road_object_class *s=(g1_road_object_class *)g2_node_man()->get_obj(stid),
	*d=(g1_road_object_class *)g2_node_man()->get_obj(dstid);
	//for the sake of simplicity (We don't jet know which outgoing link
	//we will use and I don't want to calculate the path before the
	//actual start of the voyage) I suggest the following behaviour:
	//If all outgoing links are available, we start. If only some of
	//them are available (not full) we start with some probability.
	//If none are available, we wait.
	int tot=s->total_links(G1_ALLY),i;
	i4_float proba=0;
	g1_path_object_class *path;
	link_id linkid;
	for (i=0; i<tot; i++)
	{
		path=s->get_link(G1_ALLY,i);
		linkid=s->get_link_id(G1_ALLY,i);
		if (linkid==start_link && //its the link I'm supposed to start from
			//this cannot be enshured completelly since the routing
			//doesn't guarantee that we actually take that link.
			can_enter_link(s,path)) //can enter link from s to path
		{
			proba=proba+(1.0f/(i4_float)tot);
		}
	}
	//have a 3% change of entering anyway and a small change of not entering
	//(some other problem)
	//proba=proba+0.01f;
	if (proba>=0.98)
	{
		proba=0.98f;
	}
	//Removed the possiblity of entering anyway since it hapened way to often
	//if the road was completelly congested
	g2_act_man()->scramble.rnd.set_limits(1,100); //"borrow" that random number generator...
	int rndnum=g2_act_man()->scramble.rnd.rnd();
	if (rndnum>(proba*100))
	{
		pf_car_start.stop();
		return i4_F;
	}
	g1_path_object_class *stack[G1_MAX_OBJECTS];
	g1_id_ref *way=0;
	if (get_flag(SCRATCH_BIT)||!last_route)
	{
		//w32 oldstarttime=start_time;
		//if (get_flag(SCRATCH_BIT))
		//	{//must be calculated at end of day
		//	start_time=calc_new_start_time(oldstarttime,
		//		arrival_time,required_arrival_time);
		//	}
		set_flag(SCRATCH_BIT,0);
		int t=s->find_path(G1_ALLY,d,stack,G1_MAX_OBJECTS);
		if (t<=0)
		{
			i4_warning("Can't find a way from Node %i to Node %i.",s->nid,d->nid);
			arrival_time=current_time; //ignore this car from now on.
			pf_car_start.stop();
			return i4_F;
		}
		//set_path() will copy way to path_to_follow
		way=(g1_id_ref *)I4_MALLOC((t+1)*sizeof(g1_id_ref),"way1");
		for (i=0; i<t; i++)
		{
			//Order?
			way[t-i-1]=stack[i];
		}
		way[t].id=0;
		g1_id_ref *last_way=(g1_id_ref *)I4_MALLOC((t+1)*sizeof(g1_id_ref),"way2");
		memcpy(last_way,way,(t+1)*sizeof(g1_id_ref));
		if (last_route)
		{
			i4_free(last_route);
		}
		last_route=last_way;
	}
	else
	{
		//we choose to use the same path as yesterday
		g1_id_ref *rc;
		int t=1; //one element more to copy
		for (rc=last_route; rc->id; rc++, t++)
		{
			;
		}
		way=(g1_id_ref *)I4_MALLOC(t*sizeof(g1_id_ref),"reused way");
		memcpy(way,last_route,t*sizeof(g1_id_ref));
	}
	//this must come before set_path()
	x=s->x;
	y=s->y;
	h=s->h;
	act_link_start_time=current_time;
	grab_old(); //set l(x|y|h) to x|y|h
	speed=0;
	set_path(way);
	linked=i4_T;
	occupy_location();
	get_terrain_info(); //must be done manually here since we
	//cutted down occupy_location
	g1_player_man.get(player_num)->add_object(global_id);
	pf_car_start.stop();
	return i4_T;
}

void g2_car_object_class::draw(g1_draw_context_class *context, i4_3d_vector& viewer_position)
{
	//todo: draw a point
	r1_vert v[2];
	i4_3d_point_class p(x,y,h);
	//g1_object_class *o;
	w32 c;
	if (g1_render.project_point(p, v[0], context->transform))
	{
		if ((v[0].v.z * v[0].v.z)>(g1_resources.lod_disappear_dist/2))
		{
			return;
		}
		c=((0xff-(w8)speed*10)<<16)+(((w8)speed*10)<<8); //color: green for good, red for bad
		r1_clip_clear_area((sw32)v[0].px-1, (sw32)v[0].py-1,
						   (sw32)v[0].px+1, (sw32)v[0].py+1, c, v[0].v.z, *context->context,
						   g1_render.r_api);

		//r1_set_color(&v[0],0xffff0000);
		//r1_clip_render_points(1,&v[0],g1_render.r_api);
	}
	;
}

//Not usefull. We'll insert into the list on creation and remove
//on request_remove. We just _need_ to have a way to address cars not on the map
#if 0
i4_bool g2_car_object_class::occupy_location()
{
	if (g1_map_piece_class::occupy_location())
	{
		g2_car_object_list.insert(*this);
		return i4_T;
	}
	else
	{
		return i4_F;
	}
};

void g2_car_object_class::unoccupy_location()
{
	if (get_flag(MAP_OCCUPIED))
	{
		g2_car_object_list.find_and_unlink(this);
		g1_map_piece_class::unoccupy_location();
	}
}
#else
i4_bool g2_car_object_class::occupy_location()
{
	if (x<0)
	{
		//cut off any bad roundings
		x=0;
	}
	if (x>g1_get_map()->height())
	{
		x=g1_get_map()->height();
	}
	if (y<0)
	{
		y=0;
	}
	if (y>g1_get_map()->width())
	{
		y=g1_get_map()->width();
	}
	//this is the fastest and absolutelly sufficient occupy method.
	return g1_map_piece_class::occupy_location_center();
	//any other cases where this can fail?
	//nope, not this one.
}

void g2_car_object_class::unoccupy_location()
{
	g1_map_piece_class::unoccupy_location();
}
#endif

void g2_car_object_class::request_remove()
{
	i4_free(last_route);
	last_route=0;
	g2_car_object_list.find_and_unlink(this);
	g1_map_piece_class::request_remove();
};

li_object *g2_car_object_class::message(li_symbol *name, li_object *params, li_environment *env)
{
	return g1_map_piece_class::message(name,params,env);
}

g1_object_definer<g2_car_object_class>
g1_car_def("car",
		   g1_object_definition_class::TO_MAP_PIECE |
		   //g1_object_definition_class::EDITOR_SELECTABLE |
		   g1_object_definition_class::MOVABLE,
		   g1_car_init);
