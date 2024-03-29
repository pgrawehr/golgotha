/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/

#ifndef R1_RES_HH
#define R1_RES_HH


// This is for render tree files only
// fetches a string from render.res

#include "string/string.h"

extern int r1_max_texture_size;
const i4_const_str &r1_gets(char * str, i4_bool barf_on_error=i4_T);
//Call this function after the renderer is ready with some
//unique name of the actually used renderer instance.
//the name must not
void r1_name_cache_file(const char * rendername);
const i4_const_str &r1_get_decompressed_dir();
//no more needed
//const i4_const_str &r1_get_compressed_dir();
const i4_const_str &r1_get_cache_file();


#endif
