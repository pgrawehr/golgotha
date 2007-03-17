/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/

#ifndef __TEXT_HH_
#define __TEXT_HH_

#include "window/window.h"
#include "string/string.h"
#include "window/style.h"

class i4_text_window_class :
	public i4_window_class
{
	i4_str *text;
	i4_graphical_style_class *hint;
	i4_font_class *font;
	int wrap_width;
	int numlines;

public:
	i4_text_window_class(const i4_const_str &_text,
						 i4_graphical_style_class *hint,
						 i4_font_class *_font=0)
		: i4_window_class(0, 0),
		  hint(hint),
		  text(new i4_str(_text)),
		  font(_font),
		  wrap_width(0),
		  numlines(1)
	{
		if (!font)
		{
			font=hint->font_hint->normal_font;
		}

		resize(font->width(_text), font->height(_text));
	}

	i4_text_window_class(const i4_const_str &_text,
						 i4_graphical_style_class *hint,
						 i4_font_class *_font,int _wrap_width) :
		i4_window_class(0,0),
		hint(hint),
		text(new i4_str(_text)),
		font(_font),
		wrap_width(_wrap_width),
		numlines(1)
	{
		if (!font)
		{
			font=hint->font_hint->normal_font;
		}
		wrap();
		special_resize();
	}

	void wrap()
	{
		if (wrap_width==0)
		{
			return;
		}
		numlines=1;
		w32 index=0;
		int xpos=0;
		//i4_str break_chars(" \t")
		int breakpos=10; //wrapping at 0 would yield to trouble.
		//we declare that a line should at least be breakpos characters long
		while (index<text->length())
		{
			i4_char c=text->operator [](index);
			if (xpos+font->width(c)>wrap_width)
			{
				//wrap at breakpos
				i4_str::iterator it=text->begin();
				it+=breakpos;
				text->insert(it,'\n');
				xpos=0;
				index=breakpos+1;
				breakpos+=10;
				numlines++;
			}
			else
			{
				if (c==' ')
				{
					breakpos=index+1;
					xpos+=font->width(c);
					index++;
				}
				else if (c=='\n')
				{
					numlines++;
					breakpos=index+10;
					//index=breakpos;
					index++;
					xpos=0;
				}
				else
				{
					xpos+=font->width(c);
					index++;
				}
			}
		}
	}
	void special_resize()
	{
		if (wrap_width==0)
		{
			resize(font->width(*text),font->height(*text));
		}
		else
		{
			resize(wrap_width,font->height(*text)*numlines);
		}
	}

	void resize_to_fit_text()
	{
		special_resize();
	}

	virtual void draw(i4_draw_context_class &context)
	{
		local_image->add_dirty(0,0,width()-1,height()-1,context);
		font->set_color(hint->color_hint->text_foreground);

		hint->deco_neutral_fill(local_image, 0,0, width()-1, height()-1, context);
		//    local_image->clear(hint->color_hint->window.passive.medium,context);
		if (text)
		{
			font->put_string(local_image,0,0,*text,context);
		}
	}

	~i4_text_window_class()
	{
		delete text;
	}

	void set_text(i4_str *new_text)
	{
		if (text)
		{
			delete text;
		}
		text = new_text;
		wrap();
		request_redraw();
	}

	i4_str *get_text()
	{
		return text;
	}

	void name(char *buffer)
	{
		static_name(buffer,"text_window");
	}
};


#endif
