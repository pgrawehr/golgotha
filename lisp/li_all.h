//this file is for the programmer's convenience. 
//It includes all lisp headers at once. 

#ifndef LISP_LI_ALL_H
#define LISP_LI_ALL_H
#include "lisp/lisp.h"
#include "lisp/li_types.h"
#include "lisp/li_class.h"
#include "lisp/li_init.h"
#include "lisp/abuse.h"
#include "lisp/li_alloc.h"
#include "lisp/li_error.h"
#include "lisp/li_load.h"
#include "lisp/li_vect.h"
#include "lisp/li_optr.h"

int li_read_token(char *&s, char *buffer);
int li_get_int(li_object *o, li_environment *env);
li_object *li_read_number_in_radix(int rd, char *tk);
double li_get_float(li_object *o, li_environment *env);
extern int li_last_line;
extern char li_last_file[150];

#endif
