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
#include "file/file.h"

extern li_type_number li_file_type;
class li_file :
	public li_object
{
protected:
	i4_file_class * fp;
public:
	li_file(i4_file_class * f) :
		li_object(li_file_type),
		fp(f)
	{
	};
	li_file() :
		li_object(li_file_type),
		fp(0)
	{
	};


	i4_file_class *value()
	{
		return fp;
	}                                    //don't use references with files
	void set(i4_file_class * f)
	{
		delete fp;
		fp=f;
	}
	void free()
	{
		delete fp;
	}
	static li_file *get(li_object * o, li_environment * env)
	{
		check_type(o, li_file_type, env);
		return ((li_file *)o);
	}
} ;


/*
   struct li_vect_class_member : public li_class_member
   {
   li_vect_class_member(char *name) : li_class_member(name) {}
   i4_3d_vector &operator()() { return li_vect::get(li_this->get(*this),0)->value(); }
   };
 */


#endif
