/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/

//This is a new file.
//it wraps the abuse lisp - functions to the golg-format
//i.e CAR(v) becomes li_car(v,env)

enum {
	PERM_SPACE,
	TMP_SPACE,
	USER_SPACE,
	GC_SPACE
};

#define CAR(v) li_car((li_object *)v,env)
#define lcar(v) li_car((li_object *)v,env)
#define CDR(v) li_cdr((li_object *)v,env)
#define lcdr(v) li_cdr((li_object *)v,env)

#define cons_cell li_object
#define Cell li_object
#define eval(p) li_eval((li_object *)p,env)

#define true_symbol li_true_sym
typedef long ltype;
#define lisp_symbol li_symbol
#define lisp_number li_int
#define lisp_fixed_point li_float //no other implementation currently available
#define lisp_character li_character
#define lisp_1d_array li_vector
#define lisp_string li_string
#define ushort w16

#define item_type(x) (((li_object *)x)->type())

enum
{
	L_BAD_CELL=LI_INVALID_TYPE,     // error catching type
	L_CONS_CELL=LI_LIST, L_NUMBER=LI_INT, L_SYMBOL=LI_SYMBOL,     L_SYS_FUNCTION=LI_FUNCTION,
	L_USER_FUNCTION=LI_USER_FUNCTION,
	L_STRING=LI_STRING, L_CHARACTER=LI_CHARACTER, L_C_FUNCTION=LI_FUNCTION, L_C_BOOL=LI_FUNCTION,
	L_L_FUNCTION=LI_FUNCTION,    /*L_POINTER=104,*/
	/*L_OBJECT_VAR=100, */ L_1D_ARRAY=LI_VECTOR,
	L_FIXED_POINT=LI_FLOAT    /* , L_COLLECTED_OBJECT=103*/
};
extern int li_num_sys_functions;

void *set_symbol_value(void *symbol, void *value, li_environment *env);
void *symbol_value(void *symbol, li_environment *env);
struct lisp_sys_function
{
	ltype type;
	short min_args,max_args,
		  fun_number;
} ;
/*
   template<class T> class grow_stack        // stack does not shrink
   {
   public :
   T **sdata;
   long son;

   grow_stack(int max_size) { sdata=(T **)jmalloc(max_size,"pointer stack");  son=0; }
   void push(T *data)
   {
   	sdata[son]=data;
   	son++;
   }

   T *pop(long total)
   { if (total>son) { lbreak("stack underflow\n"); exit(0); }
   	son-=total;
   	return sdata[son];
   }
   void clean_up()
   {
   	if (son!=0) fprintf(stderr,"Warning cleaning up stack and not empty\n");
   	jfree(sdata);
   	sdata=NULL;  son=0;
   }
   } ;


   extern grow_stack<void> l_user_stack;       // stack user progs can push data and have it GCed
   extern grow_stack<void *>l_ptr_stack;      // stack of user pointers, user pointers get remapped on GC
 */

/*class p_ref {
   public :
   p_ref(void *&ref) { l_ptr_stack.push(&ref); }
   ~p_ref() { l_ptr_stack.pop(1); }
   } ;*/

//I decided to replace the fixed point type by floats
//if there's a math coprocessor available, it's no big deal.
//who will run this on a 80486 or older?

//#define FIXED_TRIG_SIZE 360               // 360 degrees stored in table
//extern long sin_table[FIXED_TRIG_SIZE];   // this should be filled in by external module
//#define TBS 1662                          // atan table granularity
//extern unsigned short atan_table[TBS];
