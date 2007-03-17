/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/

#ifndef TEXT_SCROLL_WIN
#define TEXT_SCROLL_WIN

#include "window/window.h"
#include "font/font.h"
#include "gui/scroll_bar.h"
class i4_graphical_style_class;

class i4_text_scroll_window_class :
	public i4_parent_window_class
{
	i4_color fore,back;
	i4_char *term_out; //the pointer to the actual start of the character data
	i4_char *draw_start; //don't free, it points somewhere within term_out
	w32 term_size, used_size;
	sw32 dx, dy, tdx, tdy;

	sw32 term_height;  // in characters
	sw32 term_width;
	i4_char *term_end; //don't free either, it points to the end of term_out
	sw32 max_scrollback;    // in lines of text
	sw32 curr_scrollback;
	sw32 used_lines;
	i4_scroll_bar *scrollbar;
	sw32 scroll_pos;
	w32 line_height;
	i4_bool need_clear;
	i4_graphical_style_class *style;

public:
	void name(char *buffer)
	{
		static_name(buffer,"text_scroll_window");
	}

	void resize(w16 new_width, w16 new_height);

	i4_text_scroll_window_class(i4_graphical_style_class *style,
								i4_color text_foreground,
								i4_color text_background,
								w16 width, w16 height, //in pixels
								w32 scrollback);      //in lines of text
	~i4_text_scroll_window_class();

	void clear();
	void skip_first_line();

	void scroll_text_up();

	void receive_event(i4_event *ev);

	void output_char(const i4_char &ch);
	void output_string(char *string);
	void printf(char *fmt, ...);

	void parent_draw(i4_draw_context_class &context);

	virtual i4_bool need_redraw();

};



#endif
