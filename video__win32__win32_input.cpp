/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/
#include "pch.h"
#include "video/win32/win32_input.h"
#include "error/error.h"
#include "device/keys.h"
#include "device/event.h"
#include "main/win_main.h"
#include "device/kernel.h"
#include "resource.h"
#include "memory/malloc.h"
#include "lisp/lisp.h"
//#include "video/win32/dx5_util.h"
#include "video/movie_engine.h"
#include "window/window.h"
#include <mmsystem.h>

//this code is os-dependent

i4_bool win32_input_class::class_registered=i4_F;

static win32_input_class *win32_current_input=0;
HWND current_window_handle;
HANDLE myPid;

int i4_default_warning(const char *st);
int i4_default_error(const char *st);


// int i4_win32_error(const char *st)
// {
//   if (current_window_handle)
//   {
//     MessageBox(current_window_handle, st, "Error!", MB_OK | MB_APPLMODAL);
//     exit(0);
//   }
//   else
//     i4_default_error(st);
//   return 1;
// }


i4_bool win32_input_class::process_events()
{
  i4_bool ret=i4_F;

  MSG         msg;

  w32         size=20;

//   //check the DirectInput key device if it exists
//   if(lpdiKey)
//     lpdiKey->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), keyData, &size, 0);

//  w32         size=20;

  //check the DirectInput key device if it exists
  /*
  if(lpdiKey)
  {
    ret=lpdiKey->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), keyData, &size, DIGDD_PEEK);
    if(ret==DI_OK || ret==DI_BUFFEROVERFLOW)
    {
      for(int i=0;i<size;i++)
      {
        if(keyData[i].dwData&0xff)
        {  //key down
          w16 key_code=translate_windows_key(keyData[i].dwOfs);
          w16 key=i4_key_translate(key_code, 1, modifier_state);
          i4_key_press_event_class ev(key,key_code, modifier_state);
          send_event_to_agents(&ev,i4_device_class::FLAG_KEY_PRESS);
        }
        else
        {
          w16 key_code=translate_windows_key(keyData[i].dwOfs);
          w16 key=i4_key_translate(key_code, 0, modifier_state);
          i4_key_release_event_class ev(key,key_code, modifier_state);
          send_event_to_agents(&ev,i4_device_class::FLAG_KEY_RELEASE);
        }
      }
    }
  }*/
  if (active)
	  {
	  while (PeekMessage(&msg, NULL, 0,0, PM_REMOVE)==TRUE)//not only for whandle
		  {
		  TranslateMessage(&msg);
		  
		  DispatchMessage(&msg);
		  //Wie kann das gutgehen?
		  //vielleicht schaffen wir noch, dass msg.time doch weitergereicht wird
		  //process(whandle, msg.message, msg.wParam, msg.lParam, msg.time);
		  
		  ret=i4_T;
		  }
	  }
  else 
	  {
	  //li_call("gc");perhaps good moment to do some gc(), but may be a bit often
	  i4_suspend_other_threads();//otherwise, we play background music although nothing can be heard.
	  GetMessage(&msg,NULL,0,0);
	  i4_resume_other_threads();
	  TranslateMessage(&msg);
	  DispatchMessage(&msg);
	  Sleep(100);
	  while(PeekMessage(&msg,NULL,0,0,PM_REMOVE)==TRUE)
		  {//If one message is in queue, there might be others pending
		  //We need to be shure we have processed everything when 
		  //WM_ACTIVATEAPP restores window to FS.
		  TranslateMessage(&msg);
		  DispatchMessage(&msg);
		  }
	  ret=i4_T;
	  }

/*        
        if (PeekMessage (&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage (&msg);
            DispatchMessage (&msg);
            ret=i4_T;
            i4_warning("Window Message Preprocessed");
        }
*/
  return ret;
}

long FAR PASCAL WindowProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );


/*
I4_THREAD_PROC_RETURN win32_input_class(I4_THREAD_PROC_ARGLIST)
{
  MSG msg;
  win32_input_class *input=(win32_input_class *)arg_list;

  while (!stop_thread)
  {    
    GetMessage(&msg, input->get_window_handle(), 0,0);
    if (msg.message != WM_USER)
      input->message_que.que(msg);
  }
  thread_running=i4_F;
}
*/

void win32_input_class::get_window_area(int &x, int &y, int &w, int &h)
{
  RECT r;
  GetClientRect(whandle, &r);

  w=r.right-r.left;
  h=r.bottom-r.top; 

  NONCLIENTMETRICS cl;
  cl.cbSize=sizeof(NONCLIENTMETRICS);

  SystemParametersInfo(SPI_GETNONCLIENTMETRICS,
                       sizeof(NONCLIENTMETRICS),
                       &cl,
                       0);

  x=r.left + cl.iBorderWidth;
  y=r.top + cl.iCaptionHeight;
  
 
}

i4_bool win32_input_class::create_window(sw32 x, sw32 y, w32 w, w32 h,
                                         i4_display_class *display_responsible_for_close,
                                         i4_bool takeup_fullscreen)
{  
  if (!class_registered)
  {
    WNDCLASS window_class;
    window_class.style = CS_HREDRAW | CS_VREDRAW;
    window_class.lpfnWndProc = WindowProc;
    window_class.cbClsExtra = 0;
    window_class.cbWndExtra = 0;
    window_class.hInstance = i4_win32_instance;
    window_class.hIcon = LoadIcon( i4_win32_instance, MAKEINTRESOURCE(102));
//	window_class.hIcon = LoadIcon( i4_win32_instance, "102");
//    window_class.hIcon = (HICON)LoadImage( i4_win32_instance, MAKEINTRESOURCE(IDI_GOLGOTHA), IMAGE_ICON,0,0,LR_LOADTRANSPARENT);
    window_class.hCursor = LoadCursor( NULL, IDC_ARROW );//NULL, NULL hier sinnvoller
    window_class.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    window_class.lpszMenuName = 0;//window_name();??
    window_class.lpszClassName = class_name();
    RegisterClass( &window_class );
    class_registered=i4_T;
  }

  
  if (takeup_fullscreen)
    whandle = CreateWindowEx(0,
                             class_name(),
                             window_name(),
                             WS_POPUP | WS_EX_TOPMOST,
							 //|WS_SYSMENU|WS_CAPTION|WS_MINIMIZEBOX,//Gives trouble (Menu not accessible et al.)
                             0,0,
                             GetSystemMetrics(SM_CXSCREEN),
                             GetSystemMetrics(SM_CYSCREEN),
                             NULL,
                             NULL,
                             i4_win32_instance,
                             NULL );

  else
  {
    whandle = CreateWindowEx(WS_EX_APPWINDOW, // 0,
                             class_name(),
                             window_name(),
                             // WS_POPUP | WS_EX_TOPMOST, // J.C.
                             WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX |  // J.J.
                             WS_OVERLAPPEDWINDOW,
                             x, y, w, h,
                             NULL,
                             NULL,
                             i4_win32_instance,
                             NULL );
        // JJ
        RECT rt1, rt2;
        int x2,y2;
        GetWindowRect(whandle,&rt1);
        GetClientRect(whandle,&rt2);        
        x2 = (rt1.right - rt1.left) - rt2.right  + w;
        y2 = (rt1.bottom- rt1.top ) - rt2.bottom + h;
        MoveWindow(whandle,rt1.top,rt1.left,x2,y2,TRUE);
        // JJ
  }
  if (whandle)
  {
    have_window=i4_T;
    current_window_handle=whandle;


    i4_kernel.add_device(this);
    win32_current_input=this;  
    display=display_responsible_for_close;

    if (takeup_fullscreen)
		{
		ShowCursor(FALSE); 
		//if running in a window, we use the system cursor.
		}
    DragAcceptFiles(whandle, i4_T);
  }
  else
    i4_error("FATAL: failed to create window. Out of resources?");

  if (!i4_win32_window_handle)
    i4_win32_window_handle=whandle;

  //myPid=GetCurrentProcess();
  //SetPriorityClass(myPid, HIGH_PRIORITY_CLASS);

  if (takeup_fullscreen)
  {
    confine.on=i4_T;
    confine.x1=0;
    confine.y1=0;
    confine.x2=w-1;
    confine.y2=h-1;
    
    ShowWindow(whandle, SW_SHOWNORMAL);
    
    /*
    SetWindowPos(whandle,
                 HWND_TOPMOST,
                 0,0,
                 GetSystemMetrics(SM_CXSCREEN),
                 GetSystemMetrics(SM_CYSCREEN),
                 SWP_SHOWWINDOW);
                 */
  }
  else
	  {
      //ShowWindow(whandle, SW_SHOW);
	  //DirectDrawCreateClipper(0,&dx5_common.lpddclipper,0);
	  //dx5_common.lpddclipper->SetHWnd(0,whandle);
	  
	  }
  ShowWindow(whandle,SW_SHOW);
  UpdateWindow(whandle);
  fullscreen=takeup_fullscreen;
  return i4_T;
}

void win32_input_class::destroy_window()
{
  if (have_window)
  {
    PostMessage(whandle, WM_USER, 0,0);  // send message to thread to unblock it

//     stop_thread=i4_T;
//     while (!thread_running)
//       Sleep(0);

    have_window=i4_F;
 
    if (i4_win32_window_handle==whandle)
      i4_win32_window_handle=0;
	
    current_window_handle=0;
    DestroyWindow(whandle);

    i4_kernel.remove_device(this);
    win32_current_input=0;

//		if(lpdiKey) lpdiKey->Release(), lpdiKey=NULL;
//		if(lpdi) lpdi->Release(), lpdi=NULL;

	if (fullscreen)
		{
		ShowCursor(TRUE);
		}
  }



}

long FAR PASCAL WindowProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{

  if (win32_current_input)
    return win32_current_input->process(hWnd, message, wParam, lParam, 0);
  else

    return DefWindowProc(hWnd, message, wParam, lParam);

}

win32_input_class::win32_input_class(char *extra_message)
{
  mouse_locked   = i4_F;
  async_mouse    = 0;
  modifier_state = 0;
  have_window    = i4_F;
  active         = i4_F;
  
  if (extra_message && (strlen(extra_message) < 128))
    strcpy(window_message,extra_message);
  else
    window_message[0]=0;
}


void win32_input_class::set_async_mouse(dx_threaded_mouse_class *m)
{  
  //if (async_mouse)
  //{
    //dont let this mouse references us anymore
    //    async_mouse->input = 0;
  //}

  async_mouse = m;

  //if (async_mouse)
  //{    
    //    async_mouse->input = this;
    //    async_mouse->set_active(active);
  //}
}

w16 win32_input_class::translate_windows_key(w16 wkey, w32 &m)
{
  w16 key;
  m=0;
  
  switch (wkey)
  {
    case VK_BACK     : key=I4_BACKSPACE; break;
    case VK_TAB      : key=I4_TAB;       break;
    case VK_RETURN   : key=I4_ENTER;     break;
    case VK_ESCAPE   : key=I4_ESC;       break;
    case VK_SPACE    : key=I4_SPACE;     break;
    case VK_UP       : key=I4_UP;        break;
    case VK_DOWN     : key=I4_DOWN;      break;
    case VK_LEFT     : key=I4_LEFT;      break;
    case VK_RIGHT    : key=I4_RIGHT;     break;
    case VK_CONTROL  :
    case VK_LCONTROL : key=I4_CTRL_L;  m=I4_MODIFIER_CTRL_L;  break;
    case VK_RCONTROL : key=I4_CTRL_R;  m=I4_MODIFIER_CTRL_R;  break;
    case VK_MENU     :
    case VK_LMENU    : key=I4_ALT_L;   m=I4_MODIFIER_ALT_L;  break;
    case VK_RMENU    : key=I4_ALT_R;   m=I4_MODIFIER_ALT_R;  break;
    case VK_SHIFT    :
    case VK_LSHIFT   : key=I4_SHIFT_L; m=I4_MODIFIER_SHIFT_L; break;
    case VK_RSHIFT   : key=I4_SHIFT_R; m=I4_MODIFIER_SHIFT_R; break;
    case VK_CAPITAL  : key=I4_CAPS;      break;
    case VK_NUMLOCK  : key=I4_NUM_LOCK;  break;
    case VK_HOME     : key=I4_HOME;      break;
    case VK_END      : key=I4_END;       break;
    case VK_DELETE   : key=I4_DEL;       break;
    case VK_F1       : key=I4_F1;        break;
    case VK_F2       : key=I4_F2;        break;
    case VK_F3       : key=I4_F3;        break;
    case VK_F4       : key=I4_F4;        break;
    case VK_F5       : key=I4_F5;        break;
    case VK_F6       : key=I4_F6;        break;
    case VK_F7       : key=I4_F7;        break;
    case VK_F8       : key=I4_F8;        break;
    case VK_F9       : key=I4_F9;        break;
    case VK_F10      : key=I4_F10;       break;
    case VK_F11      : key=I4_F11;       break;
    case VK_F12      : key=I4_F12;       break;
    case VK_F13      : key=I4_F13;       break;
    case VK_F14      : key=I4_F14;       break;
    case VK_F15      : key=I4_F15;       break;
    case VK_INSERT   : key=I4_INSERT;    break;
    case VK_PRIOR    : key=I4_PAGEUP;    break;
    case VK_NEXT     : key=I4_PAGEDOWN;  break;    

    case VK_NUMPAD0  : key=I4_KP0;       break;
    case VK_NUMPAD1  : key=I4_KP1;       break;
    case VK_NUMPAD2  : key=I4_KP2;       break;
    case VK_NUMPAD3  : key=I4_KP3;       break;
    case VK_NUMPAD4  : key=I4_KP4;       break;
    case VK_NUMPAD5  : key=I4_KP5;       break;
    case VK_NUMPAD6  : key=I4_KP6;       break;
    case VK_NUMPAD7  : key=I4_KP7;       break;
    case VK_NUMPAD8  : key=I4_KP8;       break;
    case VK_NUMPAD9  : key=I4_KP9;       break;

    case VK_DIVIDE   : key=I4_KPSLASH;   break;
    case VK_MULTIPLY : key=I4_KPSTAR;    break;
    case VK_SUBTRACT : key=I4_KPMINUS;   break;
    case VK_ADD      : key=I4_KPPLUS;    break;
    case VK_DECIMAL  : key=I4_KPPERIOD;  break;
/*  Hardcoding this is not a really good idea. 
Newer seen an international keyboard? 
		For now, we don't bother about this. 
		Gotta use DirectInput*/
    case 0xc0        : key='`'; break;
    case 0xba        : key=';'; break;
    case 0xbb        : key='='; break;
    case 0xbc        : key=','; break;
    case 0xbd        : key='-'; break;
    case 0xbe        : key='.'; break;
    case 0xbf        : key='/'; break;
    case 0xdb        : key='['; break;
    case 0xdc        : key='\\'; break;
    case 0xdd        : key=']'; break;
    case 0xde        : key='\''; break;


	/*case VK_OEM_1:
	case VK_OEM_2:
	case VK_OEM_3:
	case VK_OEM_4:
	case VK_OEM_5:
	case VK_OEM_6:
	case VK_OEM_7:
	case VK_OEM_8:

		short sh=GetKeyState(VK_SHIFT);
		
		break;
		*/
    default :
      if ((wkey>='A' && wkey<='Z') || (wkey>='0' && wkey<='9'))
        key = wkey; 
      else
		  
        key = 0;//if it's something else, wait for the WM_CHAR message
      break;
  }
  return key;
}

i4_bool win32_input_class::lock_mouse_in_place(i4_bool yes_no)
{
  //if (async_mouse)
    //    async_mouse->lock_position(yes_no);

  mouse_locked=yes_no;
  return i4_T;
}

void win32_input_class::update_mouse_movement(sw32 new_x, sw32 new_y)
{   
  if (!active)
    return;

  i4_mouse_move_event_class ev((short)mouse_x, (short)mouse_y, (short)new_x, (short)new_y);
    
  int mx,my;

  mx = new_x;
  my = new_y;

  if (confine.on)
  {
    if (mx<confine.x1) 
    {
      mx=confine.x1;
      SetCursorPos(mx, my);
    }
    if (mouse_y<confine.y1) 
    {
      my=confine.y1;
      SetCursorPos(mx, my);
    }

    if (mouse_x>confine.x2) 
    {
      mx=confine.x2;
      SetCursorPos(mx, my);
    }
    if (mouse_y>confine.y2) 
    {
      my=confine.y2;
      SetCursorPos(mx, my);
    }
  }
       
  send_event_to_agents(&ev,i4_device_class::FLAG_MOUSE_MOVE);

  if (mouse_locked)
  {
    if (mx!=mouse_x || my!=mouse_y)
      SetCursorPos(mouse_x, mouse_y);
   }
  else
  {
    mouse_x=mx;
    mouse_y=my;
  }    
}

void win32_input_class::update_mouse_buttons(i4_mouse_button_event_class::btype type,
                                             mouse_button_state state)
{
  if (!active)
    return;
  
  i4_time_class now;
  if (state==UP)
  {
    i4_mouse_button_up_event_class ev_up(type, (short)mouse_x, (short)mouse_y, now, last_up[type]);
    send_event_to_agents(&ev_up,i4_device_class::FLAG_MOUSE_BUTTON_UP);
    last_up[type]=now;
  }
  else
  if (state==DOWN)
  {
	i4_window_class *m=i4_kernel.modal();
	if (m)
		{//ignore down-events outside the modal window.
		//don't ignore up-events, as user might be dragging something out.
		//last_down[type]=now;
		if (mouse_x<m->x()||mouse_y<m->y()) return;
		if ((mouse_x>m->x()+m->width())||(mouse_y>m->y()+m->height())) return;
		}
    i4_mouse_button_down_event_class ev_down(type, (short)mouse_x, (short)mouse_y, now, last_down[type]);
    send_event_to_agents(&ev_down,i4_device_class::FLAG_MOUSE_BUTTON_DOWN);
    last_down[type]=now;
  }    
}


sw32 win32_input_class::process(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, DWORD time)
{
  PAINTSTRUCT ps;
  HDC         hdc;
  static i4_bool initbitmapdrawn=i4_F;
  if (!have_window)
    return DefWindowProc(hWnd, message, wParam, lParam);

  switch( message )
  {
    case WM_DROPFILES :
    {
      HANDLE drop=(HANDLE) wParam;
      POINT point;
      DragQueryPoint((HDROP)drop, &point);

      i4_window_got_drop_class de(0);

      int t=DragQueryFile((HDROP)drop, 0xffffffff, 0,0);
      de.drag_info.t_filenames=t;
      de.drag_info.filenames=(i4_str **)I4_MALLOC(sizeof(i4_str *) * t, "");
      de.drag_info.drag_object_type=i4_drag_info_struct::FILENAMES;
      de.drag_info.x=(short)point.x;
      de.drag_info.y=(short)point.y;

      for (int i=0; i<t; i++)
      {
        char buf[500];
        DragQueryFile((HDROP)drop, i, buf, 500);
        de.drag_info.filenames[i]=new i4_str(buf);
      }

      DragFinish((HDROP)drop);

      send_event_to_agents(&de, i4_device_class::FLAG_DRAG_DROP_EVENTS);

      SetFocus(hWnd);

    } break;
      
    case WM_PAINT :
	  BeginPaint(hWnd,&ps);
      if ((!initbitmapdrawn)||(!active))
		  {
		  HANDLE him;
		  HDC hdcImage;
		  hdc=GetDC(hWnd);
		  him=LoadImage(i4_win32_instance,"bitmaps/loading.bmp",IMAGE_BITMAP,640,480,LR_LOADFROMFILE|LR_CREATEDIBSECTION);
		  if (him)
			  {
			  hdcImage = CreateCompatibleDC( NULL );
              SelectObject( hdcImage, him );
			  BitBlt( hdc, 0, 0, 640, 480, 
                 hdcImage, 0, 0, SRCCOPY );
			  DeleteDC(hdcImage);
			  DeleteObject(him);
			  initbitmapdrawn=i4_T;
			  }
		  
		  ReleaseDC(hWnd,hdc);
		  }
      if (window_message[0])
      {
        
        hdc = GetDC(hWnd);
        TextOut(hdc,0,0,window_message,strlen(window_message));        
        ReleaseDC(hWnd,hdc);
      }
	  EndPaint(hWnd,&ps);
      redraw();
	  
      break;

	
/*#if 0
    case WM_SETCURSOR:
    {
      if (win32_display_instance.accel->remove_cursor())
      {
        ShowCursor(FALSE);
        return TRUE;
      }
    } break;
#endif*/

	/*case WM_POWERBROADCAST: //DX7 needed this, DX8 is capable of suspending almost anything
		DWORD dwPowerEvent;
		dwPowerEvent=(DWORD)wParam;
		DWORD dwData;
		dwData=(DWORD)lParam;
		switch (dwPowerEvent)
			{
			case PBT_APMQUERYSUSPEND:
				return BROADCAST_QUERY_DENY;
			}
		return 0;*/
	case WM_GETMINMAXINFO:
		{
		LPMINMAXINFO mmi;
		RECT rc;
		mmi=(LPMINMAXINFO)lParam;
		mmi->ptMinTrackSize.x=320;
		mmi->ptMinTrackSize.y=200;
		//set upper limit to sizeof (backbuffer-surface)
		//That one is potentially smaller than the screen as 
		//full-screen 3D capabilities may not be available (vmem is finite)
		rc.top=0;
		rc.left=0;
		rc.bottom=i4_win32_startup_options.yres;
		rc.right=i4_win32_startup_options.xres;
		AdjustWindowRect(&rc,WS_CAPTION|WS_THICKFRAME|WS_SYSMENU,FALSE);
		mmi->ptMaxTrackSize.x=rc.right-rc.left;
		//left has become a negative value: the size the border would need to 
		//go over the left side if the client upper left corner where at (0,0)
		//But that doesn't care us here. All we need to know is the size.
		mmi->ptMaxTrackSize.y=rc.bottom-rc.top;
		//top and left are negative, so adding them increases size.
		mmi->ptMaxSize.x=rc.right-rc.left;
		mmi->ptMaxSize.y=rc.bottom-rc.top;
		}
		break;

	case WM_DISPLAYCHANGE:
		{
		//Needs testing
		w16 x=LOWORD(lParam), y=HIWORD(lParam);
		RECT r;
		//be aware that this can also happen if some other directx app
		//changes the resolution.
		GetClientRect(hWnd,&r);
		if (r.bottom<y) y=(w16) r.bottom;
		if (r.right<x) x=(w16) r.right;
		if ((x>0)&&(y>0)) 
			{
		resize(x,y);
		if (display->change_mode(x,y,
			(short)wParam,I4_ADJUST_FRAMEBUFFER_ONLY))
			//Returns false if nothing (to be) changed
			{
			i4_display_change_event_class dcsize(display,i4_display_change_event_class::SIZE_CHANGE);
			send_event_to_agents(&dcsize,i4_device_class::FLAG_DISPLAY_CHANGE);
			i4_display_change_event_class dcpal(display,i4_display_change_event_class::PALETTE_CHANGE);
			send_event_to_agents(&dcpal, i4_device_class::FLAG_DISPLAY_CHANGE);
			}
			}
		else
			{
			display->current_mode()->bits_per_pixel=(w8)wParam;//remember for next call of change_mode (on window restore)
			i4_display_change_event_class dcpalette(display,i4_display_change_event_class::PALETTE_CHANGE);
			send_event_to_agents(&dcpalette, i4_device_class::FLAG_DISPLAY_CHANGE);
			}
		
		}break;
    case WM_SIZE :
		{//needs testing
		w16 x=LOWORD(lParam), y=HIWORD(lParam);
		if (x!=display->current_mode()->xres&&
			y!=display->current_mode()->yres&&
			(x>0)&&(y>0))//(x==0)&&(y==0) means we just got minimized.
			//must not pass this around as directdraw3::CreateSurface() doesn't
			//like 0x0 surfaces. 
			{
		resize(x, y);	  
		if (display->change_mode(x, y,
			display->current_mode()->bits_per_pixel,
			I4_ADJUST_FRAMEBUFFER_ONLY))
			{
			//i4_current_app->get_root_window()->reparent(display->get_screen(),0);
			i4_display_change_event_class dc(display,i4_display_change_event_class::SIZE_CHANGE);
			send_event_to_agents(&dc, i4_device_class::FLAG_DISPLAY_CHANGE);
			}
			}
        
		}
      break;

    case WM_MOVE :
      move(LOWORD(lParam), HIWORD(lParam));
      break;


    case WM_MOUSEMOVE :
    {
      if(!async_mouse)
        update_mouse_movement(LOWORD(lParam),HIWORD(lParam));
    } break; 

    case WM_LBUTTONDOWN :
    {
      if (!async_mouse)
        update_mouse_buttons(i4_mouse_button_event_class::LEFT, DOWN);
    } break;

    case WM_LBUTTONUP :
    {
      if (!async_mouse)
        update_mouse_buttons(i4_mouse_button_event_class::LEFT, UP);
    } break;


    case WM_RBUTTONDOWN :
    {
      if (!async_mouse)
        update_mouse_buttons(i4_mouse_button_event_class::RIGHT, DOWN);
    } break;

    case WM_RBUTTONUP :
    {
      if (!async_mouse)
        update_mouse_buttons(i4_mouse_button_event_class::RIGHT, UP);
    } break;

    case WM_MBUTTONDOWN :
    {
      if (!async_mouse)
        update_mouse_buttons(i4_mouse_button_event_class::CENTER, DOWN);
    } break;

    case WM_MBUTTONUP :
    {
      if (!async_mouse)
        update_mouse_buttons(i4_mouse_button_event_class::CENTER, UP);
    } break;

    case WM_SETFOCUS:
		{
		set_active(i4_T);
		i4_time_class t(time);
		i4_key_release_event_class ev(0,0, 0, t);
		send_event_to_agents(&ev,i4_device_class::FLAG_KEY_RELEASE);
		modifier_state=0;
		li_call("redraw_all");
//       if (async_mouse)
//         async_mouse->set_active(i4_T);
		}
    break;

    case WM_KILLFOCUS:  
		{
      set_active(i4_F);
      i4_time_class t(time);
      //Signal to any listener that we lost the focus and nothing is pressed any more
	  //Need this 'cause if we loose focus between WM_KEYDOWN and WM_KEYUP,
	  //The second message doesn't get sent to our window.
      i4_key_release_event_class ev(0,0, 0, t);
      send_event_to_agents(&ev,i4_device_class::FLAG_KEY_RELEASE);
	  modifier_state=0;
//       if (async_mouse)
//         async_mouse->set_active(i4_F);
		}
    break;


    case WM_ACTIVATEAPP :
      set_active((i4_bool)wParam);
//       if (async_mouse)
//         async_mouse->set_active((i4_bool)wParam);
    break;

    case WM_SYSCOMMAND:
    {
      switch (wParam)
      {
	  case SC_CLOSE:
			  {
        i4_display_close_event_class dc(display);
        send_event_to_agents(&dc, i4_device_class::FLAG_DISPLAY_CLOSE);
        close_event();
        return 0;
			  
	   
			  }
	  }
	  return DefWindowProc(hWnd, message, wParam, lParam);

    }

    case WM_DESTROY:
    {
      return 0;
    } ;

	case WM_CHAR :
		{
		//i4_warning("Got WM_CHAR %i",wParam);
		i4_time_class t(time);//remember: Time has currently no meaning
		if (wParam>255) return 0;//Anything Bogus
		char c=(char)wParam;
		i4_char_send_event_class ev(c,t);
		send_event_to_agents(&ev,i4_device_class::FLAG_CHAR_SEND);
		} break;
    case WM_SYSKEYDOWN :
    case WM_KEYDOWN :
    {
      w32 m;
	  //i4_warning("Got WM_KEYDOWN %i.",wParam);
      w16 key_code=translate_windows_key(wParam, m);
      modifier_state|=m;
	  //if (key_code==0) break;//Nothing useful for now->Wait for WM_CHAR
	  //Problem with WM_CHAR: There's no WM_UPCHAR message...
      w16 key=i4_key_translate(key_code, 1, modifier_state);
      i4_time_class t(time);
      if ((key_code==0) && (key==0)) break;
      i4_key_press_event_class ev(key,key_code, modifier_state, t);
      send_event_to_agents(&ev,i4_device_class::FLAG_KEY_PRESS);
    } break;

    case WM_SYSKEYUP :
    case WM_KEYUP :
    {
      w32 m;
	  //i4_warning("Got WM_KEYUP %i.",wParam);
      w16 key_code=translate_windows_key(wParam,m);
      modifier_state&=~m;
	  
      w16 key=i4_key_translate(key_code, 0, modifier_state);
      i4_time_class t(time);
	  if ((key_code==0)&&(key==0)) break; //would signal WM_KILLFOCUS
      i4_key_release_event_class ev(key,key_code, modifier_state, t);
      send_event_to_agents(&ev,i4_device_class::FLAG_KEY_RELEASE);
    } break;
	case MM_MCINOTIFY:
		{
		i4_user_message_event_class nev(wParam);
		i4_kernel.send_event(i4_movie,&nev);
		}
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);

  }

  return 0;
}



// BOOL win32_input_class::CreateKeyboard(GUID &guid, LPDIRECTINPUTDEVICE& lpdiKey, DWORD dwAccess)
// {
/*  HRESULT err=lpdi->CreateDevice(guid, &lpdiKey, NULL);

  if(err!=DI_OK)
  {
    i4_warning("Unable to create DirectInput keyboard device!\n\0");
    goto fail;
  }
  
  // Tell DirectInput that we want to receive data in keyboard format
  err=lpdiKey->SetDataFormat(&c_dfDIKeyboard);
  if(err!=DI_OK)
  {
    i4_warning("Unable to access DirectInput device as a keyboard!\n\0");
    goto fail;
  }
  // set desired access mode
  err=lpdiKey->SetCooperativeLevel(current_window_handle, dwAccess);
  if(err != DI_OK)
  {
    i4_warning("Unable to set DirectInput keyboard cooperativity level!\n\0");
    goto fail;
  }
  return TRUE;
fail:
  if(lpdiKey) lpdiKey->Release(), lpdiKey=0; */
//  return FALSE;
//}

/*BOOL win32_input_class::InitDInput(void)
{
  HRESULT err;
  GUID    guid=GUID_SysKeyboard;

  err=DirectInputCreate(i4_win32_instance, DIRECTINPUT_VERSION, &lpdi, NULL);
  if(err!=DI_OK)
  {
    i4_warning("Unable to create DirectInput object!\n\0");
    return FALSE;
  }
  i4_warning("DirectInput object initialized...\n\0");
  
  // Create a keyboard obj
	if(CreateKeyboard(guid, lpdiKey, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND))
  {
    lpdiKey->Acquire();
    i4_warning("Keyboard found and acquired...\n\0");

    return TRUE;
  }
  if(lpdiKey) lpdiKey->Release(), lpdiKey=NULL;
  if(lpdi) lpdi->Release(), lpdi=NULL; 
  return FALSE;
} */

