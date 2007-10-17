/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/
#include "pch.h"
#include "main/main.h"
#include "main/win_main.h"
#include "error/error.h"
#include "error/alert.h"
#include "file/file.h"
#include "app/registry.h"
#include "app/app.h"
#include "editor/dialogs/debug_win.h"
#include "resource.h"
#include "lisp/lisp.h"
#include "version.h"
#ifndef _WINDOWS

#include "app/cdatafile.h"
//used for showerror (opens up a special x window)
#include "video/x11/x11_input.h"
#endif

/*! \mainpage Golgotha source code documentation
 * \par
 * You are looking at the source code documentation of the
 * Golgotha real time 3D Engine. The documentation is only partially
 * complete, so not every class is properly documented and anotaded.
 * The information contained herein is mainly thought for developers
 * working on the project to find their way trough the large code base.
 * \par
 * For a better overview of the different modules, refer to the
 * manual first. It gives a good overview over the different components
 * of the engine.
 * \par
 * To find information on a particular class or method, you can use
 * the search facility. Just enter the name of the class or method
 * you are interested in and you are there in no time.
 * \par Developer information
 * If you are a developer and need more information about how a
 * particular feature works, don't hesitate to ask on the golgotha
 * forums. We are also open for feature requests or bug reports, might
 * they be real bugs or just incomplete/false documentation. Be aware
 * that we are talking about a code base of 200'000 lines of code, so
 * it's just not possible at the moment to write the documentation in
 * a completeness we all liked to have.
 */


w32 i4_global_argc;
i4_const_str * i4_global_argv;


HINSTANCE i4_win32_instance;
int i4_win32_nCmdShow;
HWND i4_win32_window_handle=0;
i4_win32_startup_options_struct i4_win32_startup_options;
void * i4_stack_base=0;

#ifndef _WINDOWS
void OutputDebugString(char * lpOutputString)
{
	printf(lpOutputString);
	//printf("\n");
};
int i4_win32_error(const char * st)
{
	//todo: ask user.
	printf(st);
	printf("\n");

	//i4_thread_sleep(2000);
	//printf("Attempting to continue anyway\n");
	return showerror(st);

#if 0
#ifdef __linux
	static char errbuf[2048];
	sprintf(errbuf,"gmessage -buttons Abort:1,Debug:2,Ignore:3 %s",st);
	int ret=system(errbuf);
	if (ret==1)
	{
		exit(77);
	}
	if (ret==2)
	{
		//perhaps we'll access some invalid mem here, but is dangerous
		//if no debugger is running...
	}

	return 0;

#else
	//PG: Sun should have xmessage. But I think we'll include that code.
	//SUN comes here for instance
	if (strncmp("FATAL",st,5)==0)
	{
		printf("Golgotha: Quitting now.\n");
		exit(94);
	}
	i4_thread_sleep(1000);
	return 0;

#endif
#endif //#if 0
}
#define ERROR_SUCCESS 0
#define TRUE i4_T
#define FALSE i4_F
#define DWORD w32
//#include <string.h>
#endif
int i4_win32_alert(const i4_const_str &ret)
{
	char tmp[1024], * st;

	strcpy(tmp, "Alert : ");
	st=tmp+8;

	i4_const_str::iterator s=ret.begin();
	while (s!=ret.end())
	{
		*st=(char)s.get().value();
		++s;
		st++;
	}
	*st=0;
	strcat(tmp,"\n");
	g1_debug_printf(tmp);
	OutputDebugString(tmp);
	setbuf(stdout,0);
	printf(tmp);
	return 1;
}

i4_win32_startup_options_struct::~i4_win32_startup_options_struct()
{
	if (render_data)
	{
		free(render_data);
	}
}

/*
   void i4_win32_startup_options_struct::check_option(char *opt)
   {//only maxtool is currently using this one
 #ifndef _CONSOLE
   int temp=0;
   //i4_warning("This function (checkoption/1) should not be used.");
   if (i4_get_int("Fullscreen",&temp)==ERROR_SUCCESS) fullscreen=(temp?TRUE:FALSE);
   if (i4_get_int("xres",&temp)==ERROR_SUCCESS) xres=(short int)temp;
   if (i4_get_int("yres",&temp)==ERROR_SUCCESS) yres=(short int)temp;
   if (i4_get_int("bits",&temp)==ERROR_SUCCESS) bits=(short int)temp;
   if (i4_get_int("use2dsound",&temp)==ERROR_SUCCESS) use2dsound=temp;
   if (i4_get_int("use3dsound",&temp)==ERROR_SUCCESS) use3dsound=temp;
 #ifdef _WINDOWS
   HKEY key;
   if (RegOpenKeyEx(HKEY_CURRENT_USER,
   				   GOLGOTHA_REG_PATH,
   				   0,
   				   KEY_READ,
   				   &key)==ERROR_SUCCESS)
   		{
   		ULONG s=sizeof(GUID);
   		RegQueryValueEx(key,"Screen",0,
   			0,(w8*)&guid_screen,&s);
   		RegQueryValueEx(key,"Sound",0,
   			0,(w8*)&guid_sound,&s);
   		}
   if (!strcmp(opt,"-no_full"))
   	fullscreen=i4_F;
   if (!fullscreen)
   		{
   		HDC dc;
   		int i;
   		dc=GetDC(NULL);
   		i=GetDeviceCaps(dc,BITSPIXEL);
   		ReleaseDC(NULL,dc);
   		bits=i;
   		//if running in window set desired bitdepth to desktop bitdepth
   		}
 #else
   if (!strcmp(opt,"-no_full"))
   	fullscreen=i4_F;
 #endif

 #else
   fullscreen=i4_F;
 #endif
   }
 */

void i4_win32_startup_options_struct::check_option(w32 argc,i4_const_str * argv)
{
#ifndef _CONSOLE
	//render=R1_RENDER_DIRECTX5;//These options are not saved at the moment
	int temp=0;    //Call this method only once.
	if (i4_get_int("Fullscreen",&temp)==ERROR_SUCCESS)
	{
		fullscreen=(temp ? TRUE : FALSE);
	}
	if (i4_get_int("xres",&temp)==ERROR_SUCCESS)
	{
		xres=(short int)temp;
	}
	if (i4_get_int("yres",&temp)==ERROR_SUCCESS)
	{
		yres=(short int)temp;
	}
	if (i4_get_int("use2dsound",&temp)==ERROR_SUCCESS)
	{
		use2dsound=temp;
	}
	if (i4_get_int("use3dsound",&temp)==ERROR_SUCCESS)
	{
		use3dsound=temp;
	}
	if (i4_get_int("bits",&temp)==ERROR_SUCCESS)
	{
		bits=(short int)temp;
	}
	if (i4_get_int("max_view_distance",&temp)==ERROR_SUCCESS)
	{
		max_view_distance=temp;
	}
	if (i4_get_int("max_texture_quality",&temp)==ERROR_SUCCESS)
	{
		max_texture_quality=temp;
	}
	if (i4_get_int("texture_bitdepth",&temp)==ERROR_SUCCESS)
	{
		texture_bitdepth=temp;
	}
	if (i4_get_int("stereo_mode",&temp)==ERROR_SUCCESS)
	{
		stereo=temp;
	}
	if (i4_get_int("stereo_port",&temp)==ERROR_SUCCESS)
	{
		stereoport=temp;
	}
	if (i4_get_int("render_type",&temp)==ERROR_SUCCESS)
	{
		render=temp;
	}
	char buf[256];
	buf[0]=0;
	if (i4_get_registry(I4_REGISTRY_USER,0,"RenderDevice",buf,256))
	{
		if (strlen(buf)>0)
		{
			render_data_size=strlen(buf)+1;
			render_data=(char *)malloc(render_data_size);
			strcpy(render_data,buf);
		}
	}
	buf[0]=0;
	if (i4_get_registry(I4_REGISTRY_USER,0,"language",buf,256))
	{
		if (strlen(buf)>0&&strlen(buf)<9)
		{
			strcpy(langcode,buf);
		}
	}
#ifdef _WINDOWS
	HKEY key;
	if (RegOpenKeyEx(HKEY_CURRENT_USER,
					 GOLGOTHA_REG_PATH,
					 0,
					 KEY_READ,
					 &key)==ERROR_SUCCESS)
	{
		ULONG s=sizeof(GUID);
		RegQueryValueEx(key,"Screen",0,
						0,(w8 *)&guid_screen,&s);
		RegQueryValueEx(key,"Sound",0,
						0,(w8 *)&guid_sound,&s);
		RegCloseKey(key);
		key=0;
	}
#endif

#endif
	bool bImmediatellyShowOptions=false;
	for (w32 i=1; i<=argc; i++)  //Check Command-Line override
	{
		if (argv[i]==i4_const_str("-no_full"))
		{
			fullscreen=i4_F;
		}
		if (argv[i]==i4_const_str("-default"))   //with these settings,
		{
			//all systems should at least be able to show the main menu
			xres=640;
			yres=480;
			bits=16;
			texture_bitdepth=0;
			stereo=false;
			if (render_data)
			{
				free(render_data);
				render_data=0;
				render_data_size=0;
			}
			render=R1_RENDER_USEDEFAULT;
		}

		if (argv[i]==i4_const_str("-no_sound"))
		{
			use2dsound=FALSE;
			use3dsound=FALSE;
		}
		if (argv[i]==i4_const_str("-full"))
		{
			fullscreen=i4_T;
		}
		if (argv[i]==i4_const_str("-setup"))
		{
			bImmediatellyShowOptions=true;
		}
	}
#ifdef _WINDOWS
	if (!fullscreen)
	{
		HDC dc;
		int i;
		dc=GetDC(NULL);
		i=GetDeviceCaps(dc,BITSPIXEL);
		ReleaseDC(NULL,dc);
		bits=i;
		//if running in window set desired bitdepth to desktop bitdepth
	}
#endif
	if (bImmediatellyShowOptions)
	{
		ShowSystemOptions();
	}
}



int i4_win32_startup_options_struct::save_option(void)
{
#ifndef _CONSOLE
	i4_set_int("Fullscreen",fullscreen);
	i4_set_int("xres",xres);
	i4_set_int("yres",yres);
	i4_set_int("use2dsound",use2dsound);
	i4_set_int("use3dsound",use3dsound);
	i4_set_int("bits",bits);
	i4_set_int("max_texture_quality",max_texture_quality);
	i4_set_int("max_view_distance",max_view_distance);
	i4_set_int("texture_bitdepth",texture_bitdepth);
	i4_set_int("stereo_mode",stereo);
	i4_set_int("stereo_port",stereoport);
	i4_set_int("render_type",render);
	i4_set_registry(I4_REGISTRY_USER,0,"RenderDevice",render_data);
	if (langcode[0])
	{
		i4_set_registry(I4_REGISTRY_USER,0,"language",langcode);
	}
	//i4_set_registry(I4_REGISTRY_USER,0,"display",display_device);
#ifdef _WINDOWS
	HKEY key;
	if (RegCreateKey(HKEY_CURRENT_USER,
					 GOLGOTHA_REG_PATH,
					 &key)==ERROR_SUCCESS)
	{
		RegSetValueEx(key, "Screen", 0, REG_BINARY, (w8 *)&guid_screen, sizeof(GUID));
		RegSetValueEx(key, "Sound",0,REG_BINARY,(w8 *)&guid_sound,sizeof(GUID));
		//RegSetValueEx(key, "RenderDevice", 0, REG_BINARY, render_data,render_data_size);
		RegCloseKey(key);
	}
#endif
#endif
	return true;
}

DWORD FormatErrorMessage(char * input, char * buf, long bufsize)
{
	long actsize=strlen(input);

	//first part: change \n in \015 \012 (octal for 13,10)
	strcpy(buf,input);
	int i=1; //don't get trouble at i=0-1
	while(buf[i]&&(actsize<bufsize))
	{
		if (buf[i]=='\n')
		{
			buf[i]='\r';
			actsize++;
			for(int j=actsize; j>i; j--) //also copy the \0
			{
				buf[j]=buf[j-1];
			}
			buf[i+1]='\n';
			i++; //skip the new \n
		}
		i++;
	}

	//second part: identify special info
	char * first=strchr(buf,':');
	char severity[30];
	//char severitydescription[300];
	w32 dwResId=IDS_ERROR_UNKNOWN;
	if (first&&(first-buf<30))
	{
		memcpy(severity,buf,30);
		severity[first-buf]='\0';

		if (strstr(severity,"USER")!=0)
		{
			dwResId=IDS_ERROR_USER;
		}
		if (strstr(severity,"ERROR")!=0)
		{
			dwResId=IDS_ERROR_ERROR;
		}
		if (strstr(severity,"FATAL")!=0)
		{
			dwResId=IDS_ERROR_FATAL;
		}
		if (strstr(severity,"CRITICAL")!=0)
		{
			dwResId=IDS_ERROR_CRITICAL;
		}
		if (strstr(severity,"INFO")!=0)
		{
			dwResId=IDS_ERROR_INFO;
		}
		if (strstr(severity,"WARNING")!=0)
		{
			dwResId=IDS_ERROR_WARNING;
		}
		if (strstr(severity,"SEVERE")!=0)
		{
			dwResId=IDS_ERROR_SEVERE;
		}
		if (strstr(severity,"INTERNAL")!=0)
		{
			dwResId=IDS_ERROR_INTERNAL;
		}

	}
	/*char *first=strchr(buf,':');
	   char *second=0;
	   if (first) second=strchr(first+1,':');
	   char severity[20];
	   char location[40];
	   if (first&&second&&((first-buf)<20)&&((second-first)<40))
	   	{
	   	memcpy(severity,buf,20);
	   	severity[(first-buf)]=0;
	   	memcpy(location,first,40);
	   	location[(second-first)]=0;
	   	buf=second+1;
	   	}
	   else if (first&&((first-buf)<40))
	   	{
	   	severity[0]=0;
	   	strcpy(*severity,"UNKNOWN");
	   	memcpy(location,buf,40);
	   	location[(first-buf)]=0;
	   	buf=first;
	   	}
	   else
	   	{
	   	severity[0]=0;
	   	location[0]=0;
	   	}
	   //third part: give hint about severity

	   //last part: compile everything

	   if (location[0])
	   	{
	   	char *help=new char[bufsize];
	   	actsize=wsprintf(help,"Problemklasse: \t%s.\r\nKontext: \t%s.\r\nMeldung: \t%s",severity,location,buf);
	   	strcpy(buf,help);
	   	delete help;
	   	}*/

	return dwResId;
}






#ifdef _WINDOWS

INT_PTR CALLBACK ErrorDlg(
	HWND hwndDlg, // handle to dialog box
	UINT uMsg,   // message
	WPARAM wParam, // first message parameter
	LPARAM lParam // second message parameter
)
{
	static char stv[2000]; //to load the help information

	switch (uMsg)
	{
		case WM_INITDIALOG:
			{
				//Lparam is the message to be displayed
				LoadString(i4_win32_instance,IDS_ERRORHELPTEXT,stv,1999);
				SetWindowText(GetDlgItem(hwndDlg,IDC_ERRORHELP),stv);
				long size=strlen((LPCSTR)lParam)+500;
				DWORD dwSeverity=0;
				char * buf=new char[size];
				dwSeverity=FormatErrorMessage((char *)lParam,buf,size);
				if (dwSeverity)
				{
					LoadString(i4_win32_instance,dwSeverity,stv,1999);
					SetWindowText(GetDlgItem(hwndDlg,IDC_SEVERITY),stv);
					if (dwSeverity==IDS_ERROR_FATAL)
					{
						//If fatal error, disable IGNORE and CANCEL
						EnableWindow(GetDlgItem(hwndDlg,IDIGNORE),FALSE);
						EnableWindow(GetDlgItem(hwndDlg,IDCANCEL),FALSE);
					}
					else
					{
						EnableWindow(GetDlgItem(hwndDlg,IDIGNORE),TRUE);
						EnableWindow(GetDlgItem(hwndDlg,IDCANCEL),TRUE);
					}

				}
				SetWindowText(GetDlgItem(hwndDlg,IDC_ERRORMESSAGE),buf);
				delete buf;
			}
			break;
		case WM_COMMAND:
			{
				if (
					((HWND)lParam==GetDlgItem(hwndDlg,IDC_ERRORMESSAGE))||
					((HWND)lParam==GetDlgItem(hwndDlg,IDC_SEVERITY))
				)
				{
					//do we need to handle input like CTRL+C manually?
				}
				else
				if ((HWND)lParam==GetDlgItem(hwndDlg,IDC_PAUSE))
				{
					g1_resources.paused=IsDlgButtonChecked(hwndDlg,IDC_PAUSE);
				}
				else
				{
					g1_resources.paused=IsDlgButtonChecked(hwndDlg,IDC_PAUSE);
					EndDialog(hwndDlg,LOWORD(wParam)); //return the id of the control
				}
			}
			break;
		default:
			return FALSE;
	}
	return TRUE;
}

int died=0;

int i4_win32_error(const char * st)
{


	char stv[1000];

	if (died)
	{
		//i4_debug->printf("recursive error : %s", st);
		died=0; //reset this
		return 1;
	}

	died = 1;
	UINT ans=0;
	SetLastError(0);
	try
	{
		li_call("show_gdi_surface");
	}
	catch(...) //most probably already out of mem
	{
	}
	if (((UINT)st)<0x0000ffff) //ist eine Resourcen-ID
	{
		LoadString(i4_win32_instance,(UINT)st,stv,1000);
		g1_debug_printf(stv);
		OutputDebugString(stv);
		ans=MessageBox(i4_win32_window_handle,stv,"Golgotha",MB_ABORTRETRYIGNORE+MB_ICONSTOP+MB_APPLMODAL);
	}
	else
	{
		g1_debug_printf(st);
		//OutputDebugString(st);
		//ans=MessageBox(i4_win32_window_handle, st, "Fehler", MB_ABORTRETRYIGNORE | MB_APPLMODAL|MB_ICONSTOP);
		ans=DialogBoxParam(i4_win32_instance,MAKEINTRESOURCE(IDD_ERRORDIALOG),
						   i4_win32_window_handle,(DLGPROC)ErrorDlg,(LPARAM)st);
	}
	//if (GetLastError()!=0)
	//	  i4_warning("Messagebox Failed, %s",st);
	if (ans==IDABORT)
	{
#ifndef _CONSOLE
		if (i4_current_app)
		{
			i4_current_app->fastexit(23);
		}
#else
		exit(27);
#endif
		_exit(22);
	}

	try
	{
		li_call("hide_gdi_surface");
	}
	catch(...) //most probably already out of mem
	{
	}
	if (ans==-1) //there was some problem calling DialogBoxParam()
	//i.e the template doesn't exist.
	{
		MessageBox(i4_win32_window_handle,"The standard error resources are unavailable. Check your make process.",
				   "FATAL ERROR: EXE FILE CORRUPT",MB_OK|MB_SYSTEMMODAL|MB_ICONSTOP);
		exit(83);
	}
	if (ans==IDC_DEBUG)
	{
		DebugBreak();
	}

	if (ans==IDCANCEL)
	{
		died=0;
		throw "Localbreak_exception";
	}

	died=0;
	return ans;
}


#endif
//static FILE *debug_file=0;
int debug_file=0;


class g1_debug_output_writer :
	public i4_init_class,
	public i4_event_handler_class
{
	void init()
	{
		i4_kernel.request_events(this,i4_device_class::FLAG_USER_MESSAGE);
	}
	void uninit()
	{
		i4_kernel.unrequest_events(this,i4_device_class::FLAG_USER_MESSAGE);
	}
	void receive_event(i4_event * ev)
	{
		CAST_PTR(deev,g1_send_debug_message_class,ev);
		if (deev->sub_type==G1_DEBUG_SEND_EVENT)
		{
#ifdef _WINDOWS
			OutputDebugString(deev->msg);
#else
			printf(deev->msg);
#endif
		}
	}
	void name(char * buffer)
	{
		static_name(buffer,"debug_output_writer");
	};
} g1_debug_output_writer_instance;

//#ifdef _WINDOWS

int i4_windows_warning_function(const char * st)
{
	//g1_debug_printf(st);
	i4_debug->write(st,strlen(st));
	i4_debug->write("\n",1);
	//OutputDebugString(st);
	//OutputDebugString("\n");

	return 1;
}
//#else
/*
   int i4_windows_warning_function(const char *st)
   	{
   	g1_debug_printf(st);
   	//printf(st);
   	//printf("\n");
   	return 1;
   	}
 */
//#endif

class i4_win32_debug_stream_class :
	public i4_file_class
{
public:
	virtual w32 read(void * buffer, w32 size)
	{
		return 0;
	}
//  char *b;//because of possible stack problems, be shure this exists when called
//  enum {CONST_BUF_SIZE=1024};
//  char c[CONST_BUF_SIZE];
	w32 i;
	char * b;
	virtual w32 write(const void * buffer, w32 size)
	{
//    if (size>=CONST_BUF_SIZE)
//        {
//        b=(char*)malloc(size+1);//buffer might not be 0-terminated
//#ifdef _WINDOWS
//        if (!b)
//            {
//            MessageBox(0,"FATAL: Error handler internal error: "
//                "Out of Memory to allocate space for error message.",
//                "Golgotha",MB_OK|MB_SYSTEMMODAL);
//            exit(199);
//            }
//#endif
//        ZeroMemory(b,size+1);
//        }
//    else
//        {
//        b=c;
//        ZeroMemory(b,CONST_BUF_SIZE);
//        }

//	memcpy(b,buffer,size);
		if (debug_file&&i4_is_initialized()&&size>2) //avoid writing a single "\n"
		{
			//fwrite(buffer, 1, size, debug_file);
			//fflush(debug_file);
			//_write(debug_file,buffer,size);
			i4_file_class * fp=i4_open(i4_global_argv[debug_file],I4_APPEND|I4_NO_BUFFER);
			fp->write(buffer,size);
			fp->write_str("\r\n");
			delete fp;
			//_commit(debug_file);
		}
		/*

		   if (size>2047)
		   {
		   b[1]=0;
		   //for (int i=0; i<(int)size; i++)
		   //{
		   //  b[0]=*(((w8 *)buffer)+i);
		   //  OutputDebugString(b);
		   //}
		   }
		   else
		   {
		   memcpy(b,buffer, size);
		   b[size]=0;
		   //OutputDebugString(b);
		   }
		 */
		i=0;
		b=(char *)buffer;
		while (i<size)
		{
			if (b[i]==0)
			{
				break;
			}
			++i;
		}
		//if we coudn't find a zero within size and b[i] is not just that
		//zero, we ignore the call.
		if ((i==size)&&(b[i]!=0))
		{
			return size;
		}
		//don't call g1_debug_printf on non-zero terminated strings.
		g1_debug_printf(b);
//    if (size>=CONST_BUF_SIZE)
//    	free(b);
//	b=0;
		return size;
	}

	virtual w32 seek(w32 offset)
	{
		return 0;
	}
	virtual w32 size()
	{
		return 0;
	}
	virtual w32 tell()
	{
		return 0;
	}
} win32_debug_stream;



void debug_init()
{
	i4_set_error_function(i4_win32_error);
	i4_set_warning_function(i4_windows_warning_function);
	i4_set_alert_function(i4_win32_alert);

	for (int i=1; i<(int)i4_global_argc; i++)
	{
		if (i4_global_argv[i]=="-eout")
		{
			i++;
			char fn[100];
			i4_os_string(i4_global_argv[i], fn, 100);
			debug_file=i; //_open(fn, _O_TRUNC|_O_CREAT, _S_IREAD | _S_IWRITE);
		}
	}

	i4_debug=&win32_debug_stream;
}



class tmp_main_str :
	public i4_const_str
{
public:
	tmp_main_str();
	void set_ptr(char * _ptr)
	{
		ptr=_ptr;
		len=strlen(ptr);
	}

};


#ifndef _WINDOWS
//Everything but windows starts here
int main(int argc, char * * argv)
{
	long t;

	i4_stack_base=(void *)(&t);

	tmp_main_str * tmp=(tmp_main_str *)malloc(sizeof(tmp_main_str)*(argc+1));
	w32 i;
	for (i=0; i<(w32)argc; i++)
	{
		tmp[i].set_ptr(argv[i]);
	}

	i4_global_argc=argc;
	i4_global_argv=tmp;
	debug_init();
	inifile=0;
	printf("Golgotha version %s loading.\n",GOLGOTHA_VERSION_STR);
	inifile=new CDataFile("golgotha.ini");
	i4_win32_startup_options.check_option(argc,tmp);
	i4_main(argc,tmp);

	i4_win32_startup_options.save_option();
	delete inifile;
	inifile=0;
	free(tmp);
	printf("Golgotha has shut down properly.\n");
	return 0;
}
#endif

#ifdef _WINDOWS
int PASCAL WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance,
				   LPSTR lpCmdLine, int nCmdShow)


{
	long t=0xBAADFEED;

	i4_stack_base=(void *)(&t);


	/*
	   char hacked_args[512];
	   strcpy(hacked_args,"x:\\jc\\gdata\\obj\\test_text.gmod -no_full");
	 */
	w32 count, white;
	char * s=0,* p=0,* st = strdup(lpCmdLine);

	s = st;
	count = 1;
	white = 1;
	while (*s)
	{
		if (white)
		{
			if (*s != ' ')
			{
				count++;
				white = 0;
			}
			s++;
		}
		else
		{
			if (*s == ' ')
			{
				white = 1;
			}
			s++;
		}
	}

	tmp_main_str * tmp=(tmp_main_str *)malloc(sizeof(tmp_main_str)*(count+1));

	s = st;
	tmp[0].set_ptr("name");
	count = 1;
	white = 1;
	while (*s)
	{
		if (white)
		{
			if (*s != ' ')
			{
				white = 0;
				p = s;
			}
			s++;
		}
		else
		{
			if (*s == ' ')
			{
				white = 1;
				*s = 0;

				//i4_win32_startup_options.check_option(p);

				tmp[count].set_ptr(p);
				count++;
			}
			s++;
		}
	}
	if (!white)
	{
		//i4_win32_startup_options.check_option(p);
		tmp[count].set_ptr(p);
		count++;
	}
	INITCOMMONCONTROLSEX inits;
	inits.dwSize=sizeof(inits);
	inits.dwICC=ICC_WIN95_CLASSES;
	i4_win32_nCmdShow=nCmdShow;
	i4_win32_instance=hInstance;
	if (!InitCommonControlsEx(&inits))
	{
		i4_error("FATAL: InitCommonControlsEx: Initialisation of Common Controls failed!");
	}

	CoInitialize(NULL);
	if (!AfxWinInit( hInstance, hPrevInstance, lpCmdLine, nCmdShow ))
	{
		MessageBox(NULL,"Failed to initialize the MFC. Check DLL Versions.",
				   "Fatal Error",MB_OK+MB_ICONSTOP);
	}
	//AfxEnableControlContainer();
	if (AfxGetApp())
	{
		AfxGetApp()->InitInstance();
	}
/*#ifdef _AFXDLL
   	Enable3dControls();			// Diese Funktion bei Verwendung von MFC in gemeinsam genutzten DLLs aufrufen
 #else
   	Enable3dControlsStatic();	// Diese Funktion bei statischen MFC-Anbindungen aufrufen
 #endif*/

	AfxSetResourceHandle(i4_win32_instance);
	i4_global_argc=count;
	i4_global_argv=tmp;
	debug_init();
	//for testing...
	//inifile=new CDataFile("golgotha.ini");
	i4_win32_startup_options.check_option(i4_global_argc,i4_global_argv);
	char memfailbuf[200];
	ZeroMemory(memfailbuf,sizeof(memfailbuf));
	try
	{
		i4_main(count, tmp);
	}
	catch( CMemoryException* e )
	{
		e->GetErrorMessage(memfailbuf,200,0);
		e->Delete();
		FatalAppExit(0,memfailbuf);
	}
	;

	free(tmp);
	free(st);
	PostQuitMessage( 0 );
	CoUninitialize();
	i4_win32_startup_options.save_option(); //This is the latest point to do this.
	//delete inifile;
	//inifile=0;
	//Better do it after each change (i.e options-dialog)
	return 0;
}
#endif
