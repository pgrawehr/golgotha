//This file is the main source for precompiled headers
//This has no great advantage in compilling speed 
//versus the need to recompile everything when one of
//the include files changes.
#ifndef __PCH_H
#define __PCH_H

#include <stdlib.h>
#include <string.h>
#ifdef _WINDOWS
//Those are needed only for windows
//Stdafx includes windows.h
#include "StdAfx.h"
#include <ddraw.h>
//#include <d3d.h>
#else
//include the file created by the configure script
#include "config.h"
#endif
//include architecture specifics
#include "arch.h"

#include "time/profile.h"
#include "app/app.h"
#include "math/random.h"
#include "resource.h"
#include "resources.h"
#include "time/time.h"
#include "time/timedev.h"



#endif




