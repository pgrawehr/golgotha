#include "pch.h"

/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/


#include <windows.h>
#include <process.h>
#include "error/error.h"
#include "init/init.h"

//If we do not have a REAL reason for requesting self-modification
//priviledges, we shan't do it (might save memory and time)

//Currently, the software renderer requires self-modification.
//PG: The plain C implementation of the software renderer doesn't use
//self-modification any more, so we skip it if USE_ASM is not defined

#ifdef USE_ASM
class r1_self_modify_class :
	public i4_init_class
{
public:

	void init()
	{
		HMODULE OurModule = GetModuleHandle(0);
		BYTE * pBaseOfImage = 0;

		if ( (GetVersion() & 0xC0000000) == 0x80000000)
		{
			// We're on Win32s, so get the real pointer
			HMODULE Win32sKernel = GetModuleHandle("W32SKRNL.DLL");

			typedef DWORD __stdcall translator (DWORD);
			translator * pImteFromHModule =
				(translator *) GetProcAddress(Win32sKernel, "_ImteFromHModule@4");
			translator * pBaseAddrFromImte =
				(translator *) GetProcAddress(Win32sKernel, "_BaseAddrFromImte@4");

			if (pImteFromHModule && pBaseAddrFromImte)
			{
				DWORD Imte = (*pImteFromHModule)( (DWORD) OurModule);

				pBaseOfImage = (BYTE *) (*pBaseAddrFromImte)(Imte);
			}
		}
		else
		{
			pBaseOfImage = (BYTE *) OurModule;
		}


		if (pBaseOfImage)
		{
			IMAGE_OPTIONAL_HEADER * pHeader = (IMAGE_OPTIONAL_HEADER *)
											  (pBaseOfImage + ( (IMAGE_DOS_HEADER *) pBaseOfImage)->e_lfanew +
											   sizeof(IMAGE_NT_SIGNATURE) + sizeof(IMAGE_FILE_HEADER));

			DWORD OldRights;

			VirtualProtect(pBaseOfImage + pHeader->BaseOfCode, pHeader->SizeOfCode,
						   PAGE_READWRITE, &OldRights);
		}
	}

} r1_self_modify_instance;

#endif //USE_ASM
