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
 
#include "pch.h"
#include "video/x11/x11_input.h"
#include "main/main.h"
#include "video/x11/x11_display.h"
#include "video/x11/mitshm.h"
#include "video/display.h"
#include "device/event.h"
#include "device/keys.h"
#include "device/kernel.h"
#include "image/context.h"
#include "app/app.h"
#include "window/window.h"
#include "version.h"


x11_input_class *x11_input_class_ptr=0;//used to reference the
//handler when displaying the error window

i4_bool x11_input_class::open_display()
{
  if (display)
	return i4_T;

  // get the default display name from the environment variable DISPLAY
  char ds_name[512];
  memset(ds_name,0,512);

  char *ds_env = getenv("DISPLAY");

  if (ds_env)
	strcpy(ds_name,ds_env);
  else
    strcpy(ds_name,"localhost:0.0");

  if (getenv("X_REPEAT_ON"))
    repeat_on=i4_T;

  // check the command line args for another display
  w32 i=1;
  for (;i<i4_global_argc;i++)
  {
    if (i4_global_argv[i] == "-ron")
      repeat_on=i4_T;
    else if (i4_global_argv[i] == "-display")
    {
      i++;
      i4_os_string(i4_global_argv[i], ds_name, 512);
    }
  }
  display=XOpenDisplay(ds_name);  
  if (!display) {
	//We must not use i4_error() here, since that would cause
	//recursive non-reporting of the same error (if we can't open the
	//display, we cannot open the same display to show an error...)
	printf("SEVERE: Check the DISPLAY environment variable and verify an X-Server is running. \n");
	printf("Could not open the connection to the X-Server at: %s",ds_name);
    return i4_F;
  }

  for (i=1;i<i4_global_argc;i++)
  {
	if (i4_global_argv[i] == "-sync") {
	  XSynchronize(display,True);
	  break;
	}
  }

  //if (!repeat_on)  //brings only trouble in debug mode since this is a 
  //  XAutoRepeatOff(display);  //system wide setting
  return i4_T;
}


void x11_input_class::close_display()
{
  if (display)
  {
    XAutoRepeatOn(display);

    XCloseDisplay(display);
    display=NULL;
  }
}

x11_input_class::x11_input_class()
{
  repeat_on=i4_F;
  display=0;
  my_visual=0;
  need_first_time=i4_T;
  x11_input_class_ptr=this;
}

x11_input_class::~x11_input_class() 
{
  close_display();
}

XVisualInfo *x11_input_class::find_visual_with_depth(int depth)
{
  XVisualInfo vis_info, *v;
  int items;

  if (!display && !open_display())
    return 0;

  if (depth==8)
    vis_info.c_class=PseudoColor;
 
   else 
    vis_info.c_class=TrueColor;

  vis_info.depth=depth;
  
  //v=XGetVisualInfo(display,VisualClassMask | VisualDepthMask, &vis_info, &items);
  v=XGetVisualInfo(display,VisualDepthMask, &vis_info, &items);

  if (items>0)
  {
    XMatchVisualInfo(display, screen_num, vis_info.depth, vis_info.c_class, &vis_info);
    v->visual = vis_info.visual;
    return v;
  } 
  else if (depth==16)
    return find_visual_with_depth(15);
  else if (depth==15)
    return find_visual_with_depth(24);
  else if (depth==24)
    return find_visual_with_depth(32);
  else
    return 0;
}


// makes a null cursor
static Cursor x11_CreateNullCursor(Display *display, Window root)
{
  Pixmap cursormask; 
  XGCValues xgc;
  GC gc;
  XColor dummycolour;
  Cursor cursor;

  cursormask = XCreatePixmap(display, root, 1, 1, 1/*depth*/);
  xgc.function = GXclear;
  gc =	XCreateGC(display, cursormask, GCFunction, &xgc);
  XFillRectangle(display, cursormask, gc, 0, 0, 1, 1);
  dummycolour.pixel = 0;
  dummycolour.red = 0;
  dummycolour.flags = 04;
  cursor = XCreatePixmapCursor(display, cursormask, cursormask,
			       &dummycolour,&dummycolour, 0,0);
  XFreePixmap(display,cursormask);
  XFreeGC(display,gc);
  return cursor;
}

void x11_input_class::draw_error(Display *disp, const char *error_msg,int width, int height, int screen_e_num)
{
	//draw the error message to the window errorwin.
	XGCValues values;
	GC mgc=XCreateGC(disp,errorwin,0,&values);
	XSetBackground(display,mgc,WhitePixel(display,screen_e_num));
	XSetForeground(display,mgc,BlackPixel(display,screen_e_num));
	//XDrawRectangle(display,errorwin,mgc,0,0,width,height);
	XSetWindowBackground(display,errorwin,WhitePixel(display,screen_e_num));
	XClearWindow(display,errorwin);
	int ypos=20;
	int endpos=0;
	int startpos=0;
	while (endpos<strlen(error_msg))
	  {
	  while ((error_msg[endpos]!='\n') && (error_msg[endpos]!=0))
	  	{
		endpos++;
		} 
	  XDrawString(display,errorwin,mgc,5,ypos,(char*)error_msg+startpos,endpos-startpos);
	  endpos++;
	  startpos=endpos;
	  ypos+=20;
	  }
	//XDrawString(display,errorwin,mgc,5,5,(char*)error_msg,strlen(error_msg));
	XDrawString(display,errorwin,mgc,5,ypos,"Click to continue",17);
	XFreeGC(display,mgc);
}

int x11_input_class::create_error_window(const char *error_msg){
	XVisualInfo vinfo;
	bool is_fatal=false;
	if (error_msg[0]=='F' && error_msg[1]=='A' && error_msg[2]=='T')
	{
		is_fatal=true;
	}
	ZeroMemory(&vinfo,sizeof(vinfo));
	if (!open_display())
	{
		//Ack, no display at all. Cannot display message in an X window.
	    printf("The following error occured: \n%s",error_msg);
	    printf("Additionally, I was unable to properly show an error dialog: "
	    "The connection to the X server could not be openend.\n");
	    if (is_fatal)
	       exit(99);
	    return 0;
	}
		//PG: Need to look up required parameters, probably, (display,0,0,0)
		//since we just want ANY visual for this.
	int novis=0;
	XVisualInfo *visual=XGetVisualInfo(display, 0, &vinfo, &novis);

	int screen_e_num  = DefaultScreen(display);


  Colormap tmpcmap;
  
  XEvent report;
  
  tmpcmap = XCreateColormap(display, 
			    XRootWindow(display,visual->screen),visual->visual, 
			    AllocNone);

  int attribmask = CWColormap | CWBorderPixel;
  XSetWindowAttributes attribs;
  ZeroMemory(&attribs,sizeof(XSetWindowAttributes));
  attribs.border_pixel = 0;
  attribs.colormap = tmpcmap;
  int numlines=1;
  int maxlinelen=0;
  int curlinelen=0;
  for (int pos=0;pos<strlen(error_msg);pos++)
    {
	curlinelen++;
	if (error_msg[pos]=='\n')
	  {
	  numlines++;//The error message spans more than one line
	  if (curlinelen>maxlinelen)
	    maxlinelen=curlinelen;
	  curlinelen=0;
	  }
    }
    //it's not neccessary that the message ends with a line feed
  if (curlinelen>maxlinelen)
    maxlinelen=curlinelen;
  int w=(maxlinelen*8)+30;
  int h=numlines*30+20;
  int x=-1;
  int y=-1;
  w=(w+3)&(~3);  
  errorwin=XCreateWindow(display,
			XRootWindow(display,visual->screen),
			x,y,
			w,h,
			0,
			visual->depth,
			InputOutput,
			visual->visual,
			attribmask,
			&attribs);
  XFreeColormap(display,tmpcmap);

  XSelectInput(display,errorwin, 
	       KeyPressMask	| ButtonPressMask | ButtonReleaseMask | PointerMotionMask   | 
	       StructureNotifyMask | ExposureMask);

  XGCValues values;
  //XSetWMProperties(display,mainwin,&windowName,&iconName,
  //	0,0,0,0,0);
  //XFree(windowName.value);
  //XFree(iconName.value);
  GC mgc;
  mgc=XCreateGC(display,errorwin,0,&values);
  XSetBackground(display,mgc,WhitePixel(display,screen_e_num));

  XMapWindow(display,errorwin);
  do
  { 
    XNextEvent(display, &report);
  } while (report.type!= Expose);     // wait for our window to pop up

  //i4_kernel.add_device(this);

  //XDefineCursor(display, mainwin, x11_CreateNullCursor(display, mainwin));
  Atom wm_e_delete_window = XInternAtom (display, "WM_DELETE_WINDOW", True);
  Atom wm_e_protocols = XInternAtom (display, "WM_PROTOCOLS", True);

  Atom prot[2];
  prot[0]=wm_e_delete_window;
  prot[1]=wm_e_protocols;
  
  XSetWMProtocols(display, errorwin, prot, 2);
  XFreeGC(display,mgc);
  draw_error(display,error_msg,w,h, screen_e_num);
  i4_bool closed=i4_F;
  XEvent xev;
  do {
    XNextEvent(display, &xev);
    switch (xev.type)
     { 
	case Expose :
	{ 
	  draw_error(display,error_msg,w,h,screen_e_num);//I think expose is the same as WM_PAINT
	} break;


	case ClientMessage:
	{
	  /* Client messages are the means of the window manager
	   *  communicating with a program. We'll first check to
	   *  see if this is really the window manager talking
	   *  to us.
	   */
	  if (xev.xclient.message_type == wm_e_protocols)
	  {
	  	/*
	    if ((Atom) xev.xclient.data.l[0] == wm_delete_window)
	    {
	      i4_display_close_event_class dc(i4_display);
	      send_event_to_agents(&dc, i4_device_class::FLAG_DISPLAY_CLOSE);
	    }
	    */
	    closed=i4_T;
	  }
	} break;

	case ConfigureNotify :
	{
	  XFlush(display);

	  int new_width=xev.xconfigure.width&~3;  // must be word alligned
	  int new_height=xev.xconfigure.height;
	  if (new_width!=xev.xconfigure.width)
	    XResizeWindow(display,errorwin,new_width,xev.xconfigure.height);

	  XFlush(display);

	  //resize(new_width, new_height);
   
	} break;
	
	case MotionNotify : 
	{
	  //motion_occured=i4_T;
	  //final_x=xev.xmotion.x;
	  //final_y=xev.xmotion.y;
	} break;
	  
	case ButtonRelease :
	case ButtonPress :
	{   
	  i4_mouse_button_event_class::btype but=i4_mouse_button_event_class::LEFT;
	  switch (xev.xbutton.button)
	  {
	    case 1 : but=i4_mouse_button_event_class::LEFT; break;
	    case 3 : but=i4_mouse_button_event_class::RIGHT; break;
	    case 2 : but=i4_mouse_button_event_class::CENTER; break;
	  }

	  i4_time_class now;
	  if (xev.type == ButtonRelease)
	  {
	    //i4_mouse_button_up_event_class up(but, mouse_x, mouse_y,
	//				      now, last_up[but]);
	//    send_event_to_agents(&up,FLAG_MOUSE_BUTTON_UP);
	//    last_up[but]=now;
		closed=i4_T;
	  }
	  else
	  {
	    //i4_mouse_button_down_event_class up(but, mouse_x, mouse_y,
	    //					now, last_down[but]);
	    //send_event_to_agents(&up,FLAG_MOUSE_BUTTON_DOWN);
	    //last_down[but]=now;
	  }
	} break;

	case KeyPress :
	case KeyRelease :
	{
	  char buf;
	  KeySym ks;
	  XLookupString(&xev.xkey,&buf,1,&ks,NULL); 
	  w16 key, key_code;
	  w16 char_code=ks;
	  closed=i4_T;
	} break;
	default:
	  break;
     }
  } while(!closed);
  XDestroyWindow(display,errorwin); 
  XNextEvent(display, &report);
  errorwin=0; 
  //this seems to be quite dangerous in debug mode
  //if (getenv("X_GRAB_OFF") == NULL)
  //  XGrabPointer( display, mainwin, False, 
  //                ButtonPressMask |
  //                ButtonReleaseMask | ButtonMotionMask | PointerMotionMask,
  //                GrabModeAsync, GrabModeAsync, None, None, CurrentTime );
  was_exposed=i4_T; //always redraw everything after an error.
  if (is_fatal)
    exit(101);
  return 0;	
	
	}

i4_bool x11_input_class::create_window(sw32 x, sw32 y, w32 w, w32 h,
				       i4_display_class *_i4_display,
				       i4_bool takeup_fullscreen,
				       XVisualInfo *visual)
{
  i4_display=_i4_display;
  was_exposed=i4_T;
  XTextProperty windowName;
  XTextProperty iconName;
  char *listn[2];
  char *listicon[2];
  XTextProperty tprop;

  if (!open_display()) 
    return i4_F;
  //The first constant is defined
  //by the makefile in a -D flag to the compiler.
  //The second comes from version.h
  listn[0]=GOLGOTHA_WINDOW_TITLE GOLGOTHA_VERSION_STR;
  listn[1]=0;
  listicon[0]="Golgotha";
  listicon[1]=0;
  if (XStringListToTextProperty(listn, 1, &windowName) == 0)
      i4_error("SEVERE: Could not assign a name to our window");
  if (XStringListToTextProperty(listicon, 1, &iconName) == 0)
      i4_error("SEVERE: Could not assign the icon name to our window");
  XEvent report;

  // this will be 8, 16, or 32, (most likely 16 for now)
  my_visual = visual;


  if (!my_visual)
    return i4_F;

  screen_num  = DefaultScreen(display);


  Colormap tmpcmap;
  
  tmpcmap = XCreateColormap(display, 
			    XRootWindow(display,my_visual->screen),my_visual->visual, 
			    AllocNone);

  int attribmask = CWColormap | CWBorderPixel;
  XSetWindowAttributes attribs;
  ZeroMemory(&attribs,sizeof(XSetWindowAttributes));
  attribs.border_pixel = 0;
  attribs.colormap = tmpcmap;

  w=(w+3)&(~3);  
  mainwin=XCreateWindow(display,
			XRootWindow(display,my_visual->screen),
			x,y,
			w,h,
			0,
			my_visual->depth,
			InputOutput,
			my_visual->visual,
			attribmask,
			&attribs);
  XFreeColormap(display,tmpcmap);

  XSelectInput(display,mainwin, 
	       KeyPressMask	| VisibilityChangeMask | ButtonPressMask | ButtonReleaseMask |
	       ButtonMotionMask | PointerMotionMask    | KeyReleaseMask  | ExposureMask      | 
	       StructureNotifyMask | EnterWindowMask | LeaveWindowMask);

  XGCValues values;
  XSetWMProperties(display,mainwin,&windowName,&iconName,
  	0,0,0,0,0);
  XFree(windowName.value);
  XFree(iconName.value);
  
  gc=XCreateGC(display,mainwin,0,&values);
  XSetBackground(display,gc,BlackPixel(display,screen_num));

  XMapWindow(display,mainwin);
  do
  { 
    XNextEvent(display, &report);
  } while (report.type!= Expose);     // wait for our window to pop up

  i4_kernel.add_device(this);

  XDefineCursor(display, mainwin, x11_CreateNullCursor(display, mainwin));


  wm_delete_window = XInternAtom (display, "WM_DELETE_WINDOW", True);
  wm_protocols = XInternAtom (display, "WM_PROTOCOLS", True);

  Atom prot[2];
  prot[0]=wm_delete_window;
  prot[1]=wm_protocols;
  
  XSetWMProtocols(display, mainwin, prot, 2);
  
  //this seems to be quite dangerous in debug mode
  //if (getenv("X_GRAB_OFF") == NULL)
  //  XGrabPointer( display, mainwin, False, 
  //                ButtonPressMask |
  //                ButtonReleaseMask | ButtonMotionMask | PointerMotionMask,
  //                GrabModeAsync, GrabModeAsync, None, None, CurrentTime );

  return i4_T;
}


void x11_input_class::destroy_window()
{ 
  if (!my_visual)
	return;

  i4_kernel.remove_device(this);

  XFreeGC(display, gc);
  XFree((char *)my_visual);
  my_visual=0;

//   if (I4_SCREEN_DEPTH==8)
//     XFreeColormap(display,xcolor_map);
}



void x11_input_class::get_x_time(w32 xtick, i4_time_class &t)
{
  i4_time_class now;

  if (need_first_time)
  {
    first_time=xtick;
    i4_start_time=now;
    need_first_time=i4_F;
  }

  w32 xtime=xtick-first_time;
  
  t = i4_start_time;
  t.add_milli(xtime);
	      
  if (now<t)
  {
    i4_start_time=now;
    i4_start_time.add_milli(-xtime);
    t=now;
  }
}

i4_bool x11_input_class::process_events()
{
  i4_bool motion_occured=i4_F;
  sw32 final_x=0, final_y=0;

  if (display)
  {
    while (XPending(display))
    {
      XEvent xev;
      XNextEvent(display,&xev);

      note_event(xev);

      switch (xev.type)
      { 
	case Expose :
	{ 
	  if (context)
	  {
	    context->add_both_dirty(xev.xexpose.x,
				    xev.xexpose.y,
				    xev.xexpose.x+xev.xexpose.width,
				    xev.xexpose.y+xev.xexpose.height);
	  }
	  was_exposed=i4_T;
	  i4_current_app->get_root_window()->request_redraw(i4_T);
	  i4_current_app->get_root_window()->request_redraw(i4_F);
	} break;


	case ClientMessage:
	{
	  /* Client messages are the means of the window manager
	   *  communicating with a program. We'll first check to
	   *  see if this is really the window manager talking
	   *  to us.
	   */
	  if (xev.xclient.message_type == wm_protocols)
	  {
	    if ((Atom) xev.xclient.data.l[0] == wm_delete_window)
	    {
	      i4_display_close_event_class dc(i4_display);
	      send_event_to_agents(&dc, i4_device_class::FLAG_DISPLAY_CLOSE);
	    }
	  }
	} break;

	//{{{  not implemented yet
	case ConfigureNotify :
	{
	  XFlush(display);

	  int new_width=xev.xconfigure.width&~3;  // must be word alligned
	  int new_height=xev.xconfigure.height;
	  if (new_width!=xev.xconfigure.width)
	    XResizeWindow(display,mainwin,new_width,xev.xconfigure.height);

	  XFlush(display);

	  resize(new_width, new_height);
   
	  i4_display_change_event_class d_change(i4_display,
						 i4_display_change_event_class::SIZE_CHANGE);
	  
	  send_event_to_agents(&d_change,FLAG_DISPLAY_CHANGE);
	} break;
	
	case MotionNotify : 
	{
	  motion_occured=i4_T;
	  final_x=xev.xmotion.x;
	  final_y=xev.xmotion.y;
	} break;
	  
	case ButtonRelease :
	case ButtonPress :
	{   
	  i4_mouse_button_event_class::btype but=i4_mouse_button_event_class::LEFT;
	  switch (xev.xbutton.button)
	  {
	    case 1 : but=i4_mouse_button_event_class::LEFT; break;
	    case 3 : but=i4_mouse_button_event_class::RIGHT; break;
	    case 2 : but=i4_mouse_button_event_class::CENTER; break;
	  }

	  i4_time_class now;
	  if (xev.type == ButtonRelease)
	  {
	    i4_mouse_button_up_event_class up(but, mouse_x, mouse_y,
					      now, last_up[but]);
	    send_event_to_agents(&up,FLAG_MOUSE_BUTTON_UP);
	    last_up[but]=now;
	  }
	  else
	  {
	    i4_mouse_button_down_event_class up(but, mouse_x, mouse_y,
						now, last_down[but]);
	    send_event_to_agents(&up,FLAG_MOUSE_BUTTON_DOWN);
	    last_down[but]=now;
	  }
	} break;

	case KeyPress :
	case KeyRelease :
	{
	  char buf;
	  KeySym ks;
	  XLookupString(&xev.xkey,&buf,1,&ks,NULL); 
	  w16 key, key_code;
	  w16 char_code=ks;
	  
	  //This is set to false if the char event should not be sent 
	  //(because it would result in a bogus character value displayed)
	  i4_bool sc=i4_T;

	  switch (ks)
	  { 
	    case XK_Down :    key_code=I4_DOWN; sc=i4_F; break;
	    case XK_Up :      key_code=I4_UP; sc=i4_F; break;
	    case XK_Left :    key_code=I4_LEFT; sc=i4_F; break;
	    case XK_Right :	key_code=I4_RIGHT; sc=i4_F; break;
	    case XK_Control_L :   
	    { 
	      key_code=I4_CTRL_L; 
	      sc=i4_F;  
	      if (xev.type==KeyPress)
		modifier_state|=I4_MODIFIER_CTRL_L;  
	      else
		modifier_state&=~I4_MODIFIER_CTRL_L;  
	    } break;

	    case XK_Control_R :   
	    { 
	      key_code=I4_CTRL_R; 
	      sc=i4_F; 
	      if (xev.type==KeyPress)
		modifier_state|=I4_MODIFIER_CTRL_R;  
	      else
		modifier_state&=~I4_MODIFIER_CTRL_R;
	    } break;

	    case XK_Alt_L :   
	    { 
	      key_code=I4_ALT_L; 
	      sc=i4_F; 
	      if (xev.type==KeyPress)
		modifier_state|=I4_MODIFIER_ALT_L;  
	      else
		modifier_state&=~I4_MODIFIER_ALT_L;
	    } break;

	    case XK_Alt_R :   
	    { 
	      key_code=I4_ALT_R; 
	      sc=i4_F; 
	      if (xev.type==KeyPress)
		modifier_state|=I4_MODIFIER_ALT_R;  
	      else
		modifier_state&=~I4_MODIFIER_ALT_R;
	    } break;


	    case XK_Shift_L :	
	    { 
	      key_code=I4_SHIFT_L; 
	      sc=i4_F;
	      if (xev.type==KeyPress)
		modifier_state|=I4_MODIFIER_SHIFT_L;  
	      else
		modifier_state&=~I4_MODIFIER_SHIFT_L;
	    } break;

	    case XK_Shift_R :
	    { 
	      key_code=I4_SHIFT_R; 
	      sc=i4_F;
	      if (xev.type==KeyPress)
		modifier_state|=I4_MODIFIER_SHIFT_R;  
	      else
		modifier_state&=~I4_MODIFIER_SHIFT_R;
	    } break;


	    case XK_Num_Lock :		key_code=I4_NUM_LOCK; sc=i4_F;  break;
	    case XK_Home :    key_code=I4_HOME; sc=i4_F;  break;
	    case XK_End :     key_code=I4_END;sc=i4_F; 	break;
	    case XK_BackSpace : 	key_code=I4_BACKSPACE;sc=i4_F; 	break;
	    case XK_Tab :   key_code=I4_TAB; sc=i4_F;  break;
	    case XK_Return :	key_code=I4_ENTER; sc=i4_F;  break;
	    case XK_Caps_Lock : 	key_code=I4_CAPS;  break;
	    case XK_Escape :	key_code=I4_ESC;  break;
	    case XK_F1 :		key_code=I4_F1; sc=i4_F; break;
	    case XK_F2 :		key_code=I4_F2; sc=i4_F; break;
	    case XK_F3 :		key_code=I4_F3; sc=i4_F; break;
	    case XK_F4 :		key_code=I4_F4; sc=i4_F; break;
	    case XK_F5 :		key_code=I4_F5; sc=i4_F; break;
	    case XK_F6 :		key_code=I4_F6; sc=i4_F; break;
	    case XK_F7 :		key_code=I4_F7; sc=i4_F; break;
	    case XK_F8 :		key_code=I4_F8; sc=i4_F; break;
	    case XK_F9 :		key_code=I4_F9; sc=i4_F; break;
	    case XK_F10 :		key_code=I4_F10; sc=i4_F; break;
	    case XK_F11 :               key_code=I4_F11; sc=i4_F; break;
	    case XK_F12 :               key_code=I4_F12; sc=i4_F; break; 
	    case XK_Insert :		key_code=I4_INSERT; sc=i4_F; break;
	    case XK_Page_Up :		key_code=I4_PAGEUP; sc=i4_F; break;
	    case XK_Page_Down : 	key_code=I4_PAGEDOWN; sc=i4_F; break;
	    case XK_Delete :		key_code=I4_DEL; sc=i4_F; break;

	    case XK_KP_0 :		key_code=I4_KP0; break;
	    case XK_KP_1 :		key_code=I4_KP1; break;
	    case XK_KP_2 :		key_code=I4_KP2; break;
	    case XK_KP_3 :		key_code=I4_KP3; break;
	    case XK_KP_4 :		key_code=I4_KP4; break;
	    case XK_KP_5 :		key_code=I4_KP5; break;
	    case XK_KP_6 :		key_code=I4_KP6; break;
	    case XK_KP_7 :		key_code=I4_KP7; break;
	    case XK_KP_8 :		key_code=I4_KP8; break;
	    case XK_KP_9 :		key_code=I4_KP9; break;

	    case XK_KP_Insert : 	key_code=I4_KP0; sc=i4_F; break;
	    case XK_KP_End :		key_code=I4_KP1; sc=i4_F; break;
	    case XK_KP_Down :		key_code=I4_KP2; sc=i4_F; break;
	    case XK_KP_Page_Down :	key_code=I4_KP3; sc=i4_F; break;
	    case XK_KP_Left :		key_code=I4_KP4; sc=i4_F; break;
	    case XK_KP_Begin :		key_code=I4_KP5; sc=i4_F; break;
	    case XK_KP_Right :		key_code=I4_KP6; sc=i4_F; break;
	    case XK_KP_Home :		key_code=I4_KP7; sc=i4_F; break;
	    case XK_KP_Up :		key_code=I4_KP8; sc=i4_F; break;
	    case XK_KP_Page_Up :	key_code=I4_KP9; sc=i4_F; break;

	case XK_KP_Delete : sc=i4_F; //no break;
	case XK_KP_Decimal :		key_code=I4_KPPERIOD; break;

	    case ' ':                   key_code=I4_SPACE; break;
	    case '`':
	    case '~':                   key_code='`'; break;
	    case '-':
	    case '_':                   key_code='-'; break;
	    case '=':
	    case '+':                   key_code='='; break;
	    case '[':
	    case '{':                   key_code='['; break;
	    case ']':
	    case '}':                   key_code=']'; break;
	    case '\\':
	    case '|':                   key_code='\\'; break;
	    case ';':
	    case ':':                   key_code=';'; break;
	    case '\'':
	    case '"':                   key_code='\''; break;
	    case ',':
	    case '<':                   key_code=','; break;
	    case '.':
	    case '>':                   key_code='.'; break;
	    case '/':
	    case '?':                   key_code='/'; break;

	    case ')':                   key_code='0'; break;
	    case '!':                   key_code='1'; break;
	    case '@':                   key_code='2'; break;
	    case '#':                   key_code='3'; break;
	    case '$':                   key_code='4'; break;
	    case '%':                   key_code='5'; break;
	    case '^':                   key_code='6'; break;
	    case '&':                   key_code='7'; break;
	    case '*':                   key_code='8'; break;
	    case '(':                   key_code='9'; break;

	    default :
	      if ((ks>=XK_A && ks<=XK_Z) || (ks>=XK_0 && ks<=XK_9))
		key_code = ks;
	      else if (ks>=XK_a && ks<=XK_z)
		key_code = ks + 'A' - 'a';
	      else
		key_code=0;
	  }

	  if (key_code)
	  {
	    if (xev.type==KeyPress)
	    {
	      i4_time_class t;
	      get_x_time(xev.xkey.time, t);
	      
	      key = i4_key_translate(key_code,1,modifier_state);
	      i4_key_press_event_class ev(key, key_code, modifier_state, t);

	      send_event_to_agents(&ev,FLAG_KEY_PRESS);    
	      
	      //The following should send all key-codes in directly translated fashion, 
	      //including diacriticals, signs and stuff.
	      //Also the capitalization should be preserved.
	      if (sc)
	      {
	      	i4_char_send_event_class csev(char_code, t);
	      	send_event_to_agents(&csev,i4_device_class::FLAG_CHAR_SEND);
	      };
	    }
	    else 
	    {
	      key = i4_key_translate(key_code,0, modifier_state);

	      i4_time_class t;
	      get_x_time(xev.xkey.time, t);
	      
	      i4_key_release_event_class ev(key, key_code, modifier_state, t);
	      send_event_to_agents(&ev,FLAG_KEY_RELEASE);    
	    }
	  }
	} break;

	case EnterNotify:
	  if (!repeat_on)
	    XAutoRepeatOff(display);
	  break;

	case LeaveNotify:
	  XAutoRepeatOn(display);
	  break;
      }
    }
  }

	  
  if (motion_occured && (final_x!=mouse_x || final_y!=mouse_y))
  {    
    i4_mouse_move_event_class move(mouse_x, mouse_y, final_x, final_y);
    send_event_to_agents(&move,FLAG_MOUSE_MOVE);

    if (mouse_locked)
      XWarpPointer(display, None, mainwin, 0,0, 0,0, mouse_x, mouse_y);
    else
    {
      mouse_x=final_x;
      mouse_y=final_y;
    }	
  }

  return i4_F;
}

int showerror(const char *msg)
	{
		if (x11_input_class_ptr)
		{
			int ret=0;
			ret=x11_input_class_ptr->create_error_window(msg);
			return ret;
		}
		else
		{
			printf("%s",msg);
			return 1;
		}
	}
