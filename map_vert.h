/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_MAP_VERTEX_HH
#define G1_MAP_VERTEX_HH


#include "arch.h"
#include "f_tables.h"
#include "math/vector.h"
#include "math/transform.h"
#include "render/r1_clip.h"
#include "render/r1_api.h"
#include "map_man.h"

class i4_file_class;
class i4_saver_class;
class g1_cloud_class;
class i4_saver_class;
class g1_lod_context_class;

extern i4_float g1_vert_height_table[256];

class g1_map_vertex_class
{
	friend g1_map_vertex_class *g1_vertex_min(g1_map_vertex_class *v1,g1_map_vertex_class *v2);
	friend g1_map_class;
	friend g1_cloud_class; //clouds have a special way of creating shadows
	friend void g1_save_map_verts(g1_map_vertex_class *list, 
		int lsize, 
		i4_saver_class *fp,
		int mark_sections);
	friend i4_bool g1_load_map_verts(g1_map_vertex_class *list, int lsize, 
		i4_loader_class *fp,
		int goto_sections);
	friend g1_lod_context_class;
public:
	enum { 
		SELECTED            = (1<<0),            //< only used by editor
		FOGGED              = (1<<1),            //< fog of war

		TRANSFORMED         = (1<<2),
		PROJECTED           = (1<<3),
		CLIP_CODE_CALCULATED= (1<<4),
		W_CALCULATED        = (1<<5),

		NEED_UNDO_SAVE      = (1<<6),            //< only used by editor
		WAS_DRAWN_LAST_FRAME= (1<<7),            //< only used by editor
		APPLY_WAVE_FUNCTION = (1<<8),            //< if vert is part of water

		T_INTERSECTION      = (1<<9),            //< used to determine that this is a T intersection
	};

	enum {SAVED_FLAGS = SELECTED | FOGGED};
  //! view space transformed coordinates
  i4_3d_vector v;        
  //! projected screen x (any screen drawing function requires only these)
  i4_float px;
  //! projected screen y
  i4_float py;
  //! w=1/z
  i4_float w;            

private:
  //! 8-8-8 lighting values for r,g,b, dynamic light cannot be recalculated
  //! dynamic light is assummed to come from straight down, this value is changed by lightbulb objects
  //! but is normally 0
  w32 dynamic_light;

  //! sum of dynamic, static, and global light values packed into 8-8-8, top bit indicates
  //! need to recalculate.  Set need-to-recalc if dynamic light or normal changes
  w32 light_sum;

  //! 5-5-5,  x,y,z, top bit indicates needs recalc
  w16 normal;            


  w16 flags;  

  //! amount of light (0..255) visible from the directional light (in the direction of the
  //! global directional light) - cannot be calculated in game because it requires
  //! ray-tracing and radiocity (not implemented yet)
  w8  static_intensity;  

  //! 0-256 shadow subtraction for clouds
  w8 shadow_subtract; 

  //! Height above ground.
  //! This field really indicates the geographical height of a vertex. It is an entry in the
  //! g1_vert_height_table table, whose increment is 0.05 by default (but can be changed on
  //! a per level basis)
  w8 height;             

  //! Flags used for clipping. 
  w8 clip_code;

  //! Height with t-intersection adjustment
  i4_float t_height;                         


public:

  w16 get_flag(w16 f) { return (flags & f); }
  void set_flag(w16 f, int on_off=1) 
  { 
    if (on_off) 
		flags|=f;
    else 
		flags&=~f;
  }

  w8 is_transformed() { return (w8)get_flag(TRANSFORMED); }
  void set_is_transformed(w8 yes_no) { set_flag(TRANSFORMED, yes_no); }
  
  w8 is_projected() { return (w8)get_flag(PROJECTED); }
  void set_is_projected(w8 yes_no) { set_flag(PROJECTED, yes_no); }

  w8 is_selected() { return (w8)get_flag(SELECTED); }
  void set_is_selected(w8 yes_no) { set_flag(SELECTED, yes_no); }

  w8 need_undo() { return (w8)get_flag(NEED_UNDO_SAVE); }
  void set_need_undo(w8 yes_no) { set_flag(NEED_UNDO_SAVE, yes_no); }

  w8 is_clipped() { return (w8)get_flag(CLIP_CODE_CALCULATED); }
  void set_is_clipped(w8 yes_no) { set_flag(CLIP_CODE_CALCULATED, yes_no); }

  w8 is_w_calculated() { return (w8)get_flag(W_CALCULATED); }
  void set_is_w_calculated(w8 yes_no) { set_flag(W_CALCULATED, yes_no); }

  void clear_calculations()
  {
    flags &= ~(PROJECTED | TRANSFORMED | CLIP_CODE_CALCULATED | W_CALCULATED | T_INTERSECTION);
  }
  

  float get_non_dynamic_light_intensity(int cvx, int cvy);
private:
  void recalc_normal(int cvx, int cvy);
public:
  w32 recalc_light_sum(int cvx, int cvy);

  template<typename T>
  void increment_height(T amount)
  {
	  height+=(w8)amount;
	  light_sum |= 0x80000000;
	  normal=0x8000;
  }
  template<typename T>
  void set_height(T new_height)
  {
	  height=(w8)new_height;
	  light_sum |= 0x80000000;
	  normal=0x8000;
  }
  template<typename T>
  void decrement_height(T amount)
  {
	  height-=(w8) amount;
	  light_sum |= 0x80000000;
	  normal=0x8000;
  }
  w8 get_height_value()
  {
	  return height;
  }

  void set_dynamic_light(w32 new_light_color)
  {
	  dynamic_light=new_light_color;
	  light_sum |= 0x80000000;
  }
  void increment_dynamic_light(w32 extra_color)
  {
	  dynamic_light+=extra_color;
	  light_sum |= 0x80000000;
  }

  void decrement_dynamic_light(w32 extra_color)
  {
	  dynamic_light-=extra_color;
	  light_sum |= 0x80000000;
  }
  w32 get_dynamic_light()
  {
	  return dynamic_light;
  }

  w32 get_light_sum()
  {
	  return light_sum;
  }

  void set_static_intensity(w8 intensity)
  {
	  static_intensity=intensity;
  }

  void get_normal(i4_3d_vector &v, int cvx, int cvy)
  {
    if (normal & 0x8000)
      recalc_normal(cvx, cvy);

    v.x=g1_table_0_31_to_n1_1[(normal>>10)&31];
    v.y=g1_table_0_31_to_n1_1[(normal>>5)&31];
    v.z=g1_table_0_31_to_n1_1[(normal)&31];
  }


  void get_rgb(i4_float &r, i4_float &g, i4_float &b, int cvx, int cvy)
  {
    if (light_sum & 0x80000000)
      recalc_light_sum(cvx, cvy);
    w32 ls=light_sum;
    r=g1_table_0_255_to_0_1[((ls>>16)&0xff)];
    g=g1_table_0_255_to_0_1[((ls>>8)&0xff)];
    b=g1_table_0_255_to_0_1[((ls)&0xff)];
	r=1;
	g=1;
	b=1;
  }
  
  i4_float get_height() 
  { 
	  return g1_vert_height_table[height]; 
  }

  void wave_transform(i4_transform_class &t, float map_x, float map_y);

  void transform(i4_transform_class &t, int map_x, int map_y,float &xscale, float &yscale)
  {
    if (!is_transformed())
    {      
      if (flags & APPLY_WAVE_FUNCTION)
        wave_transform(t, (float)map_x, (float)map_y);
      else
        t.transform(i4_3d_point_class((float)map_x, (float)map_y, t_height), v);

      v.x *= xscale;
      v.y *= yscale;

      set_is_transformed(i4_T);
    }
  }

  void calculate_w()
  {
    w=r1_ooz(v.z);
    set_is_w_calculated(i4_T);
  }

  void project(i4_float win_center_x, i4_float win_center_y)
  {
    if (!is_projected())
    {
      if (!is_w_calculated())
        calculate_w();

      px=v.x * w * win_center_x + win_center_x;
      py=v.y * w * win_center_y + win_center_y;
      set_is_projected(i4_T);
    }
  }

  w8 calc_clip_code()
  {
    if (!is_clipped())
    {
      clip_code = r1_calc_outcode(v);

      set_is_clipped(1);
    }
    return clip_code;
  }

  void set_r1_vert(r1_vert *r)
  {
    r->v.x = v.x;
    r->v.y = v.y;
    r->v.z = v.z;
    r->px = px;
    r->py = py;
    r->w = w;
    r->outcode = clip_code;
  }

  void load_v1(i4_file_class *fp);
  void load_v2(i4_file_class *fp);
  void load_v4(i4_file_class *fp);

  void init();

};


inline g1_map_vertex_class *g1_vertex_min(g1_map_vertex_class *v1,g1_map_vertex_class *v2)
{
  if (v1->height<v2->height)
    return v1;
  else return v2;
}

i4_bool g1_load_map_verts(g1_map_vertex_class *list, int lsize, 
                          i4_loader_class *fp, int goto_sections);

void g1_save_map_verts(g1_map_vertex_class *list, int lsize, 
                       i4_saver_class *fp,  int mark_section);

inline g1_map_vertex_class *g1_get_vertex(int x, int y)
{
  return g1_verts + y*(g1_map_width+1) + x;
}

#endif




