/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef I4_REGISTRY_HH
#define I4_REGISTRY_HH

#include "arch.h"
#define GOLGOTHA_REG_PATH "Software\\Crack dot Com\\Golgotha\\1.0"
enum i4_registry_type
{
  I4_REGISTRY_MACHINE,  // associated with machine
  I4_REGISTRY_USER      // associated with user
};

void i4_language_extend(i4_str& str, i4_str::iterator where);
int i4_get_int(char *key_name,int* retval);
void i4_set_int(char *key_name,int i);

i4_bool i4_get_registry(i4_registry_type type, 
                        char *path, 
                        char *key_name,
                        char *buffer, int buf_length);

i4_bool i4_set_registry(i4_registry_type type, 
                        char *path, 
                        char *key_name,
                        char *buffer);



#endif
