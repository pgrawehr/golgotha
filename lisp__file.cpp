// lisp/li_file.cpp
/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/
//this is a new file
#include "pch.h"
#include "init/init.h"
#include "lisp/li_file.h"
#include "lisp/lisp.h"
#include "file/file.h"
#include "loaders/dir_save.h"
#include "loaders/dir_load.h"
#include "lisp/li_init.h"

li_type_number li_file_type=0;
li_type_number li_loader_file_type=0;
li_type_number li_saver_file_type=0;
class li_file_type_function_table :
	public li_type_function_table
{
public:
	// free data associated with an instance of this type
	virtual void free(li_object *o)
	{
		li_file::get(o,0)->free();
	}

	virtual int equal(li_object *o1, li_object *o2)
	{
		//not very useful on files (they have no name!)
		i4_file_class *v1=li_file::get(o1,0)->value(), *v2=li_file::get(o2,0)->value();
		return v1==v2;
	}

	virtual void print(li_object *o, i4_file_class *stream)
	{
		i4_file_class *v=li_file::get(o,0)->value();
		stream->printf("<#file address %i>",v);
	}

	virtual char *name()
	{
		return "file";
	}

	/*LISPFUN*
	   Purpose: Create a new "li_file" object
	   Special: Is a constructor
	   Syntax: (new li_file [[file_name] flags])
	   Paramdesc:
	   file_name: a string describing the file to be opened
	   Use preferably relative file names
	   flags: an expression that evaluates to some combination of
	   I4_READ(1)
	   I4_WRITE(2)
	   I4_APPEND(4)
	   I4_NO_BUFFER(8)
	   I4_SUPPORT_ASYNC(16)
	   Example:
	   (setf myfile (new file "text.txt" (or I4_READ I4_WRITE)))

	 */
	virtual li_object *create(li_object *params, li_environment *env)
	{
		i4_file_class *v=0;
		w32 flags=0;
		if (params)
		{
			li_string *s=li_string::get(li_eval(li_first(params,env),env),env);
			flags=li_get_int(li_eval(li_second(params,env),env),env);
			v=i4_open(i4_const_str(s->value()),flags);

		}

		return new li_file(v);
	}

	void save_object(i4_saver_class *fp,li_object *o, li_environment *env)
	{
		//cannot save filepointers to disk
		return;
	}
	li_object *load_object(i4_loader_class *fp,li_type_number *type_remap,li_environment *env)
	{
		return new li_file(0);
	}

};

li_automatic_add_type(li_file_type_function_table, li_file_type);

LI_HEADER(close_file) {
	li_file *f=li_file::get(li_eval(li_first(o,env),env),env);
	if (f)
	{
		f->set(0);
	}
	return li_nil;
}

LI_HEADER(is_open) {
	li_file *f=li_file::get(li_eval(li_first(o,env),env),env);
	if (f && f->value())
	{
		return li_true_sym;
	}
	else
	{
		return li_nil;
	}
}

LI_HEADER(rename_file) {

	//li_string *old=li_string::get(li_first(o,env),env);
	//li_string *n=li_string::get(li_second(o,env),env);
	li_error(env,"UNIMPLEMENTED: The desired function rename-file is unimplemented.");
	return li_nil;
}

LI_HEADER(delete_file) {
	li_string *f=li_string::get(li_eval(li_first(o,env),env),env);
	return i4_unlink(f->value()) ? li_true_sym : li_nil;
}

LI_HEADER(probe_file) {
	li_string *f=li_string::get(li_eval(li_first(o,env),env),env);
	i4_file_status_struct fstat;
	return i4_get_status(f->value(),fstat) ? li_true_sym : li_nil;
}

LI_HEADER(file_position) {
	li_file *f=li_file::get(li_eval(li_first(o,env),env),env);
	if (f&&f->value())
	{
		if (li_second(o,env))
		{
			w32 p=li_get_int(li_eval(li_second(o,env),env),env);
			//should accept keywords :end or :start
			f->value()->seek(p);
			return li_true_sym;
		}
		else
		{
			return new li_int(f->value()->tell());
		}
	}
	return li_nil;
};

LI_HEADER(streamp) {
	li_type_number t=li_eval(li_first(o,env),env)->type();
	if (t==li_file_type||t==li_loader_file_type||t==li_saver_file_type)
	{
		return li_true_sym;
	}
	return li_nil;
}

/*LISPFUN*
   	Name:read-byte, read-8
   	Description: Reads a byte from the given file
   	Syntax: read-byte filestream [t]
   	If the third parameter is t, the returned value is converted to signed
 */
LI_HEADER(read_byte) {
	li_file *f=li_file::get(li_eval(li_first(o,env),env),env);
	w32 w;
	if (li_eval(li_second(o,env),env)==li_true_sym)
	{
		w=f->value()->read_8();
		if (w>=128)
		{
			w=w &0xffffff00;
		}

	}
	else
	{
		w=f->value()->read_8();
	}
	return new li_int(w);

}

/*LISPFUN*
   	Name: read-16
   	Description: Reads a 16bit value from the given file
   	Syntax: read-16 filestream [t]
   	If the third parameter is t, the returned value is converted to signed
 */
LI_HEADER(read_16) {
	li_file *f=li_file::get(li_eval(li_first(o,env),env),env);
	w32 w;
	if (li_eval(li_second(o,env),env)==li_true_sym)
	{
		w=f->value()->read_16();
		if (w>=128)
		{
			w=w &0xffff0000;
		}

	}
	else
	{
		w=f->value()->read_16();
	}
	return new li_int(w);

}

/*LISPFUN*
   	Name: read-32
   	Description: Reads a 16bit value from the given file
   	Syntax: read-32 filestream

 */
LI_HEADER(read_32) {
	li_file *f=li_file::get(li_eval(li_first(o,env),env),env);
	w32 w;
	w=f->value()->read_32();
	return new li_int(w);

}

class li_file_init_class :
	public i4_init_class
{
public:
	int init_type()
	{
		return I4_INIT_TYPE_LISP_FUNCTIONS;
	}
	void init()
	{
		li_add_function("is-open",li_is_open);
		li_add_function("open-stream-p",li_is_open);
		li_add_function("close-file",li_close_file);
		li_add_function("delete-file",li_delete_file);
		li_add_function("probe-file",li_probe_file);
		li_add_function("file-position",li_file_position);
		li_add_function("read-byte",li_read_byte);
		li_add_function("read-8",li_read_byte);
		li_add_function("read-16",li_read_16);
		li_add_function("read-32",li_read_32);
	}
} li_file_init_class_instance;
