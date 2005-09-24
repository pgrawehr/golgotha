/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef LI_VECT_HH
#define LI_VECT_HH


#include "lisp/li_types.h"
#include "math/vector.h"
#include "lisp/li_class.h"

class li_vect : public li_object
{
  friend class li_vect_type_function_table;
  i4_3d_vector v;
public:
  li_vect(i4_3d_vector _v) : li_object(LI_VECT), v(_v)
  { 
  }
  li_vect(i4_float x,i4_float y, i4_float z):li_object(LI_VECT),
	  v(x,y,z)
  {
  }

  //li_vect(i4_3d_vector *v) : v(v), li_object(li_vect_type) {}

  i4_3d_vector &value() { return v; }  
  static li_vect *get(li_object *o, li_environment *env)
  { check_type(o, LI_VECT, env); return ((li_vect *)o); }
  i4_float x()
  {
	  return v.x;
  }
  i4_float y()
  {
	  return v.y;
  }
  i4_float z()
  {
	  return v.z;
  }
} ;



struct li_vect_class_member : public li_class_member
{
  li_vect_class_member(char *name) : li_class_member(name) {}
  i4_3d_vector &operator()() { return li_vect::get(li_this->get(*this),0)->value(); }
};


#endif
