#include "pch.h"

/********************************************************************** 

    Golgotha Forever - A portable, free 3D strategy and FPS game.
    Copyright (C) 1999 Golgotha Forever Developers

    Sources contained in this distribution were derived from
    Crack Dot Com's public release of Golgotha which can be
    found here:  http://www.crack.com

    All changes and new works are licensed under the GPL:

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    For the full license, see COPYING.

***********************************************************************/
 
#include "objs/trigger.h"
#include "object_definer.h"
#include "lisp/li_class.h"
#include "objs/model_draw.h"
#include "li_objref.h"
#include "math/random.h"
#include "map_man.h"
#include "map.h"
#include "g1_rand.h"

static li_float_class_member range_when_activated("range_when_activated"),
  range_when_deactivated("range_when_deactivated"),
  range_when_deployed("range_when_deployed"); 

static li_object_class_member team_can_trigger("team_can_trigger"),
   units_can_trigger("units_can_trigger"),
   message_to_send("message_to_send"),
   objects_to_trigger("objects_to_trigger"),
   objects_in_range("objects_in_range"),
   send_on_enter("send_on_enter"),
   send_on_leave("send_on_leave"),
   current_state("current_state"),
   toggle_state("toggle_state"),
   who_to_send("who_to_send"),
   nearby_objects("nearby_objects"),
   li_deploy_to("deploy_to"),
   objects_to_send_next_tick("objects_to_send_next_tick"),
   sendon_on("sendon_on"),sendon_off("sendon_off");
static li_int_class_member check_time("check_time"),
   cur_time("check_cur_time");
 

static li_symbol_ref on("on"), off("off"), none("none"), anyone("anyone"), team("team"),
  enemy("enemy"); 

//static g1_object_type stank_type;
 

g1_trigger_class::g1_trigger_class(g1_object_type id, g1_loader_class *fp)
    : g1_object_class(id,fp)
  {
    get_trigger_type();
    draw_params.setup("trigger");
  }

void g1_trigger_class::get_trigger_type()
    {    
    li_symbol *s=li_symbol::get(vars->get(team_can_trigger),0);
  
    char *sym[]={"everybody", "friend", "enemy",0 }, **a;
    for (a=sym, team_type=0; *a && li_get_symbol(*a)!=s; a++, team_type++);      
    }
void g1_trigger_class::object_changed_by_editor(g1_object_class *who, li_class *old_vars)
  {
    if (who==this)
      get_trigger_type();
  }
  
  
  // is the object in question one we should be triggered by?
i4_bool g1_trigger_class::triggable_object(g1_object_class *o)
  {
    switch (team_type)
    {
      case EVERYBODY:  break;
      case FRIEND: if (o->player_num!=player_num) return i4_F;
      case ENEMY: if (o->player_num==player_num) return i4_F;
    }
    li_symbol *s=li_symbol::get(vars->get_member(units_can_trigger),0);
    if (strcmp(s->name()->value(),"anyone")==0)
        return i4_T;
    if (strcmp(o->get_type()->name(),s->name()->value())==0)
        return i4_T;
    return i4_F;
  }

  // send our trigger message to all the object our list 
void g1_trigger_class::send_to_trigger_objects(li_symbol *sym,g1_object_class *who)
  {
    if (sym==none.get()) return; // don't send none, it means "no message"
  
    li_g1_ref_list *l=li_g1_ref_list::get(objects_to_trigger(),0);
    int t=l->size();
    for (int i=0; i<t; i++)
    {
      g1_object_class *o=l->value(i);
      if (o)
        o->message(sym, new li_g1_ref(who),0);
    }        
  }

void g1_trigger_class::note_enter_range(g1_object_class *who)
   {
     li_class_context context(vars);
    
     if (triggable_object(who))
     {
         send_to_trigger_objects(li_symbol::get(send_on_enter(),0),who);        
     }
   }

void g1_trigger_class::note_leave_range(g1_object_class *who)
   {
     li_class_context context(vars);

     if (triggable_object(who))
     {
         send_to_trigger_objects(li_symbol::get(send_on_leave(),0),who);                          
     }
   }
   
void g1_trigger_class::think()
    {
    li_class_context context(vars);
    if (cur_time()<=1)
        {
        cur_time()=check_time();
        float maxrange=range_when_activated();
        g1_object_class *dest_array[30];
        
        li_g1_ref_list *in_range=li_g1_ref_list::get(objects_in_range(),0);
        int num=g1_get_map()->get_objects_in_range(x,y,maxrange,
            dest_array,30,0xffffffff,
            g1_object_definition_class::MOVABLE|
            g1_object_definition_class::TO_MAP_PIECE);
        g1_object_class *o;
        int i;
        for (i=0;i<num;i++)
            {
            o=dest_array[i];
            if (!in_range->find(o))
                {
                in_range->add(o);
                note_enter_range(o);
                }
            }
        i4_bool found=i4_F;
        for (i=0;i<in_range->size();i++)
            {
            found=i4_F;
            o=in_range->value(i);
            if (o==0)
                continue;
            for (int k=0;k<num;k++)
                {
                if (dest_array[k]==o)
                    {
                    found=i4_T;
                    break;
                    }
                }
            if (found)
                continue;
            in_range->remove(o);
            note_leave_range(o);
            
            }
        in_range->compact();
        }
    cur_time()--;
    };
      

void g1_trigger_init()
{
  //stank_type=g1_get_object_type("stank");
}

g1_object_definer<g1_trigger_class>
trigger_def("trigger", 
            g1_object_definition_class::EDITOR_SELECTABLE|
            g1_object_definition_class::TO_TRIGGER, 
            g1_trigger_init);



class g1_director_class : public g1_object_class
{
public:
  g1_model_draw_parameters draw_params;
  void draw(g1_draw_context_class *context)  { g1_model_draw(this, draw_params, context); }
  virtual i4_float occupancy_radius() const { return draw_params.extent(); }
  
  g1_director_class(g1_object_type id, g1_loader_class *fp)
    : g1_object_class(id,fp)
  {
    draw_params.setup("trigger");
  }
  
  void think()
  {
    li_g1_ref_list *list=li_g1_ref_list::get(objects_to_send_next_tick(),0);
    while (list->size())
    {
      g1_object_class *o=list->value(0);
      list->remove(o->global_id);
      send_object(o);
    }    
  }

  void add_to_send_list(g1_object_class *o)
  {
    li_g1_ref_list *list=li_g1_ref_list::get(objects_to_send_next_tick(),0);
    if (list->find(o)==-1)
      list->add(o);
    
    request_think();
  }
  
  void send_object(g1_object_class *o)
  {
    if (!o) return ;
   
    li_g1_ref_list *list=li_g1_ref_list::get(li_deploy_to(),0);
    int list_size=list->size(), total=0;

    // count how many valid objects we point to
    for (int i=0; i<list_size; i++)
    {
      g1_object_class *to=list->value(i);
      if (to)
        total++;
    }

    if (total)
    {
      // pick one of the valid destinations
      int dest=g1_rand(56)%total;
      total=0;
      for (int j=0; j<list_size; j++)
      {
        g1_object_class *to=list->value(j);
        if (to)
        {
          if (total==dest)
          {
            o->deploy_to(to->x, to->y,0);
            return ;
          }
          else
            total++;
        }
      }
    }
    
    return ;
  }
  
//   void note_enter_range(g1_object_class *who, g1_fire_range_type range)
//   {
//     li_class_context context(vars);
    
//     if (range<=range_when_deployed() &&
//         (who_to_send()==anyone.get() ||
//          (who->player_num==player_num && who_to_send()==team.get()) ||
//          (who->player_num!=player_num && who_to_send()==enemy.get())))
//     {
//       li_g1_ref_list *r=li_g1_ref_list::get(nearby_objects());
//       if (r->find(who)==-1)
//       {
//         r->add(who);

//         add_to_send_list(who);
//       }
//     }
//   }

//   void note_leave_range(g1_object_class *who, g1_fire_range_type range)
//   {
//     li_class_context context(vars);
        
//     li_g1_ref_list *r=li_g1_ref_list::get(nearby_objects());
//     if (r->find(who)!=-1)
//       r->remove(who);
//   }

  li_object *message(li_symbol *message_name, li_object *message_params, li_environment *env)
  {
    li_class_context context(vars);
    
    if (message_name==on.get())
    {
      current_state()=on.get();

      // we just turned on, see if there are any nearby object we should send
      // to there destinations
      li_g1_ref_list *r=li_g1_ref_list::get(nearby_objects(),0);
      int t=r->size();
      for (int i=0; i<t; i++)
        add_to_send_list(r->value(i));
      
    } else if (message_name==off.get())
      current_state()=off.get();
	return li_true_sym;
  }


  // we were put down in the editor, scan for nearby objects and send them if we are on
  i4_bool occupy_location()
  {
    li_class_context context(vars);

    g1_object_class *olist[G1_MAX_OBJECTS];    
    int t=g1_get_map()->get_objects_in_range(x,y, range_when_deployed(), olist, G1_MAX_OBJECTS,
                                       0xffffffff,  g1_object_definition_class::MOVABLE);


    li_object *o=nearby_objects();

    li_g1_ref_list *r=li_g1_ref_list::get(o,0);
    
    while (r->size())
      r->remove(r->get_id(0));

    for (int i=0; i<t; i++)
    {
      r->add(olist[i]);
      if (current_state()==on.get())
        add_to_send_list(olist[i]);
    }        
    
    return g1_object_class::occupy_location();
  }

  
};

g1_object_definer<g1_director_class>
director_def("director", g1_object_definition_class::EDITOR_SELECTABLE);


/** Constructor for switch class. 
*   This class implements an on-off switch for scripting purposes
*/
#define SWITCH_DATA_VERSION 1
g1_switch_class::g1_switch_class(g1_object_type id, g1_loader_class *fp)
:g1_trigger_class(id,fp)
    {
    w16 ver=0,data_size=0;
    //The read/write mechanism here is just preparative. 
    //This class has no data to write except lisp vars.
    if (fp)
        fp->get_version(ver,data_size);
    switch(ver)
        {
        case SWITCH_DATA_VERSION:
            break;
        default:
            break;
        };
    if (fp)
        fp->end_version(I4_LF);
    set_flag(SELECTABLE,1);
    };

void g1_switch_class::save(g1_saver_class *fp)
    {
    g1_trigger_class::save(fp);
    fp->start_version(SWITCH_DATA_VERSION);
    fp->end_version();
    };

void g1_switch_class::load(g1_loader_class *fp)
    {
    g1_trigger_class::load(fp);
    w16 ver=0,data_size=0;
    fp->get_version(ver,data_size);
    fp->end_version(I4_LF);
    };

void g1_switch_class::skipload(g1_loader_class *fp)
    {
    g1_trigger_class::skipload(fp);
    w16 ver=0,data_size=0;
    fp->get_version(ver,data_size);
    fp->end_version(I4_LF);
    };

void g1_switch_class::note_enter_range(g1_object_class *who)
    {
    li_class_context ctx(vars);
    if (triggable_object(who))
        {
        if (current_state()==off.get())
            send_to_trigger_objects(li_symbol::get(sendon_on(),0),who); 
        current_state()=on.get();
        }
    };

void g1_switch_class::note_leave_range(g1_object_class *who)
    {
    li_class_context ctx(vars);
    if (triggable_object(who))
        {
        if (current_state()==on.get())
            {
            //check that there's no triggable_object in the range any more
            li_g1_ref_list *in_range=li_g1_ref_list::get(objects_in_range(),0);
        
            for (int i=0;i<in_range->size();i++)
                {
                if (triggable_object(who))
                    return;
                }
            send_to_trigger_objects(li_symbol::get(sendon_off(),0),who);
            current_state()=off.get();
            }
        }
    };

g1_object_definer<g1_switch_class>
    g1_switch_def("switch",
    g1_object_definition_class::EDITOR_SELECTABLE|
    g1_object_definition_class::TO_TRIGGER);

//This class will not be usefull before it is possible to select 
//objects whose owner is not the current player. 
//We should probably allow selecting a single non-player object.
g1_toggable_switch_class::g1_toggable_switch_class(g1_object_type id,
                                                   g1_loader_class *fp)
                                                   :g1_switch_class(id,fp)
    {
    set_flag(SELECTABLE,1);
    draw_params.setup("trigger");
    };

static li_symbol_ref commands_ask("commands-ask");
static li_symbol_ref commands_exec("commands-exec");
static li_symbol_ref command_toggle_switch("command-toggle_switch");
static li_symbol_ref command_to_far_switch("command-to_far_switch");

void g1_toggable_switch_class::note_enter_range(g1_object_class *who)
    {
    li_class_context ctx(vars);
    if (triggable_object(who))
        {
        current_state()=on.get();
        }
    };

void g1_toggable_switch_class::note_leave_range(g1_object_class *who)
    {
    li_class_context ctx(vars);
    if (triggable_object(who))
        {
        if (current_state()==on.get())
            {
            //check that there's no triggable_object in the range any more
            li_g1_ref_list *in_range=li_g1_ref_list::get(objects_in_range(),0);
        
            for (int i=0;i<in_range->size();i++)
                {
                if (triggable_object(who))
                    return;
                }
            current_state()=off.get();
            }
        }
    }

li_object *g1_toggable_switch_class::message(li_symbol *message_name,
                                             li_object *message_params,
                                             li_environment *env)
    {
    li_class_context ctx(vars);
    if (message_name==commands_ask.get())
		{
		//the caller is requesting the list of special options for
		//this unit. We add ourselves to what the parent returns.
        if (current_state()==on.get())
            {
		    return li_make_list(command_toggle_switch.get(),
			    g1_switch_class::message(message_name,message_params,env),
			    0);
            }
        else
            {
            return li_make_list(command_to_far_switch.get(),
                g1_switch_class::message(message_name,message_params,env),
                0);
            }
		};
	if (message_name==commands_exec.get())
		{
		//execute a command. If it is known, we do it and return true
		//such that the caller knows that the command was processed
		//if we don't know this command, we pass it on. 
		//If the caller gets a NULL-Response nobody knew how to handle the request
		//which should actually not happen. 
        
		if (message_params==command_toggle_switch.get())
			{
			if (current_state()==on.get())
                {
                if (toggle_state()==on.get())
                    {
                    send_to_trigger_objects(li_symbol::get(sendon_off(),0),this); 
                    toggle_state()=off.get();
                    }
                else
                    {
                    send_to_trigger_objects(li_symbol::get(sendon_on(),0),this);
                    toggle_state()=on.get();
                    }
                }
            return li_true_sym;
			}
        if (message_params==command_to_far_switch.get())
            return li_true_sym;
        }
	return g1_switch_class::message(message_name,message_params,env);
    };

g1_object_definer<g1_toggable_switch_class>
    g1_toggable_switch_def("toggable_switch",
    g1_object_definition_class::EDITOR_SELECTABLE|
    g1_object_definition_class::TO_TRIGGER);

g1_extended_trigger_class::g1_extended_trigger_class(g1_object_type id,
                                                     g1_loader_class *fp)
                                                     :g1_trigger_class(id,fp)
    {
    };


g1_object_definer<g1_extended_trigger_class>
    g1_extended_switch_def("extended_trigger",
    g1_object_definition_class::EDITOR_SELECTABLE|
    g1_object_definition_class::TO_TRIGGER);
