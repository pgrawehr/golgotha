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


#include "object_definer.h"
#include "lisp/li_class.h"
#include "objs/model_draw.h"
#include "objs/map_piece.h"

class g1_cobra_tank_class :
	public g1_map_piece_class
{
public:
	virtual i4_float occupancy_radius() const
	{
		return draw_params.extent();
	}

	g1_cobra_tank_class(g1_object_type id, g1_loader_class *fp)
		: g1_map_piece_class(id,fp)
	{
		radar_type=G1_RADAR_VEHICLE;
		set_flag(SHADOWED |
				 SELECTABLE |
				 TARGETABLE |
				 GROUND |
				 HIT_GROUND |
				 BLOCKING |
				 DANGEROUS,1);

		draw_params.setup("cobra");
	}


	void think()
	{
		grab_old();

		unoccupy_location();

		x += 0.01f;
		find_target();

		if (occupy_location())
		{
			request_think();
		}
	}
};

void g1_cobra_tank_init()
{

}
static g1_object_definer<g1_cobra_tank_class>
damager_type("cobra_tank_old", g1_object_definition_class::EDITOR_SELECTABLE |
			 g1_object_definition_class::TO_MAP_PIECE|
			 g1_object_definition_class::MOVABLE,g1_cobra_tank_init);
