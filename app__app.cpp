// jc@crack.com
/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/
#include "pch.h"
#include "app/app.h"
#include "window/wmanager.h"
#include "time/profile.h"
#include "app/registry.h"
#include "threads/threads.h"
#include "main/win_main.h"
//#include <windows.h>
#include "resource.h"
#include "memory/new.h"
#include "app/cdatafile.h"
CDataFile *inifile=0;
//void *inifile=0;



//#include "device/device.hh"
i4_profile_class pf_app_calc_model("app::calc_model");
i4_profile_class pf_app_refresh("app::refresh");
i4_profile_class pf_app_get_input("app::get_input");


i4_application_class *i4_current_app=0;

i4_parent_window_class *i4_application_class::get_root_window()
{
  return wm;
}

i4_graphical_style_class *i4_application_class::get_style()
{
  if (wm)
    return wm->get_style();
  else return 0;
}

void i4_application_class::receive_event(i4_event *ev)
{
  if (ev->type()==i4_event::DISPLAY_CLOSE)
    finished=i4_T;
}
extern w32 x86_get_cpu_type(); // <----------- ADDED BY JJ
void i4_application_class::run()
{  
  i4_current_app=this;
  init();
  finished=i4_F; 
  int restore_priority=0;

  do
  {
    //i4_warning("Main Loop Start...");
    pf_app_calc_model.start();
    calc_model();    
    pf_app_calc_model.stop();

    pf_app_refresh.start();
    refresh();
    pf_app_refresh.stop();
    
    if (restore_priority)
    {
      i4_set_thread_priority(i4_get_thread_id(), I4_THREAD_PRIORITY_NORMAL);
      restore_priority=0;
    
      
    }
    
    pf_app_get_input.start();
    int repeat;
    i4_kernel.events_sent=0;	 // PG What the HELL is this good for?
								 // Do we really have so much spare time?
    // Slowerer for AMD          // <------ ADDED BY JJ
    //if(x86_get_cpu_type() == 3)  // <------ ADDED BY JJ
    //{                            // <------ ADDED BY JJ
    //    Sleep(1);                // <------ ADDED BY JJ
    //}                            // <------ ADDED BY JJ
    // Slowerer for AMD          // <------ ADDED BY JJ
    do 
    {
      repeat=0;
	  
      get_input();
      if (!finished)
      {
        if (display->display_busy() || 
            (idle() && i4_kernel.events_sent==0 && 
             !wm->need_redraw() &&
             display->get_context()->both_dirty->empty()))
        {
          repeat=1;
          
          if (!restore_priority)
          {
            restore_priority=1;
            i4_set_thread_priority(i4_get_thread_id(), I4_THREAD_PRIORITY_LOW);
          }
          
          i4_thread_yield();
        }
      }
    }
    while (repeat);
    //i4_warning("Geting Input End...");
    pf_app_get_input.stop();

    //i4_warning("Main Loop End...");
  } while (!finished);
  uninit();
  i4_current_app=0;
}


void i4_application_class::calc_model()
{}

// get_input polls the hardware for changes and reports them as events to event_handlers
void i4_application_class::get_input()
{
  i4_kernel.process_events();

}
extern w32 g1_disable_all_drawing;
void i4_application_class::refresh()
{
  if (g1_disable_all_drawing>1)
	  return;
  wm->root_draw();
}


// if an exact match is not found the closest width and height are return
// Returns only modes matching the desired bitdepth
i4_display_class::mode *i4_application_class::find_mode(w16 &width, w16 &height, w16 bits, int driver_id)
{
  sw32 closest_dist=0xfffffff;
  w16 closest_width=0,
      closest_height=0;

  i4_display_class::mode *use=display->get_first_mode(driver_id);

  if (!use)
    return 0;//Coudn't find ANY mode!
  do
  {

    // if we are opening a window and we can set the xres and yres, 
    //   then make them fit as best we can to the suggested width and height

    if (use->flags & i4_display_class::mode::RESOLUTION_DETERMINED_ON_OPEN)
    {
	//don't try to open a window larger than the screen.
	//if the above flag is set, the mode's with and height give
	//the maximum window size
      if (use->xres<=width) width=use->xres;
      use->xres=width;
      if (use->yres<=height) height=use->yres;
	  use->yres=height;
	  //use->bits_per_pixel=bits;
    }
      

    if (width==use->xres &&     // did we find a matching mode?
        height==use->yres && 
		bits==use->bits_per_pixel)
      return use;

	if (bits==use->bits_per_pixel)
		{
    // see how far off we are
    sw32 dist=((sw32)use->xres-(sw32)width)*((sw32)use->xres-(sw32)width)+
              ((sw32)use->yres-(sw32)height)*((sw32)use->yres-(sw32)height);
    if (dist<closest_dist)//We should be looking for appriopriate BitDepth, too
    {
      closest_dist=dist;
      closest_width=use->xres;
      closest_height=use->yres;
    }
		}

    use=display->get_next_mode();
  } while (use);

  width=closest_width;
  height=closest_height;

  return 0;
  //the opengl driver currently only returns one mode.
  // (default_width x default_height x desktop_bitdepth)
  //This might not be what we want but we have little choice
  //return use;
}

i4_application_class::~i4_application_class()
{

}

void i4_application_class::memory_init()
{
  i4_init();
}


void i4_application_class::resource_init(char *resource_file, 
                                         void *resource_buffer)
{
  i4_string_man.load("resource/i4.res");//This one is not really sensefull to be in a subdir

  if (resource_buffer)
    i4_string_man.load_buffer(resource_buffer,
                              "internal_buffer");
  else
    i4_string_man.load(resource_file);
}


void i4_application_class::handle_no_displays()
{
  i4_error("FATAL: Could not find a working display driver!");
}


static char *i4_display_key="SOFTWARE\\Crack dot Com\\Golgotha\\1.0";

i4_bool i4_application_class::get_display_name(char *name, int max_len)
{
  return i4_get_registry(I4_REGISTRY_USER, i4_display_key, "display", name, max_len);
}

void i4_application_class::display_init()
{
  char name[256];

  i4_display_list_struct *d, *found=0, *best=0;

  if (get_display_name(name, 256))
  {
    for (d=i4_display_list; d; d=d->next)
      if (strcmp(d->name, name)==0)
        found=d;
  }
  
  //Find a display driver. Don't enter the loop if there's no driver 
  //left to try.
  if (!found&&(i4_display_list!=0))
	  {
      	  found=i4_display_list;
	  best=found;
	  if (strcmp(found->name,"Windowed GDI")==0 && found->next) 
		best=found->next;//ANYthing is better than GDI.
	  for (d=found; d; d=d->next)
		  {
		  if ((best->driver_id<0x8000)&&(d->driver_id>=0x8000))
		  {
			  best=d; //choose hardware acceleration if possible
			  continue;
		  }
		  if (best->priority<d->priority)
		  {
			  best=d;
		  }
		  
		  }
	  found=best;
	  }

  if (!found)
    handle_no_displays();
    

  w16 width=640, height=480, bits=16;//This seems to be the default resolution

  // first try to find mode specified in the resource file
  i4_const_str xres=i4gets("default_xres", i4_F);//It is configurable! TODO: Insert this in a Menu
  i4_const_str yres=i4gets("default_yres", i4_F);
 
  if (!xres.null() && !yres.null())
  {
    i4_const_str::iterator xres_str=xres.begin(),
      yres_str=yres.begin();

    width=(w16)xres_str.read_number();
    height=(w16)yres_str.read_number();
  }
  width=i4_win32_startup_options.xres;//Get the values from registry.
  height=i4_win32_startup_options.yres;
  bits=i4_win32_startup_options.bits;
  w16 found_width=width,
      found_height=height,
      found_bits=bits;

  display=found->display;

  i4_display_class::mode *use=find_mode(found_width, found_height, found_bits, found->driver_id);
  if (!use)
  {
    i4_warning("Unable to find an exact match for mode %dx%d, using %dx%d instead\n",
        width,height,
        found_width,found_height);

    use=find_mode(found_width, found_height, found_bits, found->driver_id);
//#ifdef _WINDOWS
//    if (!use||found_width==0||found_height==0)
//      i4_error((char*)IDS_INVALIDBITDEPTH);
//#else
    if (!use||found_width==0||found_height==0)
      i4_warning("WARNING: Invalid bit depth or resolution chosen or DirectX/OpenGL renderer is not ready.");
//#endif
  }
  if (use)
  {
  	i4_warning("INFO: About to initialize display to (%ix%ix%i) (Internal name %s) using driver %s.",
    	use->xres,
    	use->yres,use->bits_per_pixel,use->name,found->name);
  	sprintf(display_mode_name,"Resolution %ix%ix%i (Internal name: %s) using driver %s.",
	  use->xres,use->yres,use->bits_per_pixel,use->name,found->name);
  }
  if (!use||!display->initialize_mode())//Open the Window / switch to FS
  {
	  display->close();
	  i4_display_list_struct::remove_from_list(found->name);
	  if (i4_display_list==0)
	  {
		  //no display driver could be initialized
		handle_no_displays();
		return;
	  }
	  i4_warning("Initialisation failed, retrying with another display driver.");
	  display_init();//try another display type.
	  return;
  }
  i4_warning("Initialisation successfull.");
  wm=new i4_window_manager_class();
  wm->prepare_for_mode(display, use);

  i4_kernel.request_events(this,i4_device_class::FLAG_DISPLAY_CLOSE);
}



void i4_application_class::init()
{
  memory_init();
  resource_init("resource.res",0);
  display_init();
}

void i4_application_class::uninit()
{
  display_uninit();

  i4_uninit();
}
#ifdef _WINDOWS
extern CWnd cwnd;
#endif
void i4_application_class::fastexit(int errcode)
	{
	finished=i4_T;
	uninit();
#ifdef _WINDOWS
	cwnd.Detach();//Needed, otherwise the process GPF's
#endif
	exit(errcode);
	}

void i4_application_class::display_uninit()
{
  i4_kernel.process_events();
  delete wm;
  wm=0;
  
  i4_kernel.unrequest_events(this, i4_device_class::FLAG_DISPLAY_CLOSE);
  if (display)//is 0 if bailing out before init completes.
	display->close();
  display=0;
}

// windows_registry
/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

void i4_language_extend(i4_str &string, i4_str::iterator where)
    {
    int lang=0;
    //if it fails, we will just use the default, which is english
    char buf[10];
    buf[0]='_';
    buf[1]=0;
#ifdef _WINDOWS
    
    LoadString(i4_win32_instance,IDS_LANGUAGE,buf+1,9);
    
#endif
    char buf2[255];
    if (i4_get_registry(I4_REGISTRY_USER,0,"language",buf2,255)&&buf2[0])
        {
        strncpy(buf+1,buf2,9);
        };
    if (buf[1]==0)
        strcpy(buf+1,"en");
    //string.insert(where,'_');
    //we must not use two inserts with the same iterator, since
    //it can become invalid after an insert (when the buffer is resized)
    string.insert(where,buf);
    }

#ifdef _WINDOWS

i4_bool i4_get_registry(i4_registry_type type, 
                        char *path, 
                        char *key_name,
                        char *buffer, int buf_length)
{
  HKEY key;

  if (path==0)
	  path=GOLGOTHA_REG_PATH;//"SOFTWARE\\Crack dot Com\\Golgotha\\1.0";
  if (RegOpenKeyEx(type==I4_REGISTRY_MACHINE ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER,
                   path,
                   0,
                   KEY_READ,
                   &key)==ERROR_SUCCESS)
  {
    for (int i=0;; i++)
    {
      char name[256];
      DWORD name_size=256, ktype=0;
      DWORD data_size=buf_length;
     
      if (RegEnumValue(key, i, name, &name_size, 0, 
                     &ktype, 
                     (LPBYTE)buffer, 
                     &data_size)==ERROR_SUCCESS)
      {
        if (strcmp(name, key_name)==0)
        {
          RegCloseKey(key);
          return i4_T;
        }
      }
      else
      {
        RegCloseKey(key);
        return i4_F;
      }
    }
  }
  return i4_F;
}


i4_bool i4_set_registry(i4_registry_type type, 
                        char *path, 
                        char *key_name,
                        char *buffer)
{
  HKEY key;

  if (path==0)
	  path=GOLGOTHA_REG_PATH;
  if (buffer==0)
	  buffer="";
  if (RegCreateKey(type==I4_REGISTRY_MACHINE ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER,
                 path,
                 &key)==ERROR_SUCCESS)
  {
    RegSetValueEx(key, key_name, 0, REG_SZ, (w8 *)buffer, strlen(buffer)+1);
    RegCloseKey(key);
    return i4_T;
  }
  
  return i4_F;
}

void i4_set_int(char *key_name,int i)
	{
	HKEY key;
	if (RegCreateKey(HKEY_CURRENT_USER,GOLGOTHA_REG_PATH,&key)==ERROR_SUCCESS)
		{
		RegSetValueEx(key,key_name,0,REG_DWORD,(unsigned char*)&i,4);
		RegCloseKey(key);
		}
	return;
	}

int i4_get_int(char *key_name, int* retval)
	{
	HKEY key;
	ULONG len=4;
	if (RegOpenKeyEx(HKEY_CURRENT_USER,//Moved from HKEY_LOCAL_MACHINE
                   GOLGOTHA_REG_PATH,  //Because of security problems
                   0,	//accessing HKEY_LOCAL_MACHINE if the user
                   KEY_READ, //is not admin on NT/2000
                   &key)==ERROR_SUCCESS)
		{
		return RegQueryValueEx(key,key_name,0,NULL,(unsigned char*)retval,&len);
		}
	return 1011;
	}


	
#else 

//This file contains the functions to read/write to an ini file

/*
class INIFileInstanceHandler
	{
	public:
		int init_type(){return I4_INIT_TYPE_AFTER_ALL;};
		
		//This class is not subclass of i4_init_class because
		//it is used before the initialisation sequence begins.
		INIFileInstanceHandler(){};
		void init()
			{
			if (inifile==0)
				inifile=new CDataFile("golgotha.ini");
			};
		~INIFileInstanceHandler(){uninit();};
		void uninit()
			{
			if (inifile)
			{
				inifile->Save();
				delete inifile;
				inifile=0;
			}
			};
	}INIFileInstanceHandler_INST;
*/

i4_bool i4_get_registry(i4_registry_type type, 
                        char *path, 
                        char *key_name,
                        char *buffer, int buf_length)
{
  if (!inifile)
	  return i4_F;
  t_Str st1=inifile->GetString(t_Str(key_name),t_Str("Main"));
  strncpy(buffer,st1.c_str(),buf_length);
  return i4_T;
}

i4_bool i4_set_registry(i4_registry_type type, 
                        char *path, 
                        char *key_name,
                        char *buffer)
{
  if (!inifile)
	  return i4_F;
  inifile->CreateSection("Main","#The only section in this file");
  inifile->SetValue(t_Str(key_name),t_Str(buffer),t_Str(""),t_Str("Main"));
  inifile->Save();
  return i4_T;
}

void i4_set_int(char *key_name, int i)
{
	if (!inifile)
		return;
	inifile->CreateSection("Main","#The only section in this file");
	inifile->SetInt(t_Str(key_name),i,t_Str(""),t_Str("Main"));
	inifile->Save();
}

int i4_get_int(char *key_name, int *i)
{
	if (!inifile)
		return 1;
	*i=inifile->GetInt(key_name, t_Str("Main"));
	if (*i==0x7FFFFFFE)
	  return 1;
	return 0;
}
#endif

