/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "pch.h"
#include "player.h"
#include "error/error.h"
#include "g1_limits.h"
#include "image/color.h"
#include "map.h"
#include "window/win_evt.h"
#include "device/kernel.h"
#include "objs/model_id.h"
#include "image/image.h"
#include "loaders/load.h"
#include "init/init.h"
#include "resources.h"
#include "g1_object.h"
#include "objs/map_piece.h"
#include "sound_man.h"
#include "tile.h"
#include "resources.h"
#include "remove_man.h"
#include "math/pi.h"
#include "g1_speed.h"
#include "light.h"
#include "m_flow.h"
#include "statistics.h"
#include "input.h"
#include "time/profile.h"
#include "g1_render.h"
#include "height_info.h"
#include "checksum/checksum.h"
#include "solvemap_astar.h"
#include "cwin_man.h"
#include "map_light.h"
#include "map_man.h"
#include "lisp/li_class.h"
#include "objs/path_object.h"
#include "objs/vehic_sounds.h"
#include "map_cell.h"
#include "map_vert.h"
#include "tick_count.h"
#include "map_view.h"
#include "map_data.h"
#include "solvemap_breadth.h"
#include "solvegraph_breadth.h"
#include "map_vars.h"
#include "lisp/lisp.h"
#include "lisp/li_types.h"
#include "lisp/li_init.h"
#include "objs/model_id.h"
#include "status/status.h"
#include "map_singleton.h"
#include "mess_id.h"
#include "file/get_filename.h"


#ifdef NETWORK_INCLUDED
#include "net/client.h"
#include "net/server.h"
#endif

g2_singleton *g2_singleton::first=0;

g1_height_info::g1_height_info()
{ 
  floor_height   = 0.f;
  ceiling_height = 1.f;
  flags = BELOW_GROUND;  
}


i4_profile_class pf_map_think("map: think objects");
//i4_profile_class pf_map_post_think("map.cc post_think objects");
i4_profile_class pf_notify_target_position_change("notify_target_pos_change");

g1_statistics_counter_class g1_stat_counter;

w32 g1_map_class::get_tick()
{
  return g1_tick_counter;
}
    
//g1_map_cell_class *g1_map_class::cell(w16 x, w16 y) const { return cells + y*w + x; }
//g1_map_cell_class *g1_map_class::cell(w32 offset) const  { return cells + offset; }
//g1_map_vertex_class *g1_map_class::vertex(w16 x, w16 y) const { return verts + y*(w+1) + x; }


void g1_map_class::add_object(g1_object_chain_class &c, w32 x, w32 y)
{
  cells[c.offset = (y*w+x)].add_object(c);
}

void g1_map_class::remove_object(g1_object_chain_class &c)
{ 
  cells[c.offset].remove_object(c);
}

i4_float g1_map_class::min_terrain_height(w16 x, w16 y)
{
  g1_map_vertex_class *v1,*v2,*v3,*v4;
    
  v1 = verts+ x + y * (w+1);
  v2 = v1+1;
  v3 = v1+w+1;
  v4 = v3+1;
    
  return g1_vertex_min(v1, g1_vertex_min(v2, g1_vertex_min(v3, v4)))->get_height();
}


g1_map_solver_class *g1_map_class::get_prefered_solver()
	{
	if (solvehint==0)
		{
		li_symbol_class_member fpath("path_finding");
        li_symbol_class_member ppath("path_binding");
		li_symbol_class_member traffic("traffic_sim");
        li_symbol_ref path_anyref("any");
        li_symbol_ref path_pref("path_only"),
        path_bref("breadth_solver"), path_aref("astar_solver"),
        path_sref("sight_only"),path_gref("graph_solver"),
        bind_dbref("default_bound"),bind_fbref("force_bound"),
        bind_furef("force_unbound"),bind_duref("default_unbound");
		li_symbol *sf=g1_map_vars.vars()->get(fpath);
	    g1_map_cell_class *c;
		int y,x,mw=width(),mh=height();
		for (y=0;y<mh;y++)
			{
			c=cell(0,y);
			for (x=0;x<mw;x++,c++)
				{
				g1_object_chain_class *p=c->get_obj_list();
				g1_object_class *o=0;
				g1_map_piece_class *mp=0;
				while(p)
					{
					o=p->object;
					mp=g1_map_piece_class::cast(o);
					if (mp)
						{
						mp->solveparams=mp->solveparams & ~SF_OK;
						}
					p=p->next;
					}
				
				}
			}
		solvehint=SF_OK;
		if (critical_graph->criticals>3) //otherwise, it is not usable
			{
			solvehint|=SF_HAS_GRAPH;
			}
		if (find_object_by_id(g1_get_object_type("path_object"),0)||
			find_object_by_id(g1_get_object_type("road_object"),0))
			{
			solvehint|=SF_HAS_PATHS;
			}

		li_symbol *tr=g1_map_vars.vars()->get(traffic);
		if (tr!=li_nil)
			{
			solvehint|=SF_BOUND_ANYWAY;
			}
		li_symbol *pf=g1_map_vars.vars()->get(ppath);

		if (pf==bind_dbref.get())
			solvehint|=SF_BOUND_DEFAULT;
		else if (pf==bind_duref.get())
			solvehint|=SF_BOUND_UNBOUND;
		else if (pf==bind_fbref.get())
			solvehint|=SF_BOUND_FORCE;
		else if (pf==bind_furef.get())
			solvehint|=SF_BOUND_FORCEUNBOUND;

		if (sf==path_anyref.get())
			{
			solvehint|=SF_USE_ANY;
			if (!has_paths())
				{
				solvehint&=~SF_USE_PATHS;
				}
			if (!has_graph())
				{
				solvehint&=~SF_USE_GRAPH;
				}
			if (solvehint&SF_USE_PATHS)
				return 0;
			else if (solvehint&SF_USE_GRAPH)
				return solvegraph;
			else return solvemap[0];

			}
		solvehint&=~SF_FORCEMASK;
		if (sf==path_pref.get())
			{
			solvehint|=SF_FORCE_PATHS|SF_USE_PATHS;
			return 0;
			}
		else if (sf==path_aref.get())
			{
			solvehint|=SF_FORCE_ASTAR|SF_USE_ASTAR;
			return astar_solver;
			}
		else if (sf==path_gref.get())
			{
			//solvehint|=SF_FORCE_GRAPH|SF_USE_GRAPH;
            solvehint|=SF_USE_GRAPH;
            //if we have a graph AND a path, we start out by
            //using the path, so we return null. 
            if (solvehint&SF_USE_PATHS)
                return 0;
			return solvegraph;
			}
		solvehint|=SF_FORCE_MAP|SF_GRADE1|SF_USE_MAP;
		return solvemap[0];//not all cases handled yet.
		}
	solve_flags masked;
	if ((masked=(solvehint&SF_FORCEMASK))!=0)
		{
		if (masked==SF_FORCE_PATHS)
			return 0;
		else if (masked==SF_FORCE_MAP)
			return solvemap[0];
		else if (masked==SF_FORCE_GRAPH)
			return solvegraph;
		else if (masked==SF_FORCE_ASTAR)
			return astar_solver;
		solvehint=0;//invalid hint
		return 0;
		}
	//masked=(solvehint&SF_USE_ANY);
	if (solvehint&SF_USE_PATHS)
		return 0;
	if (solvehint&SF_USE_GRAPH)
		return solvegraph;
	return solvemap[0];
	}

i4_bool g1_map_class::has_graph()
	{
	return (solvehint&SF_HAS_GRAPH)?i4_T:i4_F;
	}

i4_bool g1_map_class::has_paths()
	{
	return (solvehint&SF_HAS_PATHS)?i4_T:i4_F;
	}

// only use this if you know what you are doing
void g1_map_class::change_map(int _w, int _h,
                              g1_map_cell_class *_cells, g1_map_vertex_class *_vertex)
{
  if (cells)
    free(cells);
  
  if (verts)
    free(verts);

  cells=_cells;
  verts=_vertex;
  w=_w;
  h=_h;
}


void g1_map_class::remove_object_type(g1_object_type type)
{
  int i,j;//,k;
  g1_map_cell_class *c=cell(0,0);
  g1_object_chain_class *cell, *l;
  g1_object_class *obj;

  for (j=0; j<height(); j++)
    for (i=0; i<width(); i++, c++)
    {
      for (cell=c->get_obj_list(); cell;)
      {
        l=cell;
        cell=cell->next;
        obj=l->object;
        if (obj->id==type)
        {          
          obj->unoccupy_location();
          obj->request_remove();
        }
      }
    }
}


g1_map_class::g1_map_class(const i4_const_str &fname)
:map_maker(),map_objects(2000,2000)
{
  recalc=0xffffffff;     // recalculate everything until further notice

  sky_name=0;
  block_map_inited=i4_F;

//   lod=0;
  //  lod=(g1_map_lod_cell *) i4_malloc(sizeof(g1_map_lod_cell) * MAX_MAP_LOD, "lod");

  filename=0;
  set_filename(fname);

  post_cell_draw=0;

  w=0;
  h=0;


  think_head=think_tail=0;

  current_movie=0;
  
  cells=0;
  verts=0;
  path_manager=0;

  //  map=(g1_map_cell_class *)i4_malloc(w*h*sizeof(g1_map_cell_class),"map");
  //  vert=(g1_map_vertex_class *)i4_malloc((w+1)*(h+1)*sizeof(g1_map_vertex_class),"map vert");

  //w32 count=w*h;//,x,y;
  critical_graph=new g1_critical_graph_class();
  astar_solver = new g1_astar_map_solver_class();
  //map_maker = new g1_critical_map_maker_class();
  map_maker.make_criticals(this, critical_graph);
  for (int i=0;i<G1_GRADE_LEVELS;i++)
	  //cannot actually initialize the solver before the block maps are loaded
	solvemap[i]=new g1_breadth_first_map_solver_class();
  solvegraph=new g1_breadth_first_graph_solver_class(critical_graph);
  solvegraph->set_graph(critical_graph);
  solvehint=0;
  //delete maker1;
  movie_in_progress=i4_F;
}

g1_map_class::~g1_map_class()
{
  g1_stop_sound_averages();

  g1_map_class *old_current=g1_current_map_PRIVATE;
  g1_set_map(this);
  i4_status_class *stat=i4_create_status("Removing map from memory...");

  
  for (g1_map_data_class *md=g1_map_data_class::first; md; md=md->next)
    md->free();

  if (critical_graph)
	  delete critical_graph;

  if (filename)
    delete filename;

  if (sky_name)
    delete sky_name;

  if (cells)
  {
    g1_object_class *olist[G1_MAX_OBJECTS];
    w32 ids[G1_MAX_OBJECTS];

    sw32 t=make_object_list(olist, G1_MAX_OBJECTS), i;

    // can't use object pointers directly because one
    // object might delete another
    for (i=0; i<t; i++)
      ids[i]=olist[i]->global_id;
      

    for (i=0; i<t; i++)
    {
	  if (stat) 
		  stat->update((float)i/t);
      if (g1_global_id.check_id(ids[i]))
      {
        g1_object_class *o=g1_global_id.get(ids[i]);        
        o->unoccupy_location();
        o->request_remove();
        g1_remove_man.process_requests();        
      }
    }
    free(cells);

  }
    

  if (verts)
    free(verts);

  if (current_movie)
    delete current_movie;
  
  if (astar_solver)
    delete astar_solver;
  for (int i=0;i<G1_GRADE_LEVELS;i++)
	  delete solvemap[i];

  if (solvegraph)
	  delete solvegraph;
  
  delete stat;
  
  if (old_current==this)
    g1_set_map(0);
  else
    g1_set_map(old_current);

  //shutdown networking
#ifdef NETWORK_INCLUDED
  i4_network_shutdown();
#endif
  li_call("unload_transims");
  g1_map_vars.var_ptr=0;//destroy this reference such that it will be 
  //created new next time
 
}

void check_olist()
{
  g1_object_class *olist[G1_MAX_OBJECTS];
  sw32 t=g1_get_map()->make_object_list(olist, G1_MAX_OBJECTS);
}

sw32 g1_map_class::make_object_list(g1_object_class **&buffer, OBJ_FILTER_TYPE obj_filter)
	{//extended version, returns only objects that match the given filter.
	//also returns managed objects if they match!
	//hm... how do we know how many objects there are?
	//we dont, but fortunatelly we now have a dynamic array for this
	int x=width()*height(), i;
	//sw32 t=0;
	g1_map_cell_class *c=cells;
	//if (buf_size==0) return 0;
	
	map_objects.clear();
	for (i=0; i<x; i++, c++)
		{
		if (c->object_list)
			{
			for (g1_object_chain_class *obj=c->get_obj_list(); obj; obj=obj->next)
				{
				g1_object_class *o=obj->object;
				if (!o->get_flag(g1_object_class::SCRATCH_BIT) && (!obj_filter||obj_filter(o)))   
					// make sure object only added once
					{
					o->set_flag(g1_object_class::SCRATCH_BIT, 1);
					map_objects.add(o);
					
					}
				}
			}
		}
	buffer=&map_objects[0];
	//this is probably a bit faster than iterating over the i4_array
	sw32 t=map_objects.size();
	for (i=0; i<t; i++)
		buffer[i]->set_flag(g1_object_class::SCRATCH_BIT, 0);
		
		
	return t;
		
		
	}


// returns total added
sw32 g1_map_class::make_object_list(g1_object_class **buffer, sw32 buf_size)  
{
  int x=width()*height(), i;
  sw32 t=0;
  g1_map_cell_class *c=cells;
  if (buf_size==0) return 0;


  for (i=0; i<x; i++, c++)
    if (c->object_list)
    {
      for (g1_object_chain_class *obj=c->get_obj_list(); obj; obj=obj->next)
      {
        g1_object_class *o=obj->object;
        if (!o->get_flag(g1_object_class::SCRATCH_BIT) && o->global_id!=0)   
		// make sure object only added once
        {
          o->set_flag(g1_object_class::SCRATCH_BIT, 1);
          buffer[t++]=o;
          if (t==buf_size) 
          {
            i=x;
            obj=0;
          }
        }
      }
    }

  for (i=0; i<t; i++)
    buffer[i]->set_flag(g1_object_class::SCRATCH_BIT, 0);

  return t;
}

sw32 g1_map_class::make_selected_objects_list(w32 *buffer, sw32 buf_size)
{
  int x=width()*height(), i;
  sw32 t=0;
  g1_map_cell_class *c=cells;
  if (buf_size==0) return 0;


  for (i=0; i<x; i++, c++)
    if (c->object_list)
    {
      for (g1_object_chain_class *obj=c->get_obj_list(); obj; obj=obj->next)
      {
        g1_object_class *o=obj->object;

        // make sure object only added once
        if (o->selected() && !o->get_flag(g1_object_class::SCRATCH_BIT) && o->global_id)   
        {
          o->set_flag(g1_object_class::SCRATCH_BIT, 1);
          buffer[t++]=o->global_id;
          if (t==buf_size) 
          {
            i=x;
            obj=0;
          }
        }
      }
    }

  for (i=0; i<t; i++)
    g1_global_id.get(buffer[i])->set_flag(g1_object_class::SCRATCH_BIT, 0);

  return t;
}
extern w32 g1_disable_all_drawing;

//let the objects think
void g1_map_class::think_objects()
{
  li_class *old_this=li_this;
  if (!g1_current_view_state())//might happen in some situations (view has just changed)
	  {
	  return;
	  }

  sw32 thinkloops=g1_disable_all_drawing>1?120:0;//just think a lot of times
  do 
	  {
  recalc_static_stuff();


  pf_map_think.start();
  
  g1_realtime_saver_class *rtfp=0;
  
#ifdef NETWORK_INCLUDED
  if (i4_network_active())
	  {
	  i4_network_poll();
	  //this returns true if we must wait (i.e because of lost sync
	  //or insufficient id's available)
	  //if (i4_network_mustwait())
	//	  {
	//	  pf_map_think.stop();
	//	  return;
	//	  }
	  if (i4_network_prepare_command(G1_PK_GAME_DATA)<0)
		  {
		  //i4_warning("Waiting for other players...");
		  pf_map_think.stop();
		  return;
		  }
	  rtfp=new g1_realtime_saver_class(network_file,i4_F);
	  }
#endif
  g1_input.que_keys(tick_time);
  //do the thinking for all singletons.
  for (g2_singleton *s=g2_singleton::first;s;s=s->next_sin)
	  {
	  s->think();
	  };
  li_symbol *loosefn=li_symbol::get(li_get_symbol("loose_function"),0);
  if (loosefn && ((g1_tick_counter & 0xf)==0))
	  {
	  
	  li_object *haslost=
		  li_call(loosefn,li_make_list(new li_int(g1_tick_counter),
		  new li_int(g1_player_man.local_player),0),0);
	  if (haslost==li_true_sym)
		  {
		  li_call("ForcePause",0,0);
		  i4_user_message_event_class loser(G1_YOU_LOSE);
		  i4_kernel.send_event(i4_current_app,&loser);
		  }
	  }
  li_symbol *winfn=li_symbol::get(li_get_symbol("winning_function"),0);
  if (winfn && ((g1_tick_counter & 0xf)==0))
	  {
	  
	  li_object *haswon=
		  li_call(winfn,li_make_list(new li_int(g1_tick_counter),
		  new li_int(g1_player_man.local_player),0),0);
	  if (haswon==li_true_sym)
		  {
		  li_call("ForcePause",0,0);
		  i4_file_open_message_class winner(G1_YOU_WIN,0);
		  i4_kernel.send_event(i4_current_app,&winner);
		  }
	  else if (haswon!=li_nil && haswon!=0)
		  {
		  li_call("ForcePause",0,0);
		  li_string *str=li_string::get(haswon,0);
		  if (str)
			  {
			  i4_file_open_message_class winner2(G1_YOU_WIN,
				new i4_str(str->value()));
		      i4_kernel.send_event(i4_current_app,&winner2);
			  }
		  else
			  {
			  i4_error("USER: The level indicates you have won, but "
				  "the info about the next level is invalid. The winning "
				  "function must return a string.");
			  }
		  }
	  }
  w32 i,h;
  
  //w32 start_tail = think_tail;
  //w32 start_head = think_head;

  //important: Although the head of the queue might go on during think
  //(new objects added, including self reinserted) we must only go
  //to that point
  w32 start_tail=0;
  w32 start_head=think_que_dyn.size();
  
  //do the think()'s  
  i = start_tail;
  h = start_head;

  g1_reset_sound_averages();

  //pf_map_think.start();
  g1_object_class *o=0;

  for (;i<h;i++)
  {
    o=think_que_dyn[i];

    //always check to make sure the pointer
    //is good. objects might have removed themselves
    //from the map while still in the que, and left
    //a null pointer in their place
    if (o)
    {
      li_this=o->vars;
      o->grab_old();
    }

    //i++;
    //if (i>=THINK_QUE_SIZE)
     // i=0;
  }

  i = 0;//start_tail;
  //h = start_head;
  for (; i<h;i++ )
  {
    o=think_que_dyn[i];

    //always check to make sure the pointer
    //is good. objects might have removed themselves
    //from the map while still in the que, and left
    //a null pointer in their place
    if (o)
    {
      li_this=o->vars;
      o->set_flag(g1_object_class::THINKING, 0);
      o->think();
	  if (!o->get_flag(g1_object_class::USES_POSTTHINK))
		  {
		  think_que_dyn[i]=0;//because of cache strategies, it's better to
		  //do this here
		  }
#ifdef NETWORK_INCLUDED
	  if (rtfp)
		  {
		  if (o->get_flag(g1_object_class::NEEDS_SYNC))
			  {
			  w32 sizebefore=rtfp->tell();
			  rtfp->write_32(o->global_id);
			  rtfp->write_16(o->id);
			  o->save(rtfp);
			  if (rtfp->tell()>=MAX_PACKET_SIZE-4)
				  {
				  rtfp->seek(sizebefore);
				  rtfp->write_32(0);
				  i4_network_prepare_command(G1_PK_PROCESS);
				  rtfp->write_32(o->global_id);
				  rtfp->write_16(o->id);
				  o->save(rtfp);
				  
				  }
			  o->set_flag(g1_object_class::NEEDS_SYNC,0);
			  o->set_flag(g1_object_class::LAST_SYNC,1);
			  }
		  //don't propagate the deletion of non-synced objects
		  //as this might propose deletion of some object that is not 
		  //really the one that is expected.
		  //??? Well, no: It should not be possible that an id has two 
		  //different meanings on two systems, but that the same object
		  //on two systems have different ids.
		  if (o->get_flag(g1_object_class::DELETED&g1_object_class::NEEDS_SYNC))
			  {//Be shure that any deletions will be consistent
			  rtfp->write_32(g1_global_id_manager_class::ID_DELETEPROPAGATE);
			  rtfp->write_32(o->global_id);
			  };
		  }
#endif
    }
	

    //i++;
    //if (i>=THINK_QUE_SIZE)
    //  i=0;
  }

  //pf_map_think.stop();


  //pf_map_post_think.start();
  
  //do the post_think()'s
  i = start_tail;
  h = start_head;

  for (; i<h; i++)
  {
    //g1_object_class *o=think_que_dyn[i];    
	think_que_dyn.deque(o);//now remove the entries
    //think_tail++;
    //if (think_tail>=THINK_QUE_SIZE)
    //  think_tail=0;

    //always check to make sure the pointer
    //is good. objects might have removed themselves
    //from the map while still in the que, and left
    //a null pointer in their place
    if (o)
    {
      li_this=o->vars;
      o->post_think();
    }

    //i++;
    //if (i>=THINK_QUE_SIZE)
    //  i=0;
  }
  //pf_map_think.stop();
#ifdef NETWORK_INCLUDED
  if (i4_network_active())
	  {
	  i4_network_poll();
	  rtfp->write_32(0);
	  i4_network_prepare_command(G1_PK_PROCESS);
	  delete rtfp;
      rtfp=0;
	  }
#endif

  g1_recalc_sound_averages();


  g1_tick_counter++;
  tick_time.add_milli((1000/G1_HZ));


  li_this=old_this;
  pf_map_think.stop();
	} 
  while(--thinkloops>0);
	  //think_loops
  
}


void g1_map_class::damage_range(g1_object_class *obj,
                                i4_float x, i4_float y, i4_float z, 
                                i4_float range, w16 damage, i4_float falloff)
// damage to vehicles centers at x,y,z and falls off by falloff*damage at range distance
//   from the center of the damage range
{
  sw32 ix,iy, 
    sx = sw32(x-range),
    ex = sw32(x+range)+1,
    sy = sw32(y-range),
    ey = sw32(y+range)+1;

  // clip region
  if (sx<0) sx=0;
  if (ex>width()) ex=width();
  if (sy<0) sy=0;
  if (ey>height()) ey=height();

  // get first one
  g1_map_cell_class *c;
  g1_object_chain_class *objlist;
  i4_float dist,dx,dy,dz;

  range *= range;
  falloff *= i4_float(damage)/range;

  for (iy=sy; iy<ey; iy++)
  {
    c = cell((w16)sx,(w16)iy);
    for (ix=sx; ix<ex; ix++, c++)
    {
      for (objlist=c->get_obj_list(); objlist; objlist=objlist->next)
      {
        g1_object_class *hurt_obj=objlist->object;

        if (objlist==&hurt_obj->occupied_squares[0]) // make sure object is only hurt once
        {
          if (hurt_obj->get_flag(g1_object_class::TARGETABLE))
// && hurt_obj->player_num!=obj->player_num)
          {
            dx=x-hurt_obj->x;
            dy=y-hurt_obj->y;
            dz=z-hurt_obj->h;
            dist = dx*dx+dy*dy+dz*dz;
            if (dist<range)
              hurt_obj->damage(obj, damage - (int)(falloff*range), i4_3d_vector (0,0,g1_resources.gravity));
          }
        }
      }
    }
  }
}



g1_object_class *g1_map_class::find_object_by_id(w32 object_id,
                                                 g1_player_type prefered_team)
{
  sw32 i,j=width()*height();
  g1_map_cell_class *c=cells;
  g1_object_chain_class *o;
  g1_object_class *best=0;

  for (i=0; i<j; i++, c++)
  {
    for (o=c->get_obj_list(); o; o=o->next)
    {
      if ((w32)o->object->id==object_id)
      {
        if (o->object->player_num==prefered_team)              
          return o->object;
        else
          best=o->object;
      }
    }
  }
  return best;
}


void g1_map_class::remove_from_think_list(g1_object_class *obj)
{
  /*w32 i = think_tail,
      h = think_head;
  
  for (; i!=h;)
  {
    if (think_que[i]==obj) think_que[i]=0;
      
    i++;
    if (i>=THINK_QUE_SIZE) 
      i=0;
  }*/
  int t=think_que_dyn.size();
  for (int i=0;i<t;i++)
	  {
	  if (think_que_dyn[i]==obj) 
		  think_que_dyn[i]=0;
	  }
}

void g1_map_class::request_remove(g1_object_class *obj)
{
  obj->flags |= g1_object_class::DELETED;

  if (obj->flags & g1_object_class::THINKING)
  {
    obj->stop_thinking();
  }
  
  g1_remove_man.request_remove(obj);
}


i4_const_str g1_map_class::get_filename()
{
  return *filename;
}

void g1_map_class::set_filename(const i4_const_str &fname)
{
  if (filename)
    delete filename;
  filename=new i4_str(fname);
}


#ifndef max
#define max(a,b) (((a)>(b))?(a):(b))
#endif

i4_float grade_max[G1_GRADE_LEVELS] = { 10, 17, 30, 256 };

void g1_map_class::make_block_maps()
{
  //static i4_float grade_max[G1_GRADE_LEVELS] = { 10, 17, 30, 256 };
  w16 grade, x,y;

  for (grade = 0; grade<G1_GRADE_LEVELS; grade++)
  {
    block[grade].init(width(),height());
    block[grade].clear();

    for (x=0; x<width(); x++)
      for (y=0; y<height(); y++)
      {//for all grades except the last blocking comes from the preferences in the texture list of the scm file
        if (x==0 || y==0 || x==width()-1 || y==height()-1 || 
            (cell(x,y)->is_blocking() && grade<G1_GRADE_LEVELS-1))
          block[grade].block(x,y, G1_NORTH|G1_SOUTH|G1_WEST|G1_EAST);
        else
        {
          g1_object_chain_class *ch = cell(x,y)->get_solid_list();
		  //skip while (there's an object) and (that object is not blocking
		  //or that object can move.)
		  //todo: skip also if object is smaller than 4 squares on the map
		  //and set the BLOCKS_MAP flag on objects that are larger. (they
		  //were skipped on creation as the blockmap didn't exist yet)
          while (ch &&
                 (!ch->object->get_flag(g1_object_class::BLOCKING) ||
				 ch->object->get_flag(g1_object_class::CAN_DRIVE_ON) ||
                 g1_object_type_array[ch->object->id]->
                 get_flag(g1_object_definition_class::MOVABLE) ||
				 ch->object->occupied_squares.size()<=4))
            ch = ch->next;

          if (ch)
			  {
              block[grade].block(x,y, G1_NORTH|G1_SOUTH|G1_WEST|G1_EAST);
			  ch->object->set_flag(g1_object_class::BLOCKS_MAP,1);
			  }
          else
          {
            i4_float max_grade = grade_max[grade];
            g1_map_vertex_class *v1, *v2, *v3, *v4;
          
            v1=verts + x + y * (w+1);
            v2=v1+1;
            v3=v1+w+1;
            v4=v3+1;
          
            w8 h1=v1->height, 
              h2=v2->height, 
              h3=v3->height, 
              h4=v4->height;
          
            i4_float
              grade_ns = max(h3-h1, h4-h2),
              grade_ew = max(h2-h1, h4-h3),
              grade_sn = max(h1-h3, h2-h4),
              grade_we = max(h1-h2, h3-h4);
          
            if (grade_ns>max_grade || grade_sn>max_grade)
            {
              if (grade_ns>max_grade)
                block[grade].block(x,y,G1_SOUTH);
              if (grade_sn>max_grade)
                block[grade].block(x,y,G1_NORTH);

              if (grade_ew>0)
                block[grade].block(x,y,G1_WEST);
              if (grade_we>0)
                block[grade].block(x,y,G1_EAST);
            }
            if (grade_ew>max_grade || grade_we>max_grade)
            {
              if (grade_ew>max_grade)
                block[grade].block(x,y,G1_WEST);
              if (grade_we>max_grade)
                block[grade].block(x,y,G1_EAST);

              if (grade_ns>0)
                block[grade].block(x,y,G1_SOUTH);
              if (grade_sn>0)
                block[grade].block(x,y,G1_NORTH);
            }
          }
        }
      }
  }
  block_map_inited=i4_T;
}

void g1_map_class::update_block_maps(w16 x, w16 y, g1_object_class *changingobj, i4_bool added)
	{//will add/remove any object passed, so call only for buildings.
	//static i4_float grade_max[G1_GRADE_LEVELS] = { 10, 17, 30, 256 };
	int i,grade;
	if (!block_map_inited)
		return;
	I4_ASSERT((x<w)&&(y<h),"INTERNAL: Invalid coordinates for block map update");
	if (added)
		{
		if (!changingobj->get_flag(g1_object_class::BLOCKING))
			{
			return;
			}
		for (i=0;i<G1_GRADE_LEVELS;i++)
			{
			block[i].block(x,y,G1_WEST|G1_EAST|G1_SOUTH|G1_NORTH);
			}
		changingobj->set_flag(g1_object_class::BLOCKS_MAP,1);
		}
	else
		{
		for (grade = 0; grade<G1_GRADE_LEVELS; grade++)
			{//for all grades except the last blocking comes from the preferences in the texture list of the scm file
			block[grade].unblock(x,y,G1_ALL_DIRS);
			if (x==0 || y==0 || x==width()-1 || y==height()-1 || 
				(cell(x,y)->is_blocking() && grade<G1_GRADE_LEVELS-1))
				block[grade].block(x,y, G1_NORTH|G1_SOUTH|G1_WEST|G1_EAST);
			else
				{
				g1_object_chain_class *ch = cell(x,y)->get_solid_list();
				while (ch &&
					(!ch->object->get_flag(g1_object_class::BLOCKING) ||
					//ch->object->get_flag(g1_object_class::CAN_DRIVE_ON) ||
					g1_object_type_array[ch->object->id]->
					get_flag(g1_object_definition_class::MOVABLE)))
					{
					ch = ch->next;
					};
				
				if (ch)
					block[grade].block(x,y, G1_NORTH|G1_SOUTH|G1_WEST|G1_EAST);
				else
					{
					i4_float max_grade = grade_max[grade];
					g1_map_vertex_class *v1, *v2, *v3, *v4;
					
					v1=verts + x + y * (w+1);
					v2=v1+1;
					v3=v1+w+1;
					v4=v3+1;
					
					w8 h1=v1->height, 
						h2=v2->height, 
						h3=v3->height, 
						h4=v4->height;
					
					i4_float
						grade_ns = max(h3-h1, h4-h2),
						grade_ew = max(h2-h1, h4-h3);
					i4_float
						grade_sn = max(h1-h3, h2-h4),
						grade_we = max(h1-h2, h3-h4);
					
					if (grade_ns>max_grade || grade_sn>max_grade)
						{
						if (grade_ns>max_grade)
							block[grade].block(x,y,G1_SOUTH);
						if (grade_sn>max_grade)
							block[grade].block(x,y,G1_NORTH);
						
						if (grade_ew>0)
							block[grade].block(x,y,G1_WEST);
						if (grade_we>0)
							block[grade].block(x,y,G1_EAST);
						}
					if (grade_ew>max_grade || grade_we>max_grade)
						{
						if (grade_ew>max_grade)
							block[grade].block(x,y,G1_WEST);
						if (grade_we>max_grade)
							block[grade].block(x,y,G1_EAST);
						
						if (grade_ns>0)
							block[grade].block(x,y,G1_SOUTH);
						if (grade_sn>0)
							block[grade].block(x,y,G1_NORTH);
						}
					}
				}
			}
		changingobj->set_flag(g1_object_class::BLOCKS_MAP,0);
		}
	}


void g1_map_class::recalc_static_stuff()
{
  if (recalc==0)//don't do anything if recalc is zero.
	  return;
  i4_bool reset_time=i4_F;

   if (recalc & (G1_RECALC_BLOCK_MAPS | G1_RECALC_CRITICAL_DATA))
     li_call("add_undo", li_make_list(new li_int(G1_MAP_CRITICAL_DATA), 0));
    
  if (recalc & G1_RECALC_STATIC_LIGHT)
    g1_calc_static_lighting();               // defined in light.cc


   if (recalc & G1_RECALC_BLOCK_MAPS)    
   {
     make_block_maps();
	 for (int i=0;i<G1_GRADE_LEVELS;i++)
		 solvemap[i]->set_block_map(get_block_map(i));
     reset_time=i4_T;
   }

  if (recalc & G1_RECALC_RADAR_VIEW)
  {
    //init_lod();   //(OLI) need to put this in the proper place
  //This is ok.

    g1_radar_recalculate_backgrounds();
    reset_time=i4_T;
  }

  if (recalc & (G1_RECALC_WATER_VERTS))
  {
    g1_map_vertex_class *v=verts+(w+1)+1;
    g1_map_cell_class *c=cells+(w)+1;

    for (int y=1; y<(int)h-1; y++)
    {
      for (int x=1; x<(int)w-1; x++, c++, v++)
      {
        if ((g1_tile_man.get(c[0].type)->flags & g1_tile_class::WAVE) &&
            (g1_tile_man.get(c[-1].type)->flags & g1_tile_class::WAVE) &&
            (g1_tile_man.get(c[-(int)w].type)->flags & g1_tile_class::WAVE) && // JJ cast -  unary minus operator applied to unsigned type, result still unsigned !
            (g1_tile_man.get(c[-(int)w-1].type)->flags & g1_tile_class::WAVE)) // JJ cast
          v->flags |= g1_map_vertex_class::APPLY_WAVE_FUNCTION;
      }
      v+=2;
      c+=2;
      v++;      
    }

    reset_time=i4_T;
  }
  if (recalc&(G1_MAP_CRITICAL_DATA|G1_MAP_CRITICAL_POINTS))
	  {
	  //map_maker.make_criticals(this,critical_graph);
	  solvegraph->set_graph(critical_graph);
	  recalc|=G1_MAP_VARS;
	  reset_time=i4_T;
	  }

  if (recalc&G1_MAP_VARS)
	  {
	  solvehint=0;
	  li_float_class_member model_sc("model_scaling");
	  get_prefered_solver();//precalc only
	  float f=g1_map_vars.vars()->get(model_sc);
	  g1_model_list_man.scale_models(f);
	  }
  
  recalc=0;

  if (reset_time)
    tick_time.get();     // don't simulate ticks for the calculation stuff
}


void g1_map_class::range_iterator::begin(float x, float y, float range)
//{{{
{
  g1_map_class *map = g1_get_map();
  int wx = map->width(), wy = map->height();

  left   = i4_f_to_i(x - range); if (left<0)      left=0;
  right  = i4_f_to_i(x + range); if (right>wx-1)  right=wx-1;
  top    = i4_f_to_i(y - range); if (top<0)       top=0;
  bottom = i4_f_to_i(y + range); if (bottom>wy-1) bottom=wy-1;

  ix = right;
  iy = top-1;
  cell = 0;
  chain = 0;

  object_mask_flags=0xffffffff;
  type_mask_flags=0xffffffff;
}
//}}}

void g1_map_class::range_iterator::safe_restart()
//{{{
{
  chain=0;
}
//}}}

i4_bool g1_map_class::range_iterator::end()
//{{{
{
  return (iy>=bottom);
}
//}}}

void g1_map_class::range_iterator::next()

{
  return;
  /*
  g1_object_class *o;

  if (chain) chain = chain->next;

  while (iy<bottom)
  {
    if (!chain)
    {
      ++ix;
      ++cell;
      if (ix<=right)
        chain = cell->get_obj_list();
      else
      {
        ix = left;
        iy++;
        cell = &g1_get_map()->cell(left, iy);
      }
    }
    else
    {
      o = chain->object;
      if (&o->occupied_squares[0]!=chain ||
          (o->flags & object_mask_flags)==0 ||
          (g1_object_type_array[o->id]->flags & type_mask_flags)==0)
        chain = chain->next;
      else
        return;
    }
  }
  */
}

sw32 g1_map_class::get_objects_in_range_fn(float x, float y, float range,
										g1_object_class *dest_array[],
										w32 array_size,
										OBJ_FILTER_TYPE fn) const
	{
	sw32 x_left,x_right,y_top,y_bottom;
  
  x_left   = i4_f_to_i(x - range); if (x_left<0)     x_left=0;
  x_right  = i4_f_to_i(x + range); if (x_right>(sw32)(w-1))  x_right=w-1;
  y_top    = i4_f_to_i(y - range); if (y_top<0)      y_top=0;
  y_bottom = i4_f_to_i(y + range); if (y_bottom>(sw32)(h-1)) y_bottom=h-1;
  
  sw32 ix,iy;  
  sw32 num_found=0;
  
  for (iy=y_top;  iy<=y_bottom; iy++)
  {
    g1_map_cell_class *cell = g1_get_map()->cell((w16)x_left, (w16)iy);

    for (ix=x_left; ix<=x_right;  ix++, cell++)
    {      
      g1_object_chain_class *p     = cell->get_solid_list();

      while (p && num_found < (int)array_size)
      {    
        g1_object_class *o = p->object;
		//second condition assures object is only found once
        if (o && (&o->occupied_squares[0]==p) && (fn(o)))
        {
          
            dest_array[num_found] = o;
            num_found++;
          
        }
      
        p = p->next_solid();
      }
    
      if (num_found >= (int)array_size)
        break;
    }
    
    if (num_found >= (int)array_size)
        break;
  }
  
  return num_found;
	}

sw32 g1_map_class::get_objects_in_range(float x, float y, float range,
                                        g1_object_class *dest_array[], w32 array_size,
                                        w32 object_mask_flags, w32 type_mask_flags) const
{
  sw32 x_left,x_right,y_top,y_bottom;
  
  x_left   = i4_f_to_i(x - range); if (x_left<0)     x_left=0;
  x_right  = i4_f_to_i(x + range); if (x_right>(sw32)(w-1))  x_right=w-1;
  y_top    = i4_f_to_i(y - range); if (y_top<0)      y_top=0;
  y_bottom = i4_f_to_i(y + range); if (y_bottom>(sw32)(h-1)) y_bottom=h-1;
  
  sw32 ix,iy;  
  sw32 num_found=0;
  
  for (iy=y_top;  iy<=y_bottom; iy++)
  {
    g1_map_cell_class *cell = g1_get_map()->cell((w16)x_left, (w16)iy);

    for (ix=x_left; ix<=x_right;  ix++, cell++)
    {      
      g1_object_chain_class *p     = cell->get_solid_list();

      while (p && num_found < (int)array_size)
      {    
        g1_object_class *o = p->object;

        if (o && (&o->occupied_squares[0]==p) && (o->flags & object_mask_flags))
        {
          if (g1_object_type_array[o->id]->flags & type_mask_flags)
          {
            dest_array[num_found] = p->object;
            num_found++;
          }
        }
      
        p = p->next_solid();
      }
    
      if (num_found >= (int)array_size)
        break;
    }
    
    if (num_found >= (int)array_size)
        break;
  }
  
  return num_found;
}

LI_HEADER(map_recalc)
	{
	if (g1_get_map())
		{
		g1_get_map()->solvehint=0;
		g1_get_map()->get_prefered_solver();//perhaps we now have roads?
		g1_get_map()->mark_for_recalc(0xffffffff);
		g1_get_map()->recalc_static_stuff();
		return li_true_sym;
		}
	return li_nil;
	}

li_automatic_add_function(li_map_recalc,"Map/Recalculate");

LI_HEADER(has_map)
    {
    if (g1_get_map())
        return li_true_sym;
    return li_nil;
    }

LI_HEADER(unload_map)
	{
	g1_destroy_map();
	return li_nil;
	}

li_automatic_add_function(li_unload_map,"Map/Unload");
li_automatic_add_function(li_has_map,"has_map");
