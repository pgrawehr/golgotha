/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef I4_SIMPLE_DIALOG_HH
#define I4_SIMPLE_DIALOG_HH


#include "string/string.h"
//#include "window/window.h"
#include "window/colorwin.h"
#include "window/style.h"
class i4_text_input_class;
class i4_event_handler_class;
class i4_event;
class i4_parent_window_class;
class i4_graphical_style_class;

i4_parent_window_class *i4_create_yes_no_dialog(i4_parent_window_class *parent,
                                                i4_graphical_style_class *style,
                                                const i4_const_str &title,
                                                const i4_const_str &message,
                                                const i4_const_str &yes, const i4_const_str &no,
                                                i4_event_handler_class *send_to,
                                                i4_event *yes_event, i4_event *no_event);

//Messagebox-Flags
//not all flags are already implemented
#define MSG_YES 1
#define MSG_NO 2
#define MSG_OK 4
#define MSG_CANCEL 8
#define MSG_IGNORE 16
#define MSG_ABORT 32
#define MSG_LASTBUTTON 32
#define MSG_ALLBUTTONS (MSG_LASTBUTTON*2-1)
#define MSG_YESNO (MSG_YES+MSG_NO)
#define MSG_OKCANCEL (MSG_OK+MSG_CANCEL)
#define MSG_YESNOCANCEL (MSG_YES+MSG_NO+MSG_CANCEL)
#define MSG_MODAL 0
#define MSG_NOTMODAL 256
#define MSG_CHECKBOX 512 /*display a "Show this message again?" checkbox*/
#define MSG_INPUTBOX 1024 /*display an input-box*/
#define MSG_ICONQUESTION 2048
#define MSG_ICONWARNING 4096
#define MSG_ICONCRITICAL 8192




class g1_message_box: public i4_color_window_class
	{
	friend w32 i4_input_box(const i4_const_str &title, const i4_const_str &message, const i4_const_str &preset, i4_str &rettext, w32 flags);

	protected:
	i4_bool modal;
	static w32 modal_result;
	static g1_message_box *dialog_active;
	//i4_event *yes_event,*no_event,*ok_event,*cancel_event,*ignore_event;
	i4_event	*textual_event,*checkbox_event;
	i4_text_input_class *inputwindow;
	i4_color bg_color;
	i4_event_handler_class *send_to;
	i4_graphical_style_class *style;
	w32 flags;
	static i4_str *text;
	public:
	virtual char *name() {return "Message Box class";};
	virtual void receive_event(i4_event *ev);
	g1_message_box(w16 w, w16 h, i4_graphical_style_class *style, 
		//i4_const_str &title, i4_const_str &message, 
		w32 flags=4);
	//g1_message_box(w16 w, w16 h, i4_graphical_style_class *style, 
		//i4_const_str &title, i4_const_str &message, i4_const_str &initial_text, 
	//	w32 flags= 132);
	virtual ~g1_message_box();
	static w32 get_result()
		{
		if (!modal_result) return 0;//not finished -> continue polling
		if (dialog_active)
			dialog_active->style->close_mp_window(dialog_active->parent);
		dialog_active=0;
		return modal_result;
		}

	static i4_const_str get_text()//specifications say that this returns
		//a copy of the object. I hope this is correct.
		{
		if (text) return *text;
		return i4_const_str("");
		}
	virtual void parent_draw(i4_draw_context_class &context)
		{
        local_image->clear(bg_color, context);
		}
	};

w32 i4_input_box(const i4_const_str &title, const i4_const_str &message, const i4_const_str &preset, i4_str &rettext, w32 flags=MSG_INPUTBOX+MSG_OK);

w32 i4_message_box(const i4_const_str &title, const i4_const_str &message, w32 flags=MSG_OK);

w32 i4_message_box(i4_parent_window_class *parent,
                   i4_graphical_style_class *style,
                   const i4_const_str &title,
                   const i4_const_str &message,
                   const i4_const_str &extra,
				   i4_event *finished_event,
				   w32 flags=4);


#endif
