/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/

#ifndef TILE_HH
#define TILE_HH

#include "init/init.h"
#include "render/tex_id.h"
#include "memory/hashtable.h"

class i4_const_str;
class g1_object_class;
class g1_quad_object_class;
class r1_render_api_class;
class g1_map_class;
class li_object;
class li_environment;
class g1_map_cell_class;

class g1_tile_class
{
public:
	r1_texture_handle texture;
	w32 filename_checksum;         // used to identify matchup tiles with saved levels
	w8 flags;
	w16 selection_order;
	//w32 alternate_checksum;

	enum {
		SELECTABLE        =1, // if user can pick it for tile selection
		WAVE              =2, // water-wave
		BLOCKING          =4  // blocks vehicle entry
	};

	float damping_fraction, damping_e, friction_fraction;
	sw16 damage;

	void init();  // initializes default values for the tile

	void set_friction(float uB);

	void get_properties(li_object * properties, li_environment * env);

	void apply_to_cell(g1_map_cell_class &cell);
} ;



class g1_tile_man_class :
	public i4_init_class
{
private:
	i4_array<g1_tile_class> array;
	int sorted_by_checksum;
	r1_texture_handle pink, default_tile_type;
	i4_hashtable<w32> original_checksums;
	i4_array<w16> select_remap;

public:
	int get_tile_from_name(i4_const_str &name);
	int get_tile_from_name(char * name);
	int get_tile_from_checksum(w32 checksum);
	i4_const_str *get_name_from_tile(w32 tilehandle); //you must delete the return string yourself!
	w32 get_original_checksum(w32 newchecksum); //used if save_name used in tile definition
	int get_tile_from_alternate_checksum(w32 checksum);
	void store_alternate_checksum(w32 nchk,w32 oldchk);

	g1_tile_man_class();

	void uninit();
	void init();

	void reset(int _max_tiles);
	bool add(li_object * o, li_environment * env);
	bool add_new(li_object * o, li_environment * env);
	//This function is not used (doesn't even have a body)
	//void get_tile_texture(li_object *o, li_object *&texture_name, li_object *&save_name);

	g1_tile_class *get(w32 handle)
	{
		return &(array[handle]);
	}
	r1_texture_handle get_texture(w32 handle)
	{
		return array[handle].texture;
	}

	r1_texture_handle get_pink()
	{
		return pink;
	}
	int get_default_tile_type()
	{
		return default_tile_type;
	}


	void finished_load();
	int remap_size();
	int get_remap(int tile_num)
	{
		return select_remap[tile_num];
	}

	w32 total()
	{
		return array.size();
	}
};


extern g1_tile_man_class g1_tile_man;


#endif
