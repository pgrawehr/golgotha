/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef I4_WIN_MAIN_HH
#define I4_WIN_MAIN_HH


//#include <windows.h>

// globals that other windows routines may need access to

extern HINSTANCE i4_win32_instance;  // instance of the application passed in through main
extern int       i4_win32_nCmdShow;  // how the window was initially opened (minimized?)
extern HWND      i4_win32_window_handle;    // main window, for some reason direct sound needs this

//Options saved in the registry to be used on next restart.
//There should be exactly one instance of this struct in the app.
//It is named i4_win32_startup_options.
//Warning: The name is missleading, the structure is used
//for all platforms.
#define R1_RENDER_NONE 0  /* No Rendering device will be initialized */
#define R1_RENDER_DIRECTX5 1
#define R1_RENDER_OPENGL 2
#define R1_RENDER_GLIDE 3
#define R1_RENDER_UNKNOWN_SOFTWARE 4
#define R1_RENDER_DIRECTX5_SOFTWARE 5
#define R1_RENDER_GDI_SOFTWARE 6
#define R1_RENDER_X11_SOFTWARE 7
#define R1_RENDER_DIRECTX5_USER_SETTING 8 /* To allow individual render settings */
#define R1_RENDER_DIRECTX8 9
#define R1_RENDER_DIRECTX8_USER_SETTING 10
#define R1_RENDER_DIRECTX9 11 //the all-default now
#define R1_RENDER_DIRECTX9_HAL 12 //Actually same as previous
#define R1_RENDER_DIRECTX9_REF 13 //Will always choose the ref device.
#define R1_RENDER_USEDEFAULT -1

class i4_win32_startup_options_struct
{
public:
  char fullscreen;
  GUID guid_screen;
  GUID guid_sound;
  short int xres;
  short int yres;
  short int bits;
  char use3dsound;
  char use2dsound;
  int volume;
  char netfirst;
  int max_texture_quality;
  int max_view_distance;
  int render;
  char *render_data;
  int render_data_size;
  int texture_bitdepth;//0= Choose wisely
  char stereo;
  int stereoport;
  float eyedifference;//not yet used (hardcoded now)
  
  i4_win32_startup_options_struct()
	  {//Gives default values
    fullscreen=1;
	xres=640;
	yres=480;
	bits=16;
	volume=63;
	max_texture_quality=256;
	max_view_distance=200;
	texture_bitdepth=0;
	stereoport=0x03BC;
	eyedifference=0.3f;
	use3dsound=true;//requires use2dsound to be true;
	use2dsound=true;//if both are false, no sound is played at all.
	netfirst=true;
	render=R1_RENDER_USEDEFAULT;
	render_data=0;
	render_data_size=0;
	stereo=false;
	ZeroMemory(&guid_sound,sizeof(GUID));
	ZeroMemory(&guid_screen,sizeof(GUID));
	  }
    ~i4_win32_startup_options_struct();
  
  void check_option(char *opt);//changes are applied (from registry or command-line)
  void check_option(w32 argc,i4_const_str* argv);
  int save_option(void);//write changes to registry
};

extern i4_win32_startup_options_struct i4_win32_startup_options;

#endif
