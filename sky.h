/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/

#ifndef G1_SKY_HH
#define G1_SKY_HH

#include "memory/array.h"
#include "string/string.h"
#include "lisp/li_all.h"
#include "window/window.h"
#include "window/colorwin.h"
#include "render/r1_vert.h"
//#include "lisp/li_types.h"


extern i4_array<i4_str *> g1_sky_list;

class i4_window_class;
class g1_camera_info_struct;
class g1_draw_context_class;
class i4_transform_class;

void g1_draw_sky(i4_window_class * window,
				 g1_camera_info_struct &current_camera,
				 i4_transform_class &transform,
				 g1_draw_context_class * context);

extern li_type_number li_g1_sky_type;
class li_g1_sky :
	public li_object
{
	friend class li_g1_sky_function;
protected:
	li_string * name;
public:
	li_g1_sky(li_string * _name) :
		li_object(li_g1_sky_type),
		name(_name)
	{
	}

	li_g1_sky(const i4_const_str &str) :
		li_object(li_g1_sky_type)
	{
		name=new li_string(str);
	}

	li_g1_sky(i4_const_str &str) :
		li_object(li_g1_sky_type)
	{
		name=new li_string(str);
	}
	li_g1_sky(const char * str) :
		li_object(li_g1_sky_type)
	{
		name=new li_string(str);
	}
	li_string *value()
	{
		return name;
	}
	static li_g1_sky *get(li_object * o, li_environment * env)
	{
		check_type(o,li_g1_sky_type,env);
		return (li_g1_sky *)o;
	}

};




class g1_sky_view :
	public i4_window_class
{
	int offset;
	i4_bool active;
	i4_event_handler_class * eh;
public:
	static int sky_scroll_offset;
	g1_sky_view(w16 w, w16 h, int offset,i4_event_handler_class * handler)
		: i4_window_class(w,h),
		  offset(offset)
	{
		active=i4_F;
		sky_scroll_offset=0;
		eh=handler;
	}
	~g1_sky_view();

	int off()
	{
		return offset+sky_scroll_offset;
	}

	void setup_vert(float x, float y, r1_vert &v)
	{
		v.px=x*(width());
		v.py=y*(height());
		v.v.z=50.0f;
		v.w=1.0f/v.v.z;
		v.s=x;
		v.t=y;
		v.r=v.g=v.b=1.0f;
		v.a=1;
	}

	void draw(i4_draw_context_class &context);

	void receive_event(i4_event * ev);

	void name(char * buffer)
	{
		static_name(buffer,"sky_view");
	}
};

class r1_render_window_class;
class g1_sky_picker_class :
	public i4_color_window_class
{
	i4_graphical_style_class * style;
	i4_array<r1_render_window_class *> render_windows;
	i4_event_handler_class * eh;


public:
	g1_sky_picker_class(i4_graphical_style_class * style,
						i4_event_handler_class * handler=0);
	~g1_sky_picker_class();

	void receive_event(i4_event * ev);
	void name(char * buffer)
	{
		static_name(buffer,"sky picker");
	}
};


#endif
