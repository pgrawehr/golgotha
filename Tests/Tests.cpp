// Tests.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "conio.h"
HMODULE hmod;
typedef int (__stdcall *LibVersion)(void);

class i4_vector3_template
{
public:
  float x,y,z;


    i4_vector3_template& cross_asm(const i4_vector3_template& a, 
                                    const i4_vector3_template& b)
  {
    _asm
    {
      mov     eax,this
      mov     ecx,a
      mov     edx,b


      fld dword ptr   [ecx+4]     ;starts & ends on cycle 0
      fmul dword ptr  [edx+8]     ;starts on cycle 1       
      fld dword ptr   [ecx+8]     ;starts & ends on cycle 2
      fmul dword ptr  [edx+0]     ;starts on cycle 3       
      fld dword ptr   [ecx+0]     ;starts & ends on cycle 4
      fmul dword ptr  [edx+4]     ;starts on cycle 5       
      fld dword ptr   [ecx+8]     ;starts & ends on cycle 6
      fmul dword ptr  [edx+4]     ;starts on cycle 7       
      fld dword ptr   [ecx+0]     ;starts & ends on cycle 8
      fmul dword ptr  [edx+8]     ;starts on cycle 9         
      fld dword ptr   [ecx+4]     ;starts & ends on cycle 10 
      fmul dword ptr  [edx+0]     ;starts on cycle 11        
      fxch            st(2)       ;no cost                   
      fsubp          st(5),st(0) ;starts on cycle 12        
      fsubp          st(3),st(0) ;starts on cycle 13        
      fsubp          st(1),st(0) ;starts on cycle 14        
      fxch            st(2)       ;no cost, stalls for cycle 15
      fstp dword ptr  [eax+0]     ;starts on cycle 16, ends on cycle 17  
      fstp dword ptr  [eax+4]     ;starts on cycle 18, ends on cycle 19
      fstp dword ptr  [eax+8]     ;starts on cycle 20, ends on cycle 21
    }
    return *this;
  }

    i4_vector3_template& cross_c(const i4_vector3_template& a, 
                                    const i4_vector3_template& b)
  {
    x = a.y*b.z - a.z*b.y;
    y = a.z*b.x - a.x*b.z;
    z = a.x*b.y - a.y*b.x;
    return *this;
  }

};

BOOL isknown(char *f)
	{
	int i=0;
	while (f[i]!=0)
		{
		if (f[i]=='.') return TRUE;
		i++;
		}
	return FALSE;
	}
int main(int argc, char* argv[])
{
	/*LibVersion libv=0;
	printf("Loading library.\n");
	SetLastError(0);
	hmod=LoadLibrary("plugin.dll");
	if (!hmod)
		{
		printf("Could not load library. Result %i.\n",GetLastError());
		return -1;
		}
	libv=GetProcAddress(hmod,"LibVersion");
	if (!libv)
		{
		printf("Could not find a LibVersion function");
		FreeLibrary(hmod);
		return -1;
		}
	printf("Version of this Plugin-Dll: %i\n",libv());
	FreeLibrary(hmod);
	*/
/*
	//The following code detects the type of a file
	WIN32_FIND_DATA wf;
	HANDLE f;
	HANDLE cf;
	char buf[50];
	char *ext;
	char newname[MAX_PATH];
	BOOL ok;
	ULONG bytesread=0;
	SetCurrentDirectory("d:\\golgotha\\unknown");
	f=FindFirstFile("*.*",&wf);
	while (f!=INVALID_HANDLE_VALUE)
		{
		if (!(isknown(wf.cFileName)))
			{
			ext="";
			cf=CreateFile(wf.cFileName,GENERIC_READ,0,0,OPEN_EXISTING,
				FILE_ATTRIBUTE_NORMAL,0);
			ReadFile(cf,buf,50,&bytesread,NULL);
			ok=FALSE;
			if (buf[6]=='J' && buf[7]=='F' && buf[8]=='I')
				{
				//is a jpg file
				ext=".jpg";
				ok=TRUE;
				}
			else if (buf[0]=='B' && buf[1]=='M')
				{
				ext=".bmp";
				ok=TRUE;
				}
			else if (buf[0]==10 && buf[2]==0x1)
				{
				ext=".pcx";
				ok=TRUE;
				}
			else if (buf[1]==0 && (buf[2]==2 || buf[2]==10))
				{
				ext=".tga";
				ok=TRUE;
				}
			else if (buf[5]==0 && buf[6]==0 && buf[7] == 0 && buf[8]==0x13)
				{
				ext=".gmod";
				ok=TRUE;
				}
			CloseHandle(cf);
			
			wsprintf(newname,"%s%s",wf.cFileName,ext);
			if (ok)
				{
				printf("Renaming %s to %s\n",wf.cFileName,newname);
				MoveFile(wf.cFileName,newname);
				}

			}
		if (!FindNextFile(f,&wf)) break;
		}
	FindClose(f);
	printf("\n");
	*/
//The following code prints the ascii table
//	for (int i=0; i<256; i++)
//		{
//		if ((i % 32)==0) printf("\n");
//		printf("(%i:%c)",i,(char)i);
//		}

	//Now doing some vector tests

    i4_vector3_template a,b,c,d;

	a.x=1;
	a.y=0;
	a.z=0;

	b.x=0;
	b.y=1;
	b.z=0;

	for (int i=0;i<10;i++)
		{
	c.cross_c(a,b);

	d.cross_asm(a,b);

	if (c.x==d.x && c.y==d.y && c.z==d.z)
		{
		printf("Check successfull\n");
		}
	else
		{
		printf("Check failed\n");
		}
		}

	getch();
	return 0;
}

