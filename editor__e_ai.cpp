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
 


#include "editor/mode/e_ai.h"
#include "editor/e_state.h"
#include "critical_graph.h"
#include "map.h"
#include "map_man.h"
#include "g1_render.h"
#include "editor/contedit.h"
#include "editor/editor.h"
#include "gui/butbox.h"
#include "render/r1_clip.h"

//#include "editor/mode/e_game.h"

g1_ai_params g1_e_ai;

g1_mode_handler::state g1_ai_mode::current_state()
{
  w8 remap[]={ ROTATE,
               ZOOM,
               DRAG_SELECT,
               DRAG_SELECT,
               OTHER,
               OTHER
  };

  //I4_ASSERT(g1_edit_state.ai.minor_mode<=sizeof(remap), "state too big");

  return (g1_mode_handler::state)remap[g1_e_ai.get_minor_mode()];
}


g1_ai_mode::g1_ai_mode(g1_controller_edit_class *c)
  : g1_mode_handler(c)
{
  sel_color=0xffff00;
  norm_color=0x7f7f7f;
}


void g1_ai_mode::post_draw(i4_draw_context_class &context)
{
  g1_map_class *map=g1_get_map();
  if (map)
  {
    g1_critical_graph_class *graph=map->get_critical_graph();
    if (graph)
    {
      for (int i=1; i<graph->criticals; i++)
      {
        float x = graph->critical[i].x, y=graph->critical[i].y, z;
        x = (float)((int)x)+0.5f;
        y = (float)((int)y)+0.5f;

        z=map->terrain_height(x,y)+0.01f;
        
        r1_vert rv;
        int w1=2,w2=1;
        if (g1_render.project_point(i4_3d_point_class(x,y,z), 
                                    rv, c->g1_context.transform))
        {          
          w32 color;
          if (graph->critical[i].selected)
            color=sel_color;
          else
            color=norm_color;

          
          r1_clip_clear_area((sw32)rv.px-w1, (sw32)rv.py-w1, 
                             (sw32)rv.px+w1, (sw32)rv.py+w1, 
                             0, 0.02f, *c->g1_context.context, g1_render.r_api);

          r1_clip_clear_area((sw32)rv.px-w2, (sw32)rv.py-w2, 
                             (sw32)rv.px+w2, (sw32)rv.py+w2, 
                             color, 0.01f, *c->g1_context.context, g1_render.r_api);
        }
      }
    }
  }

  g1_mode_handler::post_draw(context);
}

g1_critical_graph_class *g1_ai_mode::get_graph()
{
  g1_map_class *map=g1_get_map();
  if (map)
    return map->get_critical_graph();  
  return 0;
}

void g1_ai_mode::mouse_down()
{
  if (g1_e_ai.get_minor_mode()==g1_ai_params::CREATE)
  {
    g1_critical_graph_class *graph=get_graph();
    i4_float gx,gy, dx,dy;
    if (!c->view_to_game(lx(), ly(), gx,gy, dx,dy))
      return;

    g1_map_class *map=g1_get_map();
    g1_editor_instance.add_undo(G1_MAP_CRITICAL_POINTS);
    if (map) map->mark_for_recalc(G1_RECALC_CRITICAL_DATA);
    graph->add_critical_point(gx, gy);
  }
  else g1_mode_handler::mouse_down();
}



i4_bool g1_ai_mode::select_object(sw32 mx, sw32 my, 
                                  i4_float &ox, i4_float &oy, i4_float &oz,
                                  select_modifier mod)
{
  i4_bool change=i4_F;

 
  no_more_move_undos=i4_F;

  g1_critical_graph_class *graph=get_graph();
  if (!graph) return i4_F;
  g1_map_class *map=g1_get_map();

  if (mod!=FOR_CURSOR_HINT)
    g1_editor_instance.add_undo(G1_MAP_CRITICAL_POINTS);

  i4_bool ret=i4_F;
  int t=graph->criticals;
  for (int i=1; i<t && !ret; i++)
  {
    g1_critical_graph_class::critical_point_class *p=graph->critical+i;
    float x=((int)p->x)+0.5f, y=((int)p->y)+0.5f, z;
    z=map->terrain_height(x,y)+0.01f;

    r1_vert rv;
    if (g1_render.project_point(i4_3d_point_class(x,y,z), rv, c->g1_context.transform))
    {
      if (abs((sw32)rv.px-mx)<3 && abs((sw32)rv.py-my)<3)
      {
        ox=x;  oy=y;  oz=z;

        if (p->selected==0 && mod==CLEAR_OLD_IF_NO_SELECTION)
          for (int j=1; j<t; j++)
            graph->critical[j].selected=0;

        if (mod==CLEAR_OLD_IF_NO_SELECTION || mod==ADD_TO_OLD)
        {
          p->selected=1;
          change=i4_T;
        }
        else if (mod==SUB_FROM_OLD)
        {
          p->selected=0;
          change=i4_T;
        }

        ret=i4_T;        
      }
    }
  }

  if (change)
  {
    c->changed();
    c->refresh();
  }


  return ret;
}

void g1_ai_mode::select_objects_in_area(sw32 x1, sw32 y1, sw32 x2, sw32 y2, 
                                            select_modifier mod)
{
  no_more_move_undos=i4_F;


  g1_critical_graph_class *graph=get_graph();
  if (!graph) return;
  g1_map_class *map=g1_get_map();

  g1_editor_instance.add_undo(G1_MAP_CRITICAL_POINTS);

  int t=graph->criticals;
  for (int i=1; i<t; i++) 
  {
    g1_critical_graph_class::critical_point_class *p=graph->critical+i;
    float x=p->x, y=p->y, z;
    z=map->terrain_height(x,y)+0.01f;

    r1_vert rv;
    if (g1_render.project_point(i4_3d_point_class(x,y,z), rv, c->g1_context.transform))
    {
      if (rv.px>=x1 && rv.px<=x2 && rv.py>=y1 && rv.py<=y2)
      {
        if (mod==SUB_FROM_OLD)
          p->selected=0;
        else
          p->selected=1;
      }
      else if (mod==CLEAR_OLD_IF_NO_SELECTION)
        p->selected=0;
    }
  }
}

void g1_ai_mode::move_selected(i4_float xc, i4_float yc, i4_float zc, 
                               sw32 mouse_x, sw32 mouse_y)
{
  g1_critical_graph_class *graph=get_graph();
  if (!graph) return;
  g1_map_class *map=g1_get_map();

  g1_editor_instance.add_undo(G1_MAP_CRITICAL_POINTS);
  map->mark_for_recalc(G1_RECALC_CRITICAL_DATA);
  
  int t=graph->criticals;
  for (int i=1; i<t; i++) 
  {
    g1_critical_graph_class::critical_point_class *p=graph->critical+i;
    if (p->selected)
    {
      p->x+=xc;  if (p->x<0) p->x=0; else if (p->x>=map->width()) p->x=map->width()-0.1f;
      p->y+=yc;  if (p->y<0) p->y=0; else if (p->y>=map->height()) p->y=map->height()-0.1f;
    }
  } 
}

void g1_ai_mode::delete_selected()
{
  g1_critical_graph_class *graph=get_graph();
  if (!graph) return;
  g1_map_class *map=g1_get_map();

  g1_editor_instance.add_undo(G1_MAP_CRITICAL_POINTS);
  map->mark_for_recalc(G1_RECALC_CRITICAL_DATA);
  
  for (int i=1; i<graph->criticals; i++) 
  {
    if (graph->critical[i].selected)
    {
      for (int j=i;j<graph->criticals-1; j++)
        graph->critical[j]=graph->critical[j+1];
      graph->criticals--;
    }
  } 
}




void g1_ai_params::create_buttons(i4_parent_window_class *container)
{
  i4_button_box_class *box=new i4_button_box_class(&g1_edit_state);
  char *rn[]={"aROTATE", "aZOOM", "aMOVE", "aSELECT", "aCREATE", 0 };
  w32 i=ROTATE;


  for (char **a=rn; *a; i++, a++)
    g1_edit_state.add_but(box, *a, 0, (i4_bool) i==get_minor_mode(),
                          new g1_set_minor_mode_event("AI",i));

  box->arrange_right_down();

  i4_button_class *create=g1_edit_state.create_button("aRECALC", RECALC, i4_T);

  i4_graphical_style_class *s=get_style();
  //i4_color_window_class *cw=new i4_color_window_class(box->width(), 
  //                                                    box->height()+create->height(),
  //                                                    s->color_hint->window.passive.medium,
  //                                                    s);
  container->add_child(0, 0, box);
  container->add_child(0, box->height(), create);

}




i4_bool g1_ai_params::set_minor_mode(w8 m) 
{ 
  if (m<=CREATE)
    minor_mode=(minor_mode_type)m; 
  if (m==RECALC)
  {
    g1_map_class *map=g1_get_map();
    if (map)
    {
      map->mark_for_recalc(0xffffffff);
      map->recalc_static_stuff();
    }
  }
  return i4_T;
}

//editor/mode/e_game.cpp
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
 

/* 
//What's the purpose of this code? it doesn't seem to be sensefull
g1_mode_handler::state g1_game_mode::current_state()
{
  w8 remap[]={ ROTATE,
               ZOOM,
               DRAG_SELECT,
               DRAG_SELECT,
               OTHER,
               OTHER
  };

  //I4_ASSERT(g1_edit_state.ai.minor_mode<=sizeof(remap), "state too big");

  return (g1_mode_handler::state)remap[g1_edit_state.get_minor_mode()];
}


g1_game_mode::g1_game_mode(g1_controller_edit_class *c)
  : g1_mode_handler(c)
{
}
*/

