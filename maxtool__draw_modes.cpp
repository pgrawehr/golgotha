/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/
#include "pch.h"
#include "lisp/li_init.h"
#include "lisp/lisp.h"
#include "g1_render.h"
#include "render/r1_api.h"
#include "render/tmanage.h"

li_object *g1_wireframe(li_object *o, li_environment *env)
{
	g1_render.draw_mode=g1_render_class::WIREFRAME;
	li_call("redraw");
	return 0;
}

li_object *g1_textured(li_object *o, li_environment *env)
{
	g1_render.draw_mode=g1_render_class::TEXTURED;
	li_call("redraw");
	return 0;
}

li_object *g1_solid_color(li_object *o, li_environment *env)
{
	g1_render.draw_mode=g1_render_class::SOLID;
	li_call("redraw");
	return 0;
}



li_object *g1_toggle_texture_loading(li_object *o, li_environment *env)
{
	g1_render.r_api->get_tmanager()->toggle_texture_loading();

	return 0;
}

li_automatic_add_function(g1_toggle_texture_loading, "View/Toggle Texture Loading");
li_automatic_add_function(g1_wireframe, "View/Wireframe");
li_automatic_add_function(g1_textured, "View/Textured");
li_automatic_add_function(g1_solid_color, "View/Solid");
