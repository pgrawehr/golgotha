/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/

#ifndef M1_INFO_HH
#define M1_INFO_HH

#include "math/num_type.h"
#include "memory/array.h"
#include "string/string.h"


enum {
	M1_SHOW_AXIS=1,
	M1_WIREFRAME=2,
	M1_SHADING=4,
	M1_SHOW_FACE_NUMBERS=8,
	M1_SHOW_FACE_NAMES=16,
	M1_SHOW_VERT_NUMBERS=32,
	M1_SHOW_ORPHANS=64,
	M1_SHOW_OCTREE_SUBDIVISIONS=128,
	M1_SHOW_ORIGIN=256
};

class m1_poly_object_class;
class r1_render_api_class;
class i4_font_class;

struct m1_info_struct
{
	i4_array<i4_str *> models;
	i4_array<i4_str *> textures;
	i4_array<i4_str *> res_files;
	m1_poly_object_class * obj;
	r1_render_api_class * r_api;
	i4_font_class * r_font;

	i4_array<w32> vertex_flags;

	int current_model;
	int preselect_poly;
	int preselect_point;
	int selected_points[4]; //the last 4 selected points
	int selected_index; //always one more than the last valid entry
	//so 0 means no point selected (use preselect_point then)
	int num_invalid_vertices; //>0 after adding a vertex without connection
	//orphans are not preferred here.
	i4_float time;
	int current_frame;
	int current_animation;
	int current_team;
	w32 bg_color;
	int update_mode;
	w32 tman_index;

	w32 flags;
	w32 get_flags(w32 mask)
	{
		return flags&mask;
	}
	void set_flags(w32 mask, w32 value=0xffffffff)
	{
		flags=(flags&~mask) | (value&mask);
	}

	const i4_const_str current_filename();
	void set_current_filename(const i4_const_str &str);

	i4_bool textures_loaded, recalcing_textures;

	m1_info_struct() :
		models(0,32),
		textures(0,32),
		res_files(0, 32),
		vertex_flags(0,32) {
		flags=0;
		current_model=0;
		textures_loaded=i4_F;
		recalcing_textures=i4_F;
		preselect_poly=-1;
		obj=0;
		time=0;
		current_frame=0;
		current_animation=0;
		current_team=0;
		r_api=0;
		update_mode=0;
		r_font=0;
		selected_index=0;
		tman_index=0xFFFFFFFF;
		num_invalid_vertices=0;
	}

	void get_texture_list(i4_array<i4_str *> &array, i4_bool for_all=i4_T);
	void init(int argc, i4_const_str *argv);
	void uninit();
	void texture_list_changed();
};

extern m1_info_struct m1_info;

#endif
