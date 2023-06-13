/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/

#ifndef DX9_ERROR_HH
#define DX9_ERROR_HH

#include <windows.h>
#include <ddraw.h>

// Debug mode always (we're fast enough)
#define i4_dx5_check(res) i4_dx9_check_lineinfo(res,__FILE__,__LINE__)
i4_bool i4_dx9_check_lineinfo(HRESULT res,char * f,int line);
#define i4_dx9_check(res) i4_dx9_check_lineinfo(res,__FILE__,__LINE__)





#endif
