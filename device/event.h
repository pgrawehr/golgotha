/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/

#ifndef __EVENT_HPP_
#define __EVENT_HPP_

#include "arch.h"
#include "time/time.h"
#include <stdlib.h>


/*! User Interface event base class.

   All events should be derived from i4_event.
   All events should have a type() and copy() function defined.  Since all user application events
   should be be OBJECT_MESSAGE or USER_MESSAGE type does not need to be defined.

   An object_message is usefull for scoping object messages to yourself specifically.  Each object
   can then have it's own event name-space.  A USER_MESSAGE uses a global name-space and
   should probably only be used in the main program.

   A events default to being sent ANYTIME, but be over-riding the when() function.
   Events can be sent
   LATER. (i.e. qued up and sent at another time... hence the need for a copy() function). or NOW.
   If an event returns ANYTIME for when() the implementation choses.  This currently defaults to NOW.

   Events should be sent through a channel and not directly.  i.e.

   either :
   i4_kernel.send_to(send_to,&ev);
   or
   i4_time_dev.request_event(send_to,&ev,time);

   note : events are passed as pointers to event.

   whenever you receive_event() you should first check its
   type throught the type() function. Then you
   should CAST_PTR into the event type it actually is.
   Example:
 \code
   void receive_event(i4_event *ev)
   {
   	if (ev->type()==i4_event::KEY_PRESS)
   	{
   	  CAST_PTR(key_ev,i4_key_press_event_class,ev);
   	  printf("Key pressed %d\n",key_ev->key);
   	}
   }
 \endcode

   IMPORTANT : if you are derived from a parent who has a receive_event()
   you most likely want to call it
   or you will not get any of it's default behaviors.  This is especially true
   with i4_window_class derivatives.

   Note that keycodes are defined in event/keys.h
 */
class i4_event
{
public:
	enum event_type
	{
		MOUSE_MOVE,
		MOUSE_BUTTON_DOWN,
		MOUSE_BUTTON_UP,
		KEY_PRESS,
		KEY_RELEASE,
		SYSTEM_SIGNAL, //< CTRL-C, BUS, PIPE, etc
		WINDOW_MESSAGE, //< Events only seen by windows
		DISPLAY_CHANGE, //< When a refresh is need ore resolution changes
		DISPLAY_CLOSE, //< when close button on window is clicked, or the like
		OBJECT_MESSAGE, //< events specific to a specific instance (uses memory address to clarify)
		QUERY_MESSAGE, //< used by gui/ objects for querying and returning info
		USER_MESSAGE, //< global user evetns
		DO_COMMAND,   //< Window command to be executed
		END_COMMAND,  //< sent when key or menu item is depressed
		CHAR_SEND,    //< For Text-Edit-Controlls, where exact Ascii-Translations are needed
		IDLE_MESSAGE  //< This message is sent periodically, if nothing else needs to be done
	} ;

	enum dispatch_time
	{
		ANYTIME,    //< Whenever the kernel likes to (defaults to NOW)
		NOW,        //< Immediatelly
		LATER       //< Queue for later execution. Will execute next time the main loop is executed.
	};

	//! Event dispatch time.
	//! Most events can be sent now or defered, if you need to be defered till later or sent
	//! immidately, this can be replaced
	virtual dispatch_time when()
	{
		return ANYTIME;
	}

	//! Return the main event type.
	virtual event_type type() = 0;
	//! Return a copy of yourself.
	//! Usually, this calls the own constructor with the own fields as parameters.
	virtual i4_event *copy() = 0;
	//! Virtual destructor, usually not overriden for events.
	virtual ~i4_event()
	{
		;
	}

	//! translates any coordinates to client's window space
	virtual void move(int x_offset, int y_offset)
	{
		;
	}

	//! Returns a descriptive name of the event and its parameters.
	//! This is used for debugging only.
	virtual void name(char * buffer) = 0;
	static const int MAX_NAME_BUFFER_SIZE=128;
protected:

	void static_name(char * buffer, const char * str)
	{
		strncpy(buffer,str,MAX_NAME_BUFFER_SIZE);
		buffer[MAX_NAME_BUFFER_SIZE-1]=0;
	}

};

class i4_mouse_move_event_class         :
	public i4_event
{
public:
	i4_coord lx,ly; // last x,y
	i4_coord x,y;

	virtual event_type type()
	{
		return MOUSE_MOVE;
	}
	virtual i4_event *copy()
	{
		return new i4_mouse_move_event_class(lx,ly,x,y);
	}
	i4_mouse_move_event_class(i4_coord lx, i4_coord ly, i4_coord x, i4_coord y)
		: lx(lx),
		  ly(ly),
		  x(x),
		  y(y)
	{
	}
	void name(char * buffer)
	{
		static_name(buffer,"mouse_move");
	}

	void move(int x_offset, int y_offset)
	{
		x+=x_offset;
		y+=y_offset;
		lx+=x_offset;
		ly+=y_offset;
	}
} ;


class i4_mouse_button_event_class :
	public i4_event
{
public:
	enum btype {
		LEFT, CENTER, RIGHT
	} but;
	i4_coord x, y;                      // position mouse was at
	i4_time_class time, last_time;      // time pressed/depressed, time was last pressed/depressed
	i4_bool double_click;               // set by the window manager

	virtual event_type type() = 0;
	virtual i4_event *copy() = 0;

	i4_bool left()
	{
		return (i4_bool)(but==LEFT);
	}
	i4_bool right()
	{
		return (i4_bool)(but==RIGHT);
	}
	i4_bool center()
	{
		return (i4_bool)(but==CENTER);
	}

	i4_mouse_button_event_class(btype but,
								i4_coord x, i4_coord y,
								i4_time_class time, i4_time_class last_time,
								i4_bool double_click=i4_F)
		: but(but),
		  x(x),
		  y(y),
		  time(time),
		  last_time(last_time),
		  double_click(double_click)
	{
	}

	void move(int x_offset, int y_offset)
	{
		x+=x_offset;
		y+=y_offset;
	}

};

class i4_mouse_button_down_event_class  :
	public i4_mouse_button_event_class
{
public:
	event_type type()
	{
		return MOUSE_BUTTON_DOWN;
	}

	i4_mouse_button_down_event_class(btype but,
									 i4_coord x, i4_coord y,
									 i4_time_class time, i4_time_class last_time,
									 i4_bool double_click=i4_F)
		: i4_mouse_button_event_class(but,x,y, time, last_time, double_click)
	{
	}

	virtual i4_event *copy()
	{
		return new i4_mouse_button_down_event_class(but,x,y,time,last_time, double_click);
	}

	void name(char * buffer)
	{
		sprintf(buffer,"mouse_button_down at %d,%d with button %d",x,y,but);
	}
} ;

class i4_mouse_button_up_event_class    :
	public i4_mouse_button_event_class
{
public:
	virtual event_type type()
	{
		return MOUSE_BUTTON_UP;
	}
	i4_mouse_button_up_event_class(btype but,
								   i4_coord x, i4_coord y,
								   i4_time_class time, i4_time_class last_time,
								   i4_bool double_click=i4_F)
		: i4_mouse_button_event_class(but,x,y, time, last_time, double_click)
	{
	}

	virtual i4_event *copy()
	{
		return new i4_mouse_button_up_event_class(but,x,y,time,last_time, double_click);
	}

	void name(char * buffer)
	{
		sprintf(buffer,"mouse_button_up at %d,%d with button %d",x,y,but);
	}
} ;


class i4_do_command_event_class          :
	public i4_event
{
public:
	i4_time_class time;
	char command[128];
	int command_id;

	i4_do_command_event_class(char * _command, int command_id);
	i4_do_command_event_class(char * _command, int command_id, i4_time_class &time);
	virtual event_type type()
	{
		return DO_COMMAND;
	}
	virtual i4_event *copy();
	void name(char * buffer)
	{
		snprintf(buffer,MAX_NAME_BUFFER_SIZE-1,"do_command_event_class %s",command);
		buffer[MAX_NAME_BUFFER_SIZE-1]=0;
	}
} ;


class i4_end_command_event_class          :
	public i4_event
{
public:
	i4_time_class time;
	char command[128];
	int command_id;

	i4_end_command_event_class(char * command, int command_id, i4_time_class &time);
	i4_end_command_event_class(char * command, int command_id);


	virtual event_type type()
	{
		return END_COMMAND;
	}
	virtual i4_event *copy()
	{
		return new i4_end_command_event_class(command, command_id, time);
	}
	void name(char * buffer)
	{
		snprintf(buffer,MAX_NAME_BUFFER_SIZE-1,"end_command_event_class %s",command);
		buffer[MAX_NAME_BUFFER_SIZE-1]=0;
	}
} ;


class i4_key_press_event_class          :
	public i4_event
{
public:
	i4_time_class time;

	w16 key,      // includes shift info
		key_code, // raw key code
		modifiers; // includes shift, ctrl, alt, num-lock etc

	i4_key_press_event_class(w16 key, w16 key_code, w16 modifiers, i4_time_class &time)
		: key(key),
		  key_code(key_code),
		  modifiers(modifiers),
		  time(time)
	{
	}
	virtual event_type type()
	{
		return KEY_PRESS;
	}
	virtual i4_event *copy()
	{
		return new i4_key_press_event_class(key,key_code,modifiers,time);
	}
	void name(char * buffer)
	{
		sprintf(buffer,"key_press %d,%d",(w32)key,(w32)key_code);
	}
} ;

class i4_char_send_event_class :
	public i4_event
{
public:
	i4_time_class time;
	char character;    //The Character
	i4_char_send_event_class(char thekey, i4_time_class &time)
		: character(thekey),
		  time(time)
	{
	}
	virtual event_type type()
	{
		return CHAR_SEND;
	}
	virtual i4_event *copy()
	{
		return new i4_char_send_event_class(character,time);
	}
	void name(char * buffer)
	{
		sprintf(buffer,"char_send %c",character);
	}
};



class i4_key_release_event_class        :
	public i4_event
{
public:
	i4_time_class time;

	w16 key, key_code, modifiers;

	i4_key_release_event_class(w16 key, w16 key_code, w16 modifiers, i4_time_class &time)
		: key(key),
		  key_code(key_code),
		  modifiers(modifiers),
		  time(time)
	{
	}
	virtual event_type type()
	{
		return KEY_RELEASE;
	}
	virtual i4_event *copy()
	{
		return new i4_key_release_event_class(key,key_code,modifiers,time);
	}
	void name(char * buffer)
	{
		sprintf(buffer,"key_release %d,%d",(w32)key,(w32)key_code);
	}
} ;

class i4_display_class;
class i4_display_change_event_class     :
	public i4_event
{
public:
	i4_display_class * display;
	enum change_type
	{
		SIZE_CHANGE,  // resolution change
		PALETTE_CHANGE, // color mapping changed (not implemented yet)
		LOCATION_CHANGE // screen changed location (i.e. to another window or server)
	};
	change_type change;

	i4_display_change_event_class(i4_display_class * display, change_type change)
		: display(display),
		  change(change)
	{
	}

	virtual dispatch_time when()
	{
		return NOW;
	}
	virtual event_type type()
	{
		return DISPLAY_CHANGE;
	}
	virtual i4_event *copy()
	{
		return new i4_display_change_event_class(display,change);
	}
	void name(char * buffer)
	{
		static_name(buffer,"display_change");
	}
};

class i4_display_close_event_class      :
	public i4_event
{
public:
	i4_display_class * display;

	i4_display_close_event_class(i4_display_class * display) :
		display(display)
	{
	}
	virtual dispatch_time when()
	{
		return LATER;
	}
	virtual event_type type()
	{
		return DISPLAY_CLOSE;
	}
	virtual i4_event *copy()
	{
		return new i4_display_close_event_class(display);
	}
	void name(char * buffer)
	{
		static_name(buffer,"display_close");
	}
};

class i4_user_message_event_class       :
	public i4_event
{
public:
	w32 sub_type;

	virtual event_type type()
	{
		return USER_MESSAGE;
	}
	i4_user_message_event_class(w32 sub_type) :
		sub_type(sub_type)
	{
	}
	virtual i4_event *copy()
	{
		return new i4_user_message_event_class(sub_type);
	}
	void name(char * buffer)
	{
		static_name(buffer,"user_message");
	}
} ;

class i4_query_message_event_class      :
	public i4_event
{
public:
	virtual event_type type()
	{
		return QUERY_MESSAGE;
	}
	virtual i4_event *copy() = 0;
	virtual dispatch_time when()
	{
		return NOW;
	}
	void name(char * buffer)
	{
		static_name(buffer,"query_message");
	}
} ;

class i4_object_message_event_class     :
	public i4_event
{
public:
	dispatch_time dtime;
	void * object;
	w32 sub_type;

	virtual dispatch_time when()
	{
		return dtime;
	}
	virtual event_type type()
	{
		return OBJECT_MESSAGE;
	}
	i4_object_message_event_class(void * object, w32 sub_type=0, dispatch_time when=ANYTIME)
		: object(object),
		  sub_type(sub_type),
		  dtime(when)
	{
	}
	virtual i4_event *copy()
	{
		return new i4_object_message_event_class(object, sub_type);
	}
	void name(char * buffer)
	{
		static_name(buffer,"object_message");
	}
} ;

class i4_system_signal_event_class      :
	public i4_event
{
public:
	enum signal_type
	{
		HUP,   INTERRUPT,   QUIT,      ILL,    TRAP,    ABORT,    IOT,    BUS,    FPE,
		KILL,  USR1,        SEG_FAULT, USR2,   PIPE,    ALARM,    TERM,   CHILD,  CONTINUE,
		STOP,  TSTP,        TTIN,      TTOU,   IO,      URG,      CPU,    XFSZ,   VTALRM,
		PROF,  WINCH
	};
	signal_type signal_number;

	virtual event_type type()
	{
		return SYSTEM_SIGNAL;
	}
	virtual i4_event *copy()
	{
		return new i4_system_signal_event_class(signal_number);
	}

	i4_system_signal_event_class(signal_type sig) :
		signal_number(sig)
	{
	}
	void name(char * buffer)
	{
		sprintf(buffer,"system_signal %d",(w32)signal_number);
	}
} ;



class i4_user_idle_event_class        :
	public i4_event
{
public:
	virtual event_type type()
	{
		return IDLE_MESSAGE;
	}
	virtual i4_event *copy()
	{
		return new i4_user_idle_event_class;
	}
	void name(char * buffer)
	{
		static_name(buffer,"idle_event");
	}
} ;


#endif
