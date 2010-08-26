/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/
#include "pch.h"
#include "human.h"
#include "resources.h"
#include "g1_speed.h"
#include "input.h"
#include "objs/stank.h"
#include "map.h"
#include "map_man.h"
#include "controller.h"
#include "cwin_man.h"
#include "time/profile.h"
#include "player.h"
//#include "sfx_id.h"
#include "lisp/lisp.h"
#include "objs/path_object.h"
#include "border_frame.h"
#include "objs/convoy.h"
#include "light.h"
#include "render/r1_clip.h"

g1_human_class * g1_human = 0;

static li_symbol_ref li_convoy("convoy");

void g1_human_class::load(g1_loader_class * fp)
{
}

class g1_human_def_class :
	public g1_team_api_definition_class
{
public:
	int g1_human_class_references;
	g1_human_def_class()
		: g1_team_api_definition_class("human")
	{
		g1_human_class_references=0;
	}

	virtual g1_team_api_class *create(g1_loader_class * f)
	{
		if (!g1_human)
		{
			own_team_api(new g1_human_class(f));
		}
		g1_human_class_references++;

		return g1_human;
	}
} g1_human_def;

g1_human_class::g1_human_class(g1_loader_class * f)
{
	mouse_look_increment_x = 0;
	mouse_look_increment_y = 0;
	dragstartx=dragstarty=dragdestx=dragdesty=-1;
	prepared_building=NULL;
	g1_human = this;
}

g1_human_class::~g1_human_class()
{
	g1_human = 0;

	if (prepared_building)
		delete prepared_building;

}

void g1_human_class::send_selected_units(i4_float x, i4_float y)
{
	if (selected_object.get())
	{
		deploy_unit(selected_object->global_id, x,y);
	}
}


static li_symbol_ref allow_follow_mode("allow_follow_mode");
static li_symbol_ref supergun("supergun");
static li_symbol_ref engineer("engineer");
static li_symbol_ref takeover_pad("takeover_pad");

void g1_human_class::clicked_on_object(g1_object_class * o)
{
	if (o && g1_path_object_class::cast(o))
	{
		set_current_target(o->global_id);
	}
}

w8 g1_human_class::determine_cursor(g1_object_class * object_mouse_is_on)
{
	if (object_mouse_is_on)
	{

		if (object_mouse_is_on->player_num==team()&&
			object_mouse_is_on->get_flag(g1_object_class::SELECTABLE))
		{
			return G1_SELECT_CURSOR;
		}
		if (selected_object.valid()&&object_mouse_is_on->id==g1_get_object_type(takeover_pad.get()))
		{
			return G1_TAKEOVER_CURSOR;
		}
		if (g1_path_object_class::cast(object_mouse_is_on))
		{
			return G1_TAKEOVER_CURSOR;
		}
		if (selected_object.valid()&&selected_object->player_num==team() &&
			(selected_object->can_attack(object_mouse_is_on)||
			 single_sel_id(selected_object.get())==g1_get_object_type(supergun.get())))
		{
			return G1_TARGET_CURSOR;
		}
		return G1_DEFAULT_CURSOR; //nothing selected that can shoot
	}
	if (selected_object.valid()&&selected_object->player_num==team())
	{
		if (selected_object->id==g1_get_object_type(supergun.get()))
		{
			return G1_TARGET_CURSOR;
		}
		return G1_MOVE_CURSOR;

	}
	return G1_DEFAULT_CURSOR;
}
/*if (selected_object.valid())
   {
   //actually, we should show the target cursor always if something
   //is selected that cannot move. But supergun is currently the only
   //object of this type that can be selected
   if (selected_object->id == g1_get_object_type(supergun.get()))
   	return G1_TARGET_CURSOR;
   else
   	return G1_MOVE_CURSOR;
   }
   else if (object_mouse_is_on)
   {
   if (g1_path_object_class::cast(object_mouse_is_on))
   	  //        g1_path_object_class::cast(object_mouse_is_on)->valid_destination)
   	return G1_TAKEOVER_CURSOR;
   else
   	return G1_SELECT_CURSOR;
   }
   else
   return G1_DEFAULT_CURSOR;*/
//}

i4_bool g1_human_class::select_path(float gx,float gy)
{
	if (gx>=0 && gy>=0 && gx<g1_get_map()->width() && gy<g1_get_map()->height())
	{
		g1_path_object_class * best=0, * best2=0;
		float best_dist=10000;
		for (g1_path_object_class * o=g1_path_object_list.first(); o; o=o->next)
		{
			int t=o->total_links(player->get_team());
			for (int i=0; i<t; i++)
			{
				g1_path_object_class * o2=o->get_link(player->get_team(), i);
				if (o2 && !o2->get_flag(g1_object_class::SCRATCH_BIT))
				{
					i4_2d_vector p1(o->x, o->y), p2(o2->x, o2->y);
					i4_2d_vector a=p2-p1;
					i4_2d_vector x=i4_2d_vector(gx,gy);

					g1_path_object_class * use1=0, * use2=0;
					float dist;

					float alen=a.length();
					float t=(x-p1).dot(a)/(alen*alen);

					if (t<0)
					{
						dist=(x-p1).length();
					}
					else if (t>1)
					{
						dist=(x-p2).length();
					}
					else
					{
						dist=i4_fabs(a.perp_dot(x-p1)/a.length());
					}

					if (dist<best_dist)
					{
						best_dist=dist;
						best=o;
						best2=o2;
					}
				}
			}
		}

		if (best)
		{
			set_current_target(best->global_id);
			if (best2)
			{
				set_current_target(best2->global_id);
			}
			return i4_T;
		}
	}
	return i4_F;
}

void g1_human_class::attack_unit(g1_object_class * o,i4_float x, i4_float y)
{
	if (!selected_object.valid())
	{
		return;
	}
	if (selected_object->can_attack(o))
	{
		g1_map_piece_class * mp=g1_map_piece_class::cast(selected_object.get());
		if (mp)
		{
			mp->attack_target=o;
		}
	}
	else
	{
		send_selected_units(x,y);
	}
}

void g1_human_class::clear_selected()
{
	if (selected_object.valid())
	{
		selected_object->mark_as_unselected();
		selected_object->unoccupy_location();
		selected_object->request_remove();
		selected_object=0;
	}
};

i4_hashtable<special_command_entry> command_lookup_table(100,0);
class special_command_entry_cleanup_class :
	public i4_init_class
{
	void uninit()
	{
		command_lookup_table.reset(i4_T);
	}
} sce_class_instance;
static li_g1_ref_list_class_member units("units");
static li_symbol_ref commands_ask("commands-ask");

w32 g1_human_class::show_selection(g1_object_controller_class * for_who,
								   i4_transform_class &transform,
								   g1_draw_context_class * context)
{

	if (!selected_object.valid() || selected_object->id!=g1_get_object_type("convoy"))
	{
		return 0;
	}
	g1_convoy_class * c=(g1_convoy_class *) selected_object.get();
	li_class_context ctx(c->vars);
	int i;
	w32 j;
	int nums=units()->size();
	g1_screen_box * b=0,* thisbb=0;
	//doesn't work because i4_array cannot correctly handle the garbage collector
	//i4_array<li_object *> special_commands(10,10);

	li_object * lians=0,* liobj;
	li_symbol * lisym;
	char * cmdname;
	special_command_entry * sce=0;
	//we should probably optimize this if nums and/or selectables is big.
	if (nums<1)
	{
		return 0;
	}
	g1_object_class * o=0;
	for (i=0; i<nums; i++)
	{
		o=units()->value(i);
		if (!o)
		{
			continue;
		}
		b=for_who->selectable_list.recent;
		//I4_ASSERT(o,"ERROR: Selected list contains non-object.");

		//for(j=0;j<for_who->selectable_list.t_recent;j++)
		//	{
		//	if (for_who->selectable_list.recent->object_id
		//	}
		thisbb=0;

		for (j=0; j<for_who->selectable_list.t_recent; j++,b++)
		{
			if (b->object_id==o->global_id)
			{
				thisbb=b;
				break;
			}

		}
		//if (!thisbb)
		//	continue;
		if (thisbb)
		{
			g1_render.draw_outline(thisbb,o);
		}
		w32 commands_flags=o->get_selection_flags();
		if ((o->player_num==team()) ||
			(commands_flags&g1_object_class::SEL_ENEMYCANSENDCMD))
		{
			lians=new li_list(o->message(commands_ask.get(),0,0),lians);
		}
		//special_commands.add(lians);
	}
	//The list has the following format (I suppose?!?)
	//((command-a1,command-a2,command-a3),(command-b1,command-b2),...,0);
	if (nums==1)
	{
		//the easy case: only one unit selected
		//lians=special_commands[0];
		lians=li_car(lians,0); //first element of first entry
		liobj=li_car(lians,0);
		i4_float butx,buty,butz;
		//butx=(thisbb->x1+thisbb->x2)/2;
		//buty=(thisbb->y1+thisbb->y2)/2;
		//butx=thisbb->x1+10;
		//buty=thisbb->y2+10;
		butx=10;
		buty=2*g1_render.center_y-30;
		if (thisbb)
		{
			butz=thisbb->z1;
		}
		else
		{
			butz=r1_near_clip_z;
		}
		while(liobj!=0)
		{
			lisym=li_symbol::get(liobj,0);
			cmdname=lisym->name()->value();
			draw_button_model(cmdname,butx,buty,butz,o);
			lians=li_cdr(lians,0);
			liobj=li_car(lians,0);
			//The format of the list is a bit strange because it is concatenated
			if (liobj&&liobj->type()!=LI_SYMBOL)
			{
				lians=liobj;
				liobj=li_car(lians,0);
			}
			butx+=50;
		}
	}
	//special_commands.uninit();
	return nums;
}

void g1_human_class::draw_button_model(char * buttoncmd, i4_float posx, i4_float posy, i4_float posz, g1_object_class * forobj)
{
	//i4_3d_vector pos(posx/10,posy/10,0.3f);
	i4_float w=0.5f*r1_near_clip_z; //defines the size of the model
	//i4_float wcorr=r1_near_clip_z/g1_render.center_x;
	//i4_float maxxof=(g1_render.center_x)*r1_near_clip_z/g1_render.center_x;
	//i4_float maxyof=(g1_render.center_y)*r1_near_clip_z/g1_render.center_y;
	w16 button_model=g1_model_list_man.find_handle(buttoncmd);

	g1_quad_object_class * model=g1_model_list_man.get_model(button_model);

	if (!model)
	{
		return;
	}
	//posx=10;
	//posy=2*g1_render.center_y-20;
	i4_float xof=(posx-g1_render.center_x)*0.8f*r1_near_clip_z/g1_render.center_x;
	//xof+=xof/maxxof;
	i4_float yof=(posy-g1_render.center_y)*0.8f*r1_near_clip_z/g1_render.center_y;
	//yof+=yof/maxyof;
	i4_float zof=0;
	i4_3d_vector pos(xof,yof,1.1*r1_near_clip_z);

	i4_transform_class scale, rot;
	//i4_transform_class transform(*forobj->world_transform);
	i4_transform_class transform;
	//forobj->world_transform
	//transform->identity();
	//transform->i4_transform_class(forobj->world_transform);
	transform.translate(pos);
	//transform.identity();
	transform.mult_rotate_y(g1_tick_counter*i4_2pi()/100.0f);
	transform.mult_rotate_x(i4_pi_2());
	transform.mult_uniscale(w);
	//transform.mult_translate(pos);

	i4_float save_ambient = g1_lights.ambient_intensity;
	g1_screen_box * bbox=0;
	if (g1_render.current_selectable_list)
	{
		bbox=g1_render.current_selectable_list->add();
	}
	if (bbox)
	{
		bbox->x1 = 2048.0f;
		bbox->y1 = 2048.0f;
		bbox->x2 = -1.0f;
		bbox->y2 = -1.0f;
		bbox->z1 = 999999.0f;
		bbox->z2 = -999999.0f;
		bbox->w  = 1.0f/999999.0f;
		bbox->object_id = forobj->global_id;
		bbox->flags=g1_screen_box::BUTTON;
		bbox->commandnum=i4_check_sum32(buttoncmd,strlen(buttoncmd));
	}
	g1_lights.ambient_intensity = 1.0f;
	g1_render.render_object(model, &transform, 0, w,
							g1_player_man.local_player,
							0,bbox,0 /*g1_render_class::RENDER_PROJECTTOPLANE*/);
	g1_lights.ambient_intensity = save_ambient;
};

//for most cases, this takes world coordinates, but not always!!!
void g1_human_class::player_clicked(g1_object_class * obj, float gx, float gy,w32 command)
{
	g1_object_type convoy_type=g1_get_object_type(li_convoy.get());

	switch (command)
	{
		case DEFAULT:
			{
				if (obj && obj->player_num==team() && obj->get_flag(g1_object_class::SELECTABLE))
				{
					clear_selected();

					if (obj == commander())
					{
						selected_object = 0;
					}
					else
					{

						g1_convoy_class * c=(g1_convoy_class *)g1_object_type_array[convoy_type]->create_object(convoy_type,0);
						c->setup(obj);
						c->player_num=team();
						selected_object=c;
						selected_object->mark_as_selected();
					}
				}
				else if (selected_object.valid())
				{
					send_selected_units(gx,gy);
				}
				else
				{
					select_path(gx,gy);
				}
			} break;
		case ATTACK:
			{
				if (selected_object.valid()&&selected_object->player_num==team())
				{
					attack_unit(obj,gx,gy);
				}                  //remember: this only works if
				//target is in range
				break;
			}
		case GOTO:
			if (selected_object.valid()&&selected_object->player_num==team())
			{
				send_selected_units(gx,gy);
			}
			break;
		case SELECT:
			{
				//single objects can be selected, even if they don't
				//belong to the local user
				if (!obj/*||obj->player_num!=team()
						||!obj->get_flag(g1_object_class::SELECTABLE) */)
				{
					break;
				}

				clear_selected();

				g1_convoy_class * c=(g1_convoy_class *)g1_create_object(convoy_type);
				c->player_num=obj->player_num;
				if (!obj->get_flag(g1_object_class::SELECTABLE))
				{
					c->player_num=0;
				}              //a slight hack to prevent the user
							   //from actually sending commands
							   //to this unit.
				c->setup(obj);
				selected_object=c;
				selected_object->mark_as_selected();

				break;
			}

		case ADD_TO_LIST:
			{
				if ((selected_object.valid()&&selected_object->id!=g1_get_object_type(li_convoy.get())) ||
					!obj || obj->player_num!=team() ||
					!obj->get_flag(g1_object_class::SELECTABLE))
				{
					break;
				}
				if (!selected_object.valid())
				{
					g1_convoy_class * cn=(g1_convoy_class *) g1_create_object(convoy_type);
					cn->player_num=team();
					cn->setup(obj);
					selected_object=cn;
					cn->mark_as_selected();
					break;
				}
				((g1_convoy_class *)selected_object.get())->add_object(obj);

			} break;
		case START_DRAG:
			{
				dragstartx=gx; //Screen coordinates!!!
				dragstarty=gy;
				break;
			}

		case DRAGGING:
			{
				dragdestx=gx;
				dragdesty=gy;
				//w16 mod;
				//if (i4_current_app->get_window_manager()->shift_pressed())
				//  mod=ADD_TO_OLD;
				//else if (i4_current_app->get_window_manager()->alt_pressed())
				//    mod=SUB_FROM_OLD;
				//else
				//    mod=CLEAR_OLD;
				//select_objects_in_range(dragstartx,dragstarty,dragendx,dragendy,mod);
			} break;

		case END_DRAG:
			{
				dragstartx=dragstarty=dragdestx=dragdesty=-1;
			} break;

		case SELECT_PATH:
			{
				select_path(gx,gy);
			} break;

		default:
			break;
	}
}

bool g1_human_class::prepare_to_build_building(g1_object_type type)
{
	if (prepared_building)
		delete prepared_building;

	prepared_building=NULL;
	g1_object_class * o=g1_object_type_array[type]->create_object(type,0);
	if (o)
	{
		//call o->set_flag(NEEDS_SYNC,1); and o->request_think(); when the object is actually
		//going to be used.
		prepared_building=o;
	}
	return false;
}
bool g1_human_class::is_building_prepared()
{
	return prepared_building!=NULL;
}
void g1_human_class::chancel_prepared_building()
{
	delete prepared_building;
	prepared_building=NULL;
}

int g1_human_class::build_unit(g1_object_type type)
{
	return (playback) ? G1_BUILD_PLAYBACK : g1_team_api_class::build_unit(type);
}

void g1_human_class::think()
{
	i4_bool process_input = i4_T;

	if (playback_think())
	{
		process_input=i4_F;
	}


	g1_player_piece_class * stank = commander();
	if ((g1_current_controller->view.get_view_mode()!=G1_ACTION_MODE &&
		 g1_current_controller->view.get_view_mode()!=G1_FOLLOW_MODE))
	{
		process_input=i4_F;
	}

	if (!process_input) //are we controlling the stank?
	{
		g1_input.deque_time(g1_input_class::LEFT);
		g1_input.deque_time(g1_input_class::RIGHT);
		g1_input.deque_time(g1_input_class::UP);
		g1_input.deque_time(g1_input_class::DOWN);
		g1_input.deque_time(g1_input_class::STRAFE_LEFT);
		g1_input.deque_time(g1_input_class::STRAFE_RIGHT);
		g1_input.deque_time(g1_input_class::STRAFE_UP);
		g1_input.deque_time(g1_input_class::STRAFE_DOWN);
		g1_input.deque_time(g1_input_class::ZOOM_IN);
		g1_input.deque_time(g1_input_class::ZOOM_OUT);
		g1_input.deque_time(g1_input_class::ZOOM_LEFT);
		g1_input.deque_time(g1_input_class::ZOOM_RIGHT);

		g1_input.deque_time(g1_input_class::ZOOM_UP);
		g1_input.deque_time(g1_input_class::ZOOM_DOWN);
//	if (g1_current_controller->view.get_view_mode()==G1_STRATEGY_MODE)
//		{
//		g1_resources.startegy_camera_dist+=(strategy_up-strategy_down)*G1_HZ/1000.0f;
//		if (g1_resources.startegy_camera_dist<=0.1f) g1_resources.startegy_camera_dist=0.1f;
//		g1_resources.startegy_camera_angle+=(strategy_out-strategy_in)*G1_HZ/1000.0f;
//		if (g1_resources.startegy_camera_angle<=0.1f) g1_resources.startegy_camera_angle=0.1f;
//		}
	}
	else
	{
		//keys are buffered in the order pressed, so do a reversed comparison
		if (memcmp(g1_input.grab_cheat_keys(),"DOG",3)==0)
		{
			g1_input.clear_cheat_keys();
			if (stank)
			{
				stank->toggle_stank_flag(g1_player_piece_class::ST_GODMODE);
			}
		}

		//This would actually belong the receive_event() function of
		//the border frame class, but it needs to be executed regardless
		//of wheter an event is outstanding.
		if (g1_border.get())
		{
			i4_float heh;
			i4_float cx=g1_border->last_mouse_x();
			i4_float lx=g1_border->prev_mouse_x();
			i4_float cy=g1_border->last_mouse_y();
			i4_float ly=g1_border->prev_mouse_y();
			if (cy<7)
			{
				heh=-0.2f;
			}
			else if (cy>(g1_border->height()-7))
			{
				heh=0.2f;
			}
			else
			{
				heh = ((cy - ly)*0.01f);
			}
			mouse_look_increment_y += heh;

			//if (mev->x<5)
			//  heh=-1.0f;
			//else if (mev->x>(width()-5))
			//  heh=1.0f;
			//else
			//if (mev->x>20&&mev->x<(width()-20))
			//    heh = (((sw32)mev->x - (sw32)mev->lx)*0.01f);
			//else
			//  heh=0;

			if (cx<20)
			{
				heh=-0.1f;
			}
			else if (cx>(g1_border->width()-20))
			{
				heh=0.1f;
			}
			else
			{
				heh = ((cx - lx)*0.01f);
			}
			mouse_look_increment_x += heh;
			g1_border->clear_mouse_move();
		}
		g1_map_piece_class * whofor=controlled();


		sw32
		left_ms=g1_input.deque_time(g1_input_class::LEFT),
		right_ms=g1_input.deque_time(g1_input_class::RIGHT),
		up_ms=g1_input.deque_time(g1_input_class::UP),
		down_ms=g1_input.deque_time(g1_input_class::DOWN),
		sleft_ms=g1_input.deque_time(g1_input_class::STRAFE_LEFT),
		sright_ms=g1_input.deque_time(g1_input_class::STRAFE_RIGHT);
		sw32 sup_ms=g1_input.deque_time(g1_input_class::STRAFE_UP);
		sw32 sdown_ms=g1_input.deque_time(g1_input_class::STRAFE_DOWN);
		sw32 zoom_in=g1_input.deque_time(g1_input_class::ZOOM_IN),
			 zoom_out=g1_input.deque_time(g1_input_class::ZOOM_OUT),
			 zoom_left=g1_input.deque_time(g1_input_class::ZOOM_LEFT),
			 zoom_right=g1_input.deque_time(g1_input_class::ZOOM_RIGHT),
			 zoom_up=g1_input.deque_time(g1_input_class::ZOOM_UP),
			 zoom_down=g1_input.deque_time(g1_input_class::ZOOM_DOWN);

		look(mouse_look_increment_x, mouse_look_increment_y,whofor);
		mouse_look_increment_y = mouse_look_increment_x = 0;
		turn( g1_resources.player_turn_speed*(left_ms-right_ms)*G1_HZ/1000.0f, whofor );
		accelerate( (up_ms-down_ms)*G1_HZ/1000.0f,whofor);
		strafe( (sright_ms-sleft_ms)*G1_HZ/1000.0f,
			   (sup_ms-sdown_ms)*G1_HZ/1000.0f,whofor);
		if (g1_current_controller->view.get_view_mode()==G1_FOLLOW_MODE)
		{
			g1_resources.follow_camera_dist+=(zoom_out-zoom_in)*G1_HZ/1000.0f;
			if (g1_resources.follow_camera_dist<=0.5f)
			{
				g1_resources.follow_camera_dist=0.5f;
			}
			g1_resources.follow_camera_height+=(zoom_up-zoom_down)*G1_HZ/1000.0f;
			g1_resources.follow_camera_rotation+=(zoom_right-zoom_left)*G1_HZ/1000.0f;
		}

		//if (sright_ms>0)
		//  sright_ms=sright_ms+1;

		if (g1_input.button_1())
		{
			fire0(whofor);
		}
		if (g1_input.button_2())
		{
			fire1(whofor);
		}
		if (g1_input.button_3())
		{
			fire2(whofor);
		}
	}


	if (g1_input.key_pressed)
	{
		continue_game();
	}

	g1_input.key_pressed=i4_F;
}

//{{{ Emacs Locals
// Local Variables:
// folded-file: t
// End:
//}}}
