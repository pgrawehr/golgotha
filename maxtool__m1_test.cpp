/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/
#include "pch.h"
#include "maxtool/m1_info.h"
#include "maxtool/st_edit.h"
#include "maxtool/render2.h"
#include "lisp/lisp.h"
#include "lisp/li_init.h"
#include "maxtool/max_object.h"
#include "gui/smp_dial.h"

#if 0
li_object *m1_do(li_object *o, li_environment *env)
{
  char buff[1024], *p=buff;

  printf("> ");
  fgets(buff, sizeof(buff), stdin);
  lip(li_eval(li_get_expression(p,0),0));
}
li_automatic_add_function(m1_do, "command");

#endif

li_object *m1_distribute_selected(li_object *o, li_environment *env)
{
  int i;
  int num_sel=0;
  i4_float x=0,nx;

  for (i=0; i<m1_info.obj->num_quad; i++)
    if (m1_info.obj->quad[i].get_flags(g1_quad_class::SELECTED))
      num_sel++;

  if (num_sel>1)
	  {
	  w32 ans=i4_message_box("Multiple faces selected","Distribute texture over all faces (yes) or just set default texture params (no)?",MSG_YES+MSG_NO+MSG_CANCEL);
	  if (ans==MSG_CANCEL)
		  return 0;
	  if (ans==MSG_NO)
		  {
		  num_sel=1;
		  }
	  }

  for (i=0; i<m1_info.obj->num_quad; i++)
  {
    g1_quad_class *q = &m1_info.obj->quad[i];
    if (q->get_flags(g1_quad_class::SELECTED))
    {
      nx = x+1.0f/num_sel;
      q->u[0] = x;
      q->v[0] = 0;
      q->u[1] = nx;
      q->v[1] = 0;
      q->u[2] = nx;
      q->v[2] = 1;
      q->u[3] = x;
      q->v[3] = 1;
	  if (num_sel!=1)
		x = nx;//increase only if really distributing
    }
  }
  m1_render_window->request_redraw();

  return 0;
}
li_automatic_add_function(m1_distribute_selected, "distribute_sel");

li_object *m1_swap_selected(li_object *o, li_environment *env)
{
  int i;
  int num_sel=0;
  g1_quad_class *first=0;

  i4_float u[4],v[4],t;

  for (i=0; i<m1_info.obj->num_quad; i++)
  {
    g1_quad_class *q = &m1_info.obj->quad[i];
    if (q->get_flags(g1_quad_class::SELECTED))
    {
      if (!first)
        first = q;

      for (int j=0; j<4; j++)
      {
        t = q->u[j];
        q->u[j] = u[j];
        u[j] = t;
        t = q->v[j];
        q->v[j] = v[j];
        v[j] = t;
      }
    }
  }

  if (first)
    for (int j=0; j<4; j++)
    {
      t = first->u[j];
      first->u[j] = u[j];
      u[j] = t;
      t = first->v[j];
      first->v[j] = v[j];
      v[j] = t;
    }

  m1_render_window->request_redraw();

  return 0;
}
li_automatic_add_function(m1_swap_selected, "swap_sel");

li_object *m1_add_pan_selected(li_object *o, li_environment *env)
{
  int i;//,num_sel;
  m1_poly_object_class *obj = m1_info.obj;

  if (!obj)
    return 0;
  
  for (i=0; i<obj->num_quad; i++)
  {
    g1_quad_class *q = &obj->quad[i];
    if (q->get_flags(g1_quad_class::SELECTED))
    {
      int sp = obj->add_special(i);
      g1_texture_animation *t = &obj->special[sp];
      
      t->quad_number = i;
      t->max_frames = 0;
	  t->frames_x=1;//must not be 0 in any circumstance (causes floating point exception)
      t->speed = 0.01f;
      t->du = 0.0;
      t->dv = 0.0;
      for (w32 n=0; n<q->num_verts(); n++)
      {
        t->u[n] = q->u[n];
        t->v[n] = q->v[n];
      }
    }
  }
  li_call("update_edit_special");
  return 0;
}

li_automatic_add_function(m1_add_pan_selected, "pan_texture_sel");

li_object *m1_add_anim_selected(li_object *o, li_environment *env)
{//adds a texture animation to currently selected quads
  int i;//,num_sel;
  m1_poly_object_class *obj = m1_info.obj;

  if (!obj)
    return 0;
  
  for (i=0; i<obj->num_quad; i++)
  {
    g1_quad_class *q = &obj->quad[i];
    if (q->get_flags(g1_quad_class::SELECTED))
    {
      int sp = obj->add_special(i);
      g1_texture_animation *t = &obj->special[sp];
      
      t->quad_number = i;
      t->max_frames = 1;
      t->frames_x = 1;
      t->speed = 1;
      t->du = 1.0;
      t->dv = 1.0;
      for (w32 n=0; n<q->num_verts(); n++)
      {
        t->u[n] = q->u[n];
        t->v[n] = q->v[n];
      }

      obj->num_special++;
    }
  }
  li_call("update_edit_special");
  return 0;
}
li_automatic_add_function(m1_add_anim_selected, "animate_texture_sel");

li_object *m1_create_plane(li_object *o, li_environment *env)
{
  //Creates a new object (a single square with white texture)
  m1_poly_object_class *obj = new m1_poly_object_class;

  if (obj)
  {
    obj->init(); 
    
    if (m1_info.obj)
      delete m1_info.obj;

    m1_info.obj = obj;
    m1_info.set_current_filename("Untitled.gmod");
  }
  m1_info.current_animation=m1_info.current_frame=0;
  if (!obj)
    return 0;

  obj->num_vertex = 4;
  obj->vert_store.add_many(obj->num_vertex);
  obj->vert_store[0].v.set(0,0,0);
  obj->vert_store[1].v.set(1,0,0);
  obj->vert_store[2].v.set(1,1,0);
  obj->vert_store[3].v.set(0,1,0);

  obj->anim_store.add();
  obj->anim_store[0].num_frames=1;
  obj->anim_store[0].vertex = &obj->vert_store[0];
  obj->animation_names.add(new i4_str("Default"));

  obj->num_animations = obj->anim_store.size();
  obj->animation = &obj->anim_store[0];

  obj->quad_store.add();
  obj->quad_store[0].set(0,1,2,3);
  obj->quad_store[0].flags=0;
  obj->quad_store[0].set_material(0);
  obj->quad_store[0].texture_scale = 1.0;
  obj->quad_store[0].u[0] = 0.0;
  obj->quad_store[0].v[0] = 0.0;
  obj->quad_store[0].u[1] = 1.0;
  obj->quad_store[0].v[1] = 0.0;
  obj->quad_store[0].u[2] = 1.0;
  obj->quad_store[0].v[2] = 1.0;
  obj->quad_store[0].u[3] = 0.0;
  obj->quad_store[0].v[3] = 1.0;
  obj->texture_names.add(new i4_str("textures/solid_white.jpg"));

  obj->num_quad = obj->quad_store.size();
  obj->quad = &obj->quad_store[0];

  obj->calc_vert_normals();
  obj->calc_extents();

  m1_info.texture_list_changed();
  
  return 0;
}
li_automatic_add_function(m1_create_plane, "create_plane");

