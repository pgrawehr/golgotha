/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/
/* This is new Code */

//most of this code is os-dependent, only Win32 is implemented right now
#include "pch.h"
#include "lisp/li_init.h"
#include "lisp/lisp.h"
#include "init/init.h"
#include "string/string.h"
#include "video/movie_engine.h"
#include "device/kernel.h"
#include "device/event.h"
#include "sound/sound.h"
#ifdef _WINDOWS
#include <mmsystem.h>
#include "main/win_main.h"
#endif

i4_movie_engine_class *i4_movie=0;

class i4_win32_movie_engine_class: public i4_movie_engine_class, public i4_init_class
	{
	protected:
		i4_event_reaction_class *notify_who;
		i4_bool waitfornotify;
	public:
	i4_win32_movie_engine_class():notify_who(0),i4_init_class(),i4_movie_engine_class()
		{
		waitfornotify=i4_F;
		};
	void name(char* buffer){static_name(buffer,"win32 movie engine");};
	virtual void init()
		{
		if (mciSendString("open avivideo", NULL, 0, NULL) != 0)
			{
			i4_warning("WARNING: Could not permanently initialize digital video (AVI) library.");
			//not a big problem, just loads the driver later.
			}
		i4_movie=this;
		}
	virtual void uninit()
		{
		if (notify_who)
			{
			i4_kernel.send(notify_who);//still something in progress, kill it
			delete notify_who;
			notify_who=0;
			mciSendString("close golgothaplaybackfile",0,0,0);
			}
		waitfornotify=i4_F;
		mciSendString("close avivideo",NULL,0,NULL);
		//Close all open devices
		mciSendCommand(MCI_ALL_DEVICE_ID, MCI_CLOSE, MCI_WAIT, NULL);
		}
	virtual w32 play(const i4_const_str &file)
		{
		char buf[MAX_PATH];
		char cmd[MAX_PATH*2];
		char ret[MAX_PATH*2];
		DWORD error;
		w32 retval=MOVIE_SUCCESS;//Assume everything fine.
		i4_os_string(file,buf,MAX_PATH);
		wsprintf(cmd,"open %s alias golgothaplaybackfile",buf);
		i4_warning("Attempting to open %s for playback.",buf);
		error=mciSendString(cmd,ret,MAX_PATH*2,NULL);
		if (error)
			{
			mciGetErrorString(error,ret,MAX_PATH*2);
			i4_warning("ERROR: Open of %s failed: %s",buf,ret);
			mciSendString("close golgothaplaybackfile",0,0,0);
			
			return MOVIE_OPENFAILED;
			}
		//mciSendString("break golgothaplaybackfile on 27",0,0,0);
		i4_warning("Attempting to start playback.");
		mciSendString("set golgothaplaybackfile time format ms",0,0,0);
		error=mciSendString("status golgothaplaybackfile length",ret,MAX_PATH*2,0);
		if (error)
			{
			mciSendString("close golgothaplaybackfile",0,0,0);
			return MOVIE_UNKNOWNERROR;
			}
		w32 ms=atol(ret);
		w32 currms=0;
		MSG msg;
		if (error=mciSendString("play golgothaplaybackfile from 0",ret,MAX_PATH*2,i4_win32_window_handle))
			{
			mciGetErrorString(error,ret,MAX_PATH*2);
			i4_warning("ERROR: Playback of %s failed: %s",buf,ret);
			retval=MOVIE_PLAYBACKFAILED;	
			};
		for (;;)
			{
			//if (currms+2000>ms) break;
			mciSendString("status golgothaplaybackfile position",ret,MAX_PATH*2,0);
			currms=atol(ret);
			if (currms>=(ms-10)) break;
			if (PeekMessage(&msg,0,WM_KEYDOWN,WM_KEYDOWN,PM_REMOVE))
				break;
			//currms+=500;
			Sleep(500);
			}
		//GetMessage(&msg,0,WM_KEYUP,WM_KEYUP);//Risk of deadlock is greater
		//than risk of receiving a wrong upkey message.
		i4_warning("Closing file.");
		mciSendString("stop golgothaplaybackfile",0,0,0);
		mciSendString("close golgothaplaybackfile",0,0,0);
		/*MCI_DGV_OPEN_PARMS    mciOpen; 
		
		mciOpen.lpstrElementName = buf;  // Set the filename.
		mciOpen.dwStyle = WS_CHILD|WS_BORDER;            // Set the style. 
		mciOpen.hWndParent = hWnd;             // Give a window handle. 
		
		if (mciSendCommand(0, MCI_OPEN, 
			(DWORD)(MCI_OPEN_ELEMENT|MCI_DGV_OPEN), 
			(DWORD)(LPSTR)&mciOpen) == 0)
			{ 
			// Open operation is successful. Continue. 
			HWND hwnd; 
			MCI_DGV_RECT_PARMS mciRect; 
			DWORD wDeviceID=mciOpen.wDeviceID;
			// Get the movie dimensions with MCI_WHERE. 
			
			mciSendCommand(wDeviceID, MCI_WHERE, MCI_DGV_WHERE_SOURCE, 
				(DWORD)(LPSTR)&mciRect); 
			
			// Create the playback window. Make it bigger for the border. 
			// Note that the right and bottom members of RECT structures in MCI 
			// are unusual; rc.right is set to the rectangle's width, and 
			// rc.bottom is set to the rectangle's height.
			
			hwndMovie = CreateWindow("mywindow", "Playback", 
				WS_CHILD|WS_BORDER, 0,0, 
				mciRect.rc.right+(2*GetSystemMetric(SM_CXBORDER)),
				mciRect.rc.bottom+(2*GetSystemMetric(SM_CYBORDER)), 
				i4_win32_window_handle, i4_win32_instance, NULL); 
			
			if (hwndMovie){ 
				// Window created OK; make it the playback window. 
				
				MCI_DGV_WINDOW_PARMS mciWindow; 
				
				mciWindow.hWnd = hwndMovie; 
				mciSendCommand(wDeviceID, MCI_WINDOW, MCI_DGV_WINDOW_HWND, 
					(DWORD)(LPSTR)&mciWindow); 
				MCI_DGV_PLAY_PARMS mciPlay;    // play parameters 
				DWORD dwFlags = 0; 
				
				// Check dwFrom. If it is != 0 then set parameters and flags. 
				mciPlay.dwFrom = 0; // set parameter 
				dwFlags |= MCI_FROM;     // set flag to validate member 
				 
				
				// Send the MCI_PLAY command and return the result. 
				return mciSendCommand(wDevID, MCI_PLAY, dwFlags, 
					(DWORD)(LPVOID)&mciPlay); 
				
				} 
			
			} */
		return retval;
		
		}

	virtual w32 background_play(i4_const_str &file, i4_event_reaction_class* notify_ev)
		{
		char buf[MAX_PATH];
		char cmd[MAX_PATH*2];
		char ret[MAX_PATH*2];
		if (notify_who) return MOVIE_PLAYBACKINPROGRESS; //cannot play another file while still processing anything
		
		i4_os_string(file,buf,MAX_PATH);
		sprintf(cmd,"open %s alias golgothaplaybackfile",buf);
		if (mciSendString(cmd,ret,MAX_PATH*2,NULL))
			{
			i4_error("ERROR: Open of %s failed: %s",buf,ret);
			notify_who=0;
			return MOVIE_OPENFAILED;
			}
		mciSendString("break golgothaplaybackfile on 27",0,0,0);
		if (mciSendString("play golgothaplaybackfile from 0 fullscreen notify",ret,MAX_PATH*2,i4_win32_window_handle)!=0)
			{
			
			notify_who=0;
			i4_warning("ERROR: Could not play %s: %s",buf,ret);
			mciSendString("close golgothaplaybackfile",0,0,0);
			return MOVIE_PLAYBACKFAILED;
			}
		notify_who=notify_ev->copy();
		//mciSendString("close golgothaplaybackfile",0,0,0);
		return MOVIE_SUCCESS;
		}
	virtual void background_stop()
		{
		mciSendString("close golgothaplaybackfile",0,0,0);
		if (notify_who) delete notify_who;
		notify_who=0;
		}
	virtual void receive_event(i4_event *ev)//gets messages from the windows message queue
		{
		int msg=0;
		if (ev->type()==i4_event::USER_MESSAGE)
			{
			msg=((i4_user_message_event_class *)ev)->sub_type;
			//if (msg==MCI_NOTIFY_SUPERSEEDED)//Ignore this message
			//	return;
			//i4_user_message_event_class uev(msg);
			i4_kernel.send(notify_who);
			if (notify_who) delete notify_who;
			notify_who=0;
			return;
			}
		i4_movie_engine_class::receive_event(ev);
		}
	~i4_win32_movie_engine_class()
		{
		}
		
	}i4_win32_movie_engine_class_instance;

LI_HEADER(play_video)
	{
	//This code is ANYTHING but time critical
	char *path=li_get_string(li_first(o,env),env);
	
	if (i4_movie) 
		{
		i4_bool snd=i4_sound_man->is_enabled();
		/*if (snd) 
			{
			i4_sound_man->disable_sound();
			Sleep(500);//Wait until buffer is empty
			}*/
		w32 retval=i4_movie->play(path);
		//if (snd) i4_sound_man->enable_sound();
		return new li_int(retval);
		}
	//Warning: Exspect DDERR_SURFACELOST after this.
	return li_nil;
	}

li_automatic_add_function(li_play_video,"play_video");
