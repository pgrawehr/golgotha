/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef LI_CLASS_HH
#define LI_CLASS_HH


#include "lisp/lisp.h"
#include "memory/fixed_array.h"


class li_class_type;
struct sym_var;


struct li_class_member
{
  const char *name;
  int offset;
  li_type_number class_type;
  li_symbol *sym;

  li_class_member(const char *name) : name(name), sym(0), class_type(0) {}
};

struct li_string_class_member : public li_class_member
{
  li_string_class_member(const char *name) : li_class_member(name) {}
  char *operator()();
};

struct li_int_class_member : public li_class_member
{
  li_int_class_member(const char *name) : li_class_member(name) {}
  int &operator()();
};


struct li_float_class_member : public li_class_member
{
  li_float_class_member(const char *name) : li_class_member(name) {}
  float &operator()();
};


struct li_symbol_class_member : public li_class_member
{
  li_symbol_class_member(const char *name) : li_class_member(name) {}
  li_symbol *&operator()();
};


struct li_object_class_member : public li_class_member
{
  li_object_class_member(const char *name) : li_class_member(name) {}
  li_object *&operator()();
};

//WARN: This class is very sophisticated and a lot of stuff goes on
//behind the walls. Don't modify anything unless you *really* know
//what it does. And remember to test your changes!!
class li_class : public li_object
{
  void **values;
  li_class_type *get_type() const { return (li_class_type *)li_get_type(type()); }
public:
  void mark(int set); 

  void save(i4_saver_class *fp, li_environment *env);
  void load(i4_loader_class *fp, li_type_number *type_remap, li_environment *env);
  void print(i4_file_class *fp);

  li_class(li_type_number class_type, li_object *params=0, li_environment *env=0);

  void free();

  //the following two functions are not inlined because of circular dependencies
  int member_offset(const char *sym) const;
	  //{
      //return get_type()->get_var_offset(li_get_symbol(sym), 0);
	  //};    // does not type check
  int member_offset(char *sym) const;
  int member_offset(li_symbol *sym) const;
	  //{
	  //return get_type()->get_var_offset(sym, 0);
	  //};    // does not type check

  int get_offset(li_class_member &m) const;
  int get_offset(const li_class_member &m) const;
  
  int get_offset(li_class_member &m, li_type_number _type) const;
  int get_offset(const li_class_member &m, li_type_number _type) const;

  int member_offset(li_class_member &m) const   // inline version checks to see to see if cached
  {
    if (m.class_type!=type())
      get_offset(m);
    return m.offset;
  }

  int member_offset(const li_class_member &m) const
	  {
	  if (m.class_type!=type())
		  get_offset(m);
	  return m.offset;
	  }

  int member_offset(li_class_member &m, li_type_number _type) const
  {
    if (m.class_type!=type())
      get_offset(m, _type);
    return m.offset;
  }

  int member_offset(const li_class_member &m, li_type_number _type) const
	  {
	  if (m.class_type!=type())
		  get_offset(m, _type);
	  return m.offset;
	  }



  char *string_value(int member) const { return ((li_string *)values[member])->value(); }
  int &int_value(int member) { return *(((int *) (values+member))); }
  float &float_value(int member) { return *(((float *) (values+member))); }
  li_symbol *&symbol_value(int member)  { return *((li_symbol **)(values+member)); }
  li_object *&object_value(int member)  { return *((li_object **)(values+member)); }
  li_object *object_itself(int member);
  li_object **object_place(int member);
  //warn: Don't even think of modifying the following functions!
  //Even a small change as inserting a const in the wrong place
  //can screw everything up!
  char *get(li_string_class_member &c) { return string_value(member_offset(c, LI_STRING)); }
  int &get(li_int_class_member &c)  {  return int_value(member_offset(c, LI_INT)); }
  float &get(li_float_class_member &c) { return float_value(member_offset(c, LI_FLOAT)); }
  li_symbol *&get(li_symbol_class_member &c) { return symbol_value(member_offset(c, LI_SYMBOL)); }
  li_object *&get(li_class_member &c) { return object_value(member_offset(c)); }
  li_object *get_member(const char *member) {return value(member);}
  li_object *get_member(li_class_member &c) {return value(c.name);}
  //WARN: The following function will give wrong results if used
  //on ints or floats.
  li_object *&get(const char *member){ return object_value(member_offset(member));}


  li_object *value(const char *member_name);
  li_object *value(char *member_name);
  li_object *value(int member);
  void set_value(int member, li_object *value);
  void set(li_class_member &c, li_object *value) { set_value(member_offset(c), value); }
  li_object *set(char *member_name, li_object *value); // slow, but easy way to access data


#ifdef _DEBUG
  static li_class *get(li_object *o, li_environment *env); 
  static li_class *get_all(li_object *o, li_environment *env);
#else
  static li_class *get(li_object *o, li_environment *env)
      {
      return (li_class*)o;
      }
  static li_class *get_all(li_object *o, li_environment *env)
      {
      if (li_get_type(o->type())->type==0)
          return 0;
      return (li_class*)o;
      }
#endif



};

li_object *li_def_class(li_object *fields, li_environment *env);

int li_class_total_members(li_type_number type);
li_symbol *li_class_get_symbol(li_type_number type, int member_number);
li_object *li_class_get_default(li_type_number type, li_symbol *sym);
li_object *li_class_get_property_list(li_type_number type, li_symbol *sym);
void li_set_class_editor(li_type_edit_class *editor);

extern li_class *li_this;

inline char *li_string_class_member::operator()() { return li_this->get((li_string_class_member &)*this); }
inline int &li_int_class_member::operator()() { return li_this->get(*this); }
inline float &li_float_class_member::operator()() { return li_this->get(*this); }
inline li_symbol *&li_symbol_class_member::operator()() { return li_this->get(*this); }
inline li_object *&li_object_class_member::operator()() { return li_this->get(*this); }

//just create an instance of this class (in the current stack frame)
//if you need the li_this to be set to the class instance for
//the current object
class li_class_context
{
protected:
  li_class *old_context;
public:
  li_class_context(li_class *current_context) 
  { 
    old_context=li_this;
    li_this=current_context; 
  }
//unfortunatelly, we now need a virtual destructor (slight performance gap)
  virtual ~li_class_context() { li_this=old_context; }
};

//this extended version also sets the variables needed to access
//the class variables from within lisp code.
//Important: The object must not be invalidated while this
//class exists (since it's always created on the stack, this
//should not happen) Don't wana use g1_reference_class here.
class g1_object_class;
class li_ext_class_context: public li_class_context
	{
	protected: 
		li_environment *n_env;//we need to save it so that
		//we can use it in the destructor
		g1_object_class *obj;
		i4_bool pos_changes_allowed;
	public:
		//Hint: This constructor changes the value of the environment
		//parameter passed. Just drop it if the function leaves scope.
		li_ext_class_context(li_class *current_context, 
			li_environment *&env)
			: li_class_context(current_context)
			{
			n_env=new li_environment(env,i4_T);
			li_set_value("this",li_this,n_env);
			env=n_env;
			pos_changes_allowed=i4_F;
			obj=0;
			};
		//no destructor required, I think, since the "this" gets automatically
		//droped when the new environment leaves scope.
		//... (10 mins later) We need it anyway, since we want to write back 
		//the member variables
		~li_ext_class_context();

		void write_back(li_environment *env, g1_object_class *to_obj);
		//special case if an object is present
		//declared in objs__def_object.cpp, since it logically belongs there
		li_ext_class_context(li_class *current_context, 
			li_environment *&env,
			g1_object_class *for_obj, 
			i4_bool pos_changes_allowed=i4_F);
	};
#endif
