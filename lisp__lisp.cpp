/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/
#include "pch.h"
#include "error/error.h"
#include "memory/malloc.h"
#include "main/main.h"
#include "init/init.h"
#include "file/file.h"
#include "file/ram_file.h"
#include "app/app.h"
#include "app/registry.h" //for language support
#include "memory/array.h"

#include "status/status.h"
#include "threads/threads.h"
#include "time/profile.h"
#include "main/main.h"

#include "lisp/li_types.h"
#include "lisp/lisp.h"
#include "lisp/li_class.h"
#include "lisp/li_init.h"
#include "lisp/li_load.h"
#include "lisp/li_class.h"
#include "lisp/li_dialog.h"
#include "lisp/li_load.h"
#include "lisp/li_vect.h"

#include "loaders/dir_load.h"
#include "loaders/dir_save.h"

#include "gui/text_input.h"
#include "gui/text.h"
#include "gui/image_win.h"
#include "gui/button.h"
#include "gui/list_box.h"
#include "gui/smp_dial.h"
#include "menu/textitem.h"
#include "window/wmanager.h"
#include "memory/hashtable.h"



#include <stdio.h>
#include <setjmp.h>

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>


char li_last_file[200];
int li_last_line=0;
static w8 li_recursive_error=0;



// returns the length of the list
li_object *li_length_for_lisp(li_object *o, li_environment *env)
	{
	switch (o->type())
		{
		case LI_STRING:
			{
			li_string *s=li_string::get(o,env);
			return new li_int(strlen(s->value()));
			}
		case LI_LIST:
		{
		int t=0;
		while (o)
			{
			t++;
			o=li_cdr(o,env);
			}
		return new li_int(t);
		}
		case LI_VECTOR:
			{
			li_vector *v=li_vector::get(o,env);
			return new li_int(v->size());
			}
		default:
		li_error(env,"USER: length: argument must be of type list, array or string.");
		}
	return li_nil;
	};

int li_length(li_object *o, li_environment *env) 
{
  if (o->type()!=LI_LIST)
    return 0;
  else
  {
    int t=0;
    while (o)
    {
      t++;
      o=li_cdr(o, env);
    }
    return t;
  }
    
}

i4_bool li_is_number(li_object *o, li_environment *env) 
{ 
  return (i4_bool)(o->type()==LI_INT || o->type()==LI_FLOAT); 
}


double li_get_float(li_object *o, li_environment *env)  // will convert int to float
{
  if (o->type()==LI_INT)
    return (li_int::get(o, env)->value()); // JJ
  else if (o->type()==LI_FLOAT)
    return li_float::get(o, env)->value();
  else if (o->type()==LI_BIGNUM)
	  {//warning: may loose precision
	  li_bignum *a=li_bignum::get(o,env);
	  sw32 len=a->get_length()-1;
	  double res=0;
	  while (len)
		  {
		  res=res*10+ a->value()[len];
		  len--;
		  }
	  if (a->get_signum()) res= -res;
	  return res;
	  }
  else if (o->type()==LI_LIST)
	return li_get_float(li_eval(o,env),env);
  return li_float::get(o,env)->value();//give error.
}

int li_get_int(li_object *o, li_environment *env)    // will convert float to int
{
  if (o->type()==LI_FLOAT)
    return (int)li_float::get(o, env)->value();
  else if (o->type()==LI_INT)
    return li_int::get(o, env)->value();
  else if (o->type()==LI_BIGNUM)
	  {//returns maxint if number cannot be represented as int
	  li_bignum *a=li_bignum::get(o,env);
	  sw32 len=a->get_length()-1;
	  sw32 res=0;
	  while (len)
		  {
		  if (res>(0x7fffffff-10)) return 0x7ffffff;
		  res=res*10+a->value()[len];
		  len--;
		  }
	  if (a->get_signum()) res= -res;
	  return res;
	  }
  else if (o->type()==LI_LIST)
	return li_get_int(li_eval(o,env),env);
  return li_int::get(o,env)->value();//yield out the error
}

char *li_get_string(li_object *o, li_environment *env)
{
  return li_string::get(o, env)->value();
}


void li_skip_c_comment(char *&s)
{
  s+=2;
  while (*s && (*s!='*' || *(s+1)!='/'))
  {
    if (*s=='/' && *(s+1)=='*')
      li_skip_c_comment(s);
    else s++;
  }
  if (*s) s+=2;
}


int li_read_token(char *&s, char *buffer)
{
  // skip space
  while (*s==' ' || *s=='\t' || *s=='\n' || *s=='\r' || *s==26)
  {
    if (*s=='\n')
      li_last_line++;
    s++;
  }
  
  if (*s==';')  // comment
  {
    while (*s && *s!='\n' && *s!=26)
    {
      if (*s=='\n')
        li_last_line++;
      s++;
    }
    
    return li_read_token(s,buffer);
  } else if  (*s=='/' && *(s+1)=='*')   // c style comment
  {
    li_skip_c_comment(s);
    return li_read_token(s,buffer);    
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
    if (*s!='(')
      *(buffer++)=*(s++);      
    *buffer=0;
  } else
  {
    while (*s && *s!=')' && *s!='(' && *s!=' ' && 
           *s!='\n' && *s!='\r' && *s!='\t' && *s!=';' && *s!=26)
      *(buffer++)=*(s++);      
    *buffer=0;
  }
  return 1;    
}

int li_streq(char *s1, char *s2)
{
  return strcmp(s1,s2)==0;
}


long li_str_token_len(char *st)
{
  long x=1;
  while (*st && (*st!='"' || st[1]=='"'))
  {
    if (*st=='\\' || *st=='"') st++;    
    st++; x++;
  }
  return x;
}

static i4_critical_section_class token_buf_lock;
enum {MAX_LISP_TOKEN_LEN=512};
static char li_token[MAX_LISP_TOKEN_LEN];  // assume all tokens will be < 512 characters

int li_check_number(char *num)
	{
	int i=0;
	int ret=0;
	int z=0;
	int endeok=0;
	do {
	switch(z)
		{
		case 0://erwartet: +,-,digit,.
			{
			if (num[i]=='.') {ret=1;z=2;i++;break;}
			if (num[i]=='+') {z=1;i++;break;}
			if (num[i]=='-') {z=1;i++;break;}
			if (num[0]=='0'&&num[1]=='x') {z=6;i=2;break;}//standard c-notation für hex, nicht clisp
			if (isdigit(num[i])) {z=1;break;}
			return 2;
			}
		case 1://erwartet: digit,.
			{
			if (num[i]=='.') {ret=1;z=2;i++;break;}
			if (isdigit(num[i])) {i++;endeok=1;break;}
			return 2;
			}
		case 2://digit (nach Dezimalpunkt)
			{
			if (isdigit(num[i])) {i++;z=3;endeok=1;break;}
			if (num[i]=='E'||num[i]=='e') {i++;z=4;endeok=0;break;}
			return 2;
			}
		case 3://digit, E
			if (isdigit(num[i])) {i++;endeok=1;break;}
			if (num[i]=='E'||num[i]=='e') {i++;z=4;endeok=0;break;}
			return 2;
		case 4:
			{
			//+ , -, digit
			if (num[i]=='+'||num[i]=='-') {i++;endeok=0;z=5;break;}
			if (isdigit(num[i])) {z=5;break;}
			return 2;
			}
		case 5://digit
			{
			if (isdigit(num[i])) {endeok=1;i++;break;}
			return 2;
			}
		case 6://hexdigit
			{
			if (isdigit(num[i])) {endeok=1;i++;break;}
			if ((num[i]>='a')&&(num[i]<='f')) {endeok=1;i++;break;}
			if ((num[i]>='A')&&(num[i]<='F')) {endeok=1;i++;break;}
			return 2;
			}
		}
		}
	while (num[i]);
	if (endeok) return ret; else return 2;
	}

//defined in lisp__math.cpp
li_object *li_read_number_in_radix(int rd,char *tk);

li_object *li_locked_get_expression(char *&s, li_environment *env)
{

  li_object *ret=0;

  if (!li_read_token(s,li_token))
    return 0;
  if (li_streq(li_token,"nil"))
    return li_nil;
  else if (li_token[0]=='T' && !li_token[1])
    return li_true_sym;
  else if (li_token[0]=='\'')                    // short hand for quote function
    return new li_list(li_quote, new li_list(li_locked_get_expression(s, env), 0));    
  else if (li_token[0]=='`')                    // short hand for backquote function
    return new li_list(li_backquote, new li_list(li_locked_get_expression(s, env),0));
  //else if (li_token[0]==',')              // short hand for comma function
  //  return new li_list(li_comma, new li_list(li_locked_get_expression(s, env), 0));
  //use as ordinary symbol
  
  else if (li_token[0]=='(')                     // make a list of everything in ()
  {
    li_list *first=NULL,*cur=NULL,*last=NULL;   

    int done=0;
    do
    {
      char *tmp=s;
      if (!li_read_token(tmp,li_token))           // check for the end of the list
		  {
          li_error(env, "USER: unexpected end of program");
		  return li_nil;
		  }
      if (li_token[0]==')') 
      {
        done=1;
        li_read_token(s,li_token);                // read off the ')'
      }
      else
      {     
        if (li_token[0]=='.' && !li_token[1])
        {
          if (!first)
			  {
              li_error(env, "USER: token '.' not allowed here : %s\n",s);	      
			  return 0;
			  }
          else 
          {
            li_read_token(s,li_token);              // skip the '.'
            last->set_next(li_locked_get_expression(s, env));          // link the last cdr to 
            last=NULL;
          }
        } else if (!last && first)
			{
          li_error(env, "USER: illegal end of dotted list\n");//also happens if out of mem.
		  return 0;
			}
        else
        {	
          li_list *p=new li_list(li_locked_get_expression(s, env), 0);
          if (last)
            last->set_next(p);
          else
            first=p;
          last=p;
        }
      } 
    } while (!done);

    if (!first)//was just "()". 
      return li_nil;
    else return first;

  } else if (li_token[0]==')')
    li_error(env, "USER: LISP Parse error: Mismatched ) at %s. Check the number of opening and closing parenthesis.",s);
  else if (isdigit(li_token[0]) || (li_token[0]=='-' && isdigit(li_token[1]))||
	  li_token[0]=='.'||li_token[0]=='+')
  {
    int i=0,per=0,hex=0,x;
    
    if (li_token[0]=='0' && li_token[1]=='x')     // hex number
    {
      hex=1;
      i+=2;
    }
        //implement a finite state machine to detect floats.
	//
	per=li_check_number(li_token);//returns 0=integer, 1=float, 2=symbol
	/*
    for (; li_token[i] && (isdigit(li_token[i]) || li_token[i]=='.' || li_token[i]=='-'||
		li_token[i]=='+'||li_token[i]=='E'||li_token[i]=='e'); i++)
		{
      if (li_token[i]=='.')
        per=1;
		}

	*/

    if (per==1)
    {
      double y=0;
      sscanf(li_token,"%lf",&y);  //not ansi, so change to "%f" if compiler complains
	  //but beware: may result in complete garbage being scanned!
	  
      return new li_float(y);
    }
    else if (hex&&(per==0))
    {
      sscanf(li_token,"%x",&x);
      return new li_int(x);
    }
    else if (per==0)
    {
      //sscanf(li_token,"%d",&x);
      //return new li_int(x);
	  return li_read_number_in_radix(10,li_token);
    }
	else //is an ordinary symbol, only looks like a number
		{
		return li_get_symbol(li_token);
		}
  } else if (li_token[0]=='"')
  {
    li_string *r=new li_string(li_str_token_len(s));

    char *start=r->value();

    for (;*s && (*s!='"' || s[1]=='"');s++,start++)
    {
      if (*s=='\\')
      {
        s++;
		*start=*s;
        if (*s=='n') *start='\n';
        if (*s=='r') *start='\r';
        if (*s=='t') *start='\t';
        if (*s=='\\') *start='\\';
		if (*s=='\'') *start='\'';
		if (*s=='"') {*start='"';continue;}//skip if() bellow
      } else *start=*s;
      if (*s=='"') s++;
    }
    *start=0;
    s++;

    return r;
  } else if (li_token[0]=='#')//li_token[1]==0 yields reserved symbol '#'
  {
    switch (li_token[1])
		{
		case '\\':
		{
		li_read_token(s,li_token);                   // read character name
		if (li_streq(li_token,"newline"))
			ret=new li_character('\n');
		else if (li_streq(li_token,"space"))
			ret=new li_character(' ');       
		else 
			ret=new li_character(li_token[0]);  
		break;
		}
        case '\'':                           // short hand for function
        return new li_list(li_function_symbol, new li_list(li_locked_get_expression(s, env), 0));
		//case 0x0:
		//	li_error(env,"USER: Invalid use of number sign (#). -> Must be followed by something');
		//	break;
		case 'b':
			li_read_token(s,li_token);
			return li_read_number_in_radix(2,li_token);
		case 'o':
			li_read_token(s,li_token);
			return li_read_number_in_radix(8,li_token);
		case 'x':
			li_read_token(s,li_token);
			return li_read_number_in_radix(16,li_token);
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			{
			s--;
			char radix[3]={0x0,0x0,0x0};
			char *radixp=&radix[0];
			int iradix=0;
			*(radixp++)=*(s++);
			if ((*s)!='r'&&(*s)!='R')
				{
				*(radixp++)=*(s++);
				}
			if ((*s)!='r'&&(*s)!='R')
				{
				li_error(env,"USER: Invalid format for arbitrary radix integer.");
				}
			s++;//skip the 'r'
			sscanf(radix,"%d",&iradix);
			if (iradix<2||iradix>36)
				{
				li_error(env,"USER: Radix must be between 2 and 36. You used %d.",iradix);
				}
			li_read_token(s,li_token);
			return li_read_number_in_radix(iradix,li_token);
			}
		case '<': li_error(env,"USER: Cannot read in output of unprintable objects (\"#<...>\")");
			break;
		default:
		{
			return new li_list(li_vector_symbol, new li_list(li_locked_get_expression(s,env),0));
		}
  /*
    if (li_token[1]=='\\')
    {
      li_read_token(s,li_token);                   // read character name
      if (li_streq(li_token,"newline"))
        ret=new li_character('\n');
      else if (li_streq(li_token,"space"))
        ret=new li_character(' ');       
      else 
        ret=new li_character(li_token[0]);       
    }
    else if (li_token[1]=='\'')                           // short hand for function
      return new li_list(li_function_symbol, new li_list(li_locked_get_expression(s, env), 0));
    else
    {
      return new li_list(li_vector_symbol, new li_list(li_locked_get_expression(s,env),0));
    }*/
		}
  } else 
    return li_get_symbol(li_token);

  return ret;
}

// because we can only allow one thread to use the token buffer at a time
// so we don't have to allocate it on the stack (because it's fairly recursive)
// I lock access to the token buffer per thread
li_object *li_get_expression(char *&s, li_environment *env)
{
  token_buf_lock.lock();
  li_object *ret=li_locked_get_expression(s, env);
  token_buf_lock.unlock();
  return ret;
}


void lip(li_object *o)
{
  if (!o)
  {
    i4_debug->printf("(null object)\n");
    return ;
  }

  if (!li_valid_object(o))
  {
    i4_debug->printf("(invalid object)\n");
    return ;
  }

  li_get_type(o->type())->print(o, i4_debug);
  i4_debug->printf("\n");
}
LI_HEADER(user_mode_print)
	{
	//print function for user mode
	li_object *ret=li_nil;
	while(o)
		{
		ret=li_eval(li_car(o,env),env);
		lip(ret);
		o=li_cdr(o,env);
		}
	return ret;
	}

li_object *li_print(li_object *o, li_environment *env)
	{
	li_object *ret=0;
	try{//this may fail if something failed before (like invalid objects in the list)
		lip(o);
		ret=o;
		}
	catch(...){
		li_error(env,"ERROR: Exception during li_print. This is usually due to some former error.");
		}
	return ret;
	}

li_list *li_make_list(li_object *first, ...)
{
  va_list ap;
  va_start(ap, first);
  
  li_list *ret=new li_list(first,0), *last;
  last=ret;
  
  for(;;)
  {
    li_object *o=va_arg(ap, li_object *);
    if (o)
    {
      li_list *next=new li_list(o,0);
      last->set_next(next);
      last=next;
    }
    else
    {
      va_end(ap);
      return ret;
    }
  }
}

void li_define_value(li_symbol *sym, li_object *value, li_environment *env)
	{
	if (sym->flags()&LSF_VALUECONSTANT) 
	  {
	  li_error(env,"USER: set_value: %O is defined constant. Use defconstant to alter its value.",sym);
	  return;
	  }
    if (env)
      env->define_value(sym, value);
    else sym->set_value(value); 
	}



static i4_profile_class lisp_evaluation("lisp_evaluation");
/** The lisp evaluator.

  This is the lisp evaluator function. It can evaluate almost anything
  (functions, lists, symbols, constants) and returns the appropriate 
  return value.
  If the expression is a list, its first element is observed and called
  with the car of the expression as parameters.
  If the expression is a symbol, its value is returned.
  If the expression begins with another expression, that one is 
  evaluated as lambda expression.
  This function may cause errors or warn the user of bad argument values.
  Implementation note: On windows, the lisp evaluator also survives 
  really hard errors like SIGSEGV or SIGILL, on unix systems there seems
  to be no easy way of catching these. 
  @param expression The expression that should be evaluated.
  @param env Default lisp environment in which the evaluation should take place.
  @return The result of the evaluation, or :novalue if an error occured.
*/
li_object *li_eval(li_object *expression, li_environment *env)
	{
	static li_object* error_symbol=0;
	static int recurse_depth=0;//so we can quit to the top-level-loop
	if (!expression)
		return li_nil;
	recurse_depth++;
	lisp_evaluation.start();
	int type=expression->type();
	switch (type)
		{    
		case LI_SYMBOL : 
			{
			li_object *v=li_get_value(li_symbol::get(expression,env), env);
			if (!v)
				{
				li_error(env, "USER: Symbol '%O' has no value. If you meant the symbol itself, use 'symb. ", expression);
				lisp_evaluation.stop();
				return (li_get_symbol(":novalue"));
				}
			recurse_depth--;
			lisp_evaluation.stop();
			return v;
			
			} break;
			
		case LI_LIST :
			{
			li_object *ret=0;
			li_list *o=0;
			li_symbol *sym=0;
			try{
				o=li_list::get(expression,env);
				if (o->data()->type()==LI_SYMBOL)//if first element of list is not a symbol, 
					//this is probably not really a list
					//and for shure it is not to be called
					{
					sym=li_symbol::get(o->data(),env);
					ret= li_call(sym, o->next(), env);
					}
				else
					{
					//if (!(o->next()))
					//not shure about this case, perhaps also lambda
					//		ret=o;//return the list with the plain value (don't return o->data(), will crash on li_car())
					//	  else
					//could be a lambda invocation
					//		  {
					ret=li_lambda_eval(o->data(),o->next(),env);
					//		  }
					//li_error(env,"Expected function invocation or plain value, found multiple items");
					}
				}
			catch(char *str)
				{
				i4_warning("Exception encountered: %s",str);
				
				if (recurse_depth>1)
					{
					recurse_depth--;
					lisp_evaluation.stop();
					throw;
					}
				}
			catch(...)
				{
				error_symbol=sym;
				li_recursive_error=0;//reset error recusion counter, or message won't be displayed if error caused a throw.
				li_error(env,"INTERNAL: Exception during execution of lisp-function in %O. Invalid parameters for a function given?",error_symbol);
				//li_recursive_error=0;
				ret=0;
				}
			recurse_depth--;
			lisp_evaluation.stop();
			return ret;
			
			} break;
			
		default :
			recurse_depth--;
			lisp_evaluation.stop();
			return expression; 
			break;
		}
	recurse_depth--;
	lisp_evaluation.stop();
	return 0;
	}


li_object *li_load(i4_file_class *fp, li_environment *env, i4_status_class *status)
{
  li_object *ret=0;
  li_last_line=0;


  int l=fp->size();

  char *buf=(char *)I4_MALLOC(l+1,"");
  buf[l]=0;
  fp->read(buf,l);

  char *s=buf;
  

  li_object *exp;
  do
  {
    if (status)
      status->update((s-buf)/(float)l);

    exp=li_get_expression(s, env);
    if (exp)
      ret=li_eval(exp, env);    
  } while (exp);

  i4_free(buf);
  return ret;
}

li_object *li_load(li_object *name, li_environment *env)
{
  return li_load(name, env, 0);
}

li_object *li_load(li_object *name, li_environment *env, i4_status_class *status)
{
  li_object *ret=0;

  char old_file[256];
  I4_ASSERT(li_last_file,"The last file is NULL");
  strcpy(old_file, li_last_file);
  int old_line=li_last_line;
  
  li_gc();

  while (name)
  {
    char *s=li_string::get(li_eval(li_car(name,env),env),env)->value();
    strcpy(li_last_file, s);
    //we first try wheter there's a language dependent file 
    //(that is one that has a language suffix).
    i4_str langstr(s);
    i4_str::iterator it(langstr.c_str()+langstr.find_last_of("."));
    i4_language_extend(langstr,it);
    i4_file_class *fp=i4_open(langstr);
    if (!fp)
        {
        fp=i4_open(s);
        }
    if (fp)
    {
      ret=li_load(fp, env, status);
      delete fp;
    }
    else
		{
		li_error(env,"USER: li_load : file %s does not exist.", s);
		return 0;
		}

    name=li_cdr(name,env);
  }
  
  strcpy(li_last_file, old_file);
  li_last_line=old_line;
  

  return ret;
}

li_object *li_read_eval(li_object *o, li_environment *env)
{
  char line[1000], *c=line;
  int t=0;
  i4_debug->printf("eval>");
  do
  {
    if (i4_debug->read(c,1)!=1)
      return 0;
    t++;
    c++;
  } while (c[-1]!='\n' && t<998);
  
  *c=0;
  c=line;
  li_object *ret=li_eval(li_get_expression(c, env), env);
  lip(ret);
  return ret;
}

li_object *li_load(char *filename, li_environment *env, i4_status_class *status)
{
  return li_load(new li_list(new li_string(filename), 0), env, status);
}

int li_num_sys_functions=0;
void li_add_function(li_symbol *sym, 
                     li_function_type fun,
                     li_environment *env)
{
  li_function *f=new li_function(fun);
  li_num_sys_functions++;
  if (env)
    env->set_fun(sym, f);
  else
    sym->set_fun(f);
}

LI_HEADER(sys_info)
	{
	return new li_int(li_num_sys_functions);
	}


void li_add_function(char *sym_name, li_function_type fun, li_environment *env)
{
  li_add_function(li_get_symbol(sym_name), fun, env);
}

i4_bool li_get_bool(li_object *o, li_environment *env)
{ 
  if (!o) return i4_F;

  li_symbol *s=li_symbol::get(li_eval(o,env),env);

  if (s==li_nil)
    return i4_F;
  else if (s==li_true_sym)
    return i4_T;
  else
    li_error(env, "USER: Expecting T or nil, got %O", o);

  return 0;
}

static inline int fmt_char(char c)
{
  if ((c>='a' && c<='z') || (c>='A' && c<='Z'))
    return 1;
  return 0;
}



void li_vprintf(i4_file_class *fp,
                char *fmt,
                va_list ap)
{
  
  while (*fmt)
  {
    if (*fmt=='%')
    {
      char *fmt_end=fmt;
      while (!fmt_char(*fmt_end) && *fmt_end) fmt_end++;
      char f[10], out[500]; 
      memcpy(f, fmt, fmt_end-fmt+1);
      f[fmt_end-fmt+1]=0;
      out[0]=0;

      switch (*fmt_end)
      {
        case 'O' : 
        {
          li_object *o=va_arg(ap,li_object *);
		  if (!o) 
			  {
			  ::sprintf(out,"nil");
			  break;
			  }
          li_get_type(o->unmarked_type())->print(o, fp);
        } break;

        case 'd' :
        case 'i' :
        case 'x' :
        case 'X' :
        case 'o' :
          ::sprintf(out,f,va_arg(ap,int));
          break;

        case 'f' :
        {
          double fl=(va_arg(ap, double)); //JJ
          ::sprintf(out,f,fl);
        } break;

        case 'g' :
          ::sprintf(out,f,va_arg(ap,double));
          break;

		case 's' :
			{
			char *strparam=va_arg(ap,char*);
			int len=strlen(strparam);
			if (len>490) //otherwise, we have a buffer overrun vulnerability
				{
				memcpy(out,strparam,490);
				out[490]='.';
				out[491]='.';
				out[492]='.';
				out[493]=0;
				}
			else
				memcpy(out,strparam,len+1);
			} break;
        default :
          ::sprintf(out,f,va_arg(ap,void *));
          break;
      }
      fp->write(out, strlen(out));
      fmt=fmt_end;
      if (*fmt)
        fmt++;
    }
    else
    {
      fp->write_8(*fmt);
      fmt++;
    }


  }
}


void li_printf(i4_file_class *fp,
               char *fmt,                   // typical printf format, with %o == li_object
              ...)
{  
  va_list ap;
  va_start(ap, fmt);
  li_vprintf(fp, fmt, ap);
  va_end(ap);

}

int li_error(li_environment *env,
              char *fmt,
              ...)
{
  int user_choosed=0;
  if (!li_recursive_error)      // error shouldn't call error again!
  {
    li_recursive_error++;
    i4_file_class *fp=i4_open("li_error.txt", I4_WRITE);//must not I4_APPEND here
	//as the error buf may not get larger than 1000 bytes. (gives recursive exceptions then)

    if (fp)
    {
      va_list ap;
      va_start(ap, fmt);
  
      li_vprintf(fp, fmt, ap);
      fp->printf("\nCall stack:\n");
      if (env)
        env->print_call_stack(fp);
	  
	  if (li_last_file[0]!='\0')//don't print the last file if it was none (like direct user input)
		fp->printf("\nlast file %s:%d", li_last_file, li_last_line);
      delete fp;
      

      fp=i4_open("li_error.txt");
      if (fp)
      {
        int size=fp->size();
		if (size>990) size =990;//otherwise, error handler will overflow
        char *b=(char *)I4_MALLOC(size+1,"");
        fp->read(b, size);
        b[size]=0;
        delete fp;

        user_choosed=i4_get_error_function_pointer(li_last_file, 0)(b);

        i4_free(b);
      }
    }

    li_recursive_error--;
  }  
  return user_choosed;
}

li_object *li_new(char *type_name, li_object *params, li_environment *env)
{
  li_symbol *s=li_find_symbol(type_name);
  if (!s) return 0;

  li_object *v=li_get_value(s, env);
  if (!v || v->type()!=LI_TYPE) return 0;
  
  li_type_number type=li_type::get(v,env)->value();
  return li_get_type(type)->create(params, env);
}

li_object *li_new(int type, li_object *params, li_environment *env)
{
  return li_get_type(type)->create(params, env);
}

long li_detect_keyword(li_object *o, long oldkeyword)
	{
	if ((!o)||(o->type()!=LI_SYMBOL)) return LKW_NONE;
	li_symbol *s=li_symbol::get(o,0);
	char *sc=s->name()->value();
	if (sc[0]!='&') return LKW_NONE;//shortcut
	long ret=LKW_NONE;
	if (strcmp(sc,"&optional")==0) ret=LKW_OPTIONAL;
	else if (strcmp(sc,"&key")==0) ret=LKW_KEY;
	else if (strcmp(sc,"&aux")==0) ret=LKW_AUX;
	else if (strcmp(sc,"&body")==0) ret=LKW_BODY;
	else if (strcmp(sc,"&rest")==0) ret=LKW_REST;
	else if (strcmp(sc,"&whole")==0) ret=LKW_WHOLE;
	else if (strcmp(sc,"&environment")==0) ret=LKW_ENVIRONMENT;
	else if (strcmp(sc,"&allow-other-keys")==0) ret=LKW_ALLOW_OTHER_KEYS;
	else if (strcmp(sc,"&noevalonbind")==0) {ret=LKW_AMPERSANDOTHER;return ret;}
	else ret=LKW_NONE;
	if (ret<oldkeyword) //check order of apearance
		{
		li_error(0,"Keywords apear in invalid order in <unknown function>");
		return LKW_ORDER_ERROR;
		}
	return ret;
	}

i4_bool li_bind_params(li_object *funparams,li_object *actparams, li_environment *env,w32 flags)
	{
	//binds actparams to funparams, takes keywords into acount
	if (li_detect_keyword(li_car(actparams,env),0)==LKW_AMPERSANDOTHER)//THE HACK to pass noevalonbind around
		{
		actparams=li_cdr(actparams,env);
		flags=LEF_NOEVALONBIND;
		}
	li_symbol *cursym=0,*key=0;
	li_object *defval=0,*ap=actparams, *fp=funparams;//we will need to start over from the beginning, so save paramlist
	long keyword=LKW_NONE,curkey=LKW_REQPARAM;//status of binding
	i4_bool had_allow_other_keys=i4_F;
	

	while (fp&&fp!=li_nil)
		{
		
		
		keyword=li_detect_keyword(li_car(fp,env), keyword);
		while (keyword!=LKW_NONE)
			{
			curkey=keyword;
			if (keyword==LKW_ALLOW_OTHER_KEYS) had_allow_other_keys=i4_T;
			if (keyword==LKW_ORDER_ERROR) return i4_F;//the message was given
			if (keyword==LKW_KEY||keyword==LKW_AUX)
				{
				actparams=ap;//save start, we'll need to search from here
				}
			fp=li_cdr(fp,env);
			keyword=li_detect_keyword(li_car(fp,env),keyword);
			}
		switch (curkey)
			{
			case LKW_REQPARAM:
				{
				//a required parameter
				//the first params in the set
				cursym=li_symbol::get(li_car(fp,env),env);
				fp=li_cdr(fp,env);
				if (flags==LEF_NOEVALONBIND)
					{
					defval=li_car(ap,env);
					}
				else
					defval=li_eval(li_car(ap,env),env);
				if (li_detect_keyword(li_car(ap,env),0)!=LKW_NONE)
					{
					li_error(env,"USER: bind_params: keyword %O unexspected, check formal param list",defval);
					return i4_F;
					}
				li_define_value(cursym,defval,env);
				ap=li_cdr(ap,env);
				}break;
			case LKW_OPTIONAL:
				{
				//&optional params
				//given as (symbol [initform [svar]]) or just symbol sequence
				//if there are params left, symbol is bound to the one (svar = t)
				//else initform is evaled (svar = nil)
				li_object *optvar=li_car(fp,env);
				fp=li_cdr(fp,env);
				//if (ap)
				//	defval=li_car(ap,env);
				if (optvar->type()==LI_SYMBOL)
					{
					if (ap)
						{
						if (flags==LEF_NOEVALONBIND)
							defval=li_car(ap,env);
						else
							defval=li_eval(li_car(ap,env));
						ap=li_cdr(ap,env);
						}
					else 
						defval=li_nil;
					li_define_value((li_symbol*)optvar,defval,env);
					}
				else
					{
					li_list *opt2=li_list::get(optvar,env);
					optvar=li_symbol::get(li_car(opt2,env),env);
					i4_bool svar=i4_F;
					if (ap)
						//remark: it is not allowed to give a keyword-argument
						//but ignore an optional param
						{
						if (flags==LEF_NOEVALONBIND)
							defval=li_car(ap,env);
						else
							defval=li_eval(li_car(ap,env));
						ap=li_cdr(ap,env);
						svar=i4_T;
						}
					else
						{
						defval=li_cdr(opt2,env);
						if (defval!=0&&defval!=li_nil)
							{
							defval=li_eval(li_car(defval,env),env);
							}
						}
					li_define_value((li_symbol*)optvar,defval,env);
					defval=li_cdr(li_cdr(opt2,env),env);
					if (defval)
						{
						defval=li_symbol::get(li_car(defval,env),env);
						if (svar)
							{
							li_define_value((li_symbol*)defval,li_true_sym,env);
							}
						else
							li_define_value((li_symbol*)defval,li_nil,env);
						}


					}


				}break;
			case LKW_REST:
				//bind everything what's left to the given symbol
				//(even if that's processed afterwards)
				{
				li_symbol *rest=li_symbol::get(li_car(fp,env),env);
				fp=li_cdr(fp,env);
				li_define_value(rest,ap,env);//ap contains the rest of the list

				}break;
			case LKW_KEY:
				{
				//duh, complicated.

				//syntax is as follows:
				//[&key {var | ({var | (keyword var)} [initform [svar]])}*
                //[&allow-other-keys]]
				//if only var given, it is keyword, too
				//if ((keyword var)) given, :keyword is searched for, var is bound
				//if (var initform) given, like &optional
				li_symbol *bindvar,*keywordsym,*svar=0;
				li_object *initform=0;

				i4_bool svar_value=i4_F;
				li_object *kp=li_car(fp,env);
				fp=li_cdr(fp,env);
				if (kp->type()==LI_SYMBOL)
					{
					bindvar=li_symbol::get(kp,env);
					keywordsym=bindvar;
					svar=0;
					initform=0;
					}
				else
					{
					//is a list
					li_object *first=li_car(kp,env);
					if (first->type()==LI_SYMBOL)
						{
						bindvar=li_symbol::get(first,env);
						keywordsym=bindvar;
						}
					else
						{
						//(keyword sym) pair
						keywordsym=li_symbol::get(li_car(first,env),env);
						bindvar=li_symbol::get(li_car(li_cdr(first,env),env),env);
						}
					li_object *second=li_second(kp,env);
					if (second)
						{
						initform=second;
						}

					li_object *third=li_third(kp,env);
					if (third)
						{
						svar=li_symbol::get(third,env);
						if (svar==li_nil) svar=0;
						}
					}
				ap=actparams;//reset search loop
				//to be looked for. doesn't start with ':'
				char *looking_for_word=keywordsym->name()->value();
				while(ap)
					{
					li_object *cp=li_car(ap,env);
					ap=li_cdr(ap,env);
					//current keyword, must start with ':'
					char *found_word=li_symbol::get(cp,env)->name()->value();
					if (found_word[0]!=':')
						{
						//always wrong?
						li_error(env,"USER: bind_var: Exspecting :KEYWORD, found %O.",cp);
						return i4_F;
						}
					if (strcmp(looking_for_word,found_word+1)==0)
						{
						//keyword ok
						//remark: if keyword given more than once, only first occurence is used
						defval=li_eval(li_car(ap,env),env);
						ap=0;//break loop
						li_define_value(bindvar,defval,env);
						svar_value=i4_T;
						if (svar)
							li_define_value(svar,li_nil,env);
						}
					else
						ap=li_cdr(ap,env);
					}
				if (svar_value==i4_F)//not found
					{
					if (initform)
						defval=li_eval(initform,env);
					else
						defval=li_nil;
					li_define_value(bindvar,defval,env);
					if (svar)
						li_define_value(svar,li_nil,env);
					}
				
				}break;
			case LKW_AUX:
				{
				li_object *iv=li_car(fp,env);
				fp=li_cdr(fp,env);
				if (iv->type()==LI_SYMBOL)
					{
					li_define_value(li_symbol::get(iv,env),li_nil,env);
					}
				else
					{
					li_object *first=li_car(iv,env);
					li_object *second=li_car(li_cdr(iv,env),env);
					if (second)
						{
						second=li_eval(second,env);
						li_define_value(li_symbol::get(first,env),second,env);
						}
					else
						{
						li_define_value(li_symbol::get(first,env),li_nil,env);
						}
					}
				}break;
			default:
				{
				li_error(env,"USER: bind_var: Invalid keyword in lambda list of function declaration");
				}
			}//switch
		
		}//while
		return i4_T;
	}

LI_HEADER(user_function_evaluator);

LI_HEADER(quote_param_list)
	{//quote every param that is either a symbol or a list
	li_list *l=(li_list*)o,*ncar=0;
	while(l&&l->type()==LI_LIST)
		{
		w32 type=li_car(l,env)->type();
		switch(type)
			{
			case LI_SYMBOL:
			case LI_LIST: 
				{
				ncar=new li_list(li_quote,new li_list(li_car(l,env),0));
				l->set_data(ncar);
				}
				break;
			default:
				{
				;
				}
			}
		l=(li_list*)l->next();
		}
	return o;
	}



li_object *li_lambda_eval(li_object *lambda, li_object *params, li_environment *env, w32 flags)
	{
	if (!lambda) return li_nil;
	if (lambda->type()==LI_SYMBOL) //get the symbol and try to use it as a function name
		{//this is for default case only. No function-application function should ever come here
		/*li_symbol *user=(li_symbol*)lambda;
		if ((flags|LEF_NOEVALONBIND)&&(user->fun()->type()==LI_USER_FUNCTION)&&
			  (((li_user_function*)user->fun())->value()==li_user_function_evaluator))
			  {//very special HACK to inform bind_var to use LEF_NOEVALONBIND
			  //this is used for functions like mapcar and apply

			  //we are shure here, that params will be interpreted by bind_val, so we place a little hint as first parameter
			  li_list *newstart=new li_list(li_get_symbol("&noevalonbind"),params);
			  params=newstart;
			  }*/
		if (flags&LEF_NOEVALONBIND)
			{
			li_error(env,"INTERNAL: lambda_eval: attempt to use a plain symbol as lambda function arg");
			return 0;
			}
		return li_call((li_symbol*)lambda,params,env);
		}
	li_object *ret=0;
	if (li_car(lambda,env)!=li_lambda_symbol)
		{
		//li_error(0,"LAMBDA_EVAL: Exspecting lambda function, found: %O. ",lambda);
		//return li_nil;
		//perhaps some function that will return another function
		li_object *lambda_func=li_eval(lambda,env);
		if (lambda_func->type()==LI_USER_FUNCTION)
			{//unfortunatelly, we have to do everything what call does manually
			li_user_function *uf=li_user_function::get(lambda_func,env);//call function, will return a function (otherwise, I dono)
			if (!uf) return 0;//something went wrong
			li_function_type fun=uf->value();//get the function pointer
			li_symbol *old_fun=0;
			li_object *old_args=0;
			if (env)
			{     
			  old_fun=env->current_function();
			  old_args=env->current_arguments();
			}
			else //is this required?  Yes it is. see next line.
			//if the toplevel call is something like "(defvar)" we must not have an environment.
			  env=new li_environment(env, i4_F);

			li_symbol *fname=li_get_symbol(uf->name()->value());
			env->set_fun(fname,lambda_func);//usually, fname is lambda, so this is mainly a hack to pass the code around
			env->current_function()=fname;//the first time we can use that field
			env->current_arguments()=params;
    
			li_object *ret=fun(params,env);//call that function with our params
			if (fname==li_lambda_symbol) env->set_fun(fname,li_nil);
			if (old_fun)
			{
			  env->current_function()=old_fun;
			  env->current_arguments()=old_args;
			}
			return ret;
			}
		else if (lambda_func->type()==LI_FUNCTION)
			{//unfortunatelly, we have to do everything what call does manually
			li_function *fn=li_function::get(lambda_func,env);//call function, will return a function (otherwise, I dono)
			if (!fn) return 0;//something went wrong
			li_function_type fun=fn->value();//get the function pointer
			li_symbol *old_fun=0;
			li_object *old_args=0;
			if (env)
			{     
			  old_fun=env->current_function();
			  old_args=env->current_arguments();
			}
			else //is this required?  Yes it is. see next line.
			//if the toplevel call is something like "(defvar)" we must not have an environment.
			  env=new li_environment(env, i4_F);

			li_symbol *fname=li_lambda_symbol;//non-user functions don't have a fixed name
			env->set_fun(fname,lambda_func);//usually, fname is lambda, so this is mainly a hack to pass the code around
			env->current_function()=fname;//the first time we can use that field
			env->current_arguments()=params;
    
			li_object *ret=fun(params,env);//call that function with our params
			if (fname==li_lambda_symbol) env->set_fun(fname,li_nil);
			if (old_fun)
			{
			  env->current_function()=old_fun;
			  env->current_arguments()=old_args;
			}
			return ret;
			}
		else if (lambda_func->type()==LI_SYMBOL)
			{
			li_symbol *user=(li_symbol*)lambda_func;
			if (flags&LEF_NOEVALONBIND)
				{
				if ((user->fun()->type()==LI_USER_FUNCTION)&&
				  (((li_user_function*)user->fun())->value()==li_user_function_evaluator))
					{//very special HACK to inform bind_var to use LEF_NOEVALONBIND
					//this is used for functions like mapcar and apply

					//we are shure here, that params will be interpreted by bind_val, so we place a little hint as first parameter
					li_list *newstart=new li_list(li_get_symbol("&noevalonbind"),params);
					params=newstart;
					}
				else
					{
					//is a system-function
					//we quote every param which is a list or a symbol, so it gets unquoted on eval
					//EXCEPTION: lambda_func is a special-form function (which is an error in clisp)
					if (user->flags()&LSF_SPECIALFORM)
						{
						//cannot handle these as functions here
						li_error(env,"USER: lambda_eval: %O is a special form, not a function. it cannot be used here.",lambda_func);
						return 0;
						}
					params=li_quote_param_list(params,env);
					}
				}
			return li_call((li_symbol*)lambda_func,params,env);
			
			}
		else
			{
			li_error(env,"USER: Lambda_eval: %O doesn't resolve to a callable function.",lambda);
			return 0;
			}
		}
	li_environment *lambda_env=new li_environment(env,i4_T);
	if (!li_bind_params(li_car(li_cdr(lambda,env),env),params,lambda_env,flags))
		return 0;
	li_object *expr=li_cdr(li_cdr(lambda,env),env);
	while (expr)
		{
		ret=li_eval(li_car(expr,env),lambda_env);
		expr=li_cdr(expr,env);
		}
	return ret;
	}

li_object *li_call(li_symbol *val, li_object *params, li_environment *env)
{
  if (val)
  {
    li_symbol *old_fun=0;
    li_object *old_args=0;
    if (env)
    {     
      old_fun=env->current_function();
      old_args=env->current_arguments();
    }
    else //is this required?  Yes it is. see next line.
	//if the toplevel call is something like "(defvar)" we must not have an environment.
      env=new li_environment(env, i4_F);

    env->current_function()=val;
    env->current_arguments()=params;
    
    
    li_object *ret=0;    
    li_object *f=li_get_fun(val, env);
    if (f)
    {
	  li_function_type fun=0;
	  if (f->type()==LI_FUNCTION)
		fun=li_function::get(f,env)->value();
	  else
		fun=li_user_function::get(f,env)->value();
      if (fun)
        ret=fun(params, env);      
    }
    else
      li_error(env, "USER: Symbol %O has no function", val);
    
    if (old_fun)
    {
      env->current_function()=old_fun;
      env->current_arguments()=old_args;
    }

    return ret;
  }

  return 0;
}

/*LISPFUN*
This function implements the class-access operator for the general case
*/
LI_HEADER(member_access)
	{
	li_class *cl=li_class::get(li_eval(li_first(o,env),env),env);
	li_symbol *memb=li_symbol::get(li_eval(li_second(o,env),env),env);
	int membindex=cl->member_offset(memb);
	if (membindex<0)
		{
		li_error(env,"USER: Class %O has no member named %O. ",cl,memb);
		return 0;
		}
	return cl->value(membindex);
	}


li_object *li_class::object_itself(int member)
	{ 
	//doing it directly seems to confuse the compiler with the
	//complicated castings involved.
	li_object **v=(li_object**) values;
	return (v[member]); 
	}

li_object **li_class::object_place(int member)
	{
	li_object **v=(li_object **)values;
	//li_object *r=v[member];
	return &v[member];
	}

li_object *li_call(char *fun_name, li_object *params, li_environment *env)
{
  return li_call(li_get_symbol(fun_name), params, env);
}

//first and nth-implementations. the _operator functions are to be called
//from user code with one arg (either a list constructor or a symbol)
//the others are for C use and return the nth element of the given list 
//without evaluating anything

li_object  *li_first(li_object *o, li_environment *env) 
	{ 
	return li_car(o,env); 
	}

LI_HEADER(first_operator)
	{
	return li_car(li_eval(li_car(o,env),env),env);
	}

li_object  *li_second(li_object *o, li_environment *env) 
	{ 
	return li_car(li_cdr(o,env),env); 
	}

LI_HEADER(second_operator)
	{
	return li_car(li_cdr(li_eval(li_car(o,env),env),env),env);
	}

li_object  *li_third(li_object *o, li_environment *env) 
	{ 
	return li_car(li_cdr(li_cdr(o,env),env),env); 
	}

LI_HEADER(third_operator)
	{
	return li_car(li_cdr(li_cdr(li_eval(li_car(o,env),env),env),env),env);
	}

li_object  *li_fourth(li_object *o, li_environment *env) 
{ return li_car(li_cdr(li_cdr(li_cdr(o,env),env),env),env); }

LI_HEADER(fourth_operator)
	{
	return li_car(li_cdr(li_cdr(li_cdr(li_eval(li_car(o,env),env),env),env),env),env);
	}

li_object  *li_fifth(li_object *o, li_environment *env) 
{ return li_car(li_cdr(li_cdr(li_cdr(li_cdr(o,env),env),env),env),env); }

LI_HEADER(fifth_operator)
	{
	return li_car(li_cdr(li_cdr(li_cdr(li_cdr(li_eval(li_car(o,env),env),env),env),env),env),env);
	}

li_object  *li_sixth(li_object *o, li_environment *env)
	{ return li_car(li_cdr(li_cdr(li_cdr(li_cdr(li_cdr(o,env),env),env),env),env),env);}

LI_HEADER(sixth_operator)
	{
	return li_car(li_cdr(li_cdr(li_cdr(li_cdr(li_cdr(li_eval(li_car(o,env),env),env),env),env),env),env),env);
	}

li_object  *li_nth(li_object *o, int x, li_environment *env) 
	{ 
	if (x<0) 
		{
		li_error(env,"USER: li_nth: first argument must be positive.");
		return 0;
		}
	while (x--) 
		o=li_cdr(o,env); 
	return li_car(o,env); 
	}

li_object *li_seventh(li_object *o, li_environment *env)
	{return li_nth(o,7,env);}

LI_HEADER(seventh_operator)
	{
	return li_nth(li_eval(li_car(o,env),env),7,env);
	}

li_object *li_eighth(li_object *o, li_environment *env)
	{return li_nth(o,8,env);}

LI_HEADER(eighth_operator)
	{
	return li_nth(li_eval(li_car(o,env),env),8,env);
	}
li_object *li_ninth(li_object *o,li_environment *env)
	{return li_nth(o,9,env);}

LI_HEADER(ninth_operator)
	{
	return li_nth(li_eval(li_car(o,env),env),9,env);
	}

li_object *li_tenth(li_object *o,li_environment *env)
	{return li_nth(o,10,env);}

LI_HEADER(tenth_operator)
	{
	return li_nth(li_eval(li_car(o,env),env),10,env);
	}

li_object *li_nth_operator(li_object *o, li_environment *env)
	{
	return li_nth(li_eval(li_car(li_cdr(o,env),env),env),li_get_int(li_eval(li_car(o,env),env),env),env);
	}

// lisp/li_alloc.cpp
/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/


static i4_critical_section_class syms_lock;
static i4_critical_section_class cell_lock;
static volatile int threads_need_gc=0;

li_object_pointer *li_object_pointer_list=0;
i4_profile_class pf_li_gc("li_gc");


li_object *li_not(li_object *o, li_environment *env)
{
  li_object *v=li_eval(li_car(o,env),env);
  if (!v || v==li_nil)
    return li_true_sym;
  else return li_nil;
}

li_object *li_perm_space(li_object *o, li_environment *env)
	{//PG: Has no function yet, I'm unshure wheter this is needed in golg (comes from abuse)
	return 0;
	}

li_object *li_tmp_space(li_object *o, li_environment *env)
	{
	return 0;
	}

LI_HEADER(push_operator)
	{//append first argument to the list whose symbol is the second arg.
	//(second arg must be symbol)
	li_symbol *sym=li_symbol::get(li_car(li_cdr(o,env),env),env);
	li_object *o2=new li_list(li_eval(li_car(o,env),env),
		li_get_value(sym,env));
	li_set_value(sym,o2,env);
	return o2;
	}

LI_HEADER(pop_operator)
	{
	li_symbol *sym=li_symbol::get(li_car(o,env),env);
	//sym->set_value(li_cdr(sym->value(),env));
	//should newer call set_value directly
	li_set_value(sym,li_get_value(sym,env),env);
	return li_get_value(sym,env);
	}

LI_HEADER(reverse)
	{
	//reverse the order of the list
	li_object *a=0;
	a=li_eval(li_car(o,env),env);
	li_list *l=0;
	while(a)
		{
		if (l) li_push(l,li_car(a,env));else l=new li_list(li_car(a,env));
		a=li_cdr(a,env);
		}
	return l;
	}

LI_HEADER(list_star)
	{
	li_object *first=0,*current=0,*last=0;
	int noargs=li_length(o,env);
	int actarg=1;
	while(o)
		{
		current=li_eval(li_car(o,env),env);
		o=li_cdr(o,env);
		if (!first)
			{
			if (noargs==1)
				return current;
			else
				first=last=new li_list(current);
			}
		else
			{
			if (actarg==noargs)
				{
				((li_list*)last)->set_next(current);
				return first;
				}
			else
				{
				current=new li_list(current);
				((li_list*)last)->set_next(current);
				last=current;
				}
			}
		actarg++;
		}
	return first?first:li_nil;
	}

li_object *li_explicit_list_constructor(li_object *o, li_environment *env)
	{
	//in contrary to the quote operator, list evaluates its arguments
	//li_list *l=li_list::get(o,env);
	//li_object *ret=0;
	li_object *first=0,*current=0,*last=0;
	while(o)
		{
		current=li_eval(li_car(o,env),env);
		o=li_cdr(o,env);
		if (!first)
			{
			first=last=new li_list(current);
			}
		else
			{
			current=new li_list(current);
			((li_list*)last)->set_next(current);
			last=current;
			}
		}
	return first?first:li_nil;
	//neu schreiben.
	/*while(l)
		{
		li_object *o1=li_eval(li_car(l,env),env);
		o1=new li_list(o1);
		if (ret) ret=new li_list(ret,o1);else ret=o1;
		l=li_list::get(li_cdr(l,env),env);
		}
	//ret=li_reverse(ret,env);
	return ret;//return the list whose elements are all except the function name (which is not passed)
	*/
	}

li_object *li_cons_operator(li_object *o, li_environment *env)
	{//CONS: append first argument to the second arg, which is a list
	//return li_cdr(o,env);
	li_object *a=li_eval(li_car(o,env),env);
	//if (a->type()!=LI_LIST) a=new li_list(a);
	li_object *l=li_eval(li_second(o,env),env);
	//if (l==li_nil) l=0;//end of list
	//if (l->type()!=LI_LIST) l=new li_list(l);
	return new li_list(a,l);
	}

li_object *li_progn(li_object *o, li_environment *env)
{
  li_object *ret=li_nil;
  while (o)
  {
    ret=li_eval(li_car(o,env),env);
    o=li_cdr(o,env);
  }
  return ret;
}

LI_HEADER(prog1)//return result of first form.
	{
	li_object *ret=li_nil;
	ret=li_eval(li_car(o,env),env);
	o=li_cdr(o,env);
	while(o) 
		{
		li_eval(li_car(o,env),env);
		o=li_cdr(o,env);
		}
	return ret;
	}

LI_HEADER(zerop)
	{
	int i=li_get_int(li_car(o,env),env);
	return (i==0?li_true_sym:li_nil);
	}

li_object *li_if(li_object *o, li_environment *env)
{
  li_object *v=li_eval(li_car(o,env), env);

  if (v && v!=li_nil)
    return li_eval(li_second(o,env),env);
  
  o=li_cdr(li_cdr(o,env),env);
  li_object *ret=li_nil;
  while (o)
      {
      ret=li_eval(li_car(o,env), env);
      o=li_cdr(o,env);
      }
  return ret;
}



LI_HEADER(eq)//compare addresses only
	{//don't do any evaluation
    li_object *o1=li_first(o,env);
    li_object *o2=li_second(o,env);
    if (o1==o2)
        return li_true_sym;
    if (o1->type()==LI_INT && o2->type()==LI_INT)
        {
        if (li_int::get(o1,env)->value()==li_int::get(o2,env)->value())
            return li_true_sym;
        return li_nil;
        }
	
    if (o1->type()==LI_LIST &&
        o2->type()==LI_LIST &&
        (li_car(o1,env)==li_quote) 
        && (li_car(o2,env)==li_quote))
        {
        if (li_eval(o1,env)==li_eval(o2,env))
            return li_true_sym;
        }
	return li_nil;
	}

LI_HEADER(implementation_version)
	{
	//give some copyright info
	return new li_string("CRACK.COM LISP IMPLEMENTATION 2.0golg on GOLGOTHA ENGINE CORE V1.0.8.1. "
		"Implements many base CLISP features. Huge improvements done by Patrick Grawehr in 2001-2003");
	}

LI_HEADER(eql)
	{
	li_object *o1=li_first(o,env);
	li_object *o2=li_second(o,env);
	if (o1==o2)
		{
		return li_true_sym;
		}
	if (o1->type()==o2->type())
		{
		if (li_get_type(o1->type())->equal(o1,o2))
			{
			return li_true_sym;
			}
		}
	return li_nil;
	}




li_object *li_equal(li_object *o, li_environment *env)
{
  li_object *o1=li_eval(li_first(o,env),env);
  li_object *o2=li_eval(li_second(o,env),env);

  if (o1->type()!=o2->type()) return li_nil;
  if (!(li_get_type(o1->type())->equal(o1, o2)))
      return li_nil;
  
  o2=li_cdr(li_cdr(o,env),env);
  while (o2)
	  {
	  if (!(li_get_type(o1->type())->equal(o1,o2)))
		  return li_nil;
	  o2=li_cdr(o2,env);
	  }
  return li_true_sym;
}

li_object *li_assoc(li_object *o, li_environment *env)
	{
	li_object *item=li_eval(li_car(o,env),env);
	li_object *list=li_eval(li_second(o,env),env);
	while (list)
		{
		li_object *test=li_eql(li_make_list(li_car(li_car(list,env),env),item,0),env);
		if (li_get_bool(test,env))
			{
			return li_car(list,env);
			}
		list=li_cdr(list,env);
		}
	return li_nil;
	}

LI_HEADER(pairlis)
	{
	li_object *o1=li_eval(li_first(o,env),env);
	li_object *o2=li_eval(li_second(o,env),env);
	li_object *o3=li_cdr(li_cdr(o,env),env);
	if ((o1->type()!=LI_LIST)||(o2->type()!=LI_LIST))
		{
		return li_nil;
		}
	li_object *ret=0;
	int l1=li_length(o1,env);
	int l2=li_length(o2,env);
	if (l1!=l2)
		{
		li_error(env,"Error in pairlis: %O and %O have not the same length",o1,o2);
		return li_nil;
		}
	if (l1!=0)
		{
		li_object *cur=0,*last=0;
		while (o1!=0)
			{
			last=cur;
			cur=new li_list(li_car(o1,env),li_car(o2,env));
			cur=new li_list(cur,last);
			o1=li_cdr(o1,env);o2=li_cdr(o2,env);
			}
		if (o3) {
			o3=li_eval(li_car(o3,env),env);
			if (o3->type()!=LI_LIST) o3=new li_list(o3);
			ret=new li_list(li_car(cur,env),o3);
			}
		else
			ret=cur;
		}
	return ret;
	}

LI_HEADER(atom)
	{
	li_object *at=li_eval(li_car(o,env),env);
	if (at==0||at==li_nil) return li_true_sym;
	int t=at->type();
	if (t!=LI_LIST&&t!=LI_FUNCTION&&t!=LI_INVALID_TYPE&&t!=LI_TYPE&&
		t!=LI_ENVIROMENT)
		{
		return li_true_sym;
		}
	else
		return li_nil;
	}
LI_HEADER(consp)
	{
	li_object *at=li_eval(li_car(o,env),env);
	if (at==0||at==li_nil) return li_nil;
	int t=at->type();
	if (t!=LI_LIST)
		{
		return li_nil;
		}
	else
		return li_true_sym;
	}



/*LI_HEADER(append)
	{//there must be an error lying around here.
	//I suppose I missunderstood the description of append
	//retry bellow.
	li_list *l=0,*lstart=0,*last=0;
	li_list *args=(li_list*)o;
	while(args)
		{
		li_object *a=li_eval(li_car(args,env),env);
		if (!a) return 0;
		if (a->type()!=LI_LIST)
			{
			if (last)
				last->set_next(a);
			else 
				lstart=new li_list(a);
			return lstart;
			}
		else
			{
			li_object *b=a;
			while(b&&b->type()==LI_LIST)
				{
				if (!last)
					{
					last=new li_list(li_car(b,env));lstart=last;
					b=li_cdr(b,env);
					}
				else
					{
					l=new li_list(li_car(b,env));
					b=li_cdr(b,env);
					last->set_next(l);
					last=l;
					}
				}
			}
		args=(li_list*)li_cdr(args,env);


		}
	return lstart;

	}*/

LI_HEADER(append)
	{
	li_list *first=0, *next=0, *last=0;
	li_object *cur=o;
	li_object *start=o;
	li_vector *v=new li_vector();
	li_object_vect_type *args=v->reserved();
	li_object *tmp=0;
	int noargs=0;
	while (cur)
		{
		noargs++;
		tmp=li_eval(li_car(cur,env),env);
		if (tmp!=0)
			{
			args->add(tmp);
			}
		cur=li_cdr(cur,env);
		}
	for (int i=0;i<noargs;i++)
		{
		cur=args->operator [](i);
		if (!first)
			{
			if (cur->type()==LI_LIST)
				//copy arg
				first=last=new li_list(li_car(cur,env),li_cdr(cur,env));
			else
				if (noargs==1)
					return cur;
				else
					{
					li_error(env,"USER: append: %O is not a list, only last argument may be another type.",cur);
					return 0;
					}
			}
		else
			{
			if (cur->type()==LI_LIST)
				last->set_next(cur);
			else
				{
				if (i==(noargs-1))
					{
					last->set_next(cur);
					return first;
					}
				else
					{
					li_error(env,"USER: append: %O is not a list.",cur);
					return 0;
					}
				}

			
			}
		while (last->next()) last=(li_list*)last->next();//go to end of list
		}//for
	return first;
	}





LI_HEADER(let)
	{//the assignments are done "simoultaneously", so use old environment for 
	//var_list evaluations
	li_object *var_list=li_car(o,env);
	li_object *block_list=li_cdr(o,env);
	if (!env) env=new li_environment(0,i4_F);
	li_environment *local=new li_environment(env,i4_T);
	while(var_list&&(var_list!=li_nil))
		{
		li_object *a=li_car(var_list,local);
		if (a->type()==LI_SYMBOL)//for cases like (let (x) (setq x 10))
			{
			li_define_value((li_symbol*)a,li_nil,local);
			}
		else
			{
			li_symbol *sym=li_symbol::get(li_car(a,local),local);
			li_object *value=li_eval(li_car(li_cdr(a,env),env),env);
			li_define_value(sym,value,local);//create symbol in new (local) environment
			}
		var_list=li_cdr(var_list,env);
		}
	li_object *ret=0;
	while(block_list&&(block_list!=li_nil))
		{
		ret=li_eval(li_car(block_list,local),local);//setq's here will automatically find corresponding env
		block_list=li_cdr(block_list,local);
		}
	return ret;
	}

//Add a property value to a symbol
LI_HEADER(put)
	{
	li_symbol *symb=li_symbol::get(li_eval(li_first(o,env),env),env);
	li_object *value=li_eval(li_third(o,env),env);
	symb->add_property(li_eval(li_second(o,env),env),value);
	return value;
	/*
	Bad implementation idea
	li_object *propname=li_eval(li_second(o,env),env);
	li_object *value=li_eval(li_third(o,env),env);
	li_symbol *symb1=li_symbol::get(symb,env);
	li_symbol *propname2=li_symbol::get(propname,env);
	i4_str *pname=new i4_str(symb1->name()->value());
	pname->insert(pname->end(),'#');
	pname->insert(pname->end(),propname2->name()->value());
	li_define_value(pname->c_str(),value,env);
	return value;
	*/
	}

//retrieve a property value
LI_HEADER(get)
	{
	li_symbol *symb=li_symbol::get(li_eval(li_first(o,env),env),env);
	return symb->get_property(li_eval(li_second(o,env),env));
	}

LI_HEADER(symbol_plist) //symbol-plist for lisp
	{
	li_symbol *symb=li_symbol::get(li_eval(li_first(o,env),env),env);
	return symb->get_plist();
	}

LI_HEADER(setplist)
	{
	li_symbol *symb=li_symbol::get(li_eval(li_first(o,env),env),env);
	li_object *value=li_eval(li_second(o,env),env);
	symb->set_plist(value);
	return value;
	}

LI_HEADER(let_star)
	{//the assignments are done in order, 
	li_object *var_list=li_car(o,env);
	li_object *block_list=li_cdr(o,env);
	if (!env) env=new li_environment(0,i4_F);
	li_environment *local=new li_environment(env,i4_T);
	while(var_list)
		{
		li_object *a=li_car(var_list,local);
		if (a->type()==LI_SYMBOL)//for cases like (let (x) (setq x 10))
			{
			li_define_value((li_symbol*)a,li_nil,local);
			}
		else
			{
			li_symbol *sym=li_symbol::get(li_car(a,local),local);
			li_object *value=li_eval(li_car(li_cdr(a,local),local),local);
			li_define_value(sym,value,local);
			}
		var_list=li_cdr(var_list,env);
		}
	li_object *ret=0;
	while(block_list)
		{
		ret=li_eval(li_car(block_list,local),local);
		block_list=li_cdr(block_list,local);
		}
	return ret;
	}


LI_HEADER(user_function_evaluator)//called for any user function
	{
	li_symbol *val=env->current_function();
    li_object *ret=0;    
    li_object *f=li_get_fun(val, env);//get pointer to function code data
	li_user_function::user_function_data *uf=li_user_function::get(f,env)->data();
	
	if (!env) env=new li_environment(0,i4_F);
	li_environment *local=new li_environment(env,i4_T);
	li_object *p=li_list::get(uf->_params,local);//be type-safe
	li_object *act=li_list::get(o,local);
	/*while(p)//varargs not yet supported
		{
		li_symbol *sym=li_symbol::get(li_car(p,local),local);
		li_object *val=li_eval(li_car(act,local),local);
		li_set_value(sym,val,local);
		p=li_cdr(p,local);
		act=li_cdr(act,local);
		}
	*/
	li_bind_params(p,act,local);
	li_object *expr=uf->_code;
	while (expr)
		{
		ret=li_eval(li_car(expr,local),local);
		expr=li_cdr(expr,local);
		}
	//evaluate the function with the given params
	return ret;
	}

LI_HEADER(user_macro_evaluator)
	{
    //we don't only need to use noevalonbind here but need to quote
    //every argument before the call, because they will be double-evaled
    //anyway. (at least I think this will help)
    li_object *o1=li_get_type(o)->copy(o);
    //li_list *ob=li_list::get(o1,env);
    
    //while (ob)
    //    {
        //ob is the list of parameters
    //    ob->set_data(li_make_list(li_quote,ob->data(),0));
    //    ob=li_list::get(li_cdr(ob,env),env);
    //    }

	li_object *o2=new li_list(li_get_symbol("&noevalonbind"),o1);
	li_object *macroeval=li_user_function_evaluator(o2,env);
	return li_eval(macroeval,env);//evaluate result of macro expansion.
	}

LI_HEADER(makunbound)
	{
	li_symbol *s=li_symbol::get(li_eval(li_first(o,env),env),env);
	li_set_value(s,0,env);//the gc will do the rest
	return s;
	}

LI_HEADER(fmakunbound)
	{
	li_symbol *s=li_symbol::get(li_eval(li_first(o,env),env),env);
	s->set_fun(0);
	return s;
	}

	
LI_HEADER(documentation)
	{
	li_symbol *s=li_symbol::get(li_first(o,env),env);
	if (s->fun())
		{
		if (s->fun()->type()==LI_USER_FUNCTION)
			return li_user_function::get(s->fun(),env)->data()->_reserved;
		}
	return li_nil;
	}

LI_HEADER(special_function_evaluator)
	{
	li_object *ret=0;
	li_symbol *val=env->current_function();
    
    li_object *f=li_get_fun(val, env);//get pointer to function code data
	li_user_function::user_function_data *uf=li_user_function::get(f,env)->data();
	
	if (!env) env=new li_environment(0,i4_F);//very seldom if this is called with no env
	li_environment *local=uf->_locals;
	local=local->set_next(env);//include old environment in search chain (only top-level needed, i hope)
	
	
	//li_object *p=li_list::get(uf->_params,local);//be type-safe
	//li_object *act=li_list::get(o,local);
	ret=li_lambda_eval(uf->_code,o,local,LEF_NOEVALONBIND);
	return ret;
	}

LI_HEADER(symbol_value)
	{
	return li_eval(li_car(o,env),env);
	}

LI_HEADER(function)//called when function is evaluated
	{
	li_user_function *ret=0;
	li_object *fn=li_car(o,env);
	li_string *n=li_get_symbol("lambda")->name();
	//currently, name must be "lambda" otherwise, we risk infinite recusion on evaluation
	//on something like ((function +) 8 9)
	/*if (fn->type() == LI_SYMBOL)
		{
		n=li_symbol::get(fn,env)->name();
		}
	else if (fn->type() == LI_FUNCTION)
		{
		n=new li_string("#<INTERNAL SYSTEM FUNCTION>");
		}
	else if (fn->type() == LI_USER_FUNCTION)
		{
		n=li_user_function::get(fn,env)->name();
		}*/
	//if (o->type()==LI_LIST)
	//	{
		ret=new li_user_function(li_special_function_evaluator,
			0,env,0,fn,n);
	//	}
	//else if (o->type()==LI_SYMBOL)
	//	{
	//	ret=li_get_fun((li_symbol*)o,env);
	//	}
	return ret;
	}

LI_HEADER(defmacro_operator)
	{
	li_object *n1=li_first(o,env);
	//if (n1->type()==LI_LIST) n1=li_eval(li_car(n1,env),env);//special case if symbol is evaluated
	if (n1->type()!=LI_SYMBOL)
		{
		li_error(env,"USER: defmacro: First argument must be a symbol.");
		return 0;
		}
	li_symbol *n=li_symbol::get(n1,env);//name
	li_object *hd=li_second(o,env);//param list
	if (hd==li_nil) hd=0;//faster to test on evaluation
	li_object *body=li_cdr(li_cdr(o,env),env);//body
	li_string *docu=0;
	if (li_car(body,env)->type()==LI_STRING)
		{
		docu=li_string::get(li_car(body,env),env);
		//has a documentation-string
		body=li_cdr(body,env);
		}
	
	li_user_function *fn=new li_user_function(li_user_macro_evaluator,
		hd,env,0,body,n->name());
	fn->data()->_reserved=docu;
	
	li_symbol *sym=li_symbol::get(n,0);//defmacro has always global effect.
	sym->set_fun(fn);
	return n;
	}

LI_HEADER(defun_operator)
	{
	li_object *n1=li_first(o,env);
	if (n1->type()==LI_LIST) n1=li_eval(li_car(n1,env),env);//special case if symbol is evaluated
	li_symbol *n=li_symbol::get(n1,env);//name
	li_object *hd=li_second(o,env);//param list
	if (hd==li_nil) hd=0;//faster to test on evaluation
	li_object *body=li_cdr(li_cdr(o,env),env);//body
	li_string *docu=0;
	if (li_car(body,env)->type()==LI_STRING)
		{
		docu=li_string::get(li_car(body,env),env);
		//has a documentation-string
		body=li_cdr(body,env);
		}
	li_user_function *fn=new li_user_function(li_user_function_evaluator,
		hd,env,0,body,n->name());
	fn->data()->_reserved=docu;
	
	li_symbol *sym=li_symbol::get(n,0);//defun has always global effect.
	//if (env)
	//	env->set_fun(sym, fn);
	//else
		sym->set_fun(fn);
	return n;
	}

li_object *li_symbol_list(li_object *o, li_environment *env)
	{
	return li_nil; //Abuse has this like this, dono what it's for.
	}

LI_HEADER(cdr_operator)
	{
	return li_cdr(li_eval(li_car(o,env),env),env);//gives the first entry of the first param
	}

LI_HEADER(car_operator)
	{
	return li_car(li_eval(li_car(o,env),env),env);
	}

LI_HEADER(nthcdr)
	{
	int i=li_get_int(li_eval(li_car(o,env),env),env);
	o=li_eval(li_second(o,env));
	while (i>0)
		{
		o=li_cdr(o,env);
		i--;
		}
	return o;
	}


LI_HEADER(intern)
	{
	li_string *s= li_string::get(li_eval(li_car(o,env),env),env);
	return li_get_symbol(s->value());
	}


li_object *private_backquote(li_object *o, li_environment *env)
	{//don't call directly
	if (o==0||o==li_nil) return 0;//many functions insist on li_nil==0
	if (o->type()!=LI_LIST)
		return o;
	
	if (li_car(o,env)==li_comma)
		{//a comma: evaluate next element, continue then.
		//@ not yet supported
		li_object *a=li_eval(li_car(li_cdr(o,env),env),env);
		//if (a->type()==LI_LIST) a=li_car(a,env);//if this was a function, its result is here 
		li_object *b=private_backquote(li_cdr(li_cdr(o,env),env),env);
		return new li_list(a,b);
		//warning: for some arbitrary reason, my compiller would evaluate b before a, 
		//which results in strange problems (x not defined)
		//under the following example `(hallo ,(setf x 10) ,x)
		}
	else 
		{
		li_object *a2=li_car(o,env);
		//if (a2->type()==LI_LIST)//recurse only if list
		a2=private_backquote(a2,env);
		li_object *b2=private_backquote(li_cdr(o,env),env);
		return new li_list(a2,b2);
		}
	}

li_object *backquote(li_object *o,li_environment *env)
	{
	return private_backquote(li_car(o,env),env);//for some reason, this is needed
	}

li_object *li_acons(li_object *o, li_environment *env)
	{//clisp means (acons a b c) is same as (cons (cons a b) c) 
	//Abuse has something different.
	li_object *o1=li_eval(li_first(o,env),env);
	li_object *o2=li_eval(li_second(o,env),env);
	li_object *o3=li_eval(li_third(o,env),env);
	return new li_list(new li_list(o1,o2),o3);//Eh, GC is fun: We write as in java...
	}




li_object_pointer::li_object_pointer(li_object *obj)
{
  o=obj;
  next=li_object_pointer_list;
  li_object_pointer_list=this;
}

li_object_pointer::~li_object_pointer()
{
  if (this==li_object_pointer_list)
    li_object_pointer_list=next;
  else
  {
    li_object_pointer *last=0, *p;
    for (p=li_object_pointer_list; p && p!=this;)
    {
      last=p;
      p=p->next;
    }
    if (p!=this) 
      li_error(0, "INTERNAL: li_object_pointer::~li_object_pointer(): Couldn't find object pointer to unlink.");
    last->next=next;
  }
}


// global symbols
li_symbol *li_nil=0, 
  *li_true_sym=0, 
  *li_quote=0, 
  *li_backquote=0,
  *li_comma=0,
  *li_function_symbol=0,
  *li_vector_symbol=0,
  *li_lambda_symbol=0;

static li_gc_object_marker_class *gc_helpers=0;

li_gc_object_marker_class::li_gc_object_marker_class()
{
  next=gc_helpers;
  gc_helpers=this;
}

li_gc_object_marker_class::~li_gc_object_marker_class()
{
  if (gc_helpers==this)
    gc_helpers=gc_helpers->next;
  else
  {
    li_gc_object_marker_class *last=0, *p;
    for (p=gc_helpers; p!=this;)
    {
      last=p;
      p=p->next;        
    }
    if (!p) 
      li_error(0,"INTERNAL: Garbage Collector: gc_object marker not in list");
    last->next=p->next;
  }
}

void li_mark_symbols(int set);


//li_symbol *li_root=0;

class li_sym_hashtable
    {
    public:
        enum {TABLE_SIZE=16384};
    protected:
        li_symbol *table[TABLE_SIZE];
        i4_bool active;
        li_symbol *li_find_symbol(li_symbol *&li_root, const char *name)    
            {
            //Locking is not really necessary, as the only point
            //where a race condition might occur is when one thread is just
            //adding a node and another thread is passing into this node.
            //Since we can assume that a 32-bit write is integral, this can't
            //give a trouble either. 
            //syms_lock.lock();
            if (li_root)
                {
                li_symbol *p=li_root;
                for(;;)
                    {
                    int cmp=strcmp(name,p->name()->value());
                    if (cmp<0)
                        {
                        if (p->left())
                            p=p->left();
                        else
                            {
                            //syms_lock.unlock();
                            return 0;
                            }
                        } else if (cmp>0)
                        {
                        if (p->right())
                            p=p->right();
                        else
                            {
                            //syms_lock.unlock();
                            return 0;
                            }
                            } else 
                            {
                            //syms_lock.unlock();
                            return p;
                                }
                    }
                }
            
            //syms_lock.unlock();
            return 0;
            }
        
        li_symbol *li_get_symbol(li_symbol *&li_root, const char *name)     // if symbol doesn't exsist, it is created
            {
            //syms_lock.lock();
            
            if (!li_root)
                {
                li_root=new li_symbol(new li_string(name));
                //syms_lock.unlock();
                return li_root;
                }
            else
                {
                li_symbol *p=li_root;
                for(;;)
                    {
                    
                    int cmp=strcmp(name,p->name()->value());
                    if (cmp<0)
                        {
                        if (p->left())
                            p=p->left();
                        else
                            {
                            li_symbol *atleft=new li_symbol(new li_string(name));
                            p->set_left(atleft);
                            //syms_lock.unlock();
                            return p->left();
                            }
                        } else if (cmp>0)
                        {
                        if (p->right())
                            p=p->right();
                        else
                            {
                            li_symbol *atright=new li_symbol(new li_string(name));
                            p->set_right(atright);
                            //syms_lock.unlock();
                            return p->right();
                            }
                            } else
                            {
                            //syms_lock.unlock();
                            return p;
                                }
                    }
                }
            
            //syms_lock.unlock();
            return 0;
            }
        void li_recursive_mark(li_symbol *p, int set)
        {
            if (p)
            {
              li_get_type(LI_SYMBOL)->mark(p, set);
              li_recursive_mark(p->left(), set);
              li_recursive_mark(p->right(), set);
            }
        }
        static w16 li_check_sum16(const char *buf)
            {
            w8 c1=0,c2=0;
            
            while (*buf)
                {
                c1+=*buf;
                buf++;
                c2^=c1;
                }
            return (c1|(c2<<7));//we will only be using 14 bits
            }
    public:
        li_sym_hashtable()
            {
            memset(table,0,sizeof(table));
            active=i4_T;
            }
        // Creates a new entry if it doesn't exist.
        li_symbol *get_symbol(w32 key, const char *name)
            {
            syms_lock.lock();
            li_symbol *&local_root=table[key%TABLE_SIZE];
            li_symbol *ret=li_get_symbol(local_root,name);
            syms_lock.unlock();
            return ret;
            }
        //does never alter the table
        li_symbol *lookup_symbol(w32 key, const char *name)
            {
            syms_lock.lock();
            li_symbol *&local_root=table[key%TABLE_SIZE];
            li_symbol *ret=li_find_symbol(local_root,name);
            syms_lock.unlock();
            return ret;
            }
        void mark(int set)
            {
            for (int k=0;k<TABLE_SIZE;k++)
                {
                li_recursive_mark(table[k],set);
                }
            }
        static w32 fast_hash(const char *name)
            {
            return li_check_sum16(name);
            }

        i4_bool is_active() const
            {
            return active;
            }
        void uninit()
            {
            active=i4_F;
            //Just dump all the references.
            memset(table,0,sizeof(table));
            }
        void symbol_info(long &nosymb, long &maxdepth, long &empty)
            {
            nosymb=0;
            maxdepth=0;
            empty=0;
            for (int i=0;i<TABLE_SIZE;i++)
                {
                if (table[i])
                    {
                    private_li_symbol_info(table[i],0,nosymb,maxdepth);
                    }
                else
                    empty++;
                }
            }
    private:
        void private_li_symbol_info(li_symbol *p,long depth, 
            long &nosymbols, long &maxdepth)
            {
            if (p)
                {
                if (depth>maxdepth) maxdepth=depth;
                nosymbols++;
                if (p->type()!=LI_SYMBOL) li_error(0,"CRITICAL: li_symbol_info: Invalid type occured in symbol table.");
                private_li_symbol_info(p->left(),depth+1,nosymbols,maxdepth);
                char *c=p->name()->value();
                i4_warning(c);
                //Sleep(1);//required, because if OutputDebugString is called to fast, MSVC skips output.
                i4_thread_sleep(1);
                private_li_symbol_info(p->right(),depth+1,nosymbols,maxdepth);
                }
            }
    };

li_sym_hashtable li_hash;

/// Search recursive symbol tree.
/// That's all what's there about the symbol tree. This method
/// will be recoded to use another data structure, but the interface
/// will stay the same.
/// If the symbol doesn't exist, NULL is returned.
/// \param name The name of the symbol to check for
/// \return The symbol pointer or NULL.
li_symbol *li_find_symbol(const char *name)
    {
    w32 key=li_sym_hashtable::fast_hash(name);
    return li_hash.lookup_symbol(key,name);
    }

/*
li_symbol *li_find_symbol(const char *name)    
{
  syms_lock.lock();
  if (li_root)
  {
    li_symbol *p=li_root;
    for(;;)
    {
      int cmp=strcmp(name,p->name()->value());
      if (cmp<0)
      {
        if (p->left())
          p=p->left();
        else
        {
          syms_lock.unlock();
          return 0;
        }
      } else if (cmp>0)
      {
        if (p->right())
          p=p->right();
        else
        {
          syms_lock.unlock();
          return 0;
        }
      } else 
      {
        syms_lock.unlock();
        return p;
      }
    }
  }

  syms_lock.unlock();
  return 0;
}
*/

/*
i4_bool check_symtable_consistency(li_symbol *sym)
	{
	if (!sym) return i4_T;
	if (sym->type()!=LI_SYMBOL) 
		{
		li_error(0,"CRITICAL: Symbol Table inconsistency detected: %O",sym);
		return i4_F;
		}
	if (!check_symtable_consistency(sym->left())) return i4_F;
	if (!check_symtable_consistency(sym->right())) return i4_F;
	return i4_T;
	}
*/

/// Search the symbol tree for a symbol.
/// If the symbol doesn't exist, it is created.
/// \param name The symbol to be searched for.
/// \return A symbol pointer. Never NULL.
li_symbol *li_get_symbol(const char *name)
    {
    // Return a hash value of the name. 
    // Probably we'll do something faster than i4_w32_checksum, also because we only need 16 bit.
    w32 key=li_sym_hashtable::fast_hash(name);
    return li_hash.get_symbol(key,name);
    }

/*
li_symbol *li_get_symbol(const char *name)     // if symbol doesn't exsist, it is created
{
  syms_lock.lock();
  //if ((int) name==0xfdfdfdfd) //This works only in debug-mode, I suppose
  //	  i4_error("li_get_symbol: unnamed symbols cannot be searched for. Check Syntax.");
//#ifdef _DEBUG //these were for an error I just couldn't find. 
  //The problem is that, if memory is very low, some non-empty cells happen
  //to get into the free-cell list
  //see the bret->_type!=0 error in alloc.
//  check_symtable_consistency(li_root);
//#endif
  if (!li_root)
  {
    li_root=new li_symbol(new li_string(name));
    syms_lock.unlock();
    return li_root;
  }
  else
  {
    li_symbol *p=li_root;
    for(;;)
    {
//#ifdef _DEBUG
//	  if (p->type()!=LI_SYMBOL)li_error(0,"CRITICAL: Symbol table: Invalid entry found");
//#endif
      int cmp=strcmp(name,p->name()->value());
      if (cmp<0)
      {
        if (p->left())
          p=p->left();
        else
        {
		  li_symbol *atleft=new li_symbol(new li_string(name));
//#ifdef _DEBUG
//		  li_symbol::get(atleft,0);
//#endif
          p->set_left(atleft);
          
//#ifdef _DEBUG
//  check_symtable_consistency(li_root);
//#endif
		  syms_lock.unlock();
          return p->left();
        }
      } else if (cmp>0)
      {
        if (p->right())
          p=p->right();
        else
        {
		  li_symbol *atright=new li_symbol(new li_string(name));
//#ifdef _DEBUG
//		  li_symbol::get(atright,0);
//#endif
          p->set_right(atright);
//#ifdef _DEBUG
//  check_symtable_consistency(li_root);
//#endif
          syms_lock.unlock();
          return p->right();
        }
      } else
      {
        
//#ifdef _DEBUG
//  check_symtable_consistency(li_root);
//#endif
        syms_lock.unlock();
        return p;
      }
    }
  }

  syms_lock.unlock();
  return 0;
}
*/

li_symbol *li_get_symbol(char *name, li_symbol *&cache_to)
{
  if (cache_to) return cache_to;
  cache_to=li_get_symbol(name);
  return cache_to;
}



void li_mark_symbols(int set)
{
  li_hash.mark(set);    
}



LI_HEADER(symbol_info)
	{
	syms_lock.lock();
	long nosymbols=0,maxdepth=0;
    long emptyfields=0;
	li_hash.symbol_info(nosymbols,maxdepth,emptyfields);
	i4_warning("Currently are %i symbols present. The maximum symbol tree depth is %i, (%i empty fields)",nosymbols,maxdepth,emptyfields);
	syms_lock.unlock();
	return new li_list(new li_int(nosymbols),new li_int(maxdepth));
	}

void li_mark_symbol_tree(li_symbol *s, int set)
{
  if (s)
  {
    if (set!=s->is_marked())
      li_get_type(LI_SYMBOL)->mark(s, set);

    li_mark_symbol_tree(s->left(), set);
    li_mark_symbol_tree(s->right(), set);
  }
}

void li_mark_memory_region(li_list **start, li_list **end,
                           li_list *c1, li_list *c2, int set, int mask=7)
{
  if (set)
  {//the core of the garbage collection: scan a memory region and test 
  //wheter a word contained in it may be a li_object* pointer.
  //if so, mark it.
  
    for (li_list **s=start; s!=end; s++)          
      if ( ((wptr)(*s)&mask)==0 &&  *s>=c1 && *s<c2 && (*s)->type() && !(*s)->is_marked())
        li_get_type( (*s)->unmarked_type() )->mark(*s,1);
  }
  else
    for (li_list **s=start; s!=end; s++)
      if (((wptr)(*s)&mask)==0 && *s>=c1 && *s<c2 && (*s)->is_marked())
        li_get_type( (*s)->unmarked_type() )->mark(*s,0);
  
}

LI_HEADER(aref)//the default (rvalue) implementation
	{
	li_object *vect=li_eval(li_car(o,env),env);//may be some (make-array) form
	if (vect->type()==LI_SYMBOL)//this is needed because aref is used like a special form
		{//if symbol, get contents of it
		vect=li_get_value(li_symbol::get(vect,env),env);
		}
	return li_vector::get(vect,env)->element_at(li_get_int(li_eval(li_car(li_cdr(o,env),env),env),env));
	}

LI_HEADER(comma_error)
	{
	li_error(env,"USER: Comma: Comma is illegal outside backquote");
	return li_nil;
	}

LI_HEADER(explicit_make_vector)
	{
	li_vector *v=new li_vector();
	o=li_car(o,env);//must dereference once.
	while (o)
		{
		v->add_element(li_car(o,env));
		o=li_cdr(o,env);
		}
	return v;
	}
//this is a helper function for the next, don't use otherwise
static void li_replace_nth_element(li_object *o, li_environment *env, li_object *update_width, int N)
	{
	li_object *list1=li_eval(o,env);
	for (int nth=0;nth<N;nth++)
			{
			list1=li_cdr(list1,env);
			}
	li_list::get(list1,env)->set_data(update_width);
	}

/* MSVC makes weird things with this one (only one N is used all over the place)
template<int N>
void li_replace_element(li_object *o, li_environment *env, li_object *update_width)
	{
	li_object *list1=li_eval(o,env);
	for (int nth=0;nth<N;nth++)
			{
			list1=li_cdr(list1,env);
			}
	li_list::get(list1,env)->set_data(update_width);
	}
	*/

void li_eval_lvalue(li_object *o, li_environment *env, li_object *update_width)
	{
	/*for setf, it is required to have an lvalue to set.
	this must only be defined for the following functions:
	aref        car        svref    
nth         cdr        get      
elt         caar       getf             symbol-value 
rest        cadr       gethash          symbol-function 
first       cdar       documentation    symbol-plist 
second      cddr       fill-pointer     macro-function 
third       caaar      caaaar           cdaaar 
fourth      caadr      caaadr           cdaadr 
fifth       cadar      caadar           cdadar 
sixth       caddr      caaddr           cdaddr 
seventh     cdaar      cadaar           cddaar 
eighth      cdadr      cadadr           cddadr 
ninth       cddar      caddar           cdddar 
tenth       cdddr      cadddr           cddddr
member
*/
	li_list *expr=li_list::get(o,env);
	li_symbol *sym=li_symbol::get(li_car(o,env),env);
	
	char *sname=sym->name()->value();
	if (strcmp(sname,"aref")==0)
		{
		li_object *vect=li_eval(li_car(li_cdr(o,env),env),env);//may be some (make-array) form
		if (vect->type()==LI_SYMBOL)
			{//if symbol, get contents of it
			vect=li_get_value(li_symbol::get(vect,env),env);
			}
		int offset=li_get_int(li_eval(li_third(o,env),env),env);
		li_object_vect_type *a=li_vector::get(vect,env)->reserved();
		a->operator [](offset)=update_width;//update array
		return;
		}
	if (strcmp(sname,"member")==0) //class member access operator
		{
		li_class *cl=li_class::get(li_eval(li_second(o,env),env),env);
		li_symbol *memb=li_symbol::get(li_eval(li_third(o,env),env),env);
		//*(cl->object_place(cl->member_offset(memb)))=update_width;
		cl->set_value(cl->member_offset(memb),update_width);
		return;
		}
	if (strcmp(sname,"nth")==0)
		{
		int i1=li_int::get(li_eval(li_second(o,env),env),env)->value();
		li_replace_nth_element(li_third(o,env),env,update_width,i1);
		return;
		}
	if (strcmp(sname,"first")==0)
		{
		li_replace_nth_element(li_second(o,env),env,update_width,0);
		return;
		}
	if (strcmp(sname,"second")==0)
		{
		li_replace_nth_element(li_second(o,env),env,update_width,1);
		return;
		}
	if (strcmp(sname,"third")==0)
		{
		li_replace_nth_element(li_second(o,env),env,update_width,2);
		return;
		}
	if (strcmp(sname,"fourth")==0)
		{
		li_replace_nth_element(li_second(o,env),env,update_width,3);
		return;
		}
	if (strcmp(sname,"fifth")==0)
		{
		li_replace_nth_element(li_second(o,env),env,update_width,4);
		return;
		}
	//...
	if (sname[0]=='c') //some sort of car or cdr
		{
		char sec=sname[1];//the first letter, either a or d;
		li_object *list=li_eval(li_car(li_cdr(o,env),env),env);
		int i=0;
		do
			{i++;
			}
		while (sname[i+1]!='r');
		for (int j=i;j>1;j--)
			{
			if (sname[j]=='a')
				{
				list=li_car(list,env);
				}
			else
				{
				list=li_cdr(list,env);
				}

			}
		if (sec=='a')
			{
			li_list::get(list,env)->_data=update_width;
			}
		else
			{
			li_list::get(list,env)->_next=update_width;
			}
		return;
		}
	li_error(env,"USER: setf: Unsupported first arg for setf.");
	return;	
	}

LI_HEADER(defvar)//same as (setf symbol value) but doesn't do anything if 
//symbol already has a value - returns symbol, not value
	{
	//always sets global context (env=0)
	li_symbol *s=li_symbol::get(li_car(o,env),env);
	li_object *val=li_get_value(s,0);
	if (val==0)
		{
		li_object *toeval=li_cdr(o,env);
		if (toeval!=0&&toeval!=li_nil)
			{
			val=li_eval(li_car(li_cdr(o,env),env),env);
			li_set_value(s,val,0);
			}
		}
	return s;
	}

LI_HEADER(make_local_variable)
    {
    li_symbol *sym=li_symbol::get(li_eval(li_first(o,env),env),env);
    if (!env)
        {
        li_set_value(sym,li_nil,0);
        }
    li_environment *varenv=env->env_for_symbol(li_get_symbol("window_identifier"));
    if (varenv==0)
        {
        //This is non-standard, but we can just do anything here, since
        //it never happens on emacs.
        env->define_value(sym,li_nil);
        }
    else
        {
        varenv->define_value(sym,li_nil);
        }
    return sym;
    }

LI_HEADER(memq)
    {
    li_object *lookfor=li_eval(li_first(o,env),env);
    li_object *list=li_eval(li_second(o,env),env);
    while(list)
        {
        if (li_car(list,env)==lookfor)
            {
            return list;
            }
        list=li_cdr(list,env);
        }
    return li_nil;
    }

li_object *li_while(li_object *o, li_environment *env)
    {
    for (;;)
        {
        li_object *cond=li_first(o,env);
        li_object *ret=li_eval(cond,env);
        if (ret==0 || ret==li_nil)
            break;
        li_object *list=li_cdr(o,env);
        while (list && list!=li_nil)
            {
            li_object *expr=li_car(list,env);
            li_eval(expr,env);
            list=li_cdr(list,env);
            }
        }
    //GNU emacs manual mentions that the value of while is always nil.
    return li_nil;


    }


LI_HEADER(defconstant)
	{
	//always sets global context (env=0)
	li_symbol *s=li_symbol::get(li_car(o,env),env);
	li_object *val=0;
	li_object *toeval=li_cdr(o,env);
	
	w32 f=s->flags();
	if (f&LSF_FORCECONSTANT)
		{
		li_error(env,"USER: defconstant: Cannot assign a new value to %O. Its definition is reserved.",s);
		return 0;
		}
	val=li_eval(li_car(li_cdr(o,env),env),env);
	s->set_flags(f&~LSF_VALUECONSTANT);
	li_set_value(s,val,0);
	s->set_flags(f|LSF_VALUECONSTANT);
	return s;
	}

li_object *li_set(li_object *o, li_environment *env)
    {
    li_symbol *sym=li_symbol::get(li_eval(li_first(o,env),env),env);
    li_object *value=li_eval(li_second(o,env),env);
    li_set_value(sym,value,env);
    return value;
    }

li_object *li_setf(li_object *o, li_environment *env)
	{
	if ((li_length(o,env)%2)==1)
		{
		li_error(env,"USER: setf: An even number of arguments is required.");
		return li_nil;
		}
	li_object *value=li_nil;
	while (o)
		{
		if (li_car(o,env)->type()==LI_SYMBOL)
			{
			li_symbol *s=li_symbol::get(li_car(o,env),env);  
			o=li_cdr(o,env);
			value=li_eval(li_car(o,env), env);
			if (value&&((value->type()==LI_FUNCTION)||(value->type()==LI_USER_FUNCTION)))
				{//this is for cases such as (setf add3 (adder 3)) when adder 
				//is something like (defun adder (x) (function (lambda (y) (+ x y))))
				//whose result is an object of type function (usually LI_USER_FUNCTION)
				if (env)
					env->set_fun(s, value);
				else 
					s->set_fun(value);  
                //setf/setq always has global effect.
                //s->set_fun(value);
				}
			else
                {
                if (value==0)
                    value=li_nil; //never set a variable to null 
                                  //would be the same as make-unbound
                                  //which is obviously not intended.
			    li_set_value(s, value, env); 
                //s->set_value(value);
                }
			
			}
		else
			{
            //what's this used for???
			li_eval_lvalue(li_car(o,env),
				env,
				value=li_eval(li_car(li_cdr(o,env),env),env));//where the new object is to be stored
			//if (addr==0||addr==li_nil||addr==li_true_sym)
			//	li_error(env,"setf: first argument (CAR %o) doesn't resolve in an address.",o);
			o=li_cdr(o,env);
			//value=li_eval(li_car(o,env),env);//value to be stored at addr.
			//addr=value;//remap the pointer where addr points to.

			//li_get_type(addr->type())->free(addr);//free data associated to object 
			//cannot be garbage-collected, as cell is overwriten.
			//li_object *c=li_get_type(addr->type())->copy(addr);
			
			//li_get_type(addr->type())->free(addr);
			
			//memcpy(addr,value,8);//copy new value to dest
			}
		o=li_cdr(o,env);
		};
	return value;//return the last newvalue
	
	}

//This function is used for the internal quote function 
//that is the '(a b c) form. It CANNOT be used for explicit quotation using (quote a b c)
li_object *li_quote_fun(li_object *o, li_environment *env)
{
  return li_car(o,env);
}

//This is the explicit (quote ... ) form
li_object *li_explicit_quote_fun(li_object *o, li_environment *env)
    {
    return li_car(o,env);
    }

li_object *li_new(li_object *o, li_environment *env)
{
  int type=li_type::get(li_eval(li_car(o,env)),env)->value();
  return li_get_type(type)->create(li_cdr(o,env), env);
}



int li_max_cells=30*1024;

li_object *li_ptr(li_object *o, li_environment *env)
{
  return (li_object *)(li_get_int(li_eval(li_car(o,env), env),env));
}

li_object *li_null_function(li_object *o, li_environment *env)
	{
	return li_nil;
	}

class memory_block_list;
//This is the lisp memory manager.
//It contains the Garbage collector, one of the most sophisticated
//parts of golgotha's code base.
//In parentheses: Remarks for updating to full dynamic memory allocation.
class li_memory_manager_class : public i4_init_class
{
	
	protected:
	
  /*li_free8_list *small_cells,*small_first_free;
  li_list *cells, *cstart, *clast;
  li_list *first_free;
  long li_big_cells,li_small_cells;
  long li_big_cells_free,li_small_cells_free;
  */
  memory_block_list *big_blocks;
  memory_block_list *small_blocks;
  i4_bool active;
  public:
  li_memory_manager_class()
	  {
	  big_blocks=0;
	  small_blocks=0;
	  active=i4_F;
	  };
  protected:
  //(generic)
  void get_stack_range(li_object *&start, li_object *&end)
  {
    void *current_stack_object;
    li_object *current_stack=(li_object *)(&current_stack_object);

    li_list **stack_start=((li_list **)i4_stack_base);

    if ((swptr)stack_start<(swptr)current_stack) 
    { 
      start=(li_object *)stack_start; 
      end=current_stack; 
    }
    else
    { 
      end=(li_object *)stack_start; 
      start=current_stack; 
    }
  }
  memory_block_list *getblockfor(li_object *o);
  //(needs update)
  public:
  i4_bool valid_object(li_object *o)
  {
    
    if (!li_valid_type(o->type()))
      return i4_F;
	if (getblockfor(o))
		return i4_T;
    if (i4_stack_base!=0)
      {
        li_object *s,*e;
        get_stack_range(s,e);
        
        if ((o>=s && o<e))
          return i4_T;
      }

    return i4_F;
  }
  public:
  //(generic)
  int init_type() { return I4_INIT_TYPE_LISP_MEMORY; }
  private:
  //(needs update)
  void mark_stacks(int mark);
  

  // gc : Garbage Collection
  // scans & marks all cells referenced by 
  // symbols, main stack, thread stacks, global_pointers, & helper objects
  // (needs major update)
  // WARN: Don't call this member function directly, use li_gc()
  // instead. Reason: Might otherwise miss object pointers currently
  // located in a register. 
  public:
  int gc();
  //(needs update)
  protected:
  friend void *li_cell_alloc(size_t size);
  li_object *alloc_cell(size_t size);//assumes size is newer >16 (checked by new)

  friend void li_cell_free(void *ptr);
  //(needs update)
  void free_cell(li_list *l);
  public:

  //(minor changes needed)
  void init();


  void uninit();
  

} li_mem_man;

class memory_block_list
	{
	public:

	enum {SMALL=0,BIG=1,UNUSED=2};
		int type; //0 small, 1 big
		li_free8_list *small_cells;
		li_free8_list *small_first_free;
		li_free8_list *small_cells_end;
		swptr num_cells;
		swptr num_cells_free;
		li_list *big_cells;
		li_list *big_first_free;
		li_list *big_cells_end;
		memory_block_list *next;
		li_memory_manager_class *parent; //for calling gc() on it.
		void *memblock; //the exact address obtained from malloc()
		swptr blocksize;
	private:
		memory_block_list()
			{
			li_error(0,"FATAL: Unsynchronized memory management operation");
			};//never generate implicitly
	public:
		memory_block_list(int _type, w32 size, li_memory_manager_class *memman)
			{
			type=_type;
			parent=memman;
			small_cells=0;
			small_first_free=0;
			small_cells_end=0;
			num_cells=0;
			num_cells_free=0;
			big_cells=0;
			big_first_free=0;
			big_cells_end=0;
			next=0;
			memblock=0;
			blocksize=0;
			size=size&(~15);
			I4_ASSERT(size>0,"Block allocator: Use larger sizes");
			memblock=malloc(size);
			if (memblock==0)
				{
				li_error(0,"FATAL: Lisp memory management: Out of Memory to increase pool size.");
				return;
				}
			blocksize=size;
			if (type==BIG)
				{
				if (((swptr)memblock&0xf)==0)
					{
					big_cells=(li_list*)memblock;
					big_cells_end=(li_list*)((swptr)big_cells+blocksize);
					}
				else
					{
					big_cells=(li_list*)memblock;
					//big_cells must be at a multiple of 16, so we increase
					//the block start address to the next that matches this.
					big_cells=(li_list*)(((swptr)big_cells+16)&(~15));
					//we loose one cell because of this.
					big_cells_end=(li_list*)(((swptr)big_cells+blocksize-16));
					}
				num_cells=(swptr)big_cells_end-(swptr)big_cells;
				num_cells/=16;
				num_cells_free=num_cells;
				swptr i;
				for (i=num_cells_free-1;i>0;i--)
					{
					big_cells[i]._data=0;
					big_cells[i]._res=(li_object*)1;
					big_cells[i]._next=0;
					big_cells[i].mark_free();
					big_cells[i].set_next_free(big_cells+i-1);
					}
				big_cells[0]._data=0;
				big_cells[0]._res=(li_object*)1;
				big_cells[0]._next=0;
				big_cells[0].mark_free();
				big_cells[0].set_next_free(0);
				big_first_free=big_cells_end-1;
				}
			else
				{
				//small block addresses must be at a multiple of 8.
				if (((swptr)memblock&0x7)==0)
					{
					small_cells=(li_free8_list*)memblock;
					small_cells_end=(li_free8_list*)((swptr)small_cells+blocksize);
					}
				else
					{
					small_cells=(li_free8_list*)memblock;
					small_cells=(li_free8_list*)(((swptr)small_cells+8)&(~7));
					small_cells_end=(li_free8_list*)(((swptr)small_cells+blocksize-8));
					}
				num_cells=(swptr)small_cells_end-(swptr)small_cells;
				num_cells/=8;
				num_cells_free=num_cells;
				swptr j;
				for (j=num_cells_free-1;j>0;j--)
					{
					small_cells[j].mark_free();
					small_cells[j].set_next_free(small_cells+j-1);
					}
				small_cells[0].mark_free();
				small_cells[0].set_next_free(0);
				small_first_free=small_cells_end-1;
				}
			}
		//since it's highly inprobable that an entire block will become
		//free once, all blocks are cleared at the end of the program.
		~memory_block_list()
			{
			if (next)
				delete next; //recursively delete everything;
			next=0;
			if (big_cells)
				{
				for (int i=0;i<num_cells;i++)
					{
					if (big_cells[i].unmarked_type()!=LI_INVALID_TYPE)
						{
						li_get_type(big_cells[i].unmarked_type())->free(big_cells+i);
						}
					}
				}
			else
				{
				for (int i=0;i<num_cells;i++)
					{
					if (small_cells[i].unmarked_type()!=LI_INVALID_TYPE)
						{
						li_get_type(small_cells[i].unmarked_type())->free(small_cells+i);
						}
					}
				}
			free(memblock);
			memblock=0;
			blocksize=0;
			type=UNUSED;
			}
		
		//(needs update)
		li_object *alloc_cell(size_t size)//assumes size is newer >16 (checked by new)
			{
			int bailout=0;
			if (size==8)
				{
				if (!small_cells)
					return 0; //this block contains large cells
				if (!small_first_free)
					{
						return 0;
					}
				
				
				cell_lock.lock();
				li_free8_list *sret=small_first_free;
				if (sret->type()!=0)
					{
					cell_lock.unlock();
					return 0; //it's probably better to signal that we need a new block
					//li_error(0,"WARNING: Lisp Memory Management: Free Cell list inconsistent. Possible Cause: Very low memory free (%i small cells).",li_small_cells_free);
					//gc();
					//cell_lock.lock();
					//sret=small_first_free;
					}
				small_first_free=small_first_free->get_next_free();
				sret->set_next_free(0);
				num_cells_free--;
				//li_small_cells_free--;
				//There is a problem when there occurs a garbage collection right
                //now because this object is not yet in use and no references to it
                //are existing.
				sret->_type=li_object::GC_FLAG;
				cell_lock.unlock();
				return sret;
				}
			else
				{
				if (!big_cells)
					return 0;
				if (!big_first_free)
					{
						return 0;
					
					}
				
				cell_lock.lock();
				li_list* bret=big_first_free;
				if (bret->type()!=0)
					{
					cell_lock.unlock();
					return 0;
					//li_error(0,"WARNING: Lisp Memory Management: Free Cell list inconsistent. Possible Cause: Very low memory free (%i big cells).",li_big_cells_free);
					//gc();
					//cell_lock.lock();
					//bret=first_free;
					}
				big_first_free=big_first_free->get_next_free();
				//sometimes, this get's garbaged.
				/*if (((first_free>clast)||(first_free<cells))&&first_free)
				{
				li_error(0,"Memory Management failure: returning invalid address");
				}
				*/
				num_cells_free--;
				bret->_data=0;
				bret->_next=0;
				bret->_res=(li_object*)2;
				bret->_type=li_object::GC_FLAG;
				//li_big_cells_free--;
				cell_lock.unlock();
				return bret;
				}
			
			
			}
		
		//(needs update)
		void free_cell(li_list *l)
			{
			cell_lock.lock();
			swptr i;
			
			if (small_cells)
				{
				li_free8_list *l2=(li_free8_list*)l;
				i=l2-small_cells;
				small_cells[i].mark_free();
				small_cells[i].set_next_free(small_first_free);
				small_first_free=small_cells+i;
				num_cells_free++;
				}
			else
				{
				
				i=l-big_cells;
				// add to free_list
				big_cells[i].mark_free();
				big_cells[i]._res=(li_object*)3;
				big_cells[i].set_next_free(big_first_free);
				big_first_free=big_cells+i;
				num_cells_free++;
				}
			cell_lock.unlock();
			}

		void rebuild_free_list()
			{
			num_cells_free=0;
			if (small_cells)
				{
				small_first_free=0;
				for (int i=0;i<num_cells;i++)
					{
					//Hint: remember that "small_cells+i" is the same
					//as "&small_cells[i]"
					if (!small_cells[i].is_marked())
						{
						if (small_cells[i]._type!=LI_INVALID_TYPE)
							{//free this newly gc'ed cell
							li_get_type(small_cells[i]._type)->free(small_cells+i);
							small_cells[i].mark_free();
							}
						small_cells[i].set_next_free(small_first_free);
						small_first_free=small_cells+i;
						num_cells_free++;
						}
					}
				}
			else
				{
				big_first_free=0;
				for (int i=0;i<num_cells;i++)
					{
					//Hint: remember that "small_cells+i" is the same
					//as "&small_cells[i]"
					if (!big_cells[i].is_marked())
						{
						if (big_cells[i]._type!=LI_INVALID_TYPE)
							{//free this newly gc'ed cell
							li_get_type(big_cells[i]._type)->free(big_cells+i);
							big_cells[i]._next=0;
							big_cells[i].mark_free();
							big_cells[i]._res=(li_object*)4;
							}
						big_cells[i].set_next_free(big_first_free);
						big_first_free=big_cells+i;
						num_cells_free++;
						}
					}
				}
			}
		swptr num_free()
			{
			return num_cells_free;
			}
		
	};

void li_memory_manager_class::init()
  {
  //Warning C4127 "Conditional Expression is constant" is normal
  //on the following line.
    if (sizeof(li_list)!=16 || sizeof(li_free8_list)!=8)
		{
        li_error(0, "FATAL: Lisp-Engine memory init: Data size mismatch error, cannot continue.");
	    exit(95);
		}
    strcpy(li_last_file,"None (The error occured inside an internal function).");
    
	//create the first two blocks.
	big_blocks=new memory_block_list(memory_block_list::BIG,100000,this);
	small_blocks=new memory_block_list(memory_block_list::SMALL,100000,this);
	//first part: Static constants
    li_nil=li_get_symbol("nil");        li_set_value(li_nil, li_nil);
	li_nil->set_flags(LSF_FORCECONSTANT|LSF_VALUECONSTANT);

    li_true_sym=li_get_symbol("T");     li_set_value(li_true_sym, li_true_sym);
	li_true_sym->set_flags(LSF_FORCECONSTANT|LSF_VALUECONSTANT);
	li_set_value(li_get_symbol("t"),li_true_sym);
	li_get_symbol("t")->set_flags(LSF_FORCECONSTANT|LSF_VALUECONSTANT);
	li_set_value(li_get_symbol("NIL"),li_nil);
	li_get_symbol("NIL")->set_flags(LSF_FORCECONSTANT|LSF_VALUECONSTANT);
    li_quote=li_get_symbol("'");
	li_quote->set_flags(LSF_FUNCTIONCONSTANT);
    //li_backquote==li_get_symbol("`"); // JJ WHATS THIS ??
    li_backquote=li_get_symbol("`");   //JJ
	li_backquote->set_flags(LSF_FUNCTIONCONSTANT);
    li_comma=li_get_symbol(",");
	li_comma->set_flags(LSF_FUNCTIONCONSTANT);
    li_function_symbol=li_get_symbol("#\'");
	li_function_symbol->set_flags(LSF_FUNCTIONCONSTANT);
	li_vector_symbol=li_get_symbol("#");
	li_vector_symbol->set_flags(LSF_FUNCTIONCONSTANT);
	li_lambda_symbol=li_get_symbol("lambda");
	//no constant for lambda, its value and fun are altered

	//second part: some environment vars
	li_symbol *s=li_get_symbol("lambda-list-keywords");
	li_object *o=li_make_list(
		li_get_symbol("&optional"),
		li_get_symbol("&rest"),
		li_get_symbol("&key"),
		li_get_symbol("&allow-other-keys"),
		li_get_symbol("&aux"), 
		li_get_symbol("&body"), 
		li_get_symbol("&whole"), 
		li_get_symbol("&environment"),
		0);
	li_set_value(s,o);
	s->set_flags(LSF_VALUECONSTANT);
	s=li_get_symbol("lambda-parameters-limit");
	
	li_set_value(s,new li_int(500));//some arbirtrary num >50
	s->set_flags(LSF_VALUECONSTANT);
	s=li_get_symbol("most-positive-fixnum");
	li_set_value(s,new li_int(2147483647));//2^32 -1, 
	s->set_flags(LSF_VALUECONSTANT);
	s=li_get_symbol("most-negative-fixnum");
	li_set_value(s,new li_int((int) -2147483647));
	s->set_flags(LSF_VALUECONSTANT);

	//third part: the core (together with the lisp__functions code)
	li_add_function(li_vector_symbol,li_explicit_make_vector);
    li_add_function("not", li_not);
	li_add_function("null", li_not);//same as not.
    li_add_function("progn", li_progn);
    li_add_function("equal", li_equal);
	li_add_function("eq",li_eq);
	li_add_function("eql",li_eql);
	li_add_function("=",li_equal);
    li_add_function("if", li_if);
    li_add_function("load", li_load);
    li_add_function("setf", li_setf);
	li_get_symbol("setf")->set_flags(LSF_SPECIALFORM);
	li_add_function("setq", li_setf); //Abuse lisps use this one
	li_get_symbol("setq")->set_flags(LSF_SPECIALFORM);
    li_add_function("set",li_set);
	li_add_function("cdr",li_cdr_operator);
	li_add_function("car",li_car_operator);
	li_add_function("cons",li_cons_operator);
	li_add_function("consp",li_consp);
	li_add_function("list",li_explicit_list_constructor);
	li_add_function("list*",li_list_star);
    li_add_function("sys-print", li_print);
	li_add_function("print",li_user_mode_print);
    li_add_function(li_quote, li_quote_fun);
	li_quote->set_flags(LSF_SPECIALFORM&LSF_FUNCTIONCONSTANT);
	li_add_function(li_backquote,backquote);
	li_backquote->set_flags(LSF_SPECIALFORM&LSF_FUNCTIONCONSTANT);
	li_add_function(li_comma, li_comma_error);
	li_add_function(li_function_symbol,li_function);
	//li_add_function(li_function_symbol,li_quote_fun);
	li_function_symbol->set_flags(LSF_SPECIALFORM);
	li_add_function("function",li_function);
	//li_add_function("function",li_quote_fun);
	li_add_function("symbol-function",li_function);
	li_add_function("symbol-value",li_symbol_value);
	li_get_symbol("function")->set_flags(LSF_SPECIALFORM&LSF_FUNCTIONCONSTANT);
    //due to an implementation speciality of golg, this is different from the ' function.
	li_add_function("quote",li_explicit_quote_fun);
	li_get_symbol("quote")->set_flags(LSF_SPECIALFORM&LSF_FUNCTIONCONSTANT);
    li_add_function("new", li_new);
    li_add_function("read-eval", li_read_eval);
    li_add_function("ptr", li_ptr);
	li_add_function("perm-space",li_perm_space);
	li_add_function("tmp-space",li_tmp_space);
	li_add_function("length",li_length_for_lisp);
	li_add_function("first",li_first_operator);
	li_add_function("second",li_second_operator);
	li_add_function("third",li_third_operator);
	li_add_function("fourth",li_fourth_operator);
    li_add_function("fifth",li_fifth_operator);
	li_add_function("sixth",li_sixth_operator);
	li_add_function("seventh",li_seventh_operator);
	li_add_function("eighth",li_eighth_operator);
	li_add_function("ninth",li_ninth_operator);
	li_add_function("tenth",li_tenth_operator);
	li_add_function("nth",li_nth_operator);
	li_add_function("acons",li_acons);
	li_add_function("assoc",li_assoc); //see function
	li_add_function("symbol-list",li_null_function);
	li_add_function("pairlis",li_pairlis);
	li_add_function("atom",li_atom);
	li_add_function("defun",li_defun_operator);
	li_add_function("defmacro",li_defmacro_operator);
	li_add_function("reverse",li_reverse);
	li_add_function("push",li_push_operator);
	li_add_function("pop",li_pop_operator);
	li_add_function("li-sys-info",li_sys_info);
	li_add_function("let",li_let);
	li_add_function("let*",li_let_star);
	li_add_function("backquote",backquote);
	li_add_function("defvar",li_defvar);
	li_get_symbol("defvar")->set_flags(LSF_SPECIALFORM);
	li_add_function("defconstant",li_defconstant);
	li_get_symbol("defconstant")->set_flags(LSF_SPECIALFORM);
	li_add_function("aref",li_aref);
	li_add_function("lisp-implementation-version",li_implementation_version);
	li_add_function("documentation",li_documentation);
	li_add_function("symbol-info",li_symbol_info);
	li_add_function("intern",li_intern);
	li_add_function("nthcdr",li_nthcdr);
	li_add_function("append",li_append);
	li_add_function("prog1",li_prog1);
	li_add_function("makunbound",li_makunbound);
	li_add_function("fmakunbound",li_fmakunbound);
	li_add_function("member",li_member_access);	
	li_add_function("put",li_put);
	li_add_function("get",li_get);
	li_add_function("symbol-plist",li_symbol_plist);
	li_add_function("setplist",li_setplist);
    li_add_function("make-local-variable",li_make_local_variable);
    li_add_function("while",li_while);
    li_add_function("memq",li_memq);
    active=i4_T;
	
  }

int li_memory_manager_class::gc()
{
    if (active==i4_F)
		return 0; //The Lisp engine is not ready, cannot do anything.
    int t_free=0;

    i4_warning("Starting Garbage collector run.");
    if (i4_get_thread_id()!=i4_get_main_thread_id())
    {
      // if this is called from a thread stop and let main program do gc()
      threads_need_gc=1;
      while (threads_need_gc)
        i4_thread_yield();

      cell_lock.lock();
	  /*int i=0;
      for (; i<li_big_cells; i++)
      {
        if (cells[i]._type==LI_INVALID_TYPE)
          t_free++;
      }
	  for (i=0;i<li_small_cells; i++)
		  {
		  if (small_cells[i]._type==LI_INVALID_TYPE)
			  t_free++;
		  }
	  */
	  memory_block_list *curr=big_blocks;
	  while (curr!=0)
		  {
		  t_free+=curr->num_free();
		  curr=curr->next;
		  }
	  curr=small_blocks;
	  while (curr!=0)
		  {
		  t_free+=curr->num_free();
		  curr=curr->next;
		  }
      cell_lock.unlock();
    }
    else
    {
      li_object_pointer *pl;
      int i;

      if (!i4_stack_base)
        i4_error("FATAL: Lisp Garbage Collector internal error: Cannot locate top of stack.");
      
      pf_li_gc.start();

	  
      cell_lock.lock();//calls in this order make sure that no thread currently has the lock
	  i4_suspend_other_threads();
      
      mark_stacks(1);

      li_mark_symbols(1);

      if (li_hash.is_active())     // if the system has shut down, don't mark type's objects
      {
        for (i=1; i<li_max_types(); i++)
        {
          li_type_function_table *t=li_get_type(i);
          if (t)
            t->mark(1);
        }
      }

      li_gc_object_marker_class *helpers;
      for (helpers=gc_helpers; helpers; helpers=helpers->next)
        helpers->mark_objects(1);

      for (pl=li_object_pointer_list; pl; pl=pl->next)
        if (pl->o && !pl->o->is_marked())
          li_get_type(pl->o->type())->mark(pl->o, 1);

	  memory_block_list *curr=big_blocks;
	  while (curr)
		  {
		  curr->rebuild_free_list();
		  t_free+=curr->num_free();
		  curr=curr->next;
		  }
	  curr=small_blocks;
	  while (curr)
		  {
		  curr->rebuild_free_list();
		  t_free+=curr->num_free();
		  curr=curr->next;
		  }
		/*
      first_free=0;
	  li_big_cells_free=0;
	  //order them from top to bottom, to allow the memory to be dynamically resized
      for (i=li_big_cells-1; i>=0; i--)//if first_free==0, mem is full
      {
        if (!cells[i].is_marked())
        {
          if (cells[i]._type!=LI_INVALID_TYPE)
          {
            li_get_type(cells[i].type())->free(cells+i);
            cells[i].mark_free();
            cells[i]._type=LI_INVALID_TYPE;
          }


          // add to free_list
          cells[i].set_next_free(first_free);
		  li_big_cells_free++;
          first_free=cells+i;
          t_free++;
        }
      }


	  
	  small_first_free=0;
	  li_small_cells_free=0;
      for (i=0; i<li_small_cells; i++)
      {
        if (!small_cells[i].is_marked())
        {
          if (small_cells[i]._type!=LI_INVALID_TYPE)
          {
            li_get_type(small_cells[i]._type)->free(small_cells+i);
            small_cells[i].mark_free();
            small_cells[i]._type=LI_INVALID_TYPE;
          }


          // add to free_list
          small_cells[i].set_next_free(small_first_free);
          small_first_free=small_cells+i;
		  li_small_cells_free++;
          t_free++;
        }
      }
	  */
      // unmark the stacks
      mark_stacks(0);

      // unmark symbols
      li_mark_symbols(0);

      if (li_hash.is_active())
      {
        for (i=1; i<li_max_types(); i++)
        {
          li_type_function_table *t=li_get_type(i);
          if (t)
            t->mark(0);
        }
      }


      for (helpers=gc_helpers; helpers; helpers=helpers->next)
        helpers->mark_objects(0);

      for (pl=li_object_pointer_list; pl; pl=pl->next)
        if (pl->o && pl->o->is_marked())
          li_get_type(pl->o->unmarked_type())->mark(pl->o, 0);

#if 0
	  curr=big_blocks;
	  while (curr)
		  {
		  for (int k=0;k<curr->num_cells;k++)
			  {
			  if (curr->big_cells[k].is_marked()&&(curr->big_cells[k].unmarked_type()!=0))
				  {
				  i4_error("SEVERE: Inconsitent garbage collection.");
				  }
			  }
		  
		  curr=curr->next;
		  }
	  curr=small_blocks;
	  while (curr)
		  {
		  for (int l=0;l<curr->num_cells;l++)
			  {
			  if (curr->small_cells[l].is_marked()&&(curr->small_cells[l].unmarked_type()!=0))
				  {
				  i4_error("SEVERE: Small memory blocks inconsistent.");
				  }
			  }
		  curr=curr->next;
		  }
#endif
      cell_lock.unlock();
      threads_need_gc=0;
      i4_resume_other_threads();
      pf_li_gc.stop();
    }


    return t_free;
}

void li_memory_manager_class::free_cell(li_list *l)
  {
    memory_block_list *li=getblockfor(l);
	li->free_cell(l);
  };



li_object *li_memory_manager_class::alloc_cell(size_t size)
	{
	if (size<=8)
		{
		memory_block_list *curr=small_blocks;
		li_object *sret=0;
		while (curr)
			{
			sret=curr->alloc_cell(size);
			if (sret)
				return sret;
			curr=curr->next;
			}
		
		//we looped through all active blocks and no free cell was found
		if (li_gc())
			{
			curr=small_blocks;
			while (curr)
				{
				sret=curr->alloc_cell(size);
				if (sret)
					return sret;
				curr=curr->next;
				}
			}
		//still no luck? Need more cells.
		cell_lock.lock();//need to update the block list
		curr=new memory_block_list(memory_block_list::SMALL,50000,this);
		curr->next=small_blocks;
		small_blocks=curr;
		cell_lock.unlock();
		return curr->alloc_cell(size); //*must* work now.
		}
	else
		{
		memory_block_list *curr=big_blocks;
		li_object *bret=0;
		while (curr)
			{
			bret=curr->alloc_cell(size);
			if (bret)
				return bret;
			curr=curr->next;
			}
		if (li_gc())
			{
			curr=big_blocks;
			while(curr)
				{
				bret=curr->alloc_cell(size);
				if (bret)
					return bret;
				curr=curr->next;
				}
			}
		cell_lock.lock();
		curr=new memory_block_list(memory_block_list::BIG,50000,this);
		curr->next=big_blocks;
		big_blocks=curr;
		cell_lock.unlock();
		return curr->alloc_cell(size);
		}
	};

void li_memory_manager_class::uninit()
	{
    //li_root=0;//We force dangling references. This is fun :-P
    li_hash.uninit();

    // clear all pointer references
    for (li_object_pointer *pl=li_object_pointer_list; pl; pl=pl->next)
      pl->o=0;

    int t_free=li_gc();
	delete big_blocks;
	delete small_blocks;
	big_blocks=0;
	small_blocks=0;
	active=i4_F;

/*
    if (t_free!=li_max_cells)
    {
      i4_warning("li_cleanup : possibly %d items still referenced. Forcing cleanup.", 
                 (li_big_cells+li_small_cells)-t_free);
	  int i;
      for (i=0; i<li_big_cells; i++)
		  {
		  if (cells[i]._type!=LI_INVALID_TYPE)
			  li_get_type(cells[i]._type)->free(cells+i);
		  cells[i].mark_free();
		  }
	  for (i=0; i<li_small_cells; i++)
		  {
		  if (small_cells[i]._type!=LI_INVALID_TYPE)
			  li_get_type(small_cells[i]._type)->free(small_cells+i);
		  small_cells[i].mark_free();
		  }
    }
	*/

    // delete all types
    for (int t=0; t<li_max_types(); t++)
      if (t==0 || li_valid_type(t))//0 is LI_INVALID_TYPE, but here it is valid, too.
        li_remove_type(t);

    li_cleanup_types();
	/*
	li_small_cells=0;
	li_big_cells=0;
	li_big_cells_free=0;
	li_small_cells_free=0;
	small_cells=0;
	cells=0;
    i4_free(cstart);
	cstart=0;
	clast=0;
	first_free=0;
	small_first_free=0;
	*/
	
}

	memory_block_list *li_memory_manager_class::getblockfor(li_object *o)
	  {
	  memory_block_list *curr=big_blocks;
	  while (curr!=0)
		  {
		  if ((o>=curr->big_cells) && (o<(curr->big_cells_end))
			  && (((wptr)o & 0x0F)==0))
			  {
			     return curr;
			  }
		  curr=curr->next;
		  }
	  curr=small_blocks;
	  while (curr!=0)
		  {
		  if ((o>=curr->small_cells) && (o<(curr->small_cells_end))
			  && (((wptr)o & 0x7)==0))
			  {
			     return curr;
			  }
		  curr=curr->next;
		  }
	  return 0;
	  }

	void li_memory_manager_class::mark_stacks(int mark)
  {
    int id=i4_get_first_thread_id();
    do
    {
      void *base, *top;
      i4_get_thread_stack(id, base,top);
	  if (base>top)
		  {
		  void *tmp=0;
		  tmp=base;
		  base=top;
		  top=tmp;
		  }
	  //li_free8_list *small_cell_helper=small_cells+li_small_cells;//to force the compiller to use 8Byte sizes
      //    li_mark_memory_region((li_list **)base,(li_list **)top,     //c2 is exclusive                          
      //                        (li_list*)small_cells, (li_list*)small_cell_helper, mark);
	  //	  li_mark_memory_region((li_list **)base,(li_list **)top,
	  //		  cells,cells+li_big_cells,mark,15);
	  memory_block_list *curr=big_blocks;
	  while (curr!=0)
		  {
		  li_mark_memory_region((li_list**)base,(li_list**)top,
			  curr->big_cells,curr->big_cells_end,mark,15);
		  curr=curr->next;
		  }
	  curr=small_blocks;
	  while (curr!=0)
		  {
		  li_mark_memory_region((li_list**)base,(li_list**)top,
			  (li_list*)curr->small_cells,
			  (li_list*)curr->small_cells_end,
			  mark);
		  curr=curr->next;
		  }
    } while (i4_get_next_thread_id(id, id));
  }
void *li_cell_alloc(size_t size)
{
  if (threads_need_gc && i4_get_thread_id()==i4_get_main_thread_id())
   li_gc();
  
  
  void *r= li_mem_man.alloc_cell(size);
  //if (((li_object*)r)->type()!=0)
  //	  i4_error("STRANGE: This object is already in use.");
  
  //*((w32*)r)=li_object::GC_FLAG;
  return r;
}


void li_cell_free(void *ptr)
{
  li_mem_man.free_cell((li_list *)ptr);
}

int li_gc()
{
  jmp_buf env;       // save all registers on the stack
  setjmp(env);


  return li_mem_man.gc();
}


i4_bool li_valid_object(li_object *o)
{
  return li_mem_man.valid_object(o);
}

// lisp/li_class.cpp
/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/


static li_type_edit_class *li_class_editor=0;

void li_set_class_editor(li_type_edit_class *editor)
{
  li_class_editor=editor;
}


////////////////////////////// li_class_type members ///////////////////////////////////////////////

li_class *li_this;

class li_class_type : public li_type_function_table
{
public:

  struct var
  {
    li_object  *default_value;
    li_object  *property_list;
    li_symbol  *sym;
    int        original_order;

    void init()
    {
      sym=0;
      default_value=0;
      property_list=0;
      original_order=0;
    }
  };

  static int var_compare(const var *a, const var *b);

  i4_fixed_array<var> vars;

  int old_tvars;
  sw16 *value_remap;     // used during loading of a li_class

  li_class_type *derived_from;
  li_symbol *sym;
  var *get_var(li_symbol *sym);

  //PG: moved the following to the base class, to be able to safely
  //check wheter a given object is of class type.
  //int type; 

  static li_class_type *get(li_type_function_table *o, li_environment *env)
  { 
    li_class_type *c=(li_class_type *)o;
#ifdef LI_TYPE_CHECK
    if (c!=li_get_type(c->type))
      li_error(env, "INTERNAL: function table does not point to a class");
#endif      
    return c;
  }

  li_object *create(li_object *params, li_environment *env);

  void mark(int set);
  void mark(li_object   *o, int set);
  void free(li_object   *o);
  void print(li_object  *o, i4_file_class *stream);
  char *name();

  li_class_type(li_symbol *sym, li_class_type *derived_from)  
    : sym(sym), derived_from(derived_from)
  {
    value_remap=0;
  }


  int get_var_offset(li_symbol *sym, int die_on_error)
  {
    w32 r=vars.size();

    if (!r) return 0;
    int l=0,m;
    li_symbol *s1;//,*s2; JJ not in use

    while (l<(int)r) // JJ cast
    {
      m = (l+r)/2;
      s1=vars[m].sym;

      if (s1==sym) 
        return m;

      if (sym<s1)
        r = m;
      else
        l = m+1;
    }

    if (l==(int)r || vars[l].sym!=sym) //JJ cast
      if (die_on_error)
        li_error(0, "USER: variable not in class %O", sym);
      else return -1;

    return l;
  }

  ~li_class_type()
  {
    vars.uninit();
  }

  // these load and save type information
  virtual void save(i4_saver_class *fp, li_environment *env);
  virtual void load(i4_loader_class *fp, li_type_number *type_remap,
                    li_environment *env);
                    
  virtual void load_done();

  // load & save type instance information
  virtual void save_object(i4_saver_class *fp, li_object *o, li_environment *env);
  virtual li_object *load_object(i4_loader_class *fp, li_type_number *type_remap,
                                 li_environment *env);

};


struct sym_var
{
  li_class_type::var *var;
  li_object     *value;
};



char *li_class_type::name() 
{ 
  if (sym)
    return sym->name()->value();
  else
    return "anonymous-class"; 
}


void li_class_type::mark(int set)
{
  for (int i=0; i<vars.size(); i++)
  {
    if (vars[i].default_value)
      if (vars[i].default_value->is_marked()!=set)
        li_get_type(vars[i].default_value->unmarked_type())->mark(vars[i].default_value, set);

    if (vars[i].property_list)
      if (vars[i].property_list->is_marked()!=set)
        li_get_type(vars[i].property_list->unmarked_type())->mark(vars[i].property_list, set);
  }     
}

void li_class::mark(int set)
{
  if (!set)
    li_object::mark(set);

  li_class_type *t=get_type();
  for (int i=0; i<t->vars.size(); i++)
  {   
    int type=t->vars[i].default_value->unmarked_type();

    li_object *o=object_value(i);
    // int's and - floats no more - are stored directly and don't need marking
    if (type!=LI_INT && type!=LI_FLOAT && o->is_marked()!=set)
      li_get_type(o->unmarked_type())->mark(o, set);
  }      

  if (set)
    li_object::mark(set);
}

li_class_type::var *li_class_type::get_var(li_symbol *sym)
{
  for (int i=0; i<vars.size(); i++)
    if (vars[i].sym==sym) 
      return &vars[i];

  if (derived_from)
    return derived_from->get_var(sym);

  return 0;
}

void li_class_type::mark(li_object   *o, int set)   
{  
  ((li_class *)o)->mark(set);
}

void li_class_type::free(li_object   *o) 
{ 
  li_class::get(o,0)->free(); 
}

void li_class_type::print(li_object  *o, i4_file_class *stream) 
{ 
  li_class::get(o,0)->print(stream); 
}

li_object *li_class_type::create(li_object *params, li_environment *env)
{
  return new li_class(type, params, env);
}


// these load and save type information
void li_class_type::save(i4_saver_class *fp, li_environment *env)
{
  fp->write_32(vars.size());
  for (int i=0; i<vars.size(); i++)
    li_save_object(fp,vars[i].sym, env);
}


void li_class_type::load_done()
{
  if (value_remap)
  {
    i4_free(value_remap);
    value_remap=0;
  }
}

void li_class_type::load(i4_loader_class *fp, li_type_number *type_remap,
                         li_environment *env)
{
  old_tvars=fp->read_32();
  if (old_tvars)
  {
    value_remap=(sw16 *)I4_MALLOC(sizeof(sw16) * old_tvars, ""); 
    for (int j=0; j<old_tvars; j++)
      value_remap[j]=-1;

    for (int i=0; i<old_tvars; i++)
    {
      li_symbol *old_sym=li_symbol::get(li_load_object(fp, type_remap,env), env);
      for (int j=0; j<vars.size(); j++)
        if (old_sym==vars[j].sym)
          value_remap[i]=j;
    } 
  }
}



void li_class::save(i4_saver_class *fp, li_environment *env)
{
  li_class_type *ct=get_type();

  int t_vars=ct->vars.size();
  for (int i=0; i<t_vars; i++)
  {
    li_object *def=ct->vars[i].default_value;
    li_object *v=value(i);

    if (li_get_type(def->type())->equal(def, v))
      li_save_object(fp, 0, env);
    else
      li_save_object(fp, value(i), env);
  }
}

  // load & save type instance information
void li_class_type::save_object(i4_saver_class *fp, li_object *o, li_environment *env)
{
  li_class::get(o,env)->save(fp, env);
}

void li_class::load(i4_loader_class *fp, li_type_number *type_remap, li_environment *env)
{
  li_class_type *ct=get_type();
  int old_tvars=ct->old_tvars;
  sw16 *value_remap=ct->value_remap;

  for (int i=0; i<old_tvars; i++)
  {
    li_object *o=li_load_object(fp, type_remap, env);
    int remap=value_remap[i];
    if (remap!=-1)
    {
      li_object *def=ct->vars[remap].default_value;
         
      // if type has changed use default value      
      if ( (def && o) && o->type()==def->type())
        set_value(remap, o);
    }
  }
}

li_object *li_class_type::load_object(i4_loader_class *fp, li_type_number *type_remap, 
                                      li_environment *env)
{
  li_class *c=new li_class(type);
  c->load(fp, type_remap, env);
  return c;
}


//////////////////////////////////// li_class members /////////////////////////////////

li_class::li_class(li_type_number class_type,
                   li_object *params,
                   li_environment *env)
  : li_object(class_type)
{
  li_class_type *ct=get_type();
  int t_vars=ct->vars.size();

  values=(void **)I4_MALLOC(sizeof(void *) * t_vars, "");


  int i;
  for (i=0; i<t_vars; i++)
    set_value(i, ct->vars[i].default_value);



  i=0;
  while (params)
  {
    li_object *val=li_eval(li_car(params,env));
    

    for (int j=0; j<t_vars; j++)
      if (ct->vars[j].original_order==i)
      {
        set_value(j, val);
        j=t_vars;
      }

    params=li_cdr(params,env);
    i++;
  }
  

}


void li_class::print(i4_file_class *fp)
{        
  fp->write("#inst-",6);

  li_class_type *c=get_type();

  char *name=c->name();
  fp->write(name,strlen(name));

  fp->write_8('<');

  for (int i=0; i<c->vars.size(); i++)
  {
    li_symbol *sym=c->vars[i].sym;

    fp->write(" (",2);
    li_get_type(LI_SYMBOL)->print(sym, fp);
    fp->write_8(' ');

    li_object *v=value(i);
    li_get_type(v->type())->print(v, fp);

    fp->write_8(')');
  }

  fp->write_8('>');

}

void li_class::free()
{
  i4_free(values);
}

//Not inlined because of dependency problems
int li_class::member_offset(char *sym) const
{
  return get_type()->get_var_offset(li_get_symbol(sym), 0);
}

int li_class::member_offset(const char *sym) const
	{
	return get_type()->get_var_offset(li_get_symbol(sym),0);
	}

int li_class::member_offset(li_symbol *sym) const
{
  return get_type()->get_var_offset(sym, 0);
}



int li_class::get_offset(li_class_member &c, li_type_number _type) const
{
  li_class_type *ct=get_type();

  if (!c.sym)
    c.sym=li_get_symbol(c.name);
  
  c.class_type=type();
  c.offset=ct->get_var_offset(c.sym, 1);

  if (c.offset==-1)
    li_error(0, "USER: class %s does not have a member %s", ct->name(), c.name);

#ifdef LI_TYPE_CHECK
  if (ct->vars[c.offset].default_value->type()!=_type)
    li_error(0, "USER: class member %O is wrong type (%s should be %s)", 
             c.sym,
             li_get_type(_type)->name(),
             li_get_type(ct->vars[c.offset].default_value->type())->name());  
#endif


  return c.offset;
}

int li_class::get_offset(const li_class_member &c, li_type_number _type) const
{
  li_class_type *ct=get_type();
  //cannot cache anything on const objects, but
  //they are usually created on the fly anyways.

  return ct->get_var_offset(li_get_symbol(c.name),1);

}


int li_class::get_offset(li_class_member &c) const
{
  li_class_type *ct=get_type();

  if (!c.sym)
    c.sym=li_get_symbol(c.name);
  
  c.class_type=type();
  c.offset=ct->get_var_offset(c.sym, 0);

  return c.offset;
}

int li_class::get_offset(const li_class_member &c) const
{
  li_class_type *ct=get_type();

  return ct->get_var_offset(li_get_symbol(c.name), 1);
}

//These are inlined on release builds. 
#ifdef _DEBUG
li_class *li_class::get(li_object *o, li_environment *env)
{ 
  check_type(o, ((li_class_type *)li_get_type(o->type()))->type, env);   
  return ((li_class *)o); 
}


li_class *li_class::get_all(li_object *o, li_environment *env)
	{
	if (li_get_type(o->type())->type!=0)
		return (li_class *)o;
	return 0;
	}
#endif

li_object *li_class::value(int member)
{
  switch (get_type()->vars[member].default_value->type())
  {
    case LI_INT : return new li_int(int_value(member)); break;
    case LI_FLOAT : return new li_float(float_value(member)); break;
    default : return object_value(member); break;
  }
}

li_object *li_class::value(char *member_name)
{
  return value(member_offset(member_name));
}

li_object *li_class::value(const char *member_name)
	{
	return value(member_offset(member_name));
	};


void li_class::set_value(int member, li_object *value)
{    
  li_class_type *ct=get_type();
  li_object *def_value=ct->vars[member].default_value;

  int t=def_value->type();
  switch (t) 
  {
    case LI_INT : int_value(member) = li_int::get(value,0)->value(); break;
    case LI_FLOAT : float_value(member) = (float) li_float::get(value,0)->value(); break;
    default : object_value(member)=value;
  }
}



///////////////////////////////////// li_def_class ///////////////////////////////////////////

li_object *li_def_class(li_object *fields, li_environment *env)
{
  li_symbol *sym=li_symbol::get(li_car(fields,env),env);  fields=li_cdr(fields,env);
  li_object *derived=li_eval(li_car(fields,env), env); fields=li_cdr(fields,env);
  li_class_type  *d=0;
  int derived_type=0;
  
  if (derived!=li_nil) 
  {
    derived_type=li_type::get(derived,env)->value();
    if (derived_type)
    {   
      d=(li_class_type *)li_get_type(derived_type);
      if (d->type!=derived_type)
		  {
		  li_error(env, "USER: cannot derive a class from '%O', base must be a class.", derived);
		  return 0;
		  }
    }
    else 
		{
		li_error(env, "USER: no such type '%O' to derive from.", derived);
		return 0;
		}
  }

  li_class_type *me=new li_class_type(sym, d);

  li_object *c;
  int t_vars=0;

  // how many variables in the parent class
  if (derived_type)
    t_vars+=li_class_total_members(derived_type);  
  
  for (c=fields; c; c=li_cdr(c,env))      // count how many variables were added
    t_vars++;

  me->vars.resize(t_vars);

  t_vars=0;

  if (derived_type)
  {
    int t_from_derived_class=li_class_total_members(derived_type);
    for (int i=0; i<t_from_derived_class; i++)
    {
      me->vars[t_vars].init();
      me->vars[t_vars].original_order=t_vars;
      li_symbol *s=li_class_get_symbol(derived_type, i);
      me->vars[t_vars].sym=s;
      me->vars[t_vars].default_value=li_class_get_default(derived_type, s);
      me->vars[t_vars].property_list=li_class_get_property_list(derived_type, s);
      t_vars++;
    }
  }
    

  for (c=fields; c; c=li_cdr(c,env))
  {
    li_object *var=li_car(c,env);
    me->vars[t_vars].init();
    me->vars[t_vars].original_order=t_vars;
    
    
    me->vars[t_vars].sym=li_symbol::get(li_car(var,env),env);  var=li_cdr(var,env);
    
    if (var)
    {
      me->vars[t_vars].default_value=li_eval(li_car(var,env), env);  var=li_cdr(var,env);      

      li_symbol *s=me->vars[t_vars].sym;
      li_object *d=me->vars[t_vars].default_value;


      if (var)
        me->vars[t_vars].property_list=li_eval(li_car(var,env), env);
    }

    t_vars++;
  }
    
  me->vars.sort(li_class_type::var_compare);
  me->editor=li_class_editor;
  ///type must match exactly for parameters passed by reference.
  li_type_function_table *me_p=me;
  int newtype=li_add_type(me_p);
  //if type already existed,
  //the type class is deleted and me is set to zero. 
  // (and setting me->type to something is forbidden)
  if (me_p)
	  {
	  me->type=newtype;
	  }

  return new li_type(newtype);
}

li_object *li_class::set(char *member_name, li_object *value) // slow, but easy way to access data
{
  int off=member_offset(member_name);
  if (off==-1) 
    li_error(0, "USER: class %O does not have member %s", member_name);
  set_value(off, value);
  return value;
}


int li_class_type::var_compare(const var *a, const var *b)
{
  if (a->sym<b->sym)
    return -1;
  else if (a->sym>b->sym)
    return 1;
  else return 0;
}


int li_class_total_members(li_type_number type)
{
  return li_class_type::get(li_get_type(type),0)->vars.size();
}

li_symbol *li_class_get_symbol(li_type_number type, int member_number)
{
  li_class_type *ct=li_class_type::get(li_get_type(type),0);
  return ct->vars[member_number].sym;
}



li_object *li_class_get_default(li_type_number type, li_symbol *sym)
{
  li_class_type *ct=li_class_type::get(li_get_type(type),0);
  return ct->vars[ct->get_var_offset(sym, 1)].default_value;
}

li_object *li_class_get_property_list(li_type_number type, li_symbol *sym)
{
  li_class_type *ct=li_class_type::get(li_get_type(type),0);
  return ct->vars[ct->get_var_offset(sym, 1)].property_list;
}

li_object *li_setm(li_object *o, li_environment *env)
{
  li_class *c=li_class::get(li_eval(li_first(o,env),env),env);
  li_symbol *member=li_symbol::get(li_second(o,env),env);
  li_object *value=li_eval(li_third(o,env), env);
  c->set_value(c->member_offset(member), value);
  return value;
}

li_object *li_getm(li_object *o, li_environment *env)
{
  li_class *c=li_class::get(li_eval(li_first(o,env),env),env);
  return c->value(c->member_offset(li_symbol::get(li_second(o,env),env)));  
}



li_automatic_add_function(li_def_class, "def_class");
li_automatic_add_function(li_setm, "setm");
li_automatic_add_function(li_getm, "getm");

// lisp/li_dialog.cpp

/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/



static li_symbol_ref li_list_box("list_box");

class li_generic_edit_class : public li_type_edit_class
{
public:
  enum input_type { TEXT_INPUT,
                    LIST_BOX,
					CHECK_BOX};

  input_type get_type(li_object *value, li_object *property_list, li_environment *env)
  {
    if (property_list && property_list->type()==LI_LIST &&
        li_list_box.get()==li_car(property_list,env))
      return LIST_BOX;
    else if ((value->type()==LI_SYMBOL)&& 
		(li_symbol::get(value,env)->value()==li_nil || 
		li_symbol::get(value,env)->value()==li_true_sym))
		return CHECK_BOX;
	else
      return TEXT_INPUT;
  }
  

  virtual int create_edit_controls(const i4_const_str &name,
                                   li_object *o, 
                                   li_object *property_list,
                                   i4_window_class **windows, 
                                   int max_windows,
                                   li_environment *env)
  {
    if (max_windows<2) return 0;
    
    char buf[300];
    i4_ram_file_class rf(buf, 260);
    li_get_type(o->type())->print(o, &rf);
    buf[rf.tell()]=0;

    i4_graphical_style_class *style=i4_current_app->get_style();

    windows[0]=new i4_text_window_class(name, style);
	input_type ctype=get_type(o,property_list,env);
    if (ctype==LIST_BOX)
    {
      property_list=li_cdr(property_list, env);
    
      i4_list_box_class *lb=new i4_list_box_class(260, style, 
                                                  i4_current_app->get_window_manager());

      int on=0;
      for (;property_list; property_list=li_cdr(property_list,env), on++)
      {
        char buf[100];
        i4_ram_file_class rf(buf, 100);
        li_object *v=li_car(property_list,env);


        li_get_type(v->type())->print(v, &rf);
        buf[rf.tell()]=0;

        lb->add_item(new i4_text_item_class(buf, style)); 

        if (v->type()==o->type() && li_get_type(v->type())->equal(o,v))
          lb->set_current_item(on);
       

      }
      windows[1]=lb;   
    }    
    else if(ctype==CHECK_BOX)
		{
		i4_checkbox_class *box=new i4_checkbox_class("",
			i4_checkbox_class::CHECKBOX,
			style);
		box->set_state(li_get_bool(o,env)?i4_checkbox_class::CHECKED:i4_checkbox_class::UNCHECKED);

		windows[1]=box;
		}
	else
    {
      if (o->type()==LI_STRING)
      {
	    if (strlen(buf)==0)
			{
			buf[1]=0;
			windows[1]=new i4_text_input_class(style,buf+1,200,300);
			}
		else
			{
			buf[strlen(buf)-1]=0;   // chop of end quote
			windows[1]=new i4_text_input_class(style,buf+1, 200,300);
			}
      }
      else
        windows[1]=new i4_text_input_class(style,buf, 200,300);
    }
                                       
    return 2;
  }

  i4_bool can_apply_edit_controls(li_object *o, 
                                          li_object *property_list,
                                          i4_window_class **windows,
										  li_environment *env)
  {
    if (get_type(o,property_list, env)==LIST_BOX)
		return i4_T;
	if (get_type(o,property_list,env)==CHECK_BOX)
		return i4_T;
    i4_bool ok=i4_T;
	//the following assignment doesn't always hold, but we won't access 
	//it, if the type doesn't fit.
    i4_text_input_class *w=((i4_text_input_class *)windows[1]);
    //i4_const_str::iterator i=w->get_edit_string()->begin();
    if (o->type()==LI_INT)
		{
		try 
			{
			i4_const_str::iterator i=w->get_edit_string()->begin();
			i.read_number(i4_T);
			}
		catch(...)
			{
			i4_message_box("Number format error","The value you entered seems not to be a natural number",MSG_OK);
			ok=i4_F;
			}
		}
      else if (o->type()==LI_FLOAT)
		  {
		  try{
			  i4_const_str::iterator f=w->get_edit_string()->begin();
			  f.read_float(i4_T);
			  }
		  catch (...)
			  {
			  i4_message_box("Number format error","You didn't enter a valid floating point number",MSG_OK);
			  ok=i4_F;
			  }
		  }
	  else if (o->type()==LI_SYMBOL)
		  {//this test is only valid for _some_ cases. but which?
		  //uh, oh: this is for symbols, not strings...
			  if (w->get_edit_string()->length()==0)
				  {
				  i4_message_box("Invalid input","You entered a zero-length name for an identifier.",MSG_OK);
				  ok=i4_F;
				  }
		  }

    return ok; 
  
  }

  virtual li_object *apply_edit_controls(li_object *o, 
                                         li_object *property_list,
                                         i4_window_class **windows,
                                         li_environment *env)
  {
    if (get_type(o,property_list, env)==LIST_BOX)
    {
      i4_list_box_class *ib=((i4_list_box_class *)windows[1]);
      return li_nth(property_list, ib->get_current()+1, env);
    } 
    else if (get_type(o,property_list,env)==CHECK_BOX)
		{
		i4_checkbox_class *cb=((i4_checkbox_class *)windows[1]);
		return ((cb->get_state()==i4_checkbox_class::CHECKED)?li_true_sym:li_nil);
		}
	else if (get_type(o,property_list, env)==TEXT_INPUT)
    {
      i4_text_input_class *w=((i4_text_input_class *)windows[1]);
      
      i4_const_str::iterator i=w->get_edit_string()->begin();
      if (o->type()==LI_INT)
        return new li_int(i.read_number());
      else if (o->type()==LI_FLOAT)
        return new li_float(i.read_float()); // JJ cast
      else if (o->type()==LI_SYMBOL)
      {
        char buf[300];
        i4_os_string(*w->get_edit_string(), buf, 100);
        return li_get_symbol(buf);
      }
      else if (o->type()==LI_STRING)
        return new li_string(*w->get_edit_string());
      else
        return o;
    }
    else 
      return o;
  }
      

} li_generic_edit_instance;


class li_class_dialog_item : public li_dialog_item
{
protected:
	li_class_dialog_item():items(5,5)
	{
	};
public:
  char *name() { return "li_class_item"; }

  i4_array<li_dialog_item *> items;

  li_class_dialog_item(li_class *c, li_object *_prop_list, li_environment *env)
    : items(5,5)
  {
    prop_list=_prop_list; 
    o=c;

    int t=li_class_total_members(c->type()), i, max_colums=0;
    int colums[50];
    memset(colums, 0, sizeof(colums));

    for (i=0; i<t; i++)
    {
      li_symbol *sym=li_class_get_symbol(c->type(), i);
      li_object *val=c->value(i);
      li_object *prop_list=li_class_get_property_list(c->type(), sym);
      
      li_dialog_item *item=new li_dialog_item(sym->name()->value(), val, prop_list, env);
       items.add(item);

       int t_win=items[i]->t_windows;
       if (t_win>max_colums)
         max_colums=t_win;
       for (int j=0; j<t_win; j++)
         if (items[i]->windows[j] && items[i]->windows[j]->width()>colums[j])
           colums[j]=items[i]->windows[j]->width();
     }

     int dy=0, maxw=0;
     for (i=0; i<t; i++)
     {
       int dx=0, maxh=0;
       add_child(dx,dy, items[i]);

       for (int j=0; j<items[i]->t_windows; j++)
       {
         if (items[i]->windows[j])
         {
           int xoff=dx-(items[i]->windows[j]->x()-items[i]->x());
           items[i]->windows[j]->move(xoff,0);

           if (items[i]->windows[j]->height()>maxh)
             maxh=items[i]->windows[j]->height();
         }

         dx+=colums[j]+3;
         if (dx>maxw) maxw=dx;
       }
       items[i]->resize_to_fit_children();

       dy+=maxh+1;        
     }      

     private_resize(maxw,dy);
   }

   i4_bool can_apply(li_environment *env)
   {
     for (int i=0; i<items.size(); i++)
       if (!items[i]->can_apply(env))
         return i4_F;
     return i4_T;
   }

   li_object *apply(li_environment *env)
   {
     li_class *c=(li_class *)li_new(o.get()->type());

     for (int i=0; i<items.size(); i++)
       c->set_value(i, items[i]->apply(env));

     return c;
   }
 };

class li_dummy_class_dialog_item:public li_class_dialog_item
{
protected:
	i4_window_class *rw;
public:

	li_dummy_class_dialog_item(i4_window_class *realwindow)
		:li_class_dialog_item()
	{
		rw=realwindow;
		add_child(0,0,rw);
		resize_to_fit_children();
	}

	virtual void resize(w16 new_width, w16 new_height)
	{
		rw->resize(new_width,new_height);
		li_class_dialog_item::resize(new_width,new_height);
		//must not call this from resize(), results in infinite recursion.
		//resize_to_fit_children();
	}

	i4_window_class *window(){return rw;};

	char *name()
	{
		return "dummy_class_dialog_item";
	}
	i4_bool can_apply(li_environment *env)
	{
		return i4_T;
	}

	li_object *apply(li_environment *env)
	{
		return 0;
	}

	virtual i4_bool is_fake()
	{
		return i4_T;
	}
};

 class li_class_edit_class : public li_type_edit_class
 {
 public:
   int create_edit_controls(const i4_const_str &name,
                            li_object *object, 
                            li_object *property_list,
                            i4_window_class **windows, 
                            int max_windows,
                            li_environment *env)
   {
     if (max_windows)
     {
       windows[0]=new li_class_dialog_item(li_class::get(object,env), property_list, env);
       return 1;
     } else return 0;    
   }

   i4_bool can_apply_edit_controls(li_object *objectw, 
                                   li_object *property_list,
                                   i4_window_class **windows,
                                   li_environment *env)
   {
	   li_class_dialog_item *w=(li_class_dialog_item*)windows[0];
	   if (w->is_fake())
		   return i4_T;
	   else
           return ((li_class_dialog_item *)windows[0])->can_apply(env);
   }

   li_object *apply_edit_controls(li_object *o, 
                                  li_object *property_list,
                                  i4_window_class **windows,
                                  li_environment *env)
   {
	   li_class_dialog_item *w=(li_class_dialog_item*)windows[0];
	   if (w->is_fake())
		   return 0;
	   else
           return ((li_class_dialog_item *)windows[0])->apply(env);   
   }

 } li_class_edit_instance;


 class li_generic_edit_initer : public i4_init_class
 {
 public:
   int init_type() { return I4_INIT_TYPE_LISP_FUNCTIONS; }

   void init()
   {
     li_get_type(LI_INT)->editor=&li_generic_edit_instance;
     li_get_type(LI_FLOAT)->editor=&li_generic_edit_instance;
     li_get_type(LI_SYMBOL)->editor=&li_generic_edit_instance;
     li_get_type(LI_STRING)->editor=&li_generic_edit_instance;
     li_set_class_editor(&li_class_edit_instance);
   }

 } li_generic_edit_initer_instance;


 li_dialog_item::li_dialog_item() 
   : i4_color_window_class(0,0, i4_current_app->get_style()->color_hint->neutral(),
                           i4_current_app->get_style())
 { 
   windows=0; 
   t_windows=0; 
   o=0;
   prop_list=0;
   has_extra_label=i4_F;
 }


 li_dialog_item::li_dialog_item(const i4_const_str &name, 
                                li_object *_o, 
                                li_object *prop_list,
                                li_environment *env)

   : i4_color_window_class(0,0, i4_current_app->get_style()->color_hint->neutral(),
                           i4_current_app->get_style()),
     prop_list(prop_list)
 {
   o=_o;
   windows=0;
   t_windows=0;
   has_extra_label=i4_F;
   i4_window_class *w[10];

   if (li_get_type(o->type())->editor)
   {
     if (prop_list!=li_get_symbol("no_edit"))
     {
	   
	 li_class *cl=li_class::get_all(_o,env);
	 if (cl)
		 {
		 //it's a class contained in another one. Add an extra text item
		 //with the name of the superclass.
		 w[0]=new li_dummy_class_dialog_item(new i4_text_window_class(name, i4_current_app->get_style()));
		 t_windows+=1;
		 has_extra_label=i4_T;
		 }
	   
       t_windows+=li_get_type(o->type())->editor->create_edit_controls(name,
                                                                      o.get(),
                                                                      prop_list,
                                                                      &w[t_windows], 10-t_windows, env);
       if (t_windows)
       {       
         windows=(i4_window_class **)I4_MALLOC(sizeof(i4_window_class *) * t_windows,
			 "lisp dialog sub windows");
         int x=0, i, maxh=0;
         for (i=0; i<t_windows; i++)
           if (w[i] && w[i]->height()>maxh)
             maxh=w[i]->height();


         for (i=0; i<t_windows; i++)
         {
           windows[i]=w[i];

           add_child(x, maxh/2 - windows[i]->height()/2 , windows[i]);
           x+=w[i]->width();
         }
         resize_to_fit_children();
       }
     }
   }
 }



 i4_bool li_dialog_item::can_apply(li_environment *env)
 {
   if (!li_get_type(o->type())->editor)  return i4_T;

   //if we have an extra label, don't try to apply anything on it, would
   //break the class chain structure of o.
   return li_get_type(o->type())->editor->can_apply_edit_controls(
	   o.get(), prop_list, has_extra_label?windows+1:windows,env);
 }


 li_object *li_dialog_item::apply(li_environment *env)
 {
   if (li_get_type(o->type())->editor)
     return li_get_type(o->type())->editor->apply_edit_controls(
		o.get(), prop_list, has_extra_label?windows+1:windows, env);
   return o.get();
 }  

 li_dialog_item::~li_dialog_item()
 {
	 //delete the array, but NOT it's contents.
     if (windows)
         i4_free(windows);
 }


 i4_graphical_style_class *li_dialog_window_class::style()
 {
   return i4_current_app->get_style(); 
 }



 li_dialog_window_class::~li_dialog_window_class()
 {
   if (called_on_close)
   {
     if (new_value.get())
	     called_on_close(li_make_list(new_value.get(), o.get(), 0),0);
	 else
		 called_on_close(0,0);
     called_on_close=0;
   }
 }

 li_dialog_window_class::li_dialog_window_class(const i4_const_str &name,
                                                li_object *_o, 
                                                li_object *_prop_list,
                                                li_function_type called_on_close,
                                                li_environment *env)
   : i4_color_window_class(0,0, style()->color_hint->neutral(), style()),
     called_on_close(called_on_close), enviroment(env)
 {
   o=_o;
   prop_list=_prop_list;


   mp_handle=0;
   int t=li_get_type(o->type())->editor->create_edit_controls(name, o.get(), 
                                                             prop_list.get(), w, 10,
                                                              env);
  int x=0, maxh=0;
  for (int i=0; i<t; i++)
  {
    add_child(x, 0, w[i]);
    if (w[i]->height()>maxh)
      maxh=w[i]->height();
  }

  i4_window_class *ok, *cancel;

  if (style()->icon_hint->ok_icon && style()->icon_hint->cancel_icon)
  {
    ok=new i4_image_window_class(style()->icon_hint->ok_icon);
    cancel=new i4_image_window_class(style()->icon_hint->cancel_icon);
  }
  else
  {
    ok=new i4_text_window_class(i4gets("ok"), style());
    cancel=new i4_text_window_class(i4gets("cancel"), style());
  }
    
  resize_to_fit_children();

  i4_button_class *okb=new i4_button_class(0, ok, style(), //sub_type==1 for ok
                                           new i4_event_reaction_class(this, 1));
  i4_button_class *cancelb=new i4_button_class(0, cancel, style(), //sub_type==2 for cancel
                                               new i4_event_reaction_class(this, 2));
  x=width()/2-okb->width()/2-cancelb->width()/2;
  if (x<0) x=0;

  add_child(x, maxh+1, okb);
  add_child(x+okb->width(), maxh+1, cancelb);

  resize_to_fit_children();

}

void li_dialog_window_class::receive_event(i4_event *ev)
{
  if (ev->type()==i4_event::USER_MESSAGE)
  {
    if (((i4_user_message_event_class *)ev)->sub_type==1)
    {
      if (!li_get_type(o->type())->editor->can_apply_edit_controls(o.get(), prop_list.get(), w, env()))
        return;
      else
        new_value=li_get_type(o->type())->editor->apply_edit_controls(o.get(),
                                                                      prop_list.get(), w, env());

    }

    if (mp_handle)
      style()->close_mp_window(mp_handle);        

    if (called_on_close)
    {
      if (new_value.get())
        called_on_close(li_make_list( new_value.get(), o.get(),  0),0);
	  else
		  called_on_close(0,0);
      called_on_close=0;
    }

  }
  else
    i4_color_window_class::receive_event(ev);
}
  


li_dialog_window_class *li_create_dialog(const i4_const_str &name,
                                         li_object *o, 
                                         li_object *prop_list,
                                         char *close_fun,
                                         li_environment *env)
{
  li_function_type fun=0;
  if (close_fun)
    fun=li_function::get(li_get_fun(li_get_symbol(close_fun), env),env)->value();

  li_dialog_window_class *d=new li_dialog_window_class(name, o,prop_list, fun, env);

  i4_parent_window_class *mp;
  mp=i4_current_app->get_style()->create_mp_window(-1,-1, d->width(), d->height(),
                                                   name, 0);
  d->mp_handle=mp;
  mp->add_child(0,0,d);
  return d;
}




li_dialog_window_class *li_create_dialog(const i4_const_str &name,
                                         li_object *o, 
                                         li_object *prop_list,
                                         li_function_type fun,
                                         li_environment *env)
{
  li_dialog_window_class *d=new li_dialog_window_class(name, o,prop_list, fun, env);

  i4_parent_window_class *mp;
  mp=i4_current_app->get_style()->create_mp_window(-1,-1, d->width(), d->height(),
                                                   name, 0);
  d->mp_handle=mp;
  mp->add_child(0,0,d);
  return d;
}



// lisp/li_load.cpp

/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/



li_type_number *li_load_type_info(i4_loader_class *fp, li_environment *env)
{
  int t_types=fp->read_16(), i;
  if (!t_types)
    return 0;

  
  li_type_number *remap=(li_type_number *)I4_MALLOC(sizeof(li_type_number) * t_types, "");
  memset(remap, 0, sizeof(li_type_number) * t_types);

  for (i=1; i<t_types; i++)
  {
    char buf[300];
    int l=fp->read_16();
    if (l>sizeof(buf)) 
      li_error(env, "INTERNAL: load type name too long");

    fp->read(buf, l);
   
    for (int j=1; j<li_max_types(); j++)
      if (li_valid_type(j))
        if (strcmp(buf, li_get_type(j)->name())==0)
          remap[i]=j;    
  }

  for (i=1; i<t_types; i++)
  {
    w32 skip=fp->read_32();

    if (remap[i])
    {
      //      i4_warning("%d : remap for %s", i, li_get_type(remap[i])->name());
      li_get_type(remap[i])->load(fp, remap, env);
    }
    else
      fp->seek(fp->tell() + skip);
  }


  return remap;
}


void li_free_type_info(li_type_number *remap)
{
  if (remap)
    i4_free(remap);

  for (int i=1; i<li_max_types(); i++)
    if (li_valid_type(i))
      li_get_type(i)->load_done();
}

void li_save_type_info(i4_saver_class *fp, li_environment *env)
{
  int t_types=1, i;
  for (i=1; i<li_max_types(); i++)
    if (li_valid_type(i))
      t_types++;

  // save the name and number of each type
  fp->write_16(t_types);
  for (i=1; i<li_max_types(); i++)
  {
    if (li_valid_type(i))
    {
      char *n=li_get_type(i)->name();
      int nl=strlen(n)+1;
      fp->write_16(nl);
      fp->write(n,nl);
    }
    else 
      fp->write_16(0);
  }

  for (i=1; i<li_max_types(); i++)
  {
    if (li_valid_type(i))
    {
      int handle=fp->mark_size();
      li_get_type(i)->save(fp, env);
      fp->end_mark_size(handle);
    }

  }
}



li_object *li_load_typed_object(char *type_name, i4_loader_class *fp, 
                                li_type_number *type_remap,
                                li_environment *env)
{
  int type=li_find_type(type_name);
  if (!type)
    li_error(env,"INTERNAL: type %s unknown", type_name);
  else
  {
    li_object *o=li_load_object(fp, type_remap, env);
    if (!o || o->type()!=(w32)type) // JJ cast
      return li_new(type);
    else
      return o;
  }

  return 0;
}

li_object *li_load_typed_object(int type, i4_loader_class *fp, li_type_number *type_remap,
                                li_environment *env)
{  
  li_object *o=li_load_object(fp, type_remap, env);
  if (!o || o->type()!=(w32)type)  // JJ cast
  {
    if (type)   
      return li_new(type);
    else return 0;
  }
  else
    return o;
}

// lisp/li_types.cpp
/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/




li_string::li_string(const char *name)
  : li_object(LI_STRING)
{ 
  int l=strlen(name)+1; 
  _name=(char *)I4_MALLOC(l,"");
  memcpy(_name, name, l);
}

li_string::li_string(int len)
  : li_object(LI_STRING)
{
  _name=(char *)I4_MALLOC(len,"");
}

li_string::li_string(const i4_const_str &str)
  : li_object(LI_STRING)
{
  int len=str.length()+1;
  _name=(char *)I4_MALLOC(len,"");
  i4_os_string(str, _name, len);
}



void li_save_type(i4_file_class *fp, li_type_number type)
{
  fp->write_16((w16)type); // JJ cast
}

li_type_number  li_load_type(i4_file_class *fp, li_type_number *type_remap)
{
  I4_ASSERT(type_remap, "call li_load_type_info before li_load_type");

  return type_remap[fp->read_16()];
}


void li_save_object(i4_saver_class *fp, li_object *o, li_environment *env)
{
  if (!o)
    fp->write_16(0);
  else
  {
    li_save_type(fp, o->type());

    int h=0;
    if (o->type()>LI_TYPE)
      h=fp->mark_size();

    li_get_type(o->type())->save_object(fp, o, env);

    if (o->type()>LI_TYPE)
      fp->end_mark_size(h);
  }
}


li_object *li_load_object(i4_loader_class *fp, li_type_number *type_remap, li_environment *env)
{
  li_type_number old_type=fp->read_16();  
  if (old_type==0)
    return 0;
  li_type_number type;
  if (type_remap==0)//if reading from the network
	  {
	  type=old_type;
	  }
  else
	  {
	  type=type_remap[old_type];
	  }
  
  w32 skip=0;
  if (old_type>LI_TYPE)
    skip=fp->read_32();
  else if (type==0)
    i4_error("huh?");   // shouldn't happen (please, please)

  if (type)
    return li_get_type(type)->load_object(fp, type_remap, env);
  else if (type>0 && type<=LI_TYPE)
  {
    li_error(env, "SEVERE: type not found, but should be");
    return 0;
  }
  else
  {
    fp->seek(fp->tell() + skip);
    return 0;
  }
}

class li_invalid_type_function : public li_type_function_table
{
  virtual void mark(li_object   *o, int set) 
	  {
	  // objects are marked but invalid when they where just created,
	  // until their constructor is called for the first time.
	  // i4_error("INTERNAL: marking invalid object"); 
	  }
  virtual void free(li_object   *o) { i4_error("INTERNAL: freeing invalid object"); }
  virtual int equal(li_object  *o1, li_object *o2)  
  {  
    i4_error("INTERNAL: comparing invalid object"); 
    return 0;
  }

  virtual void print(li_object  *o, i4_file_class *stream)   
	{ 
		li_error(0,"INTERNAL: printing invalid object"); 
	}
  virtual char *name() { i4_error("INTERNAL: getting name for invalid object"); return 0;}

  virtual void save_object(i4_saver_class *fp, li_object *o, li_environment *env) 
  { li_error(env, "INTERNAL: saving invalid object"); }

  virtual li_object *load_object(i4_loader_class *fp, li_type_number *type_remap, li_environment *env)
  { 
    li_error(env, "INTERNAL loading invalid object"); 
    return 0;
  }

};


void li_symbol::add_property(li_object *name, li_object *value)
	{
	if (data->_proplist==0)
		{
		data->_proplist=li_make_list(name,value,0);
		return;
		}
	li_list *c=li_list::get(data->_proplist,0);
	li_object *n=0;
	i4_bool bname=i4_T;
	while (c)
		{
		n=li_car(c,0);
		c=(li_list*)li_cdr(c,0);
		if (bname)
			{
			if (li_eq(li_make_list(n,name,0),0)==li_true_sym)
				{
				if (c!=0)
					c->set_data(value);
				else
					li_error(0,"USER: Invalid property list detected on symbol %O.",this);
				return;
				}
			}
		bname= !bname;
		}
	//the property doesn't exist yet.
	//we add two cells to the beginning of the list: the name and the value.
	data->_proplist=new li_list(name,new li_list(value,data->_proplist));
	};

li_object *li_symbol::get_property(li_object *name)
	{
	if (data->_proplist==0)
		return 0;
	li_list *c=li_list::get(data->_proplist,0);
	li_object *n=0;
	i4_bool bname=i4_T;
	while (c)
		{
		n=li_car(c,0);
		c=(li_list*)li_cdr(c,0);
		if (bname)
			{
			if (li_eq(li_make_list(n,name,0),0)==li_true_sym)
				{
				if (c!=0)
					return c->data();
				else
					li_error(0,"USER: Invalid property list detected on symbol %O.",this);
				return 0;
				}
			}
		bname= !bname;
		}
	return 0;
	}
	


class li_symbol_type_function : public li_type_function_table
{
  virtual void mark(li_object *o, int set)
  { 
    li_symbol *s=(li_symbol *)o;
    s->mark(set);

    if (s->value())
    {
      if (set!=s->value()->is_marked())
        li_get_type(s->value()->unmarked_type())->mark(s->value(), set);    
    }

    li_object *fun=s->fun();
    if (fun)
    {
      if (set!=fun->is_marked())
        li_get_type(fun->unmarked_type())->mark(fun, set);    
    }

    li_object *name=s->name();
    if (set!=name->is_marked())
      li_get_type(name->unmarked_type())->mark(name, set);   
	
	li_object *prop=s->get_plist();
	if (prop&&set!=prop->is_marked())
		{
		li_get_type(prop->unmarked_type())->mark(prop,set);
		}
  }

  virtual void free(li_object   *o)
  {
    li_symbol::get(o,0)->free();
  }
 
  virtual void print(li_object  *o, i4_file_class *stream)   
  {  
    li_symbol *s=li_symbol::get(o,0);
    char *name=s->name()->value();    
    stream->write(name, strlen(name));
  }
  virtual char *name() { return "symbol"; }

  virtual void save_object(i4_saver_class *fp, li_object *o, li_environment *env)
  {
    li_symbol *s=li_symbol::get(o,env);
    char *name=s->name()->value();
    int name_len=strlen(name)+1;

    fp->write_16(name_len);
    fp->write(name, name_len);
  }

  virtual li_object *load_object(i4_loader_class *fp, li_type_number *type_remap,
                                 li_environment *env)
  { 
    char buf[200];
    int len=fp->read_16();
    if (len>200)
      li_error(env, "symbol name too long");
    fp->read(buf, len);
    return li_get_symbol(buf);
  }
};



char *li_get_type_name(li_type_number type)
{
  return li_get_type(type)->name();
}

li_string::li_string(i4_file_class *fp) : li_object(LI_STRING)
{
  int l=fp->read_32();
  _name=(char *)I4_MALLOC(l,"");
  fp->read(_name, l);
}

class li_string_type_function : public li_type_function_table
{ 
  virtual li_object *copy(li_object *o)
	  {
	  return new li_string(li_string::get(o,0)->value());
	  }
  virtual void free(li_object   *o) 
  { 
    i4_free(li_string::get(o,0)->value());
  }

  virtual void print(li_object  *o, i4_file_class *stream)   
  { 
    stream->printf("\"%s\"", li_string::get(o,0)->value());
  }

  virtual int equal(li_object  *o1, li_object *o2)  
  {   
    return (strcmp(li_string::get(o1,0)->value(), li_string::get(o2,0)->value())==0);
  }

  virtual char *name() { return "string"; }

  virtual void save_object(i4_saver_class *fp, li_object *o, li_environment *env)
  {
    char *s=li_string::get(o,env)->value();
    int l=strlen(s)+1;
    fp->write_32(l);
    fp->write(s,l);
  }

  virtual li_object *load_object(i4_loader_class *fp, li_type_number *type_remap, li_environment *env)
  {
    return new li_string(fp);
  }

};



class li_int_type_function : public li_type_function_table
{
  virtual li_object *copy(li_object *o)
	  {
	  return new li_int(li_int::get(o,0)->value());
	  }
  virtual int equal(li_object  *o1, li_object *o2)
  { 
    return li_int::get(o1,0)->value()==li_int::get(o2, 0)->value(); 
  }

  virtual void print(li_object  *o, i4_file_class *stream)   
  { 
    stream->printf("%d", li_int::get(o,0)->value());
  }

  virtual char *name() { return "int"; }

  virtual void save_object(i4_saver_class *fp, li_object *o, li_environment *env)
  {
    fp->write_32(li_int::get(o,0)->value());
  }

  virtual li_object *load_object(i4_loader_class *fp,  li_type_number *type_remap,
                                 li_environment *env)
  {
    return new li_int(fp->read_32());
  }
    

};


class li_type_type_function : public li_type_function_table
{
  
  virtual int equal(li_object  *o1, li_object *o2)  
  { 
    return li_int::get(o1,0)->value()==li_int::get(o2,0)->value(); 
  }

  virtual void print(li_object  *o, i4_file_class *stream)   
  { 
    stream->printf("type-%s", li_get_type(li_type::get(o,0)->value())->name());
  }

  virtual char *name() { return "type"; }

  virtual void save_object(i4_saver_class *fp, li_object *o,
                           li_environment *env)
  {
    li_save_type(fp, li_type::get(o,env)->value());
  }

  virtual li_object *load_object(i4_loader_class *fp, li_type_number *type_remap,
                                 li_environment *env)
  {
    int new_type=li_load_type(fp, type_remap);
    if (new_type)
      return new li_type(new_type);
    else
      return 0;
  }

};



class li_float_type_function : public li_type_function_table
{
  virtual li_object *copy(li_object *o)
	  {
	  return new li_float(li_float::get(o,0)->value());
	  }
  
  virtual int equal(li_object  *o1, li_object *o2)  
  { return li_float::get(o1,0)->value()==li_float::get(o2,0)->value(); }

  virtual void print(li_object  *o, i4_file_class *stream)   
  { 
    char buf[200], dec=0;
    sprintf(buf, "%f", li_float::get(o,0)->value());
    
    for (char *c=buf; *c; c++)
      if (*c=='.') dec=1;
    
    if (dec)
    {
      while (buf[strlen(buf)-1]=='0')
        buf[strlen(buf)-1]=0;
    
      if (buf[strlen(buf)-1]=='.')
        buf[strlen(buf)-1]=0;
    }


    stream->write(buf,strlen(buf));
  }

  virtual char *name() { return "float"; }

  virtual void save_object(i4_saver_class *fp, li_object *o, li_environment *env)
  {
    fp->write_float((float)li_float::get(o,env)->value());//data loss ok here
  }

  virtual li_object *load_object(i4_loader_class *fp, li_type_number *type_remap,
                                 li_environment *env)
  {
    return new li_float(fp->read_float());
  }

};


class li_character_type_function : public li_type_function_table
{
  virtual int equal(li_object  *o1, li_object *o2)  
  { 
    return li_character::get(o1,0)->value()==li_character::get(o2,0)->value(); 
  }

  virtual void print(li_object  *o, i4_file_class *stream)   
  { 
    stream->printf("#%c",li_character::get(o,0)->value());
  }

  virtual char *name() { return "character"; }

  virtual void save_object(i4_saver_class *fp, li_object *o, li_environment *env)
  {
    fp->write_16(li_character::get(o,env)->value());
  }

  virtual li_object *load_object(i4_loader_class *fp, li_type_number *type_remap,
                                 li_environment *env)
  {
    return new li_character((w8)fp->read_16()); // JJ cast
  }

};



class li_list_type_function : public li_type_function_table
{
  virtual li_object *copy(li_object *o)
	  {
	  li_object *ret=0,*newdata=0,*newcell=0;
      li_list *p=li_list::get(o,0);
	  if (o->is_marked()) return o;
      o->mark(1);          // mark to prevent recursive copying

      
      newdata=li_get_type(p->data()->type())->copy(p->data());
	  if (p->next()) newcell=li_get_type(p->next()->type())->copy(p->next());
	  newcell=new li_list(newdata,newcell);
      
    
	  o->mark(0);
	  return newcell;
	  }

  virtual void mark(li_object   *o, int set) 
  { 
    if (o->is_marked() && set)
      return ;

    li_list *l=(li_list *)o;
    if (l->data())
    {
      for (li_list *p=l; p;)
      {
        p->mark(set);
        if (p->data())
        {
          if (set!=p->data()->is_marked())
            li_get_type(p->data()->unmarked_type())->mark(p->data(), set);

          if (p->next() && (set!=p->next()->is_marked()))
          {
            if (p->next()->unmarked_type()==LI_LIST)
              p=(li_list *)p->next();
            else
            {
              li_get_type(p->next()->unmarked_type())->mark(p->next(), set);
              p=0;
            }
          } else p=0;
        }
        else p=0;

      }
    }
	else
		{
		o->mark(set);//sometimes, a list might be temporarilly (0 . 0)
	                 //but is in use anyway (i.e right after new_cons_cell()
		if (l->next())
			l->next()->mark(set);
		}
  }
  
  virtual void free(li_object   *o) 
  { 
    li_list *l=(li_list *)o;
    l->cleanup();

  }

  virtual int equal(li_object  *o1, li_object *o2)  
  { 
    if (o1==o2) return 1; 
    li_list *p1=li_list::get(o1,0), *p2=li_list::get(o2,0);

    for (;p1;)
    {
      if (!o2) return 0;

      if (p1->data()->type() != p2->data()->type()) return 0;

      if (li_get_type(p1->data()->type())->equal(p1->data(), p2->data())==0)  return 0;
       
      if (p1->next()->type()==LI_LIST)
      {
        if (p2->next()->type()!=LI_LIST)   return 0;
        p1=(li_list *)p1->next();
        p2=(li_list *)p2->next();
      }
      else if (p1->next()->type()!=p2->next()->type()) return 0;
      else return li_get_type(p1->next()->type())->equal(p1->next(), p2->next());
    }

    if (!p2) return 1;
    else return 0;
  }


  virtual void print(li_object  *o, i4_file_class *stream)   
  { 
    stream->write_8('(');
    li_list *p=li_list::get(o,0);
	if (o->is_marked()) 
		{
		stream->printf("[cancelling recursive print...]");
		return;
		}
    o->mark(1);          // mark to prevent recursive prints

    for (; p; )
    {  
      li_get_type(p->data()->type())->print(p->data(), stream);

      if (p->next())
      {
        if (p->next()->type()!=LI_LIST)
        {//next is last element of list
		  if (p->next()->is_marked())
			  {
			  stream->printf("[cancelling recursive print...]");
			  
			  }
			  
		  else
			  {
			  if (p->next()!=li_nil)
				//don't print (x . nil)
			  	{
				stream->write(" . ",3);//if p->next() is nil, don't print it.
				li_get_type(p->next()->type())->print(p->next(), stream);
				
				}
			  
			  }
		  p=0;
        }
        else
        {//next is continuation of list
          p=(li_list *)p->next();
          stream->write_8(' ');
        }
      }
      else p=0;
    }

    o->mark(0);

    stream->write_8(')');
  }

  virtual void save_object(i4_saver_class *fp, li_object *o, li_environment *env)
  {
    int t=0;
    int last_is_cons=0;
    li_list *l;
    for (l=li_list::get(o,env); l;)
    {
      t++;
      if (t>2000000)
        li_error(env, "CRITICAL: list is really big : trying to save a circular structure doesn't work");

      li_object *next=l->next();
      if (next)
      {
        if (next->type()!=LI_LIST)
        {
          l=0;
          last_is_cons=0;
        }
        else l=(li_list *)next;
      }
      else l=0;
    }


    fp->write_32(t);

    if (last_is_cons)
      fp->write_8(1);
    else
      fp->write_8(0);

    for (l=li_list::get(o, env); l;)
    {
      li_object *data=l->data();

      li_save_object(fp, data, env);

      li_object *next=l->next();
      if (next)
      {
        if (next->type()==LI_LIST)
          l=(li_list *)next;
        else
        {
          li_save_object(fp, next, env);
          l=0;
        }
      } else l=0;
    }
  }

  virtual li_object *load_object(i4_loader_class *fp, li_type_number *type_remap,
                                 li_environment *env)
  {
    int t=fp->read_32();
    int last_is_cons=fp->read_8();
    li_list *last=0, *first=0;

    for (int i=0; i<t; i++)
    {
      li_object *data=li_load_object(fp, type_remap, env);
      li_list *l=new li_list(data, 0);
      if (!first)
        first=l;
      else
        last->set_next(l);
      last=l;
    }

    if (last_is_cons)
      last->set_next(li_load_object(fp,type_remap,env));

    return first;
  }


  virtual char *name() { return "list"; }
};


class li_user_function_type_function : public li_type_function_table
	{
	virtual void print(li_object *o, i4_file_class *stream)
		{
		stream->printf("#<function ");
		li_user_function *fn=li_user_function::get(o,0);
		li_string *n=0;//the name is always "lambda" (internal requirement)
		li_object *c=fn->data()->_code;
		if (c->type()==LI_SYMBOL)
			{
			n=li_symbol::get(c,0)->name();
			}
		if (n) li_get_type(n->type())->print(n,stream); else stream->printf("[unknown name]");
		stream->printf(">");
	};
	virtual char *name() { return "user_function";};
	virtual void save_object(i4_saver_class *fp,li_object *o,li_environment *env)
		{
		fp->write_16((w16)li_type::get(o,env)->value());
		//saving this kind of object in this context is not needed, just reload the original code
		//li_user_function::user_function_data d=li_user_function::get(o,env)->data();
		//d->_params->save_object(fp);
		//d->_bindings->save_object(fp);
		}
	virtual li_object *load_object(i4_loader_class *fp, li_type_number *type_remap,li_environment *env)
		{
		int t=type_remap[fp->read_16()];
		if (t)
		  return new li_type(t);
		else
		  return 0;
		}
	virtual void mark(li_object *o, int set)
		{
		if (o->is_marked() && set)
			return ;
		li_user_function *l=(li_user_function *)o;
		l->mark(set);//don't forget to mark ourselves
		if (l->data())
		{
			li_user_function::user_function_data *d=l->data();
			li_get_type(d->_name->unmarked_type())->mark(d->_name,set);
			if (d->_bindings)
				li_get_type(d->_bindings->unmarked_type())->mark(d->_bindings,set);
			if (d->_params)
				li_get_type(d->_params->unmarked_type())->mark(d->_params,set);
			if (d->_locals)
				li_get_type(d->_locals->unmarked_type())->mark(d->_locals,set);
			if (d->_code)
				li_get_type(d->_code->unmarked_type())->mark(d->_code,set);
			if (d->_reserved) 
				li_get_type(d->_reserved->unmarked_type())->mark(d->_reserved,set);

		}

		}
	virtual void free(li_object *o)
		{
		li_user_function *u=li_user_function::get(o,0);
		u->cleanup();
		}
	virtual int equal(li_object *o1,li_object *o2)
		{
		if (o1==o2) return i4_T;
		return i4_F;
		}
	};


class li_function_type_function : public li_type_function_table
{
  virtual void print(li_object  *o, i4_file_class *stream)
  { 
    stream->printf("#<compiled function @ 0x%x>", (long)(li_function::get(o,0)->value()));
  }

  virtual char *name() { return "function"; }

  virtual void save_object(i4_saver_class *fp, li_object *o, li_environment *env)
  {
    fp->write_16((w16)li_type::get(o,env)->value()); // JJ cast
  }

  virtual li_object *load_object(i4_loader_class *fp, li_type_number *type_remap, li_environment *env)
  {
    int t=type_remap[fp->read_16()];
    if (t)
      return new li_type(t);
    else
      return 0;
  }

};
/** Dynamic vector (array) class type.
* this type implements the common lisp vector type. This is actually 
* a dynamic array class. Do not confuse with the vector class used
* for math!
*/

class li_vector_type_function : public li_type_function_table
{ 
public:
  
  virtual li_object *copy(li_object *o)
	  {
	  li_vector *l=(li_vector *)o;
	  li_vector *cp=new li_vector();
	  for (int i=0;i<l->size();i++)//the known length makes things easy
		  {
		  li_object *e=l->element_at(i);
		  cp->add_element(li_get_type(e->type())->copy(e));
		  }
	  return cp;
	  }

// free data associated with an instance of this type
  virtual void free(li_object   *o)
  {
	li_vector::get(o,0)->cleanup();
  }

  //helper function for gc.
  virtual void mark(li_object *o,int set)
	  {
	  if (o->is_marked()&&set)
		  return;
	  li_vector *l=(li_vector *)o;
	  l->mark(set);//mark self
	  for (int i=0;i<l->size();i++)//the known length makes things easy
		  {
		  li_object *e=l->element_at(i);
		  if (e)
		      li_get_type(e->unmarked_type())->mark(e,set);
		  }
      
	  }

  virtual int equal(li_object  *o1, li_object *o2) 
  { 
  li_vector *a=li_vector::get(o1,0),*b=li_vector::get(o2,0);
  if (a->size()==b->size())
	  {
	  for (int i=0;i<a->size();i++)
		  {
		  li_object *a1=a->element_at(i),*b1=b->element_at(i);
		  if(!(a1->type()==b1->type()&&li_get_type(a1->type())->equal(a1,b1)))
			  return i4_F;
		  }
	  return i4_T;
	  }
  return i4_F;
  }

  virtual void print(li_object  *o, i4_file_class *stream)
  {
	li_vector *a=li_vector::get(o,0);
	stream->printf("#(");
	for (int i=0;i<a->size();i++)
		{
		li_get_type(a->element_at(i)->type())->print(a->element_at(i),stream);
		stream->printf(" ");
		}
	stream->printf(")");

  }

  virtual char *name() { return "vector_array"; }

  virtual li_object *create(li_object *params, li_environment *env)
  {
  li_vector *v=new li_vector();
  while(params)
	  {
	  li_object *e=li_car(params,env);
	  v->add_element(e);
	  params=li_cdr(params,env);
	  }
  return v;
    
  }


  virtual void save_object(i4_saver_class *fp, li_object *o, li_environment *env)
  {
  //not yet supported

  }

  virtual li_object *load_object(i4_loader_class *fp, li_type_number *type_remap,
                                 li_environment *env)
  {
    //not yet supported
    return new li_vector(0);
  }
};


li_symbol *&li_environment::current_function()
{
  return data->current_function;
}

li_object *&li_environment::current_arguments()
{
  return data->current_args;
}
li_environment *li_environment::set_next(li_environment *_next)
	{
	  li_environment *act=_next;
	  while (act)//be shure, we don't construct a circular list
		  {
		  if (act==this) return _next;
		  act=act->data->next;
		  }

	  data->next=_next;
	  data->local_namespace=i4_T;
	  return this;
	};


void li_environment::print_call_stack(i4_file_class *fp)
{
  li_symbol *s=current_function();
  li_object *o=current_arguments();

  if (s && o)
    li_printf(fp, "%O %O\n", s,o);
  else if (s)
    li_printf(fp, "%O\n", s);
  else 
	  li_printf(fp, "<Unknown function>\n");

  if (data->next)
    data->next->print_call_stack(fp);
}


li_object *li_environment::value(li_symbol *s)
{
  for (value_data *p=data->value_list; p; p=p->next)
    if (p->symbol==s)
      return p->value;

  if (data->next)
    return data->next->value(s);

  return s->value();
}

li_environment *li_environment::env_for_symbol(li_symbol *s)
    {
    for (value_data *p=data->value_list; p; p=p->next)
    if (p->symbol==s)
      return this;

    if (data->next)
      return data->next->env_for_symbol(s);

    return 0;
    }


li_object *li_environment::fun(li_symbol *s)
{
  for (fun_data *p=data->fun_list; p; p=p->next)
    if (p->symbol==s)
      return p->fun;

  if (data->next)
    return data->next->fun(s);

  return s->fun();
}

void li_environment::define_value(li_symbol *s, li_object *value)
{
	if (data->local_namespace)
  {
    for (value_data *p=data->value_list; p; p=p->next)
		{
		if (p->symbol==s)
			{
			p->value=value;
			return;
			}
		}
	
    value_data *v=new value_data;
    v->symbol=s;
    v->value=value;
    v->next=data->value_list;
    data->value_list=v;
  }
  else if (data->next)
    data->next->set_value(s,value);
  else
    s->set_value(value);
}



void li_environment::set_value(li_symbol *s, li_object *value)
{
  if (data->local_namespace)
  {
    for (value_data *p=data->value_list; p; p=p->next)
		{
		if (p->symbol==s)
			{
			p->value=value;
			return;
			}
		}
	if (data->next) 
		data->next->set_value(s,value);
	else 
		s->set_value(value);
    /*value_data *v=new value_data;
    v->symbol=s;
    v->value=value;
    v->next=data->value_list;
    data->value_list=v;*/
  }
  else if (data->next)
    data->next->set_value(s,value);
  else
    s->set_value(value);
}


void li_environment::set_fun(li_symbol *s, li_object *fun)
{
  if (data->local_namespace)
  {
    for (fun_data *p=data->fun_list; p; p=p->next)
      if (p->symbol==s)
        p->fun=fun;
  
    fun_data *f=new fun_data;
    f->symbol=s;
    f->fun=fun;
    f->next=data->fun_list;
    data->fun_list=f;
  }
  else if (data->next)
    data->next->set_fun(s, fun);
  else 
    s->set_fun(fun);
}


void li_environment::mark(int set)
{
  li_object::mark(set);

  for (value_data *v=data->value_list; v; v=v->next)
    if (set!=v->value->is_marked())
      li_get_type(v->value->unmarked_type())->mark(v->value,set);

  for (fun_data *f=data->fun_list; f; f=f->next)
    if (set!=f->fun->is_marked())
      li_get_type(f->fun->unmarked_type())->mark(f->fun,set);

  if (data->next && data->next->is_marked()!=set)
    li_get_type(LI_ENVIROMENT)->mark(data->next, set);
}

void li_environment::free()
{
  for (value_data *v=data->value_list; v; )
  {   
    value_data *last=v;
    v=v->next;
    delete last;
  }

  for (fun_data *f=data->fun_list; f; )
  {   
    fun_data *last=f;
    f=f->next;
    delete last;
  }

  delete data;
}

void li_environment::print(i4_file_class *s)
{
  s->printf("#env-(syms=");

  for (value_data *v=data->value_list; v; v=v->next)
  {
    s->write_8('(');
    li_get_type(v->symbol->type())->print(v->symbol, s);
    s->write_8(' ');
    li_get_type(v->value->type())->print(v->value,  s);    
    s->write_8(')');
  }

  s->printf("funs=");
  for (fun_data *f=data->fun_list; f; f=f->next)
  {
    s->write_8('(');
    li_get_type(f->symbol->type())->print(f->symbol, s);
    s->write_8(' ');
    li_get_type(f->fun->type())->print(f->fun,  s);    
    s->write_8(')');
  }
  s->write_8(')');

}



class li_environment_type_function : public li_type_function_table
{
public:
  virtual void mark(li_object   *o, int set)   { ((li_environment *)o)->mark(set); }
  virtual void free(li_object   *o) {  li_environment::get(o,0)->free();  }
  virtual void print(li_object  *o, i4_file_class *s) { li_environment::get(o,0)->print(s); }
  virtual char *name() { return "environment"; }

  
  virtual void save_object(i4_saver_class *fp, li_object *o, li_environment *env) 
  { li_error(env, "INTERNAL: li_environments cannot be saved"); }

  virtual li_object *load_object(i4_loader_class *fp, li_type_number *type_remap, li_environment *env) 
  { 
    li_error(env, "INTERNAL: li_environments cannot be loaded"); 
    return 0;
  }

};

i4_bool li_equal_bignum(li_bignum *a, li_bignum *b);

class li_bignum_type_function : public li_type_function_table
{ 
  virtual li_object *copy(li_object *o)
	  {
	  li_bignum *b=li_bignum::get(o,0);
	  return new li_bignum(b->get_length(),b->value(),b->get_signum());
	  }
  virtual void free(li_object   *o) 
  { 
    li_bignum::get(o,0)->cleanup();
  }

  virtual void print(li_object  *o, i4_file_class *stream)   
  { 
  li_bignum *b=li_bignum::get(o,0);
  if (b->get_signum()) stream->printf("-");
  w32 l=b->get_length();
  char *c=b->value();
  while (l>1&&(*c==0x0)) {c++,l--;};//skip leading zeroes
  while (l)
	  {
	  stream->printf("%c",(*c)+'0');
	  c++;
	  l--;
	  }
  }

  virtual int equal(li_object  *o1, li_object *o2)  
	  {   
	  li_bignum *a=li_bignum::get(o1,0),*b=li_bignum::get(o2,0);
	  if (a->get_signum()!=b->get_signum())
	  {
	  return i4_F;
	  }
      return li_equal_bignum(a,b);
	  }
	  

  virtual char *name() { return "bignum"; }

  virtual void save_object(i4_saver_class *fp, li_object *o, li_environment *env)
  {
    char *s=li_bignum::get(o,env)->value();
    int l=li_bignum::get(o,env)->get_length();
    fp->write_32(l);
    fp->write(s,l);
	fp->write_8(li_bignum::get(o,env)->get_signum());
  }

  virtual li_object *load_object(i4_loader_class *fp, li_type_number *type_remap, li_environment *env)
  {
    int l=fp->read_32();
	char *buf=new char[l];
	fp->read(buf,l);
	return new li_bignum(l,buf,fp->read_8());
  }

};

class li_type_manager_class : public i4_init_class
{

public: 
  i4_array<li_type_function_table *> table;

  int add_init(li_type_function_table *type_functions)
	  {
	  li_type_number new_type=table.size();
	  table.add(type_functions);
	  return new_type;
	  }
  int add(li_type_function_table *&type_functions,
          li_environment *env=0,
          int anon=0)

  {
    li_type_number old_type=0, new_type=table.size();    

    if (!anon)
    {
      li_symbol *sym=li_get_symbol(type_functions->name());
      if (sym->value() && sym->value()->type()==LI_TYPE)  
      {
        old_type=li_type::get(sym->value(), env)->value();
        i4_warning("attempt to reassign type %s ignored", type_functions->name());
        delete type_functions;
		type_functions=0;
        return old_type;
      }
      
      li_set_value(sym, new li_type(new_type), env);
    }

    table.add(type_functions);

    
    return new_type;
  }

  li_type_manager_class() : table(0,32) {}

  void remove(int type_num)
  {
    delete table[type_num];
    table[type_num]=0;
  }

  li_type_function_table *get(int num)
  {
    return table[num&0x3fffffff];
  }

  int init_type() { return I4_INIT_TYPE_LISP_BASE_TYPES; }
  void init()
  {
    li_invalid_type_function *invalid=new li_invalid_type_function;  
    for (int i=0; i<LI_LAST_TYPE; i++)
		{
      //add(invalid,0,1);
		add_init(invalid);
		}
    
    table[LI_SYMBOL]=new li_symbol_type_function;
    table[LI_STRING]=new li_string_type_function;
    table[LI_INT]=new li_int_type_function;
    table[LI_FLOAT]=new li_float_type_function;
    table[LI_LIST]=new li_list_type_function;
    
    table[LI_CHARACTER]=new li_character_type_function;
    table[LI_FUNCTION]=new li_function_type_function;
    table[LI_ENVIROMENT]=new li_environment_type_function;
    table[LI_TYPE]=new li_type_type_function;
	table[LI_USER_FUNCTION]=new li_user_function_type_function;
	table[LI_VECTOR]=new li_vector_type_function;
	table[LI_BIGNUM]=new li_bignum_type_function;
  }

  int find(char *name)
  {
    for (int i=1; i<table.size(); i++)
      if (strcmp(table[i]->name(), name)==0)
        return i;

    return 0;
  }

};

static li_type_manager_class li_type_man;

int li_add_type(li_type_function_table *&type_functions,   // return type number for type
                li_environment *env,
                int anon)

{
  return li_type_man.add(type_functions, env, anon);
}

void li_remove_type(int type_num)
{
  li_type_man.remove(type_num);
}

void li_cleanup_types()
{
  li_type_man.table.uninit();
}

li_type_function_table *li_get_type(li_type_number type_num)
{
  return li_type_man.get(type_num);
}



li_type_number li_find_type(char *name, li_environment *env)
{
  li_symbol *s=li_find_symbol(name);
  if (s)
    return li_type::get(li_get_value(s, env),env)->value();
  else
    return 0;
}

li_type_number li_find_type(char *name, li_environment *env, li_type_number &cache_to)
{
  if (cache_to)
    return cache_to;
  else
  {
    cache_to=li_type::get(li_get_value(li_get_symbol(name), env), env)->value();
    return cache_to;
  }
}



i4_bool li_valid_type(li_type_number type_number)
{
  return type_number>=0 && type_number< (w32)li_type_man.table.size() &&  // JJ cast
    li_type_man.table[type_number]!=0;
}

int li_max_types()
{
  return li_type_man.table.size();
}

// lisp/li_vect.cpp
/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/


li_type_number li_vect_type;
class li_vect_type_function_table : public li_type_function_table
{ 
public:
  // free data associated with an instance of this type
  virtual void free(li_object   *o)
  {
    delete li_vect::get(o,0)->v;
  }

  virtual int equal(li_object  *o1, li_object *o2) 
  { 
    i4_3d_vector v1=li_vect::get(o1,0)->value(), v2=li_vect::get(o2,0)->value();
    return v1.x==v2.x && v1.y==v2.y && v1.z==v1.z;
  }

  virtual void print(li_object  *o, i4_file_class *stream)
  {
    i4_3d_vector v=li_vect::get(o,0)->value();
    stream->printf("(vector %f %f %f)",v.x, v.y, v.z);
  }

  virtual char *name() { return "vector"; }

  virtual li_object *create(li_object *params, li_environment *env)
  {
    i4_3d_vector v;
    if (params)
    {
      v.x=(float)li_get_float(li_eval(li_car(params,env), env),env); params=li_cdr(params,env);
      v.y=(float)li_get_float(li_eval(li_car(params,env), env),env); params=li_cdr(params,env);
      v.z=(float)li_get_float(li_eval(li_car(params,env), env),env); params=li_cdr(params,env);
    }
	else
		{
		v.x=0;
		v.y=0;
		v.z=0;
		}
      
    return new li_vect(v);
  }


  virtual void save_object(i4_saver_class *fp, li_object *o, li_environment *env)
  {
    i4_3d_vector v=li_vect::get(o,env)->value();
    fp->write_float(v.x);
    fp->write_float(v.y);
    fp->write_float(v.z);

  }

  virtual li_object *load_object(i4_loader_class *fp, li_type_number *type_remap,
                                 li_environment *env)
  {
    i4_3d_vector v;
    v.x=fp->read_float();
    v.y=fp->read_float();
    v.z=fp->read_float();
    return new li_vect(v);
  }
};

li_automatic_add_type(li_vect_type_function_table, li_vect_type);


#ifndef __sgi
//SGI Mipspro doesn't like this form of operator delete. 
//Don't know how to do it. It (should) be unused anyway.
void li_object::operator delete(void *ptr, char *file, int line)
	{
	li_object *o = (li_object *)ptr;
    li_get_type(o->type())->free(o);
	li_cell_free(o);
	}
#endif



// lisp/msvc_lip.cpp
/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/


#ifdef _WINDOWS
static li_object *msvc_inspect=0;
static char FP_SAVE[108];

void msvc_lip()
{
  __asm {
    pushfd
    pushad
    fsave FP_SAVE
    frstor FP_SAVE    
  }

  lip(msvc_inspect);

  __asm {
    frstor FP_SAVE    
    popad
    popfd
  }
}
#endif

