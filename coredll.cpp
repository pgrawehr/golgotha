//This file contains dummy code to create a core.lib file
#define BLD_CORE
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


void CoreExport InitCustomControls( HINSTANCE hInst )
{
}


CoreExport void DisableAccelerators()
{
};

CoreExport void EnableAccelerators()
{
};

CoreExport BOOL AcceleratorsEnabled()
{
	return true;
};

CoreExport void TempStore::PutBytes(int n, void *data, void *ptr)
{
};

CoreExport void TempStore::GetBytes(int n, void *data, void *ptr)
{
};

CoreExport Class_ID triObjectClassID;

CoreExport void *CreateInstance(unsigned long i, Class_ID c)
{
	return 0;
};

CoreExport TriObject *CreateNewTriObject(void)
{
	return 0;
};
