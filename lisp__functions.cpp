/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

//This file is actually a 90% copy of the abuse lisp-engine system function core
//But it pi... me off to implement everything again. PG
#include "pch.h"
#include "error/error.h"
#include "main/main.h"
#include "init/init.h"
#include "file/file.h"
#include "lisp/li_types.h"
#include "lisp/lisp.h"
#include "status/status.h"
#include "threads/threads.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>

#define TYPE_CHECKING 1
#include "lisp/abuse.h"
//#include "bus_type.hpp"

//#include "lisp.hpp"
//#include "jmalloc.hpp"
//#include "macs.hpp"
//#include "specs.hpp"
//#include "dprint.hpp"
//#include "cache.hpp"
//#include "dev.hpp"


/* To bypass the whole garbage collection issue of lisp I am going to have seperate spaces
   where lisp objects can reside.  Compiled code and gloabal varibles will reside in permanant
   space.  Eveything else will reside in tmp space which gets thrown away after completion of eval. 
     system functions reside in permant space.
*/


//lisp_symbol *lsym_root=NULL;//no memory management here
long ltotal_syms=0;

//grow_stack<void> l_user_stack;       // stack user progs can push data and have it GCed
//grow_stack<void *>l_ptr_stack;      // stack of user pointers, user pointers get remapped on GC


char *space[4],*free_space[4];
int space_size[4],print_level=0,trace_level=0,trace_print_level=1000;
int total_user_functions;

li_symbol *colon_initial_contents,*colon_initial_element;//optional args for (make-array)
li_symbol *string_symbol,*list_symbol,*in_symbol,*do_symbol;

void lprint(void *i)
	{
	lip((li_object *)i);
	};

int current_space;  // normally set to TMP_SPACE, unless compiling or other needs 

inline int streq(char *s1, char *s2)   // when you don't need as much as strcmp, this is faster...
{
  while (*s1)
  {
    if (*(s1++)!=*(s2++)) 
      return 0;
  }
  return (*s2==0);
}


int break_level=0;

/*void l1print(void *block)
{
  if (!block)
    lprint(block);
  else
  {
    if (item_type(block)==L_CONS_CELL)
    {
      dprintf("(");
      for (;block && item_type(block)==L_CONS_CELL;block=CDR(block))
      {
	void *a=CAR(block);
	if (item_type(a)==L_CONS_CELL)
	  dprintf("[...]");
	else lprint(a);
      }
      if (block)
      {
        dprintf(" . ");
	lprint(block);
      }
      dprintf(")");
    } else lprint(block);
  }
}

void where_print(int max_lev=-1)
{
  dprintf("Main program\n");   
  if (max_lev==-1) max_lev=l_ptr_stack.son;
  else if (max_lev>=l_ptr_stack.son) max_lev=l_ptr_stack.son-1;

  for (int i=0;i<max_lev;i++)
  {
    dprintf("%d> ",i);
    lprint(*l_ptr_stack.sdata[i]);
  }
}

void print_trace_stack(int max_levels)
{
  where_print(max_levels);
}

void lbreak(const char *format, ...)
{
  break_level++;
  bFILE *old_file=current_print_file;
  current_print_file=NULL;
  char st[300];
  va_list ap;
  va_start(ap, format);
  vsprintf(st,format,ap);
  va_end(ap);
  dprintf("%s\n",st);
  int cont=0;
  do
  {
    dprintf("type q to quit\n");
    dprintf("%d. Break> ",break_level);
    dgets(st,300);
    if (!strcmp(st,"c") || !strcmp(st,"cont") || !strcmp(st,"continue"))    
      cont=1;
    else if (!strcmp(st,"w") || !strcmp(st,"where"))    
      where_print();
    else if (!strcmp(st,"q") || !strcmp(st,"quit"))    
      exit(1);
    else if (!strcmp(st,"e") || !strcmp(st,"env") || !strcmp(st,"environment"))    
    {
      dprintf("Enviorment : \nnot supported right now\n");

    } else if (!strcmp(st,"h") || !strcmp(st,"help") || !strcmp(st,"?"))    
    {
      dprintf("CLIVE Debugger\n");
      dprintf(" w, where : show calling parents\n"
	      " e, env   : show enviroment\n"
	      " c, cont  : continue if possible\n"
	      " q, quit  : quits the program\n"
	      " h, help  : this\n");
    }
    else
    {
      char *s=st;
      do
      {
				void *prog=compile(s);
				p_ref r1(prog);
				while (*s==' ' || *s=='\t' || *s=='\r' || *s=='\n') s++;
				lprint(eval(prog));
      } while (*s);
    }

  } while (!cont);
  current_print_file=old_file;
  break_level--;
}
*/
void lbreak(const char *format, ...)
	{
	//li_error(0,s);
	static char st[300];//must not throw local symbols
	va_list ap;
	va_start(ap, format);
	vsprintf(st,format,ap);
	va_end(ap);
	li_error(0,st);
	throw st;
	}

void dprintf(char *format,...)
	{
	char st[300];
	va_list ap;
	va_start(ap,format);
	vsprintf(st,format,ap);
	va_end(ap);
	i4_warning(st);
	}

/*
void need_perm_space(char *why)
{
  if (current_space!=PERM_SPACE && current_space!=GC_SPACE)
  {  
    lbreak("%s : action requires permanent space\n",why);
    //AfxThrowNotSupportedException();
	//throw "Error object thrown";
  }
}
*/



void *eval_block(void *list,li_environment *env)
{
  //p_ref r1(list);
  void *ret=NULL;
  while (list) 
  { 
    ret=eval(CAR(list));
    list=CDR(list);
  }
  return ret;
}

lisp_1d_array *new_lisp_1d_array(ushort size, void *rest, li_environment *env)
	{
	/*p_ref r11(rest);
	long s=sizeof(lisp_1d_array)+size*sizeof(void *);
	if (s<8) s=8;
	void *p=(lisp_1d_array *)lmalloc(s,current_space);
	((lisp_1d_array *)p)->type=L_1D_ARRAY;
	((lisp_1d_array *)p)->size=size;
	void **data=(void **)(((lisp_1d_array *)p)+1);
	memset(data,0,size*sizeof(void *));
	p_ref r1(p);
	*/
	lisp_1d_array *a=new lisp_1d_array();
	for (int i=0;i<size;i++)
		{
		a->add_element(li_nil);
		};
	if (rest)
		{
		li_object *x=eval(CAR(rest));
		if (x==colon_initial_contents)
			{
			x=eval(CAR(CDR(rest)));
			//data=(void **)(((lisp_1d_array *)p)+1);
			for (int i=0;i<size;i++,x=CDR(x))
				{
				if (!x) 
					{ 
					lprint(rest); 
					lbreak("(make-array) incorrect list length\n"); 
					
					}
				a->update_at(CAR(x),i);
				}
			if (x) { lprint(rest); lbreak("(make-array) incorrect list length\n");  }
			}
		else if (x==colon_initial_element)
			{
			x=eval(CAR(CDR(rest)));
			//data=(void **)(((lisp_1d_array *)p)+1);
			for (int i=0;i<size;i++)
				a->update_at(x,i);
			}
		else
			{
			lprint(x);
			dprintf("Warning: unknown option argument to make-array\n");
			
			}
		}
	
	return (a);
	}


li_int *new_lisp_number(int x)
	{
	return new li_int(x);
	}

li_float *new_lisp_number(float x)
	{
	return new li_float(x);
	}

li_float *new_lisp_number(double x)
	{
	return new li_float(x);
	}

lisp_fixed_point *new_lisp_fixed_point(double x)
{
  lisp_fixed_point *p=new li_float(x);
  return p;
}

/*
lisp_object_var *new_lisp_object_var(short number)
{//What is this used for?
  lisp_object_var *p=(lisp_object_var *)lmalloc(sizeof(lisp_object_var),current_space);
  p->type=L_OBJECT_VAR;
  p->number=number;
  return p;
}


struct lisp_pointer *new_lisp_pointer(void *addr)
{//and this?
  if (addr==NULL) return NULL;
  lisp_pointer *p=(lisp_pointer *)lmalloc(sizeof(lisp_pointer),current_space);
  p->type=L_POINTER;
  p->addr=addr;
  return p;
}
*/
lisp_character *new_lisp_character(unsigned char ch)
{
  lisp_character *c=new li_character(ch);
  return c;
}

lisp_string *new_lisp_string(char *string)
{
  lisp_string *s=new li_string(string);
  return s;
}

lisp_string *new_lisp_string(char *string, int length)
{
  
  lisp_string *s=new li_string(length);
  memcpy(s->value(),string,length);
  return s;
}

lisp_string *new_lisp_string(long length)
{
  return new li_string(length);
}
/*
#ifdef NO_LIBS
lisp_user_function *new_lisp_user_function(void *arg_list, void *block_list)
{
  p_ref r1(arg_list),r2(block_list);
  lisp_user_function *lu=(lisp_user_function *)lmalloc(sizeof(lisp_user_function),current_space);
  lu->type=L_USER_FUNCTION;
  lu->arg_list=arg_list;
  lu->block_list=block_list;
  return lu;
}
#else
lisp_user_function *new_lisp_user_function(long arg_list, long block_list)
{
  int sp=current_space;
  if (current_space!=GC_SPACE)
    current_space=PERM_SPACE;       // make sure all functions get defined in permanant space

  lisp_user_function *lu=(lisp_user_function *)lmalloc(sizeof(lisp_user_function),current_space);
  lu->type=L_USER_FUNCTION;
  lu->alist=arg_list;
  lu->blist=block_list;

  current_space=sp;

  return lu;
}
#endif


lisp_sys_function *new_lisp_sys_function(int min_args, int max_args, int fun_number)
{
  // sys functions should reside in permanant space
  lisp_sys_function *ls=(lisp_sys_function *)lmalloc(sizeof(lisp_sys_function),
						     current_space==GC_SPACE ? GC_SPACE : PERM_SPACE);
  ls->type=L_SYS_FUNCTION;
  ls->min_args=min_args;
  ls->max_args=max_args;
  ls->fun_number=fun_number;
  return ls;
}

lisp_sys_function *new_lisp_c_function(int min_args, int max_args, int fun_number)
{
  // sys functions should reside in permanant space
  lisp_sys_function *ls=(lisp_sys_function *)lmalloc(sizeof(lisp_sys_function),
						     current_space==GC_SPACE ? GC_SPACE : PERM_SPACE);
  ls->type=L_C_FUNCTION;
  ls->min_args=min_args;
  ls->max_args=max_args;
  ls->fun_number=fun_number;
  return ls;
}

lisp_sys_function *new_lisp_c_bool(int min_args, int max_args, int fun_number)
{
  // sys functions should reside in permanant space
  lisp_sys_function *ls=(lisp_sys_function *)lmalloc(sizeof(lisp_sys_function),
						     current_space==GC_SPACE ? GC_SPACE : PERM_SPACE);
  ls->type=L_C_BOOL;
  ls->min_args=min_args;
  ls->max_args=max_args;
  ls->fun_number=fun_number;
  return ls;
}

lisp_sys_function *new_user_lisp_function(int min_args, int max_args, int fun_number)
{
  // sys functions should reside in permanant space
  lisp_sys_function *ls=(lisp_sys_function *)lmalloc(sizeof(lisp_sys_function),
						     current_space==GC_SPACE ? GC_SPACE : PERM_SPACE);
  ls->type=L_L_FUNCTION;
  ls->min_args=min_args;
  ls->max_args=max_args;
  ls->fun_number=fun_number;
  return ls;
}

lisp_number *new_lisp_node(long num)
{
  lisp_number *n=(lisp_number *)lmalloc(sizeof(lisp_number),current_space);
  n->type=L_NUMBER;
  n->num=num;
  return n;
}

lisp_symbol *new_lisp_symbol(char *name)
{
  lisp_symbol *s=(lisp_symbol *)lmalloc(sizeof(lisp_symbol),current_space);  
  s->type=L_SYMBOL;
  s->name=new_lisp_string(name);
  s->value=l_undefined;
  s->function=l_undefined;
#ifdef L_PROFILE
  s->time_taken=0;
#endif
  return s;
}

lisp_number *new_lisp_number(long num)
{
  lisp_number *s=(lisp_number *)lmalloc(sizeof(lisp_number),current_space);
  s->type=L_NUMBER;
  s->num=num;
  return s;
}

*/
/*
cons_cell *new_cons_cell()
{
  cons_cell *c=(cons_cell *)lmalloc(sizeof(cons_cell),current_space);
  c->type=L_CONS_CELL;
  c->car=NULL;
  c->cdr=NULL;
  return c;
}
*/


li_list *new_cons_cell()
	{
	return new li_list(0,0);//we have a working garbage collection now!
	}

char *lerror(char *loc, char *cause)
{
  int lines;
  if (loc)
  {
    for (lines=0;*loc && lines<10;loc++)
    {
      if (*loc=='\n') lines++;
      dprintf("%c",*loc);
    }
    dprintf("\nPROGRAM LOCATION : \n");
  }
  if (cause)
    dprintf("ERROR MESSAGE : %s\n",cause);
  //lbreak("");
  
  return NULL;
}

void *nth(int num, void *list,li_environment *env)
{
  if (num<0) 
  { 
    lbreak("NTH: %d is not a nonnegative fixnum and therefore not a valid index\n",num);
    
  }

  while (list && num)
  {
    list=CDR(list);
    num--;
  }
  if (!list) return NULL;
  else return CAR(list);
}

double lnumber_valuef(void *lnumber)
{
  switch (item_type(lnumber))
  {
    case L_NUMBER :
      return ((li_int *)lnumber)->value();
    case L_FIXED_POINT :
      return ((li_float *)lnumber)->value();
    //case L_STRING :
    //  return (long)((li_string *)lnumber)->value();
    case L_CHARACTER :
      return (double)((li_character *)lnumber)->value();
    default :
    {
      //lprint(lnumber);
      //lbreak(" is not a number\n");
	  li_error(0,"USER: %O is not a number. ",(li_object*)lnumber);
      
    }
  }
  return 0;
}

long lnumber_value(void *lnumber)
{
  switch (item_type(lnumber))
  {
    case L_NUMBER :
      return ((li_int *)lnumber)->value();
    case L_FIXED_POINT :
      return (long)((li_float *)lnumber)->value();
    case L_STRING :
      return (long)((li_string *)lnumber)->value();
    case L_CHARACTER :
      return (long)((li_character *)lnumber)->value();
    default :
    {
      lprint(lnumber);
      lbreak(" is not a number\n");
      
    }
  }
  return 0;
}

char *lstring_value(void *lstring)
{
#ifdef TYPE_CHECKING
  if (item_type(lstring)!=(ltype)L_STRING)
  {
    lprint(lstring);
    lbreak(" is not a string\n");
    
  }
#endif
  return ((li_string*)lstring)->value();
}


/*
void *lisp_atom(void *i)
{
  if (item_type(i)==(ltype)L_CONS_CELL)
    return li_nil;
  else return true_symbol;
}

void *lisp_consp(void *i)
	{
	if(item_type(i)==(ltype)L_CONS_CELL)
		return true_symbol;
	else return li_nil;
	}
*/

unsigned short lcharacter_value(void *c)
{
#ifdef TYPE_CHECKING
  if (item_type(c)!=L_CHARACTER)
  {
    lprint(c);
    lbreak("is not a character\n");
    
  }
#endif
  return ((lisp_character *)c)->value();
}

double lfixed_point_value(void *c)
{
  switch (item_type(c))
  {
    case L_NUMBER :
      return ((lisp_number *)c)->value(); break;
    case L_FIXED_POINT :
      return (((lisp_fixed_point *)c)->value()); break;
    default :
    {
      lprint(c);
      lbreak(" is not a number\n");
      
    }
  }
  return 0;
}


void *lget_array_element(void *a, long x)
{
#ifdef TYPE_CHECKING
  if (item_type(a)!=L_1D_ARRAY)
  {
    lprint(a);
    lbreak("is not an array\n");
    
  }
#endif
  if (x>=((lisp_1d_array *)a)->size() || x<0)
  {
    lbreak("array reference out of bounds (%d)\n",x);
    
  }
  return ((lisp_1d_array *)a)->element_at(x);
}

inline double lisp_cos(double x)
	{
	return cos(x);
	}

inline double lisp_sin(double x)
	{
	return sin(x);
	}

inline double lisp_atan2(double dy, double dx)
	{
	if (dy==0) return dx>0?0.0f:180.0f;
	return atan(dx/dy);
	}
/*
long lisp_cos(long x)
{
  x=(x+FIXED_TRIG_SIZE/4)%FIXED_TRIG_SIZE;
  if (x<0) return sin_table[FIXED_TRIG_SIZE+x];
  else return sin_table[x];
}

long lisp_sin(long x)
{
  x=x%FIXED_TRIG_SIZE;
  if (x<0) return sin_table[FIXED_TRIG_SIZE+x];
  else return sin_table[x];
}

long lisp_atan2(long dy, long dx)
{
  if (dy==0)
  {
    if (dx>0) return 0;
    else return 180;
  } else if (dx==0)
  {
    if (dy>0) return 90;
    else return 270;
  } else
  {
    if (dx>0)
    {      
      if (dy>0)
      {
	if (abs(dx)>abs(dy))
	{
	  long a=dx*29/dy;
	  if (a>=TBS) return 0;
	  else return 45-atan_table[a];
	}
	else 
	{
	  long a=dy*29/dx;
	  if (a>=TBS) return 90;
	  else return 45+atan_table[a];
	}
      } else
      {
	if (abs(dx)>abs(dy))
	{
	  long a=dx*29/abs(dy);
	  if (a>=TBS)
	    return 0;
	  else
	    return 315+atan_table[a];
	}
	else
	{
	  long a=abs(dy)*29/dx;
	  if (a>=TBS)
	    return 260;
	  else
	    return 315-atan_table[a];
	}
      } 
    } else
    {
      if (dy>0)
      {
	if (abs(dx)>abs(dy))
	{
	  long a=-dx*29/dy;
	  if (a>=TBS)
	    return 135+45;
	  else
	    return 135+atan_table[a];
	}
	else 
	{
	  long a=dy*29/-dx;
	  if (a>=TBS)
	    return 135-45;
	  else
	    return 135-atan_table[a];
	}
      } else
      {
	if (abs(dx)>abs(dy))
	{
	  long a=-dx*29/abs(dy);
	  if (a>=TBS)
	    return 225-45;
	  else return 225-atan_table[a];
	}
	else 
	{
	  long a=abs(dy)*29/abs(dx);
	  if (a>=TBS)
	    return 225+45;	  
	  else return 225+atan_table[a];
	}
      } 
    }
  }  
}
*/


/*lisp_symbol *find_symbol(char *name)
{
  cons_cell *cs;
  for (cs=(cons_cell *)symbol_list;cs;cs=(cons_cell *)CDR(cs))
  {
    if (streq( ((char *)((lisp_symbol *)cs->car)->name)+sizeof(lisp_string),name))
      return (lisp_symbol *)(cs->car);   
  }
  return NULL;
}
*/

lisp_symbol *make_find_symbol(char *name)    // find a symbol, if it doesn't exsist it is created
{
  lisp_symbol *s=li_get_symbol(name);
  return s;
}


/*
lisp_symbol *find_symbol(char *name)
{
  lisp_symbol *p=lsym_root;
  while (p)
  {
    int cmp=strcmp(name,((char *)p->name)+sizeof(lisp_string));
    if (cmp==0) return p;
    else if (cmp<0) p=p->left;
    else p=p->right;
  }
  return NULL;
}
*/

/*
lisp_symbol *make_find_symbol(char *name)
{
  lisp_symbol *p=lsym_root;
  lisp_symbol **parent=&lsym_root;
  while (p)
  {
    int cmp=strcmp(name,((char *)p->name)+sizeof(lisp_string));
    if (cmp==0) return p;
    else if (cmp<0) 
    { 
      parent=&p->left;
      p=p->left;
    }
    else 
    {
      parent=&p->right;
      p=p->right;
    }
  }
  int sp=current_space;
  if (current_space!=GC_SPACE)
     current_space=PERM_SPACE;       // make sure all symbols get defined in permanant space

  p=(lisp_symbol *)jmalloc(sizeof(lisp_symbol),"lsymbol");
  p->type=L_SYMBOL;
  p->name=new_lisp_string(name);

  if (name[0]==':')     // constant, set the value to ourself
    p->value=p;
  else
    p->value=l_undefined;
  p->function=l_undefined;

  p->left=p->right=NULL;
  *parent=p;
  ltotal_syms++;

  current_space=sp;
  return p;
}
*/

/*
void *assoc(void *item, void *list)
{
  if (item_type(list)!=(ltype)L_CONS_CELL)
    return NULL;
  else
  {
    while (list)
    {
      if (lisp_eq(CAR(CAR(list)),item))
        return lcar(list);	     
      list=(cons_cell *)(CDR(list));
    }
  }
  return NULL;
}*/

long list_length(void *i)
{
  long x;

#ifdef TYPE_CHECKING
  if (i && item_type(i)!=(ltype)L_CONS_CELL)
  {
    lprint(i);
    lbreak(" is not a sequence\n");
    
  } 
#endif

  for (x=0;i;x++,i=li_cdr((li_object*)i,0));
  return x;
}

	 
/*
void *pairlis(void *list1, void *list2, void *list3)
{	  
  if (item_type(list1)!=(ltype)L_CONS_CELL || item_type(list1)!=item_type(list2))
    return NULL;

  void *ret=NULL;  
  long l1=list_length(list1),l2=list_length(list2);
  if (l1!=l2)
  {	   
    lprint(list1);
    lprint(list2);
    lbreak("... are not the same length (pairlis)\n");
    
  }
  if (l1!=0)
  {
    void *first=NULL,*last=NULL,*cur=NULL;
    p_ref r1(first),r2(last),r3(cur);
    while (list1)
    {
      cur=new_cons_cell();
      if (!first) first=cur;
      if (last)
        ((cons_cell *)last)->cdr=cur;
      last=cur;
	      
      cons_cell *cell=new_cons_cell();	      
      ((cons_cell *)cell)->car=lcar(list1);
      ((cons_cell *)cell)->cdr=lcar(list2);
      ((cons_cell *)cur)->car=cell;

      list1=((cons_cell *)list1)->cdr;
      list2=((cons_cell *)list2)->cdr;
    }
    ((cons_cell *)cur)->cdr=list3;
    ret=first;
  } else ret=NULL;
  return ret;
}
*/
li_object *lookup_symbol_function(li_symbol *symbol)
{
  return symbol->fun();
}

void set_symbol_function(li_symbol *symbol, li_object *function)
{
  symbol->set_fun(function);
}

inline li_object *lookup_symbol_value(li_object *symbol, li_environment *env)
{
	return li_get_value(li_symbol::get(symbol,env), env);
}

inline void set_variable_value(li_object *symbol, li_object *value, li_environment *env)
{
	li_set_value(li_symbol::get(symbol,env),value,env);
}
/*
lisp_symbol *add_sys_function(char *name, short min_args, short max_args, short number)
{
  need_perm_space("add_sys_function");
  lisp_symbol *s=make_find_symbol(name);
  if (s->function!=l_undefined)
  {
    lbreak("add_sys_fucntion -> symbol %s already has a function\n",name);
    
  }
  else s->function=new_lisp_sys_function(min_args,max_args,number);
  return s;
}

lisp_symbol *add_c_object(void *symbol, short number)
{
  need_perm_space("add_c_object");
  lisp_symbol *s=(lisp_symbol *)symbol;
  if (s->value!=l_undefined)
  {
    lbreak("add_c_object -> symbol %s already has a value\n",lstring_value(symbol_name(s)));
    
  }
  else s->value=new_lisp_object_var(number); 
  return NULL;
}

lisp_symbol *add_c_function(char *name, short min_args, short max_args, short number)
{
  total_user_functions++;
  need_perm_space("add_c_function");
  lisp_symbol *s=make_find_symbol(name);
  if (s->function!=l_undefined)
  {
    lbreak("add_sys_fucntion -> symbol %s already has a function\n",name);
    
  }
  else s->function=new_lisp_c_function(min_args,max_args,number);
  return s;
}

lisp_symbol *add_c_bool_fun(char *name, short min_args, short max_args, short number)
{
  total_user_functions++;
  need_perm_space("add_c_bool_fun");
  lisp_symbol *s=make_find_symbol(name);
  if (s->function!=l_undefined)
  {
    lbreak("add_sys_fucntion -> symbol %s already has a function\n",name);
    
  }
  else s->function=new_lisp_c_bool(min_args,max_args,number);
  return s;
}


lisp_symbol *add_lisp_function(char *name, short min_args, short max_args, short number)
{
  total_user_functions++;
  need_perm_space("add_c_bool_fun");
  lisp_symbol *s=make_find_symbol(name);
  if (s->function!=l_undefined)
  {
    lbreak("add_sys_fucntion -> symbol %s already has a function\n",name);
    
  }
  else s->function=new_user_lisp_function(min_args,max_args,number);
  return s;
}*/
/*
void skip_c_comment(char *&s)
{
  s+=2;
  while (*s && (*s!='*' || *(s+1)!='/'))
  {
    if (*s=='/' && *(s+1)=='*')
      skip_c_comment(s);
    else s++;
  }
  if (*s) s+=2;
}

long str_token_len(char *st)
{
  long x=1;
  while (*st && (*st!='"' || st[1]=='"'))
  {
    if (*st=='\\' || *st=='"') st++;    
    st++; x++;
  }
  return x;
}

int read_ltoken(char *&s, char *buffer)
{
  // skip space
  while (*s==' ' || *s=='\t' || *s=='\n' || *s=='\r' || *s==26) s++;
  if (*s==';')  // comment
  {
    while (*s && *s!='\n' && *s!='\r' && *s!=26) s++;
    return read_ltoken(s,buffer);
  } else if  (*s=='/' && *(s+1)=='*')   // c style comment
  {
    skip_c_comment(s);
    return read_ltoken(s,buffer);    
  }
  else if (*s==0)
    return 0;
  else if (*s==')' || *s=='(' || *s=='\'' || *s=='`' || *s==',' || *s==26)
  {
    *(buffer++)=*(s++);
    *buffer=0;
  } else if (*s=='"')    // string
  {
    *(buffer++)=*(s++);          // don't read off the string because it
                                 // may be to long to fit in the token buffer
                                 // so just read the '"' so the compiler knows to scan the rest.
    *buffer=0;
  } else if (*s=='#')
  {
    *(buffer++)=*(s++);      
    if (*s!='\'')
      *(buffer++)=*(s++);      
    *buffer=0;
  } else
  {
    while (*s && *s!=')' && *s!='(' && *s!=' ' && *s!='\n' && *s!='\r' && *s!='\t' && *s!=';' && *s!=26)
      *(buffer++)=*(s++);      
    *buffer=0;
  }
  return 1;    
}
*/
/*
char n[MAX_LISP_TOKEN_LEN];  // assume all tokens will be < 200 characters

int end_of_program(char *s)
{
  return !read_ltoken(s,n);
}


void push_onto_list(void *object, void *&list)
{
  p_ref r1(object),r2(list);
  cons_cell *c=new_cons_cell();
  c->car=object;
  c->cdr=list;
  list=c;
}

void *comp_optimize(void *list);
//This is abuse code!! 
//looks _very_ similar to the golgotha tokenizer...
void *compile(char *&s)
{
  void *ret=NULL;
  if (!read_ltoken(s,n))
    lerror(NULL,"unexpected end of program");
  if (streq(n,"nil"))
    return NULL;
  else if (toupper(n[0])=='T' && !n[1])
    return true_symbol;
  else if (n[0]=='\'')                    // short hand for quote function
  {
    void *cs=new_cons_cell(),*c2=NULL;
    p_ref r1(cs),r2(c2);

    ((cons_cell *)cs)->car=quote_symbol;
    c2=new_cons_cell();
    ((cons_cell *)c2)->car=compile(s);
    ((cons_cell *)c2)->cdr=NULL;
    ((cons_cell *)cs)->cdr=c2;
    ret=cs;
  }
  else if (n[0]=='`')                    // short hand for backquote function
  {
    void *cs=new_cons_cell(),*c2=NULL;
    p_ref r1(cs),r2(c2);

    ((cons_cell *)cs)->car=backquote_symbol;
    c2=new_cons_cell();
    ((cons_cell *)c2)->car=compile(s);
    ((cons_cell *)c2)->cdr=NULL;
    ((cons_cell *)cs)->cdr=c2;
    ret=cs;
  }  else if (n[0]==',')              // short hand for comma function
  {
    void *cs=new_cons_cell(),*c2=NULL;
    p_ref r1(cs),r2(c2);

    ((cons_cell *)cs)->car=comma_symbol;
    c2=new_cons_cell();
    ((cons_cell *)c2)->car=compile(s);
    ((cons_cell *)c2)->cdr=NULL;
    ((cons_cell *)cs)->cdr=c2;
    ret=cs;
  }
  else if (n[0]=='(')                     // make a list of everything in ()
  {
    void *first=NULL,*cur=NULL,*last=NULL;   
    p_ref r1(first),r2(cur),r3(last);
    int done=0;
    do
    {
      char *tmp=s;
      if (!read_ltoken(tmp,n))           // check for the end of the list
        lerror(NULL,"unexpected end of program");
      if (n[0]==')') 
      {
				done=1;
				read_ltoken(s,n);                // read off the ')'
      }
      else
      {     
				if (n[0]=='.' && !n[1])
				{
				  if (!first)
				    lerror(s,"token '.' not allowed here\n");	      
				  else 
				  {
				    read_ltoken(s,n);              // skip the '.'
				    ((cons_cell *)last)->cdr=compile(s);          // link the last cdr to 
				    last=NULL;
				  }
				} else if (!last && first)
				  lerror(s,"illegal end of dotted list\n");
				else
				{		 
				  cur=new_cons_cell();
				  p_ref r1(cur);
				  if (!first) first=cur;
				  ((cons_cell *)cur)->car=compile(s);	
				  if (last)
				    ((cons_cell *)last)->cdr=cur;
				  last=cur;
				}
      } 
    } while (!done);
    ret=comp_optimize(first);

  } else if (n[0]==')')
    lerror(s,"mismatched )");
  else if (isdigit(n[0]) || (n[0]=='-' && isdigit(n[1])))
  {
    lisp_number *num=new_lisp_number(0);
    sscanf(n,"%d",&num->num);
    ret=num;
  } else if (n[0]=='"')
  {
    ret=new_lisp_string(str_token_len(s));
    char *start=lstring_value(ret);
    for (;*s && (*s!='"' || s[1]=='"');s++,start++)
    {
      if (*s=='\\')
      {
				s++;
				if (*s=='n') *start='\n';
				if (*s=='r') *start='\r';
				if (*s=='t') *start='\t';
				if (*s=='\\') *start='\\';
      } else *start=*s;
      if (*s=='"') s++;
    }
    *start=0;
    s++;
  } else if (n[0]=='#')
  {
    if (n[1]=='\\')
    {
      read_ltoken(s,n);                   // read character name
      if (streq(n,"newline"))
        ret=new_lisp_character('\n');
      else if (streq(n,"space"))
        ret=new_lisp_character(' ');       
      else 
        ret=new_lisp_character(n[0]);       
    }
    else if (n[1]==0)                           // short hand for function
    {
      void *cs=new_cons_cell(),*c2=NULL;
      p_ref r4(cs),r5(c2);
      ((cons_cell *)cs)->car=make_find_symbol("function");
      c2=new_cons_cell();
      ((cons_cell *)c2)->car=compile(s);
      ((cons_cell *)cs)->cdr=c2;
      ret=cs;
    }
    else
    {
      lbreak("Unknown #\\ notation : %s\n",n);
      
    }
  } else return make_find_symbol(n);
  return ret;
}
*/
/*
static void lprint_string(char *st)
{
    dprintf(st);
}
*/
//objects now print themselves, needs replacing
void lprint(li_object *i)
{
  print_level++;
  lip(i);
  /*if (!i)
    lprint_string("nil");
  else
  {
    switch ((short)item_type(i))
    {      
      case L_CONS_CELL :
      {
				cons_cell *cs=(cons_cell *)i;
        lprint_string("(");
        for (;cs;cs=(cons_cell *)lcdr(cs))	
				{
				  if (item_type(cs)==(ltype)L_CONS_CELL)
				  {
			            lprint(cs->car);
				    if (cs->cdr)
				      lprint_string(" ");
				  }
				  else
				  {
				    lprint_string(". ");
				    lprint(cs);
				    cs=NULL;
				  }
				}
        lprint_string(")");
      }
      break;
      case L_NUMBER :
      {
				char num[10];
				sprintf(num,"%d",((lisp_number *)i)->num);
        lprint_string(num);
      }
      break;
      case L_SYMBOL :        
        lprint_string((char *)(((lisp_symbol *)i)->name)+sizeof(lisp_string));
      break;
      case L_USER_FUNCTION :
      case L_SYS_FUNCTION :      
        lprint_string("err... function?");
      break;
      case L_C_FUNCTION :
        lprint_string("C function, returns number\n");
      break;
      case L_C_BOOL :
        lprint_string("C boolean function\n");
      break;
      case L_L_FUNCTION :
        lprint_string("External lisp function\n");
			break;
      case L_STRING :
      {
				if (current_print_file)
				 	lprint_string(lstring_value(i));
				else
         	dprintf("\"%s\"",lstring_value(i));
      }
      break;

      case L_POINTER :
      {

      }
      break;
      case L_FIXED_POINT :
      { 
				char num[20];
				sprintf(num,"%g",(lfixed_point_value(i)>>16)+
					      ((lfixed_point_value(i)&0xffff))/(double)0x10000); 
				lprint_string(num);
      } break;
      case L_CHARACTER :
      {
				if (current_print_file)
				{
				  uchar ch=((lisp_character *)i)->ch;
				  current_print_file->write(&ch,1);
				} else
				{
				  unsigned short ch=((lisp_character *)i)->ch;
				  dprintf("#\\");
				  switch (ch)
				  {
				    case '\n' : 
				    { dprintf("newline"); break; }
				    case ' ' : 
				    { dprintf("space"); break; }
				    default :
				      dprintf("%c",ch);
				  }
				}       
      } break;
      case L_OBJECT_VAR :
      {
				l_obj_print(((lisp_object_var *)i)->number);
      } break;
      case L_1D_ARRAY :
      {
				lisp_1d_array *a=(lisp_1d_array *)i;
				void **data=(void **)(a+1);
				dprintf("#(");
				for (int j=0;j<a->size;j++)
				{
				  lprint(data[j]);
				  if (j!=a->size-1)
				    dprintf(" ");
				}
				dprintf(")");
      } break;
      case L_COLLECTED_OBJECT :
      {
				lprint_string("GC_refrence->");
				lprint(((lisp_collected_object *)i)->new_reference);
      } break;
      default :
        dprintf("Shouldn't happen\n");
    }
  }*/
  print_level--;
  dprintf("\n");
}



void *eval_sys_function(lisp_sys_function *fun, void *arg_list, li_environment *env);

/*
void *eval_function(lisp_symbol *sym, void *arg_list, li_environment *env)
{


#ifdef TYPE_CHECKING  
  int args,req_min,req_max;
  if (item_type(sym)!=L_SYMBOL)
  {
    lprint(sym);
    lbreak("EVAL : is not a function name (not symbol either)");
    
  } 
#endif

  void *fun=(lisp_sys_function *)(((lisp_symbol *)sym)->function);
  p_ref ref2( fun  );

  // make sure the arguments given to the function are the correct number
  ltype t=item_type(fun);

#ifdef TYPE_CHECKING
  switch (t)
  {
    case L_SYS_FUNCTION :
    case L_C_FUNCTION :
    case L_C_BOOL :
    case L_L_FUNCTION :    
    {
      req_min=((lisp_sys_function *)fun)->min_args;
      req_max=((lisp_sys_function *)fun)->max_args;
    } break;
    case L_USER_FUNCTION :
    {
      return eval_user_fun(sym,arg_list);
    } break;
    default :
    {
      lprint(sym);
      lbreak(" is not a function name");
      	
    } break;
  }

  if (req_min!=-1)
  {
    void *a=arg_list;
    for (args=0;a;a=CDR(a)) args++;    // count number of paramaters

    if (args<req_min)
    {
      lprint(arg_list);
      lprint(sym->name);
      lbreak("\nToo few parameters to function\n");
      
    } else if (req_max!=-1 && args>req_max)
    {
      lprint(arg_list);
      lprint(sym->name);
      lbreak("\nToo many parameters to function\n");
      
    }
  }
#endif

 


  p_ref ref1(arg_list);
  void *ret=NULL;

  switch (t)
  {
    case L_SYS_FUNCTION :
    { ret=eval_sys_function( ((lisp_sys_function *)fun),arg_list); } break;    
    case L_L_FUNCTION :
    { ret=l_caller( ((lisp_sys_function *)fun)->fun_number,arg_list); } break;
    case L_USER_FUNCTION :
    {
      return eval_user_fun(sym,arg_list);
    } break;
    case L_C_FUNCTION :
    {
      void *first=NULL,*cur=NULL;
      p_ref r1(first),r2(cur);
      while (arg_list)
      {
				if (first)
				  cur=((cons_cell *)cur)->cdr=new_cons_cell();
				else
				  cur=first=new_cons_cell();
			
				void *val=eval(CAR(arg_list));
				((cons_cell *)cur)->car=val;
				arg_list=lcdr(arg_list);
      }        
      ret=new_lisp_number(c_caller( ((lisp_sys_function *)fun)->fun_number,first));
    } break;
    case L_C_BOOL :
    {
      void *first=NULL,*cur=NULL;
      p_ref r1(first),r2(cur);
      while (arg_list)
      {
				if (first)
				  cur=((cons_cell *)cur)->cdr=new_cons_cell();
				else
				  cur=first=new_cons_cell();
			
				void *val=eval(CAR(arg_list));
				((cons_cell *)cur)->car=val;
				arg_list=lcdr(arg_list);
      }        

      if (c_caller( ((lisp_sys_function *)fun)->fun_number,first))
        ret=true_symbol;
      else ret=NULL;
    } break;
    default :
      fprintf(stderr,"not a fun, sholdn't happed\n");
  }



  return ret;
}	  

*/
li_object *li_user_function_evaluator(li_object *o, li_environment *env);

void *mapcar(void *arg_list,li_environment *env)
{
  
  
  //sym must be either 'SYMBOL or
  //(lambda ....) or (function (lambda...))
  //if (CAR(sym)!=li_function_symbol)
//	  lbreak("mapcar: First argument must be 'symbol or '(lambda ...)");
  //
  //3 possible cases known:
  //1st: (mapcar VAR LIST) - use the function with the name of the _value_ of VAR (usefull?)
  //2nd: (mapcar 'FN LIST) - use the function FN
  //3rd: (mapcar (lambda ...) LIST) - use the lambda function.

  //instead of (lambda ...) (function ...) is also possible


  

  //sym=CAR(CDR(sym));
  li_object *sym=CAR(arg_list);//don't eval, might be a lambda - expr
  int num_args=list_length(CDR(arg_list)),i,stop=0;
  if (!num_args) return 0;

  //void **arg_on=(void **)jmalloc(sizeof(void *)*num_args,"mapcar tmp array");
  li_vector *v=new li_vector();
  li_object_vect_type *arg_on=v->reserved();//How to solve this problem here: 
  //arg_on is destructed on exit, freing the copy of the pointer of the vector??
  //we must use a pointer to the data vector instead of a reference
  cons_cell *list_on=CDR(arg_list);
  //long old_ptr_son=l_ptr_stack.son;
  li_object *alist=0;
  for (i=0;i<num_args;i++)
  {
    alist=li_eval(CAR(list_on),env);
    arg_on->add(alist);//li_lambda_eval (or li_call) will eval again, so be carefull.
    //l_ptr_stack.push(&arg_on[i]);

    list_on=(cons_cell *)CDR(list_on);
    if (!alist) stop=1;
  }
  
  if (stop)
  {
    //jfree(arg_on);
    return NULL;
  }

  li_list *na_list=NULL,*return_list=NULL,*last_return=NULL;
  int callno=0;
  do
  {
    na_list=NULL;          // create a cons list with all of the parameters for the function

    li_list *first=0;                       // save the start of the list
	li_list *current=0;
	li_object *curelem=0;
    for (i=0;!stop &&i<num_args;i++)
    {
	  current=li_list::get(arg_on->operator [](i),env);
	  curelem=li_nth(current,callno,env);
	  if (!curelem)
		  {
		  stop=1;
		  continue;
		  }
      if (!na_list)
        first=na_list=new_cons_cell();
      else
      {
        na_list->set_next(new_cons_cell());
		na_list=(li_list *)CDR(na_list);
      }
	  na_list->set_data(curelem);

	  //This is quite dangerous, because it might fool the gc.
      /*
      if (arg_on->operator [](i))
      {
	  //the paramenters are the nth element of the arg_on[i]'s.
	  //we advance arg_on so we need not count the calls
				na_list->set_data(CAR(arg_on->operator [](i)));
				arg_on->operator [](i)=CDR(arg_on->operator [](i));
      }
      else stop=1;        
	  */
	  
	  
    }
    if (!stop)
    {
      li_list *c=new_cons_cell();
	 

	//	c->set_data(li_call((li_symbol *)sym,first,env));
	//  else
	//	  {
		  c->set_data(li_lambda_eval(sym,first,env,LEF_NOEVALONBIND));
	//	  }
      if (return_list)
        last_return->set_next(c);
      else
        return_list=c;
      last_return=c;
    }
	callno++;
  }
  while (!stop);
  //l_ptr_stack.son=old_ptr_son;

  //jfree(arg_on);
  arg_on=0;//fool any atempt to destroy this object.
  v=0;
  return return_list;
}

void *concatenate(void *prog_list,li_environment *env)
	{
	void *el_list=CDR(prog_list);
	//p_ref ref1(prog_list),ref2(el_list);
	void *ret=NULL;
	void *rtype=eval(CAR(prog_list));//the type requested
	
	long len=0;                                // determine the length of the resulting string
	if (rtype==string_symbol)
		{
		int elements=list_length(el_list);       // see how many things we need to concat
		if (!elements) 
			ret=new_lisp_string("");
		else
			{
			//void **str_eval=(void **)jmalloc(elements*sizeof(void *),"tmp eval array");
			//int i,old_ptr_stack_start=l_ptr_stack.son;
			li_object **str_eval=(li_object **)malloc(elements*sizeof(li_object *));
			// evaluate all the strings and count their lengths
			int i;
			for (i=0;i<elements;i++,el_list=CDR(el_list))
				{
				str_eval[i]=eval(CAR(el_list));
				//l_ptr_stack.push(&str_eval[i]);
				
				switch ((short)item_type(str_eval[i]))
					{
					case L_CONS_CELL :
						{
						cons_cell *char_list=(cons_cell *)str_eval[i];
						while (char_list)
							{
							if (item_type(CAR(char_list))==(ltype)L_CHARACTER)
								len++;
							else
								{
								lprint(str_eval[i]);
								lbreak(" is not a character\n");		
								
								}
							char_list=(cons_cell *)CDR(char_list);
							}
						} break;
					case L_STRING : len+=strlen(lstring_value(str_eval[i])); break;
					default :
						lprint(prog_list);
						lbreak("USER: concatenate: Type not supported\n");
						
						break;
						
					}
				}
			lisp_string *st=new_lisp_string(len+1);
			char *s=lstring_value(st);
			
			// now add the string up into the new string
			for (i=0;i<elements;i++)
				{
				switch ((short)item_type(str_eval[i]))
					{
					case L_CONS_CELL :
						{
						cons_cell *char_list=(cons_cell *)str_eval[i];
						while (char_list)
							{
							if (item_type(CAR(char_list))==L_CHARACTER)
								*(s++)=((lisp_character *)CAR(char_list))->value();
							char_list=(cons_cell *)CDR(char_list);
							}
						} break;
					case L_STRING : 
						{
						memcpy(s,lstring_value(str_eval[i]),strlen(lstring_value(str_eval[i])));
						s+=strlen(lstring_value(str_eval[i]));
						} break;
					default : ;     // already checked for, but make compiler happy
					}
				}
			free(str_eval);
			//l_ptr_stack.son=old_ptr_stack_start;   // restore pointer GC stack
			*s=0;      
			ret=st;
			}
		}
	else 
		{
		if (rtype==list_symbol)
			{
			int elements=list_length(el_list);
			if (!elements) return li_nil;
			//for this to work as required by the standard, the types must define a ->copy() method

			li_list *ll,*lst;
			ll=li_list::get(li_get_type(CAR(el_list)->type())->copy(CAR(el_list)),env);
			lst=ll;
			for (int i=1;i<elements;i++)
				{
				while(ll->next()!=0&&ll->next()!=li_nil)
					{
					ll=(li_list*)CDR(ll);
					}
				ll->set_next(new li_list(CAR(el_list)));
				
				el_list=CDR(el_list);
				}
			ret=lst;

			}
		else
		{
		lprint(prog_list);
		lbreak("concat operation not supported, try 'string or 'list\n");
		
		}
		}
	return ret;
	}

/*
void *backquote_eval(void *args)
{
  if (item_type(args)!=L_CONS_CELL)
    return args;
  else if (args==NULL)
    return NULL;
  else if ((lisp_symbol *) (((cons_cell *)args)->car)==comma_symbol)
    return eval(CAR(CDR(args)));
  else
  {
    void *first=NULL,*last=NULL,*cur=NULL;
    p_ref ref1(first),ref2(last),ref3(cur),ref4(args);
    while (args)
    {
      if (item_type(args)==L_CONS_CELL)
      {
	if (CAR(args)==comma_symbol)               // dot list with a comma?
	{
	  ((cons_cell *)last)->cdr=eval(CAR(CDR(args)));
	  args=NULL;
	}
	else
	{
	  cur=new_cons_cell();
	  if (first)
	    ((cons_cell *)last)->cdr=cur;
	  else 
            first=cur;
	  last=cur;
          ((cons_cell *)cur)->car=backquote_eval(CAR(args));
 	  args=CDR(args);
	}
      } else
      {
	((cons_cell *)last)->cdr=backquote_eval(args);
	args=NULL;
      }

    }
    return (void *)first;
  }
  return NULL;       // for stupid compiler messages
}
*/

void invcall()
	{
	lbreak("Call to invalid system function (shan't happen)");
	}

void *eval_sys_function(lisp_sys_function *fun, void *arg_list, li_environment *env)
{
  //p_ref ref1(arg_list);
  void *ret=NULL;
  switch (fun->fun_number)
  {
    case 0 :                                                    // print
    { 
      ret=NULL;
      while (arg_list)
      {
        ret=eval(CAR(arg_list));  arg_list=CDR(arg_list);
	lprint(ret); 
      }
      return ret; 
    } break;
    case 1 :                                                    // car
    { ret=lcar(eval(CAR(arg_list))); } break;
    case 2 :                                                    // cdr
    { ret=lcdr(eval(CAR(arg_list))); } break;
    case 3 :                                                    // length
    { 
      invcall();
    } break;						
    case 4 :                                                    // list
    { 
      invcall();
    } break;
    case 5 :                                             // cons
    { 
	  invcall();
    } break;
    case 6 :                                             // quote
    //ret=CAR(arg_list);
		invcall();
    break;
    case 7 :                                             // eq
    {
      invcall();
    } break;
    case 24 :                                             // equal
    {
      invcall();
    } break;
    case 8 :                                           // +
    {
      invcall();
    }
    break;
    case 28 :                                          // *
    {
	invcall();
	}
    break;
    case 29 :                                           // /
    {
      invcall();
    }
    break;
    case 9 :                                           // -
    {
      invcall();
    }
    break;
    case 10 :                                         // if
    {
      invcall();
      
    } break;
    case 63 :
    case 11 :                                         // setf
    {     
		invcall();
	

	
    } break;
    case 12 :                                      // symbol-list
      ret=NULL;
    break;
    case 13 :                                      // assoc
    {
	invcall();
      /*void *item=eval(CAR(arg_list));
      p_ref r1(item);
      void *list=(cons_cell *)eval(CAR(CDR(arg_list)));
      p_ref r2(list);
      ret=assoc(item,(cons_cell *)list);*/
    } break;
    case 20 :                                       // not is the same as null
    case 14 :                                       // null
    if (eval(CAR(arg_list))==NULL) ret=true_symbol; else ret=NULL;
    break;
    case 15 :                                       // acons
    {
	  invcall();
      /*void *i1=eval(CAR(arg_list)),*i2=eval(CAR(CDR(arg_list)));
      p_ref r1(i1);
      cons_cell *cs=new_cons_cell();
      cs->car=i1;
      cs->cdr=i2;
      ret=cs;*/
    } break;

    case 16 :                                       // pairlis
    {	 
		invcall();
		/*
      l_user_stack.push(eval(CAR(arg_list))); arg_list=CDR(arg_list);
      l_user_stack.push(eval(CAR(arg_list))); arg_list=CDR(arg_list);
      void *n3=eval(CAR(arg_list));
      void *n2=l_user_stack.pop(1);
      void *n1=l_user_stack.pop(1);      
      ret=pairlis(n1,n2,n3);*/
    } break;
    case 17 :                                      // map
    {
	  mapcar(arg_list,env);
    
    } break;
    case 19 :                                       // zerop
    { 
	int i=li_get_int(li_eval(CAR(arg_list),env),env);
	if (i==0) ret=li_true_sym; else ret=li_nil; 

	}break;
    case 21 :                                           // and
    {
      void *l=arg_list;
      
      ret=true_symbol;
      while (l)
      {
	    ret=eval(CAR(l));//retval is either nil or the retval of the last expression
		if (!ret||ret==li_nil)
		{
		  //ret=li_nil;
		  l=0;             // short-circuit
		} else l=CDR(l);
      }
    } break;
    case 22 :                                           // or
    {
      void *l=arg_list;
      //p_ref r1(l);
      ret=NULL;
      while (l)
      {
	    ret=eval(CAR(l));
		if (ret&&ret!=li_nil)
		{
		ret=true_symbol;
		l=NULL;            // short circuit
		} else l=CDR(l);
      }
    } break;
    case 23 :                                          // progn
    { ret=eval_block(arg_list,env); } break;
    case 25 :                                        // concatenate
      ret=concatenate(arg_list,env);
    break;
    case 26 :                                        // char-code
    {
      void *i=eval(CAR(arg_list));    
      //p_ref r1(i);
      ret=NULL;
      switch (item_type(i))
      {
	case L_CHARACTER : 
	{ ret=new_lisp_number(((lisp_character *)i)->value()); } break;
	case L_STRING :
	{  ret=new_lisp_number(*lstring_value(i)); } break;
	default :
	{
	  lprint(i);
	  lbreak(" is not character type\n");
	  
	}
      }		    
    } break;
    case 27 :                                        // code-char
    {
      void *i=eval(CAR(arg_list));
      //p_ref r1(i);
      if (item_type(i)!=L_NUMBER)
      {
	lprint(i);
	lbreak(" is not number type\n");
	
      }
      ret=new_lisp_character(((lisp_number *)i)->value());
    } break;
    case 30 :                                       // cond
    {
      void *block_list=arg_list;
      //p_ref r1(block_list);
      if (!CAR(block_list)) ret=NULL;
      else
      {
		ret=NULL;
        while (block_list)
		{
		li_object *test=(li_object *)CAR(CAR(block_list));
		if (test->type()==LI_LIST)
			  {
			  test=li_eval(test,env);
			  }
		if (test&&test!=li_nil)
			{
			block_list=CDR(CAR(block_list));
			while (block_list)
				{
				ret=eval(CAR(block_list));
				block_list=CDR(block_list);
				}
			break;
			}
		block_list=CDR(block_list);
		}
      }
    } break;
    case 31 :                                       // select, not CLISP
    {
      li_object *selector=eval(CAR(arg_list));
      li_object *sel=CDR(arg_list);
      //p_ref r1(selector),r2(sel);
      while (sel)
      {
	  li_list *selexpr=li_make_list(selector,eval(CAR(CAR(sel))));
	  li_object *test=li_call("li_eql",selexpr,env);
	if (test&&test!=li_nil)
	{
	  sel=CDR(CAR(sel));
	  while (sel)
	  {
	    ret=eval(CAR(sel));
	    sel=CDR(sel);
	  }
	  sel=NULL;
	} else sel=CDR(sel);
      }
    } break;
    case 32 :                                      // apply    
		{//UH, UH: NEWER EVER manipulate the arg_list itself, if not required by function description
		//may even alter the code of a user-function to complete garbage!!!
		li_object *n1=CAR(arg_list);
		/*li_object *lst=(li_object*)arg_list,*cur=0;
		while(CDR(CDR(lst))&&(CDR(CDR(lst))!=li_nil))
			{
			lst=CDR(lst);
			}
		cur=eval(CAR(CDR(lst)));
		((li_list*)lst)->set_next(cur);*/
		li_object *lst=li_call("list*",CDR(arg_list),env);
	    ret=li_lambda_eval(n1,lst,env,LEF_NOEVALONBIND);
		}

    break;
    case 33 :                                      // mapcar
      ret=mapcar(arg_list,env);    
	  break;
    case 34 :                                      // funcall
    {
	  li_object *n1=CAR(arg_list);
	  ret=li_lambda_eval(n1,CDR(arg_list),env);
      //ret=li_call(n1,CDR(arg_list),env);      
    } break;
    case 35 :                                                   // >
    {
      double n1=lnumber_valuef(eval(CAR(arg_list)));
      double n2=lnumber_valuef(eval(CAR(CDR(arg_list))));
      if (n1>n2) ret=li_true_sym; else ret=li_nil;
    }
    break;      
    case 36 :                                                   // <
    {
      double n1=lnumber_valuef(eval(CAR(arg_list)));
      double n2=lnumber_valuef(eval(CAR(CDR(arg_list))));
      if (n1<n2) ret=li_true_sym; else ret=li_nil;
    }    
    break;
    case 47 :                                                   // >=
    {
      double n1=lnumber_valuef(eval(CAR(arg_list)));
      double n2=lnumber_valuef(eval(CAR(CDR(arg_list))));
      if (n1>=n2) ret=li_true_sym; else ret=li_nil;
    }
    break;      
    case 48 :                                                   // <=
    {
      double n1=lnumber_valuef(eval(CAR(arg_list)));
      double n2=lnumber_valuef(eval(CAR(CDR(arg_list))));
      if (n1<=n2) ret=li_true_sym; else ret=li_nil;
    }    
    break;

    case 37 :                                                  // tmp-space
      current_space=TMP_SPACE;
      ret=true_symbol;
    break;
    case 38 :                                                  // perm-space
      current_space=PERM_SPACE;
      ret=true_symbol;
    break;
    case 39 ://symbol-name
      void *symb;
      symb=eval(CAR(arg_list));
#ifdef TYPE_CHECKING
      if (item_type(symb)!=L_SYMBOL)
      {
	lprint(symb);
	lbreak(" is not a symbol (symbol-name)\n");
	
      }
#endif
      ret=((lisp_symbol *)symb)->name();    
    break;
    case 40 :                                                  // trace
      trace_level++;
      if (arg_list)
        trace_print_level=lnumber_value(eval(CAR(arg_list)));
      ret=true_symbol;
    break;
    case 41 :                                                  // untrace
      if (trace_level>0)
      {
				trace_level--;
				ret=true_symbol;
      } else ret=NULL;
    break;
    case 42 :                                                 // digitstr
    {
      char tmp[50],*tp;
      long num=lnumber_value(eval(CAR(arg_list)));
      long dig=lnumber_value(eval(CAR(CDR(arg_list))));
      tp=tmp+49;
      *(tp--)=0;
      for (;num;)
      {
				int d;
				d=num%10;
				*(tp--)=d+'0';
				num/=10;
				dig--;
      }
      while (dig--)
        *(tp--)='0';	
      ret=new_lisp_string(tp+1);      
    } break;
    case 98 :  
    case 66 :
    case 43 :                                                // compile-file
    {
		//invcall();
	  ret=li_true_sym;//we don't compile anything, just interpret if necessary
	/*
      void *fn=eval(CAR(arg_list));
      char *st=lstring_value(fn);
      p_ref r1(fn);
      bFILE *fp;
      if (fun->fun_number==98)                              // local load
        fp=new jFILE(st,"rb");
      else
        fp=open_file(st,"rb");

      if (fp->open_failure())
      {
				delete fp;
				if (DEFINEDP(symbol_value(load_warning)) && symbol_value(load_warning))
			          dprintf("Warning : file %s does not exists\n",st);
				ret=NULL;
      }
      else
      {
				long l=fp->file_size();
				char *s=(char *)jmalloc(l+1,"loaded script");
				if (!s)
				{
				  printf("Malloc error in load_script\n");  
			
				}
			
				fp->read(s,l);  
				s[l]=0;
				delete fp;
				char *cs=s;
			#ifndef NO_LIBS      
				char msg[100];
				sprintf(msg,"(load \"%s\")",st);
				if (stat_man) stat_man->push(msg,NULL);
				crc_man.get_filenumber(st);               // make sure this file gets crc'ed
			#endif
				void *compiled_form=NULL;
				p_ref r11(compiled_form);
				while (!end_of_program(cs))  // see if there is anything left to compile and run
				{
			#ifndef NO_LIBS      
				  if (stat_man) stat_man->update((cs-s)*100/l);
			#endif
				  void *m=mark_heap(TMP_SPACE);
				  compiled_form=compile(cs);
				  eval(compiled_form);
				  compiled_form=NULL;
				  restore_heap(m,TMP_SPACE);
				}	
			#ifndef NO_LIBS
                                if (stat_man) stat_man->update(100);
				if (stat_man) stat_man->pop();
			#endif      
				jfree(s);
				ret=fn;
			*/
      
    } break;
    case 44 :                                                 // abs
		{
		li_object *n=eval(CAR(arg_list));
		if (n->type()==LI_INT) 
			ret=new_lisp_number((int)abs(lnumber_value(n)));
		else
			ret=new_lisp_number(fabs(lnumber_valuef(n)));
      //ret=new_lisp_number(fabs(lnumber_valuef(eval(CAR(arg_list))))); break;
		}
		break;
    case 45 :                                                 // min
    {
	  li_object *n1=eval(CAR(arg_list)),*n2=eval(CAR(CDR(arg_list)));
	  if (n1->type()==LI_INT&&n2->type()==LI_INT)
		  {
		  int x=lnumber_value(n1),y=lnumber_value(n2);
		  if (x<y) ret=new_lisp_number(x); else ret=new_lisp_number(y);
		  }
	  else
		  {
	      double x=lnumber_valuef(n1),y=lnumber_valuef(n2);
          if (x<y) ret=new_lisp_number(x); else ret=new_lisp_number(y);
		  }
    } break;
    case 46 :                                                 // max
    {
      li_object *n1=eval(CAR(arg_list)),*n2=eval(CAR(CDR(arg_list)));
	  if (n1->type()==LI_INT&&n2->type()==LI_INT)
		  {
		  int x=lnumber_value(n1),y=lnumber_value(n2);
		  if (x>y) ret=new_lisp_number(x); else ret=new_lisp_number(y);
		  }
	  else
		  {
	      double x=lnumber_valuef(n1),y=lnumber_valuef(n2);
          if (x>y) ret=new_lisp_number(x); else ret=new_lisp_number(y);
		  }
    } break;
    case 49 :                        // backquote
    {
	invcall();
      //ret=backquote_eval(CAR(arg_list));
    } break;
    case 50 : 
    {
      lprint(arg_list);
      lbreak("comma is illegal outside of backquote\n");
   
      ret=NULL;
    } break;
    case 51 : 
    {
      invcall();
	  //long x=lnumber_value(eval(CAR(arg_list)));
      //ret=nth(x,eval(CAR(CDR(arg_list)))); 
    } break;
    case 52 : invcall();break;//resize_temp
    case 53 : invcall(); break;    //resize_perm
    case 54 : ret=new_lisp_fixed_point(lisp_cos(lnumber_valuef(eval(CAR(arg_list))))); break;
    case 55 : ret=new_lisp_fixed_point(lisp_sin(lnumber_valuef(eval(CAR(arg_list))))); break;
    case 56 ://atan
    {
      double y=(lnumber_valuef(eval(CAR(arg_list))));   arg_list=CDR(arg_list);
      double x=(lnumber_valuef(eval(CAR(arg_list))));
      //ret=new_lisp_number(lisp_atan2(y,x));      
	  ret=0;
    } break;
    case 57 ://enum (not CLISP)
    {
      int sp=current_space;
      current_space=PERM_SPACE;
      long x=0;
      while (arg_list)
      {
	void *sym=eval(CAR(arg_list));
	//p_ref r1(sym);
	switch (item_type(sym))
	{
	  case L_SYMBOL : 
	  { li_set_value((li_symbol*)sym,new li_int(x),env); } break;
	  case L_CONS_CELL :
	  {
	    void *s=eval(CAR(sym));
	    //p_ref r1(s);
#ifdef TYPE_CHECKING
	    if (item_type(s)!=L_SYMBOL)
	    { lprint(arg_list);
	      lbreak("expecting (symbol value) for enum\n");
	   
	    }
#endif
	    x=lnumber_value(eval(CAR(CDR(sym))));
	    //((lisp_symbol *)sym)->set_value(new_lisp_number(x));
		li_set_value((lisp_symbol*)sym,new li_int(x),env);
	  } break;
	  default :
	  {
	    lprint(arg_list);
	    lbreak("expecting symbol or (symbol value) in enum\n");
	    
	  }
	}
	arg_list=CDR(arg_list);
	x++;
      }      
      current_space=sp;
    } break;
    case 58 :
    {
      
    } break;
    case 59 ://explicit eval, means: do eval twice, usefull if code is first concatenated using backquote
    {
      ret=eval(eval(CAR(arg_list)));
    } break;
    case 60 : lbreak("User break"); break;
    case 61 ://mod
    {
      long x=lnumber_value(eval(CAR(arg_list))); arg_list=CDR(arg_list);
      long y=lnumber_value(eval(CAR(arg_list)));
      if (y==0) { lbreak("mod : division by zero\n"); y=1; }      
      ret=new_lisp_number((int)(x%y));
    } break;
/*    case 62 :
    {
      char *fn=lstring_value(eval(CAR(arg_list)));
      FILE *fp=fopen(fn,"wb");
      if (!fp)
        lbreak("could not open %s for writing",fn);
      else
      {	
	for (void *s=symbol_list;s;s=CDR(s))		  
	  fprintf(fp,"%8d  %s\n",((lisp_symbol *)(CAR(s)))->call_counter,
		  lstring_value(((lisp_symbol *)(CAR(s)))->name));
	fclose(fp);
      }
    } break;*/
    case 64 ://for [not CLISP, would be (loop for ...)]
    {//syntax: (for SYMBOL in LIST do BLOCK)
	  li_symbol *bind_var=li_symbol::get(CAR(arg_list),env); arg_list=CDR(arg_list);
      //p_ref r1(bind_var);
      //if (item_type(bind_var)!=L_SYMBOL)
      //{ lbreak("expecting for iterator to be a symbol\n"); exit(1); }

      if (CAR(arg_list)!=in_symbol)
      { lbreak("expecting in after 'for iterator'\n"); exit(1); }
      arg_list=CDR(arg_list);

      void *ilist=eval(CAR(arg_list)); arg_list=CDR(arg_list);
      //p_ref r2(ilist);
      
      if (CAR(arg_list)!=do_symbol)
      { lbreak("expecting do after 'for iterator in list'\n"); exit(1); }
      arg_list=CDR(arg_list);

      void *block=NULL,*ret=NULL;
      //p_ref r3(block);
	  li_object *val=bind_var->value();
      //l_user_stack.push(symbol_value(bind_var));  // save old symbol value
      while (ilist)
      {
				//set_symbol_value(bind_var,CAR(ilist));
				bind_var->set_value(CAR(ilist));
				for (block=arg_list;block;block=CDR(block))
				  ret=eval(CAR(block));
				ilist=CDR(ilist);
      }
	  bind_var->set_value(val);
      ret=ret;
    } break;
    case 65 :
    {
	invcall();
      /*bFILE *old_file=current_print_file;
      void *str1=eval(CAR(arg_list));
      p_ref r1(str1);
      void *str2=eval(CAR(CDR(arg_list)));
      
      
      current_print_file=open_file(lstring_value(str1),
				   lstring_value(str2));

      if (!current_print_file->open_failure())
      {
				while (arg_list)
				{
				  ret=eval(CAR(arg_list));	  
				  arg_list=CDR(arg_list);
				}
      }     
      delete current_print_file;
      current_print_file=old_file;      
*/
    } break;
    case 67 :
    {
      long first=lnumber_value(eval(CAR(arg_list))); arg_list=CDR(arg_list);
      while (arg_list)
      {
        first&=lnumber_value(eval(CAR(arg_list)));
				arg_list=CDR(arg_list);
      } 
      ret=new_lisp_number((int)first);
    } break;
    case 68 :
    {
      long first=lnumber_value(eval(CAR(arg_list))); arg_list=CDR(arg_list);
      while (arg_list)
      {
        first|=lnumber_value(eval(CAR(arg_list)));
				arg_list=CDR(arg_list);
      } 
      ret=new_lisp_number((int)first);
    } break;
    case 69 :
    {
      long first=lnumber_value(eval(CAR(arg_list))); arg_list=CDR(arg_list);
      while (arg_list)
      {
        first^=lnumber_value(eval(CAR(arg_list)));
				arg_list=CDR(arg_list);
      } 
      ret=new_lisp_number((int)first);
    } break;
    case 70 :  // make-array
    {
      long l=lnumber_value(eval(CAR(arg_list)));
      if (l>=2<<16 || l<=0)
      {
				lbreak("bad array size %d\n",l);
				
      }
      ret=new_lisp_1d_array((w16)l,CDR(arg_list),env);
    } break;
    case 71 : // aref
    {
      long x=lnumber_value(eval(CAR(CDR(arg_list))));
      ret=lget_array_element(eval(CAR(arg_list)),x);
    } break;
    case 72 : // if-1progn
    {
	  li_object *res=eval(CAR(arg_list));
      if (res&&res!=li_nil)
        ret=eval_block(CAR(CDR(arg_list)),env);
      else ret=eval(CAR(CDR(CDR(arg_list))));

    } break;
    case 73 : // if-2progn
    {
      li_object *res=eval(CAR(arg_list));
      if (res&&res!=li_nil)
        ret=eval(CAR(CDR(arg_list)));
      else ret=eval_block(CAR(CDR(CDR(arg_list))),env);

    } break;
    case 74 : // if-12progn
    {
      li_object *res=eval(CAR(arg_list));
      if (res&&res!=li_nil)
        ret=eval_block(CAR(CDR(arg_list)),env);
      else ret=eval_block(CAR(CDR(CDR(arg_list))),env);

    } break;
    case 75 : // eq0
    {
      void *v=eval(CAR(arg_list));
      if (item_type(v)!=L_NUMBER || li_get_value(li_symbol::get((li_symbol*)v,env),env)!=0)
        ret=NULL;
      else ret=true_symbol;
    } break;
    case 76 : // preport
    {

    } break;
    case 77 : // search
    {
      void *arg1=eval(CAR(arg_list)); arg_list=CDR(arg_list);
      //p_ref r1(arg1);       // protect this refrence
      char *haystack=lstring_value(eval(CAR(arg_list)));     
      char *needle=lstring_value(arg1);

      char *find=strstr(haystack,needle);
      if (find)
        ret=new_lisp_number(find-haystack);
      else ret=NULL;
    } break;
    case 78 : // elt
    {//needs updating for lists, vectors
      void *arg1=eval(CAR(arg_list)); arg_list=CDR(arg_list);
      //p_ref r1(arg1);       // protect this refrence
      long x=lnumber_value(eval(CAR(arg_list)));           
      char *st=lstring_value(arg1);
      if (x<0 || x>=(long)strlen(st))
      { lbreak("elt : out of range of string\n"); ret=NULL; }
      else
        ret=new_lisp_character(st[x]);      
    } break;
    case 79 : // listp
    {
	  void *t=eval(CAR(arg_list));
	  if (!t||t==li_nil) return true_symbol; 
      return item_type(t)==L_CONS_CELL ? true_symbol : NULL;
    } break;
    case 80 : // numberp
    {
      int t=item_type(eval(CAR(arg_list)));
      if (t==L_NUMBER || t==L_FIXED_POINT || t==LI_BIGNUM) return true_symbol; else return li_nil;
    } break;
    case 81 : // do 
    {//needs reimplementing (user_stack is not available)
	/*  (isn't CLISP either. need to implement LOOP)
      void *init_var=CAR(arg_list);
      //p_ref r1(init_var);
      //int i,ustack_start=l_user_stack.son;      // restore stack at end
      void *sym=NULL;
      //p_ref r2(sym);

      // check to make sure iter vars are symbol and push old values
      for (init_var=CAR(arg_list);init_var;init_var=CDR(init_var))
      {
				sym=CAR(CAR(init_var));
				if (item_type(sym)!=L_SYMBOL)
				{ lbreak("expecting symbol name for iteration var\n");  }
				l_user_stack.push(symbol_value(sym,env));
      }
      
      void **do_evaled=l_user_stack.sdata+l_user_stack.son;
      // push all of the init forms, so we can set the symbol
      for (init_var=CAR(arg_list);init_var;init_var=CDR(init_var))    
				l_user_stack.push(eval(CAR(CDR(CAR((init_var))))));

      // now set all the symbols
      for (init_var=CAR(arg_list);init_var;init_var=CDR(init_var),do_evaled++)
      {
				sym=CAR(CAR(init_var));
				set_symbol_value(sym,*do_evaled,env);
      }

      i=0;       // set i to 1 when terminate conditions are meet
      do
      {
				i=(eval(CAR(CAR(CDR(arg_list))))!=NULL);
				if (!i)
				{
				  eval_block(CDR(CDR(arg_list)),env);
				  for (init_var=CAR(arg_list);init_var;init_var=CDR(init_var))
				    eval(CAR(CDR(CDR(CAR(init_var)))));
				}
      } while (!i);
      
      ret=eval(CAR(CDR(CAR(CDR(arg_list)))));

      // restore old values for symbols
      do_evaled=l_user_stack.sdata+ustack_start;
      for (init_var=CAR(arg_list);init_var;init_var=CDR(init_var),do_evaled++)      
      {
				sym=CAR(CAR(init_var));
				set_symbol_value(sym,*do_evaled,env);
      }

      l_user_stack.son=ustack_start;
      */
    } break;
    case 82 : // gc
    { 
      return new li_int(li_gc());
    } break;
    case 83 : // schar
    {
      char *s=lstring_value(eval(CAR(arg_list)));      arg_list=CDR(arg_list);
      long x=lnumber_value(eval(CAR(arg_list)));

      if (x>=(long)strlen(s))
      { lbreak("SCHAR: index %d should be less than the length of the string\n",x);  }
      else if (x<0)
      { lbreak("SCHAR: index should not be negative\n");  }
      return new_lisp_character(s[x]);
    } break;
    case 84 :// symbolp
    { if (item_type(eval(CAR(arg_list)))==L_SYMBOL) return true_symbol;
      else return NULL; } break;
    case 85 :  // num2str
    {
      char str[10];
      sprintf(str,"%d",lnumber_value(eval(CAR(arg_list))));
      ret=new_lisp_string(str);
    } break;
    case 86 : // nconc
    {
      /*void *l1=eval(CAR(arg_list)); arg_list=CDR(arg_list);            
      //p_ref r1(l1);      
      void *first=l1,*next;
      //p_ref r2(first);

      if (!l1)
      {
				l1=first=eval(CAR(arg_list));
				arg_list=CDR(arg_list);
      }
     
      if (item_type(l1)!=L_CONS_CELL)
      { lprint(l1); lbreak("first arg should be a list\n"); }
      do
      {
				next=l1;
				while (next) { l1=next; next=lcdr(next); }
				((li_list *)l1)->set_next(eval(CAR(arg_list)));	
				arg_list=CDR(arg_list);
      } while (arg_list);      */
	  li_object *l1=eval(CAR(arg_list));
	  
	  li_object *l2=0,*next=0;
	  arg_list=CDR(arg_list);
	  ret=l1;
	  if (!l1) break;
	  do
		  {
		  l2=eval(CAR(arg_list));
		  next=CDR(l1);
		  while (next&&next!=li_nil)
			  {
			  l1=next;
			  next=CDR(l1);
			  }
		  ((li_list*)l1)->set_next(l2);
		  
		  arg_list=CDR(arg_list);
		  } while (arg_list);
      
    } break;
    case 87 : // endp
    {
	if (arg_list) ret= li_true_sym;
	else
	ret= li_nil;
	} break;
    case 88 : // rest
    { ret=CDR(arg_list); } break;
    case 89 : // last
    { 
	li_list *o=li_list::get(li_eval((li_object*)CAR(arg_list),env),env);
	li_list *last=0;
	while (o&&o->type()==LI_LIST)
		{
		last=o;
		o=(li_list*)o->next();
		}
	ret=last;

	} break;
    case 90 : // make-list
    { 
	int n=li_get_int(li_eval(CAR(arg_list),env),env);
	
	li_symbol *o=li_symbol::get(CAR(CDR(arg_list)),env);
	li_object *elem=li_nil;
	if (o==colon_initial_element)
		{
		elem=li_eval(CAR(CDR(CDR(arg_list))),env);
		}
	li_list *last=0;
	while (n>0)
		{
		last=new li_list(elem,last);
		}
	ret=last;

	} break;
    case 91 : // fifth
    { ret=CAR(CDR(CDR(CDR(CDR(eval(CAR(arg_list))))))); } break;
    case 92 : // sixth
    { ret=CAR(CDR(CDR(CDR(CDR(CDR(eval(CAR(arg_list)))))))); } break;
    case 93 : // seventh
    { ret=CAR(CDR(CDR(CDR(CDR(CDR(CDR(eval(CAR(arg_list))))))))); } break;
    case 94 : // eight
    { ret=CAR(CDR(CDR(CDR(CDR(CDR(CDR(CDR(eval(CAR(arg_list)))))))))); } break;
    case 95 : // ninth
    { ret=CAR(CDR(CDR(CDR(CDR(CDR(CDR(CDR(CDR(eval(CAR(arg_list))))))))))); } break;
    case 96 : // tenth
    { ret=CAR(CDR(CDR(CDR(CDR(CDR(CDR(CDR(CDR(CDR(eval(CAR(arg_list)))))))))))); } break;
    case 97 ://substr
    {
      long x1=lnumber_value(eval(CAR(arg_list))); arg_list=CDR(arg_list);
      long x2=lnumber_value(eval(CAR(arg_list))); arg_list=CDR(arg_list);
      void *st=eval(CAR(arg_list));
      //p_ref r1(st);

      if (x1<0 || x1>x2 || x2>=(long)strlen(lstring_value(st)))
        lbreak("substr : bad x1 or x2 value");
      
      lisp_string *s=new_lisp_string(x2-x1+2);
      if (x2-x1)
        memcpy(lstring_value(s),lstring_value(st)+x1,x2-x1+1);

      *(lstring_value(s)+(x2-x1+1))=0;
      ret=s;
    } break;
    case 99 ://unknown function
    {/*
      void *r=NULL,*rstart=NULL;
      //p_ref r1(r),r2(rstart);
      while (arg_list)
      {
				void *q=eval(CAR(arg_list));
				if (!rstart) rstart=q;
				while (r && CDR(r)) r=CDR(r);
				CDR(r)=q;	  
				arg_list=CDR(arg_list);
      }
      return rstart;*/
    } break;

    default : 
    { dprintf("INTERNAL: Undefined system function number %d\n",((lisp_sys_function *)fun)->fun_number); }
  }
  return ret;
}

void tmp_space()
{
  current_space=TMP_SPACE;
}

void perm_space()
{
  current_space=PERM_SPACE;
}

void use_user_space(void *addr, long size)
{
  current_space=2;
  free_space[USER_SPACE]=space[USER_SPACE]=(char *)addr;
  space_size[USER_SPACE]=size;
}



#define TOTAL_SYS_FUNCS 99
//function names starting with a colon are implemented the new way
//and are here as space-fillers only
                                 //  0      1    2       3       4      5      6      7
const char *sys_funcs[TOTAL_SYS_FUNCS]={":print",":car",":cdr",":length",":list",":cons",":quote",":eq",
				// 8   9   10    11       12          13     14      15      16
				  ":+",":-",":if",":setf",":symbol-list",":assoc",":null",":acons",":pairlis",
				// 17     18     19     20     21     22    23      24
				  "map",":defun","zerop",":not", "and", "or",":progn",":equal",
				// 25               26          27       28  29   30     31
				  "concatenate","char-code","code-char",":*",":/","cond","select",
				// 32        33         34     35    36    37        
				  "apply", "mapcar", "funcall", ">", "<", ":tmp-space",
				//   38              39        40       41         42
				  ":perm-space","symbol-name","trace","untrace","digstr",
				//   43            44   45    46    47  48       49
				  "compile-file","abs","min","max",">=","<=",":backquote",
				//  50      51      52         53           54    55     56
				  "comma",":nth","resize-tmp","resize-perm","cos","sin",":atan2",
				  // 57       58     59     60     61   62              63
                  "enum", ":quit","eval","break","mod","write_profile",":setq",
				  // 64    65          66      67       68        69        70
				  "for", ":open_file",":load","bit-and","bit-or","bit-xor","make-array",
				  // 71      72          73          74        75      76
				  ":aref","if-1progn","if-2progn","if-12progn","eq0","preport",
				  // 77     78         79        80       81     82     83
				  "search","elt",    "listp", "numberp", "do",  "gc", "schar",
				  // 84       85        86      87      88        89    90
				  "symbolp","num2str","nconc","endp","rest","last","make-list",
				  // 91       92       93       94       95      96
				  ":fifth", ":sixth", ":seventh",":eighth",":ninth",":tenth",
				  "substr",       // 97
				  "local_load"    // 98, filename
				};

#define TOTAL_CAAR_FUNCTIONS 28

const char* caar_functions[TOTAL_CAAR_FUNCTIONS]={"caar","cadr","cdar","cddr",
"caaar",      "caaaar",           "cdaaar", 
"caadr",      "caaadr",           "cdaadr", 
"cadar",      "caadar",           "cdadar", 
"caddr",      "caaddr",           "cdaddr",
"cdaar",      "cadaar",           "cddaar", 
"cdadr",      "cadadr",           "cddadr", 
"cddar",      "caddar",           "cdddar", 
"cdddr",      "cadddr",           "cddddr"};

/* select, digistr, load-file are not a common lisp functions! */

short sys_args[TOTAL_SYS_FUNCS*2]={

// 0      1       2        3       4         5       6      7        8
 1, -1,   1, 1,   1, 1,   0, -1,   0, -1,   2, 2,   1, 1,   2, 2,  0, -1, 
// 9      10      11      12       13       14      15      16      17
 1, -1,   2, 3,   2, -1,   0, 0,    2, 2,    1, 1,   2, 2,   2, 2,   2, -1, 
// 18     19      20      21       22       23      24      25      26
 2, -1,  1, 1,   1, 1,  -1, -1,  -1, -1,  -1, -1,  2, 2,   1,-1,   1, 1,
// 27      28      29     30       31      32        33,     34      35
 1, 1,   -1,-1,  1,-1,  -1, -1,   1,-1,    2, -1,   2, -1,  1,-1,   2,2,
// 36     37     38       39       40       41      42      43      44
 2,2,    0,0,   0,0,      1,1,    0,-1,    0,-1,   2,2,    1,1,    1,1,
// 45     46     47       48       49       50      51      52      53
 2,2,    2,2,   2,2,     2,2,     1,1,     1,1,    2,2,    1,1,    1,1,
// 54     55     56       57       58       59      60      61      62
 1,1,    1,1,   2,2,     1,-1,    0,0,     1,1,    0,0,    2,2,    1,1,
// 63     64     65      66        67       68      69      70      71
 2,2,    4,-1,  2,-1,    1,1,     1,-1,    1,-1,   1,-1,   1,-1,    2,2,
// 72     73     74      75        76       77      78      79       80
 2,3,     2,3,  2,3,     1,1,     1,1,     2,2,    2,2,    1,1,     1,1,
// 81     82     83      84        85       86      87       88      89
 2,3,     0,0,  2,2,     1,1,     1,1,     2,-1,   1,1,     1,1,    1,1,
// 90      91    92      93        94       95      96       97     98
 1,3,     1,1,   1,1,    1,1,     1,1,      1,1,     1,1,   3,3,    1,1
  
};  
/*
int total_symbols()
{
  return ltotal_syms;
}

void resize_perm(int new_size)
{
  if (new_size<((char *)free_space[PERM_SPACE]-(char *)space[PERM_SPACE]))
  {
    lbreak("resize perm : %d is to small to hold current heap\n",new_size);
    
  } else if (new_size>space_size[PERM_SPACE])
  {
    lbreak("Only smaller resizes allowed for now.\n");
    
  } else 
    dprintf("doesn't work yet!\n");
}

void resize_tmp(int new_size)
{
  if (new_size<((char *)free_space[TMP_SPACE]-(char *)space[TMP_SPACE]))
  {
    lbreak("resize perm : %d is to small to hold current heap\n",new_size);
    
  } else if (new_size>space_size[TMP_SPACE])
  {
    printf("Only smaller resizes allowed for now.\n");
    
  } else if (free_space[TMP_SPACE]==space[TMP_SPACE])
  {
    free_space[TMP_SPACE]=space[TMP_SPACE]=(char *)jrealloc(space[TMP_SPACE],new_size,"lisp tmp space");
    space_size[TMP_SPACE]=new_size;
    dprintf("Lisp : tmp space resized to %d\n",new_size);
  } else dprintf("Lisp :tmp not empty, cannot resize\n");
}

void l_comp_init();
*/
li_object *li_sys_function_evaluator(li_object *o,li_environment *env)
	{
	li_symbol *val=env->current_function();
    li_object *ret=li_nil;    
    li_object *f=li_get_fun(val, env);//get pointer to function code data
	li_user_function::user_function_data *uf=li_user_function::get(f,env)->data();
	
	int args,req_min,req_max;
	lisp_sys_function fun;
	fun.fun_number=(short)uf->fnumber;
	fun.min_args=sys_args[fun.fun_number*2];
	fun.max_args=sys_args[fun.fun_number*2+1];
	fun.type=L_SYS_FUNCTION;
	
	//void *fun=(lisp_sys_function *)(((lisp_symbol *)sym)->function);
	
	// make sure the arguments given to the function are the correct number
	req_min=fun.min_args;
	req_max=fun.max_args;
    
	
	if (req_min!=-1)
		{
		void *a=o;
		for (args=0;a;a=CDR(a)) args++;    // count number of parameters
		
		if (args<req_min)
			{
			lprint(o);
			lprint(uf->_name);
			lbreak("\nToo few parameters to function\n");
			//
			} else if (req_max!=-1 && args>req_max)
			{
			lprint(o);
			lprint(uf->_name);
			lbreak("\nToo many parameters to function\n");
			//exit(0);
				}
		}
	
	ret=(li_object *)eval_sys_function( &fun,o,env);    
    
	return ret;
	}

li_object *li_car_cdr_evaluator(li_object *o,li_environment *env)
	{//evaluator for all caar cdar cadr etc. forms
	li_symbol *val=env->current_function();
    li_object *ret=o;    
    li_object *f=li_get_fun(val, env);//get pointer to function code data
	li_user_function::user_function_data *uf=li_user_function::get(f,env)->data();
	char *symname=uf->_name->value();
	int i=2;//from 2 upwards (elementar car and cdr are not evaled here)
	while (symname[i+1]!='r') i++;
//caadar
//012345 from j=i=4 to j=1

	for (int j=i;j>=1;j--)
		{
		if (symname[j]=='a')
			ret=CAR(ret);
		else
			ret=CDR(ret);
		}
	return ret;
	}

class li_add_system_functions_class : public i4_init_class             
{                                                                  
  int init_type() { return I4_INIT_TYPE_LISP_FUNCTIONS; }          
  void init()  { 
	  total_user_functions=0;
	  memset(free_space,0,sizeof(free_space));
	  memset(space,0,sizeof(space));
	  space_size[0]=0;//no memory available here
	  space_size[1]=0;
	  current_space=PERM_SPACE;
	  colon_initial_contents=(li_symbol*)make_find_symbol(":initial-contents");
	  colon_initial_element=(li_symbol*)make_find_symbol(":initial-element");
	  string_symbol=(li_symbol*)make_find_symbol("string");
	  list_symbol=(li_symbol*)make_find_symbol("list");//remark: symbol should already exist!
	  in_symbol=(li_symbol*)make_find_symbol("in");
	  do_symbol=(li_symbol*)make_find_symbol("do");
	  int i;
	  for (i=0;i<TOTAL_SYS_FUNCS;i++)
		  {
		  if (!(sys_funcs[i][0]==':'))//These are already implemented directly
			  {
			  li_num_sys_functions++;
			  li_symbol *n=li_get_symbol(sys_funcs[i]);
			  if (n->fun()) li_error(0,"INTERNAL: lisp_init_sys_functions: Symbol %O has already a function on init.",n);
			  li_user_function *fn=new li_user_function(li_sys_function_evaluator,
				0,0,0,0,n->name());
			  fn->data()->fnumber=i;
		      
			  //li_symbol *sym=li_symbol::get(n,0);
			  n->set_fun(fn);			  
			  }
		  }
	  for (i=0;i<TOTAL_CAAR_FUNCTIONS;i++)
		  {
		  li_num_sys_functions++;
		  li_symbol *n=li_get_symbol(caar_functions[i]);
		  li_user_function *fn=new li_user_function(li_car_cdr_evaluator,
			  0,0,0,0,n->name());
		  n->set_fun(fn);
		  }

	  }          
} li_add_system_functions_instance;

/*
void lisp_init(long perm_size, long tmp_size)
{
  int i;
  lsym_root=NULL;
  total_user_functions=0;
  free_space[0]=space[0]=(char *)jmalloc(perm_size,"lisp perm space");  
  space_size[0]=perm_size;
  

  free_space[1]=space[1]=(char *)jmalloc(tmp_size,"lisp tmp space");
  space_size[1]=tmp_size;


  current_space=PERM_SPACE;  
  
  
  l_comp_init();
  for (i=0;i<TOTAL_SYS_FUNCS;i++)
    add_sys_function(sys_funcs[i],sys_args[i*2],sys_args[i*2+1],i);
  clisp_init();
  current_space=TMP_SPACE;
  dprintf("Lisp : %d symbols defined, %d system functions, %d pre-compiled functions\n",
	  total_symbols(),TOTAL_SYS_FUNCS,total_user_functions);
}

void lisp_uninit()
{
  jfree(space[0]);
  jfree(space[1]);
  ldelete_syms(lsym_root);
  lsym_root=NULL;
  ltotal_syms=0;
}*/

void clear_tmp()
{
  free_space[TMP_SPACE]=space[TMP_SPACE];
}

void *symbol_name(void *symbol)
{
  return ((lisp_symbol *)symbol)->name();
}

/*
void *set_symbol_number(void *symbol, long num)
{
#ifdef TYPE_CHECKING
  if (item_type(symbol)!=L_SYMBOL)
  {
    lprint(symbol);
    lbreak("is not a symbol\n");
    
  }
#endif
  if (((lisp_symbol *)symbol)->value!=l_undefined &&
      item_type(((lisp_symbol *)symbol)->value)==L_NUMBER)
    ((lisp_number *)((lisp_symbol *)symbol)->value)->num=num;
  else 
    ((lisp_symbol *)(symbol))->value=new_lisp_number(num);

  return ((lisp_symbol *)(symbol))->value;
}*/

void *set_symbol_value(void *symbol, void *value, li_environment *env)
{
  //li_symbol::get((li_object*)symbol,env)->set_value((li_object*)value);
  li_set_value(li_symbol::get((li_object*)symbol,env),(li_object*)value,env);
  return value;
}
/*
void *symbol_function(void *symbol)
{
#ifdef TYPE_CHECKING
  if (item_type(symbol)!=L_SYMBOL)
  {
    lprint(symbol);
    lbreak("is not a symbol\n");
    
  }
#endif
  return ((lisp_symbol *)symbol)->function;
}*/

void *symbol_value(void *symbol,li_environment *env)
{
//return li_symbol::get((li_object*)symbol,env)->value();
return li_get_value(li_symbol::get((li_object*)symbol,env),env);
}






