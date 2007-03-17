#define BLD_UTIL
#include <windows.h>
#include "crkutil.h"
#include "obj3d.h"
#include <utilapi.h>
#include <stdmat.h>
#include <stdio.h>
#include "maxcomm.h"
#include "file/file.h"
#include "string/string.h"
#include "max_object.h"
#include "memory/array.h"
#include "debug.h"

UtilExport CStr::CStr()
{
};

UtilExport CStr::CStr(const char *cs)
{
};

UtilExport CStr::CStr(const wchar_t *wcstr)
{
};

UtilExport CStr::CStr(const CStr& ws)
{
};

UtilExport CStr::~CStr()
{
};

UtilExport char CStr::*data()
{
	return 0;
};

UtilExport CStr::operator char*(){};



// realloc to nchars (padding with blanks)

UtilExport void CStr::Resize(int nchars)
{
};



UtilExport int CStr::Length()
{
	return 0;
};
