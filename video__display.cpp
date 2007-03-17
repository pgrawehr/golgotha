/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/

#include "pch.h"
#include "video/display.h"
#include "device/kernel.h"
#include "error/error.h"
#include "init/init.h"

i4_display_list_struct *i4_display_list=0;


void i4_display_list_struct::add_to_list(char *_name, int _driver_id,
										 int _priority,
										 i4_display_class *_display,
										 i4_display_list_struct *_next)
{
	name=_name;
	driver_id=_driver_id;
	priority=_priority;
	display=_display;
	next=_next;
	i4_display_list=this;
}

void i4_display_list_struct::remove_from_list(char *_name)
{
	i4_display_list_struct *c,*n;
	c=i4_display_list;
	if (strcmp(c->name,_name)==0)
	{
		i4_display_list=c->next;
		delete c;
		return;
	}
	n=c->next;
	while (n) //we don't need to care about duplicate entries.
	{
		if (strcmp(n->name,_name)==0)
		{
			c->next=n->next;
			delete n;
			return;
		}
		c=c->next;
		n=n->next;
	}
}

class i4_display_list_deleter :
	public i4_init_class
{
	virtual void uninit()
	{
		//???? Why is this only for windows?
		//FIX: Fixed.
// #ifdef _WINDOWS
		i4_display_list_struct *temp;
		while (i4_display_list)
		{
			temp=i4_display_list;
			i4_display_list=i4_display_list->next;
			delete temp;
		}
// #endif
	}
	virtual int init_type()
	{
		return I4_INIT_TYPE_AFTER_ALL;
	}
	virtual void init()
	{
		i4_display_list_struct *iter;
		iter=i4_display_list;
		while(iter)
		{
			if (iter->name)
			{
				i4_warning("Display registered: %s",iter->name);
			}
			iter=iter->next;
		}
	}
} i4_display_list_deleter_instance;
