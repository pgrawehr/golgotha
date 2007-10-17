/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/

#ifndef __TEXTITEM_HPP_
#define __TEXTITEM_HPP_

#include "menu/menu.h"
#include "font/font.h"
#include "window/style.h"

class i4_text_item_class :
	public i4_menu_item_class
{
protected:

	i4_color_hint_class * color;
	i4_font_class * font;

	i4_str * text;
	i4_str * check_enable_fn;
	w16 pad_lr;

public:
	w32 bg_color;
	i4_text_item_class(
		const i4_const_str &_text,
		i4_graphical_style_class * style,

		i4_color_hint_class * color_hint=0,
		i4_font_class * font=0,


		i4_event_reaction_class * press=0,
		i4_event_reaction_class * depress=0,
		i4_event_reaction_class * activate=0,
		i4_event_reaction_class * deactivate=0,
		w16 pad_left_right=0,
		w16 pad_up_down=0,
		const char * _check_enable_fn=0
	);
	~i4_text_item_class()
	{
		delete text;
		delete check_enable_fn;
	}

	void name(char * buffer)
	{
		static_name(buffer,"text_item");
	}

	virtual void parent_draw(i4_draw_context_class &context);
	virtual void receive_event(i4_event * ev);
	void change_text(const i4_const_str &new_st);
	i4_const_str get_text()
	{
		return *text;
	}
	void check_enable();

	virtual i4_menu_item_class *copy();
} ;

//this class will load the checkbox-images at load time
//I don't want the images to be loaded on every instantiation of checkboxes
class i4_checkbox_images_class :
	public i4_init_class
{
	friend class i4_checkbox_class;
protected:
	i4_image_class * checkbox;
	i4_image_class * radiobox;
	w32 maxwidth,maxheight;
public:
	i4_checkbox_images_class()
	{
		checkbox=0;
		radiobox=0;
	};
	virtual void init();
	virtual void uninit();
	virtual int init_type()
	{
		return I4_INIT_TYPE_AFTER_ALL;
	}                                                      //must init after image-loaders
};

class i4_checkbox_class :
	public i4_text_item_class
{
protected:
	w32 flags;    //Contains info about type of object (checkbox, radiobutton...)
	w32 state;    //Checked, unchecked or overriden
	i4_checkbox_images_class * images;    //reference to the static images
public:
	enum {
		NONE=0,    //visually same as parent class
		CHECKBOX=1,
		RADIOBUTTON=3,    //Ensures that newer two states are active
		FONT_TOGGLE=7,
		TYPE_MASK=7,
		THREESTATES=8,
		DISABLED=16
	};
	enum {
		UNCHECKED=0,
		CHECKED=1,
		OVERRIDEN=2
	};

	i4_checkbox_class(
		const i4_const_str &_text,
		w32 _flags,
		i4_graphical_style_class * style,

		i4_color_hint_class * color_hint=0,
		i4_font_class * font=0,


		i4_event_reaction_class * press=0,
		i4_event_reaction_class * depress=0,
		i4_event_reaction_class * activate=0,
		i4_event_reaction_class * deactivate=0,
		w16 pad_left_right=0,
		w16 pad_up_down=0
	);
	virtual void name(char * buffer)
	{
		static_name(buffer,"Checkbox");
	};

	virtual i4_bool set_state(w32 newstate)
	{
		if ((state>2) || (((flags&THREESTATES)==0)&&(state>1)))
		{
			return i4_F;
		}               //Invalid state for current setting.
		state=newstate;
		request_redraw();
		return i4_T;
	}
	virtual w32 get_state()
	{
		return state;
	}

	virtual bool getValue() //Vk Compatibility function name
	{
		return (CHECKED==state);
	}

	virtual void set_flags(w32 newflags)
	{
		flags=newflags;
		request_redraw();
	}

	virtual w32 get_flags()
	{
		return flags;
	}
	virtual void parent_draw(i4_draw_context_class &context);
	virtual void toggle_state();
	virtual void receive_event(i4_event * ev);
	virtual i4_menu_item_class *copy()
	{
		return new i4_checkbox_class(*text, flags, hint, color, font,
									 send.press ? send.press->copy() : 0,
									 send.depress ? send.depress->copy() : 0,
									 send.activate ? send.activate->copy() : 0,
									 send.deactivate ? send.deactivate->copy() : 0);
	}

	~i4_checkbox_class()
	{
	};

};

#endif
