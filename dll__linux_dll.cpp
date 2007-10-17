/**********************************************************************

   	Golgotha Forever - A portable, free 3D strategy and FPS game.
   	Copyright (C) 1999 Golgotha Forever Developers

   	Sources contained in this distribution were derived from
   	Crack Dot Com's public release of Golgotha which can be
   	found here:  http://www.crack.com

   	All changes and new works are licensed under the GPL:

   	This program is free software; you can redistribute it and/or modify
   	it under the terms of the GNU General Public License as published by
   	the Free Software Foundation; either version 2 of the License, or
   	(at your option) any later version.

   	This program is distributed in the hope that it will be useful,
   	but WITHOUT ANY WARRANTY; without even the implied warranty of
   	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   	GNU General Public License for more details.

   	You should have received a copy of the GNU General Public License
   	along with this program; if not, write to the Free Software
   	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

   	For the full license, see COPYING.

 ***********************************************************************/

#include "pch.h"
#include "dll/dll.h"
#include <dlfcn.h>

class i4_linux_dll_file :
	public i4_dll_file
{
	void * handle;
public:
	i4_linux_dll_file(void * handle) :
		handle(handle)
	{
	}

	virtual ~i4_linux_dll_file()
	{

	}

	virtual void *find_function(const char * name)
	{
		return NULL;
	}
};

i4_dll_file *i4_open_dll(const i4_const_str &filename)
{
	return NULL;
}

/*

 #include "dll/dll.hh"
 #include <dlfcn.h>

   class i4_linux_dll_file : public i4_dll_file
   {
   void *handle;
   public:
   i4_linux_dll_file(void *handle) : handle(handle) {}

   virtual ~i4_linux_dll_file()
   {
   	dlclose(handle);
   }

   virtual void *find_function(const char *name)
   {
   	return dlsym(handle, name);
   }
   };

   i4_dll_file *i4_open_dll(const i4_const_str &filename)
   {
   char buf[1024];
   i4_os_string(filename, buf, 1024);

   void *handle=dlopen(buf,  RTLD_NOW);
   if (handle)
   	return new i4_linux_dll_file(handle);
   else
   {
   	i4_warning("DLL load %s failed : %s", buf, dlerror());
   	return 0;
   }

   }

 */
