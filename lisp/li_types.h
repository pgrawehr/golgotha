/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef LI_TYPES_HH
#define LI_TYPES_HH



#include "error/error.h"
#include "lisp/li_alloc.h"
#include "string/string.h"
#include "lisp/li_error.h"
#include "memory/array.h"//for li_vector

#undef new

//#ifdef DEBUG  //As users can use lisp now, it's a good idea to have this allways enabled
//checking overhead shouldn't be critical
#define LI_TYPE_CHECK
//#endif


// default types available, type numbers for these types shouldn't be changed
enum { 
  LI_INVALID_TYPE,
  LI_SYMBOL,
  LI_STRING,
  LI_INT,
  LI_FLOAT,
  LI_LIST,
  LI_CHARACTER,
  LI_FUNCTION,
  LI_ENVIROMENT,
  LI_TYPE,
  LI_USER_FUNCTION,
  LI_VECTOR,//not to be mistaken for a li_vect (3d-vector only)
  LI_BIGNUM,
  LI_LAST_TYPE
};

extern li_symbol *li_nil;
class li_environment;
class i4_loader_class;
class i4_saver_class;
typedef w32 li_type_number;

char *li_get_type_name(li_type_number type);

// base object from which all object instances in li_ should be derived
// all objects derived should add 4 or 8 bytes to the structure.  li_objects cannot
// contain virtual functions.

//#define GC_FLAG (1<<31) 
class li_object
{  
protected:
  li_type_number _type;
public:
  enum { GC_FLAG=1<<30 }; //ICC seems not to be sure wheter this should be 
    //signed or unsigned
  i4_bool is_marked() const { if (_type & GC_FLAG) return i4_T; else return i4_F; }
  void mark(int set) { if (set) _type |= GC_FLAG; else _type &= ~GC_FLAG; }
  void mark_free() { _type=LI_INVALID_TYPE; }

  li_object(li_type_number type) { _type=type; }
  li_type_number type() const { return _type; }
  li_type_number unmarked_type() const { return _type & (~GC_FLAG); }

  static void check_type(li_object *o, li_type_number type, li_environment *env)
  {
#ifdef LI_TYPE_CHECK
	if (!o||(((li_symbol*)o)==li_nil)) return;//nil is always a valid type
	
    if (o->type()!=type)
      li_error(env, "USER: Expecting type %s, but got %O",
               li_get_type_name(type),o);
#endif
  }

  void *operator new(size_t size)
  { 
#ifdef _DEBUG
    if (size>16) 
      i4_error(0, "li_objects should be <=16 bytes");
#endif
    return li_cell_alloc(size); 
  }

  void *operator new(size_t size, char *file, int line)
  { 
#ifdef _DEBUG
    if (size>16) 
      i4_error(0, "li_objects should be <=16 bytes");
#endif
    return li_cell_alloc(size); 
  }

  void operator delete(void *ptr);
#ifndef __sgi
  //SGI MIPSPRO doesn't support placement delete.
  void operator delete(void *ptr, char *file, int line);
#endif
};
/*
class li_virtual_test: public li_object
	{//virtual functions in a derived class are possible even if base has none.
	virtual void somevirtualfn();
	virtual int anothervirtualfn(int x);
	int amember;
	li_virtual_test():li_object(0){amember=0;}
	};
*/
// the li_ system does not know about gui stuff directly, it goes through this type_edit_class
// which can be added to each type later
class i4_window_class;   
class li_type_edit_class;

// if you want to add a new type into the system, implement one of these
// and call li_add_type
class li_type_function_table
{ 
public:
  li_type_edit_class *editor;
  //if non-zero, this is a class type (and it should(?) be safe
  //to cast it to li_class_type and the corresponding object to li_class
  int type;

  // mark any data you the type have
  virtual void mark(int set) { ; }

  // mark any data an instance of this type has
  virtual void mark(li_object   *o, int set) { o->mark(set); }

  // free data associated with an instance of this type
  virtual void free(li_object   *o) { ; } // during free, you will not be marked

  virtual int equal(li_object  *o1, li_object *o2) { return o1==o2; }
  virtual void print(li_object  *o, i4_file_class *stream) = 0;
  //! name is used to sync types across a network & saves and for dynamic object
  //creation using (new ...)
  virtual char *name() = 0;   

  //! This is needed by new. Implement it for all types that can be created
  // using new. 
  virtual li_object *create(li_object *params, li_environment *env) { return 0; }

  // these load and save type information
  virtual void save(i4_saver_class *fp, li_environment *env) { ; }
  virtual void load(i4_loader_class *fp, li_type_number *type_remap, li_environment *env) { ; }
  virtual void load_done() { ; }

  // load & save type instance information
  virtual void save_object(i4_saver_class *fp, li_object *o, li_environment *env) = 0;
  virtual li_object *load_object(i4_loader_class *fp, li_type_number *type_remap,
                                 li_environment *env) = 0;

  li_type_function_table() { editor=0; type=0;}
  virtual li_object *copy(li_object *o) //make a copy of self
	  {
	  //just returns o if copying is not meaningfull or allowed (i.e types, symbols)
	  return o;
	  }
  virtual ~li_type_function_table() { ; }
};

extern li_type_function_table **li_types;

// return type number for type
// if type is not anonymous it's name's symbol value will be set to the type number
int li_add_type(li_type_function_table *&type_functions, 
                li_environment *env=0,
                int anonymous=0);

void li_remove_type(int type_num);
void li_cleanup_types();

li_type_function_table *li_get_type(li_type_number type_num);
inline li_type_function_table *li_get_type(li_object *obj)
	{
	return li_get_type(obj->type());
	}
li_type_number li_find_type(char *name, li_environment *env=0);
li_type_number li_find_type(char *name, li_environment *env, li_type_number &cache_to);
i4_bool li_valid_type(li_type_number type_number);
int li_max_types();    // returns the highest type number currently registered


inline void li_object::operator delete(void *ptr)
{  
  li_object *o = (li_object *)ptr;

  li_get_type(o->type())->free(o);
  li_cell_free(o);
}

class li_string : public li_object
{
  char *_name;
public:
  li_string(i4_file_class *fp);

  li_string(const char *name);
  li_string(int len);
  li_string(const i4_const_str &str);

  char *value() const { return _name; }

  static li_string *get(li_object *o, li_environment *env) 
  { check_type(o, LI_STRING, env); return ((li_string *)o); }
};


enum 
	{
	LSF_NONE=0,
	LSF_VALUECONSTANT=1,
	LSF_SPECIALFORM=2,//special-form-implementation tells li_lambda_eval not to quote any plain symbols on NOEVALONBIND
	LSF_MACRO=4,
	LSF_KEYWORD=8,
	LSF_FUNCTIONCONSTANT=16,
	LSF_FORCECONSTANT=32  //for symbols such as t or nil. Bug(?) in clisp: (defconstant t nil) actually works!!!
	};

//IMPORTANT NOTE: functions, wich require one arg to be of type value (neither a list nor a symbol) may want to
//perform eval again if first eval leads to a symbol. This is for situations like (mapcar '+ '(2 x y) '(4 5 6))
//clisp cannot handle this case.
//a workaround is to change li_get_int() in such a way to allow the param to be of type (quote sym)

//This definition is inlined since lisp/lisp.h depends on this file and
//including lisp.h in this file therefore doesn't work.
li_object *li_call(li_symbol *fun_name, li_object *params=0, li_environment *env=0);

class li_symbol : public li_object
{
public:
  struct symbol_data
  {
    li_object *_value;//the value of the symbol
    li_object *_fun;//the function of the symbol
    li_string *_name;//its name
	li_object *_proplist; //the associated property list
	w32	_flags;//special purpose flags (constant, special form...)
    li_symbol *left, *right;//used to build the symbol table
    
    symbol_data(li_string *name) { _value=0; _fun=0; _name=name; left=0; right=0; _flags=0;_proplist=0;}
	symbol_data(li_string *name,w32 flags) {_value=0; _fun=0; _name=name;left=0; right=0;_flags=flags;_proplist=0;}
  };

private:
  symbol_data *data;

public:  
	void free(){delete data;}
  li_symbol *left() { return data->left; }//get left child (symtable only)
  li_symbol *right() { return data->right; }//get right child (symtable only)
  li_symbol *set_left(li_symbol *left) { return data->left=left; }//don't use directly
  li_symbol *set_right(li_symbol *right) { return data->right=right; }//don't use directly

  li_symbol(li_string *name) : li_object(LI_SYMBOL), data(new symbol_data(name)) {}
  li_symbol(li_string *name,w32 flags): li_object(LI_SYMBOL), data(new symbol_data(name,flags)) {}

  li_object *fun() const { return data->_fun; }//return function [use li_get_fun(sym,env) instead]
  li_object *value() const { return data->_value; }//return value [use li_get_val(sym,env) instead]
  li_string *name() const { return data->_name; }
  li_object *operator()(li_object *o, li_environment *env)
	  {return li_call(this,o,env);}
  w32 flags() { return data->_flags; }
  void set_flags(w32 fl) {data->_flags=fl;}
  void set_value(li_object *value) { data->_value=value; }
  void set_fun(li_object *fun) { data->_fun=fun; }
  void add_property(li_object *name, li_object *value);
  li_object *get_property(li_object *name);
  li_object *get_plist(){return data->_proplist;}
  void set_plist(li_object *l){data->_proplist=l;}

  static li_symbol *get(li_object *o, li_environment *env) 
  { check_type(o, LI_SYMBOL, env); return ((li_symbol *)o); }

  int compare(const li_symbol *a) const { return strcmp(name()->value(), a->name()->value()); }

};


typedef li_object *(*li_function_type)(li_object *o, li_environment *env);

class li_function : public li_object
{
  li_function_type _fun;
  w16 _minargs,_maxargs;
public:
  li_function(li_function_type fun)  : li_object(LI_FUNCTION), _fun(fun) 
	  {_minargs=0xffff;_maxargs=0xffff;};
  li_function_type value() { return _fun; }
  void set_arg_count(w16 minargs,w16 maxargs)
	  {_minargs=minargs;_maxargs=maxargs;};
  w16 get_minargs(){return _minargs;};
  w16 get_maxargs(){return _maxargs;};
  static li_function *get(li_object *o, li_environment *env) 
  { check_type(o, LI_FUNCTION, env); return (li_function *)o; }
};

class li_user_function : public li_object
	{
	public:
		struct user_function_data{
			li_function_type _fun;//the common code interpreter
			li_object *_params;//param list
			li_object *_bindings;//bound params
			li_environment *_locals;//environment to call function in.
			li_object *_code;//Compiled code
			char *_plaincode;//the plain text of the function body
			li_string *_name;//function name (if known)
			li_object *_reserved;//some intermediate values
			w32 fnumber;//for abuse functions
			user_function_data(li_function_type fun,li_object *params,li_environment *env,
				char *plaincode, li_object *code, li_string *name)
				{
				_fun=fun;
				_params=params;
				_locals=env;
				_plaincode=plaincode;
				_code=code;
				_name=name;
				_bindings=0;
				_reserved=0;
				fnumber=0;
				};
			};
	private:
		user_function_data *_data;
	public:
		li_user_function(li_function_type fun,li_object *params, li_environment *env,
			char *plaincode, li_object *code, li_string *name):
			li_object(LI_USER_FUNCTION)
				{
				_data=new user_function_data(fun,params,env,plaincode,code,name);
				};
		li_function_type value() {return _data->_fun;};//others only on special request
		static li_user_function *get(li_object *o,li_environment *env)
			{check_type(o,LI_USER_FUNCTION,env); return ((li_user_function *)o);}
		//void free();
		//void mark(int set);
		li_object *params(){return _data->_params;};
		li_object *bindings(){return _data->_bindings;};
		li_environment *locals(){return _data->_locals;};
		li_string *name(){return _data->_name;};
		struct user_function_data *data(){return _data;};
		void cleanup(){
			if (_data->_plaincode) delete _data->_plaincode;
			delete _data;_data=0;
			};
	};


class li_bignum: public li_object 
	{
	//public:
	//	struct li_bignum_struct{
	//		w32 _length;
	//		char *_num;
	//		li_bignum_struct(w32 length, char *data):_length(length),_num(data){};
	//		}
	private:
		char *num;//num[0] is the most significant digit, num[length-1] the least significant
		w32 length;//must allways be strictly greater than 1
		char mysignum;//0=positive, everything else= negative
	public:
		//number must be given in unpacked binary coded decimal format (not ascii)
		li_bignum(w32 _length, char *number, char _signum=0)
			:li_object(LI_BIGNUM)
			{
			mysignum=_signum;
			length=_length;
			num=new char[length];
			memcpy(num,number,length);
			};
		char *value() {return num;};
		w32 get_length(){return length;}
		char get_signum(){return mysignum;};
		void cleanup(){delete[] num;};
		static li_bignum *get(li_object *o,li_environment *env)
			{
			check_type(o,LI_BIGNUM, env); return ((li_bignum *)o);
			}
	};


class li_int : public li_object
{
  int _x;
  //char *_longdata;//prepare the type to take long ints
public:
  li_int(int x) : li_object(LI_INT) { _x=x;}
  int value() { return _x; }  
  static li_int *get(li_object *o, li_environment *env) 
  { check_type(o, LI_INT, env); return ((li_int *)o); }
} ;



class li_type : public li_object
{
  li_type_number _x;
public:
  li_type(li_type_number x) : li_object(LI_TYPE) { _x=x; }
  li_type_number value() { return _x; }  
  static li_type *get(li_object *o, li_environment *env) 
  { check_type(o, LI_TYPE, env); return ((li_type *)o); }
} ;

//this class remembers the position of the float
//class li_float_ref :public li_object ... 

class li_float : public li_object
{
  //float _x;//float would perfectly match, 
  //but we have big trouble on implicit conversions from int to float (not exact)
  double _x;
public:
	
  li_float(double x) : li_object(LI_FLOAT) { _x=x;}
  double value() { return _x; }  
  static li_float *get(li_object *o, li_environment *env) 
  { check_type(o, LI_FLOAT, env); return ((li_float *)o); }
} ;

class li_character : public li_object
{
  w32 _ch;
public:
  li_character(w8 ch) : li_object(LI_CHARACTER) {  _ch=ch; }
  w8 value() { return (w8)_ch; }  
  static li_character *get(li_object *o, li_environment *env)
  { 
    check_type(o, LI_CHARACTER, env); 
    return ((li_character *)o); 
  }
} ;
typedef i4_array<li_object*> li_object_vect_type;
class li_vector: public li_object
	{
	//friend class li_memory_manager_class;
	
	private:
		li_object_vect_type *_vector;
	public:
		void cleanup(){delete _vector;}//called by mark()
		li_vector():li_object(LI_VECTOR)//create an object
			{_vector=new i4_array<li_object*>(128,128);}
		li_vector(li_object *first):li_object(LI_VECTOR)
			{
			_vector=new i4_array<li_object*>(128,128);
			_vector->add(first);
			}
		li_object *element_at(int l)//get the element at the location l
			{
			return _vector->operator [](l);//I newer thought I would once use this syntax...
			}
		li_object *add_element(li_object *o)//add element at the end
			{_vector->add(o); return this;}
		li_object *add_element_at(li_object *o,int l)
			{_vector->add_at(o,l); return this;}
		li_object *update_at(li_object *o,int l)//change entry l to o
			{_vector->operator [](l)=o;return this;}
		i4_array<li_object*> *reserved()//returns the vector. Be VERY carefull and use only pointers to this, nothing that might destroy the object.
			{return _vector;}
		int size(){return _vector->size();}//returns number of entries in vector
		li_object *data(){return _vector->operator [](0);};//for compatiblity only: returns first entry
		li_object *remove(int l){_vector->remove(l);return this;};
		static li_vector *get(li_object *o, li_environment *env)
			{check_type(o, LI_VECTOR,env); return (li_vector *)o;}
	};


class li_free8_list: public li_object
	{
	friend class li_memory_manager_class;
	friend class memory_block_list;
	//used for the small (8-byte) free list
	li_free8_list *next_free;
	li_free8_list *get_next_free() {return next_free;}
	void set_next_free(li_free8_list *nf) {next_free=nf;}
	public:
	li_free8_list():li_object(LI_INVALID_TYPE){next_free=0;}//actually, 
		//this constructor should newer be called
	};
//#define MASSIVE_DEBUG

class li_list : public li_object
{
	//also used as placeholder for free-list in memman (16-Byte cells)
  friend class li_memory_manager_class;
  friend class memory_block_list;
  friend void li_eval_lvalue(li_object *o, li_environment *env, li_object *update_width);

  /*struct list_data
  {
    li_object *_data, *_next;
    list_data(li_object *data, li_object *next) : _data(data), _next(next) {}
  } *_data;
  */
  li_object *_data, *_next, *_res;//lisp-objects have become up to 16 bytes in size (speedup)
  li_list *get_next_free() { return (li_list *)_data; }
  void set_next_free(li_list *next_free) 
	  {
#ifdef MASSIVE_DEBUG
	  if (next_free==this)
		  li_error(0,"SEVERE: Circular list generated");
#endif
	  _data=next_free; 
	  }

public: 
  void cleanup() { /*delete _data*/; }

  li_list(li_object *data, li_object *next=0) 
    : li_object(LI_LIST), _data(data) , _next(next), _res(0)
	  {
#ifdef MASSIVE_DEBUG
	  if ((data==this)||(next==this))
		  li_error(0,"SEVERE: Using list the wrong way.");
#endif
	  }

  //seems to be VERY strange bellow, but these are nonvirtual functions, so this works.
  li_object *data() { if (this&&(this!=(li_object*)li_nil)) return _data; else return 0;}//for car
  li_object *next() { if (this&&(this!=(li_object*)li_nil)) return _next; else return 0;}//for cdr

  void set_next(li_object *next) 
	  {
#ifdef MASSIVE_DEBUG
	  if ((next==this)||(_type==0))
		  li_error(0,"SEVERE: Setting something wrong here.");
#endif
	  _next=next; 
	  }//set cdr
  void set_data(li_object *data) 
	  {
#ifdef MASSIVE_DEBUG
	  if ((data==this)||(_type==0))
		  li_error(0, "SEVERE: Setting something completelly wrong here");
#endif
	  _data=data; 
	  }//set car

  static li_list *get(li_object *o, li_environment *env) 
  { check_type(o, LI_LIST,env); return (li_list *)o; }
};


class li_environment : public li_object
{ 
  struct value_data
  {
    li_symbol *symbol;
    li_object *value;
    value_data *next;
  };

  struct fun_data
  {
    li_symbol *symbol;
    li_object *fun;
    fun_data  *next;
  };

  struct env_data
  {
    value_data     *value_list;
    fun_data       *fun_list;
    li_environment *next;
    li_symbol      *current_function;   // set by li_call
    li_object      *current_args;
    i4_bool        local_namespace;


    env_data(li_environment *top_env, i4_bool local_namespace) 
      : next(top_env), local_namespace(local_namespace)
    
    { value_list=0; fun_list=0; current_function=0; current_args=0; }

  } *data;


public:
  li_symbol *&current_function();
  li_object *&current_arguments();


  li_environment(li_environment *top_env, i4_bool local_namespace)
    : li_object(LI_ENVIROMENT), data(new env_data(top_env, local_namespace)) {}

  //insert ourselves in the environment list, returns new environment
  //(either this or _next, if this is already in list)
  li_environment *set_next(li_environment *_next);
	  

  li_object *value(li_symbol *s);
  li_object *fun(li_symbol *s);

  void set_value(li_symbol *s, li_object *value);//changes symbol in the nearest env, newer creates new symbol
  void set_fun(li_symbol *s, li_object *fun);
  void define_value(li_symbol *s, li_object *value);//changes symbol in current environment, regardless of preexisting ones

  // don't call these functions directly!
  void mark(int set);
  void print(i4_file_class *stream);
  void free();

  static li_environment *get(li_object *o, li_environment *env)
  { check_type(o, LI_ENVIROMENT, env); return (li_environment *)o; }

  void print_call_stack(i4_file_class *fp);
};


#include "memory/new.h"      // make sure old definition of new is redeclared

#endif
