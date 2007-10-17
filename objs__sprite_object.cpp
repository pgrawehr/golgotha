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


#include "objs/sprite_object.h"
#include "render/r1_api.h"

void g1_sprite_object_class::think()
{
	frame++;
	if (frame >= anim_length)
	{
		unoccupy_location();
		request_remove();
	}
	else
	{
		//unoccupy_location();
		request_think();
	}
}


g1_sprite_object_class::g1_sprite_object_class(g1_object_type id,
											   g1_loader_class * fp)
	: g1_object_class(id,fp)
{
	model_id=0;
}


void g1_sprite_object_class::init()
{
	/// get animation length now rather than in the constructor to make sure model_id is valid
	g1_sprite_class * sprite=g1_sprite_list_man.get_sprite(model_id);

	anim_length = sprite->num_animation_frames;
}
