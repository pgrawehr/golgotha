/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't helpf, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/
#include "pch.h"
#include "math/num_type.h"
#include "lisp/lisp.h"
#include "lisp/li_init.h"
#include "init/init.h"

i4_float g1_vert_height_table[256] =

{
	0.0f, 0.05f, 0.1f, 0.15f, 0.2f, 0.25f, 0.3f, 0.35f, 0.4f, 0.45f, 0.5f, 0.55f, 0.6f,
	0.65f, 0.7f, 0.75f, 0.8f, 0.85f, 0.9f, 0.95f, 1.0f, 1.05f, 1.1f, 1.15f, 1.2f, 1.25f,
	1.3f, 1.35f, 1.4f, 1.45f, 1.5f, 1.55f, 1.6f, 1.65f, 1.7f, 1.75f, 1.8f, 1.85f, 1.9f,
	1.95f, 2.0f, 2.05f, 2.1f, 2.15f, 2.2f, 2.25f, 2.3f, 2.35f, 2.4f, 2.45f, 2.5f, 2.55f,
	2.6f, 2.65f, 2.7f, 2.75f, 2.8f, 2.85f, 2.9f, 2.95f, 3.0f, 3.05f, 3.1f, 3.15f, 3.2f,
	3.25f, 3.3f, 3.35f, 3.4f, 3.45f, 3.5f, 3.55f, 3.6f, 3.65f, 3.7f, 3.75f, 3.8f,
	3.85f, 3.9f, 3.95f, 4.0f, 4.05f, 4.1f, 4.15f, 4.2f, 4.25f, 4.3f, 4.35f, 4.4f, 4.45f,
	4.5f, 4.55f, 4.6f, 4.65f, 4.7f, 4.75f, 4.8f, 4.85f, 4.9f, 4.95f, 5.0f, 5.05f, 5.1f,
	5.15f, 5.2f, 5.25f, 5.3f, 5.35f, 5.4f, 5.45f, 5.5f, 5.55f, 5.6f, 5.65f, 5.7f,
	5.75f, 5.8f, 5.85f, 5.9f, 5.95f, 6.0f, 6.05f, 6.1f, 6.15f, 6.2f, 6.25f, 6.3f, 6.35f,
	6.4f, 6.45f, 6.5f, 6.55f, 6.6f, 6.65f, 6.7f, 6.75f, 6.8f, 6.85f, 6.9f, 6.95f, 7.0f,
	7.05f, 7.1f, 7.15f, 7.2f, 7.25f, 7.3f, 7.35f, 7.4f, 7.45f, 7.5f, 7.55f, 7.6f,
	7.65f, 7.7f, 7.75f, 7.8f, 7.85f, 7.9f, 7.95f, 8.0f, 8.05f, 8.1f, 8.15f, 8.2f, 8.25f,
	8.3f, 8.35f, 8.4f, 8.45f, 8.5f, 8.55f, 8.6f, 8.65f, 8.7f, 8.75f, 8.8f, 8.85f, 8.9f,
	8.95f, 9.0f, 9.05f, 9.1f, 9.15f, 9.2f, 9.25f, 9.3f, 9.35f, 9.4f, 9.45f, 9.5f, 9.55f,
	9.6f, 9.65f, 9.7f, 9.75f, 9.8f, 9.85f, 9.9f, 9.95f, 10.0f, 10.05f, 10.1f, 10.15f,
	10.2f, 10.25f, 10.3f, 10.35f, 10.4f, 10.45f, 10.5f, 10.55f, 10.6f, 10.65f, 10.7f,
	10.75f, 10.8f, 10.85f, 10.9f, 10.95f, 11.0f, 11.05f, 11.1f, 11.15f, 11.2f, 11.25f,
	11.3f, 11.35f, 11.4f, 11.45f, 11.5f, 11.55f, 11.6f, 11.65f, 11.7f, 11.75f, 11.8f,
	11.85f, 11.9f, 11.95f, 12.0f, 12.05f, 12.1f, 12.15f, 12.2f, 12.25f, 12.3f, 12.35f,
	12.4f, 12.45f, 12.5f, 12.55f, 12.6f, 12.65f, 12.7f, 12.75
};

static i4_bool g_bIsAequidistant=i4_T;

LI_HEADER(restore_default_heights) {
	i4_float val=0.0;
	for (int i=0; i<256; i++)
	{
		g1_vert_height_table[i]=val;
		val+=0.05f;
	}
	g_bIsAequidistant=i4_T;
	return li_true_sym;
}

LI_HEADER(get_height_entry) {
	w32 in=li_get_int(li_eval(li_first(o,env),env),env);
	if (in<0||in>255)
	{
		li_error(env,"USER: The height table has only 256 entries (0-255).");
	}
	return new li_float(g1_vert_height_table[in]);
}

LI_HEADER(set_height_entry) {
	w32 in=li_get_int(li_eval(li_first(o,env),env),env);
	if (in<0||in>255)
	{
		li_error(env,"USER: The height table has only 256 entries (0-255).");
	}
	i4_float val=(i4_float)li_get_float(li_eval(li_second(o,env),env),env);
	g1_vert_height_table[in]=val;
	g_bIsAequidistant=i4_F;
	return li_true_sym;
}

LI_HEADER(scale_heights_by) {
	i4_float by=li_get_float(li_eval(li_first(o,env),env),env);
	for (int i=0; i<255; i++)
	{
		g1_vert_height_table[i]*=by;
	}
	return li_true_sym;
}

LI_HEADER(set_height_aequidistance) {
	i4_float val=0.0;
	i4_float diff=li_get_float(li_eval(li_first(o,env),env),env);
	for (int i=0; i<256; i++)
	{
		g1_vert_height_table[i]=val;
		val+=diff;
	}
	g_bIsAequidistant=i4_T;
	return li_true_sym;
}

LI_HEADER(get_height_aequidistance) {
	i4_float diff=g1_vert_height_table[1]-g1_vert_height_table[0];
	if (g_bIsAequidistant)
	{
		return new li_float(diff);
	}
	else
	{
		double sum=0;
		for (int i=0; i<255; i++)
		{
			sum+=g1_vert_height_table[i+1]-g1_vert_height_table[i];

		}
		return new li_float(sum/255.0);
	}
}

li_automatic_add_function(li_set_height_aequidistance,"set_height_aequidistance");
li_automatic_add_function(li_get_height_aequidistance,"get_height_aequidistance");
li_automatic_add_function(li_scale_heights_by,"scale_heights_by");
li_automatic_add_function(li_restore_default_heights,"map_height_restore");
li_automatic_add_function(li_get_height_entry,"get_height_entry");
li_automatic_add_function(li_set_height_entry,"set_height_entry");
