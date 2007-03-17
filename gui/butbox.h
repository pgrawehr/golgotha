/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/

#ifndef __BUTBOX_HPP_
#define __BUTBOX_HPP_

#include "window/window.h"
#include "gui/button.h"
#include "menu/textitem.h"

class i4_button_box_class :
	public i4_menu_item_parent_class
{
protected:
	i4_event_handler_class *receiver;
	i4_menu_item_class *current_down;
	i4_bool require_one_down;

	void expand_if_needed();

	// I made this private so you have to call add_button
	virtual void add_child(i4_coord x, i4_coord y, i4_window_class *child);

public:
	void name(char *buffer)
	{
		static_name(buffer,"button_box");
	}

	i4_button_box_class(i4_event_handler_class *receiver,
						i4_bool require_one_down=i4_T);

	void parent_draw(i4_draw_context_class &context);
	virtual i4_menu_item_class *get_current_down()
	{
		return current_down;
	};
	virtual void add_button(i4_coord x, i4_coord y, i4_button_class *child);

	virtual void note_reaction_sent(i4_menu_item_class *who,     // this is who sent it
									i4_event_reaction_class *ev, // who it was to
									i4_menu_item_class::reaction_type type);


	virtual void push_button(i4_button_class *which, i4_bool send_event);

	// arranges child windows from left to right then down, also enlarges self to
	// fit this arrangement order
	virtual void arrange_right_down();

	// arranges child windows from top to bottom then right, also enlarges self to
	// fit this arrangement order
	virtual void arrange_down_right();
} ;

//Warning: Only add checkboxes to the group. Will behave unpredictably otherwise.
class i4_button_group_class :
	public i4_button_box_class
{
protected:
	w32 flags;
	i4_const_str text;
public:
	enum {
		NOTEXT=256,
		NOBORDER=512
	};
	virtual void parent_draw(i4_draw_context_class &context);
	//virtual void receive_event(i4_event *ev);
	i4_button_group_class(i4_const_str &text,
						  w16 width,w16 height,
						  i4_event_handler_class *receiver, w32 flags);
	virtual void note_reaction_sent(i4_menu_item_class *who,       // this is who sent it
									i4_event_reaction_class *ev, // who it was to
									i4_menu_item_class::reaction_type type);

	//virtual void add_button(i4_coord x, i4_coord y, i4_button_class *child);
	virtual void push_button(i4_button_class *which,i4_bool send_event);
	//virtual void arrange_right_down();
	//virtual void arrange_down_right();
	void name(char *buffer)
	{
		static_name(buffer,"i4_button_group_class");
	};
};


#endif
