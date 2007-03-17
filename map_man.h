/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/

#ifndef G1_MAP_MAN_HH
#define G1_MAP_MAN_HH

#include "init/init.h"
#include "memory/array.h"
#include "g1_limits.h"
#include "error/error.h"

class g1_map_class;
class g1_map_cell_class;
class g1_map_vertex_class;

// these are mirrored from the current map, don't
// change them directly
extern g1_map_cell_class *g1_cells;
extern g1_map_vertex_class *g1_verts;
extern int g1_map_width, g1_map_height;
extern int g1_map_width_plus_one;

extern g1_map_class *g1_current_map_PRIVATE;

void g1_set_map(g1_map_class *map);

/*! Returns true if a map is loaded
   	Many commands will fail if this returns i4_F, so be sure to always check this first.
 \return i4_T if map loaded, i4_F otherwise
 */
inline i4_bool g1_map_is_loaded()
{
	if (g1_current_map_PRIVATE)
	{
		return i4_T;
	}
	else
	{
		return i4_F;
	}
}
/*! This function returns the current map
 \return g1_current_map_PRIVATE (the current map)
 */
inline g1_map_class *g1_get_map()
{
//#ifdef _DEBUG
//  if (!g1_current_map_PRIVATE)
//    i4_error("map missing");
//#endif
	return g1_current_map_PRIVATE;
}

void g1_destroy_map();

#endif
