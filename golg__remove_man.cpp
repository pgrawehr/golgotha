/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/
#include "pch.h"
#include "map.h"
#include "memory/array.h"
#include "error/error.h"
#include "remove_man.h"
#include "g1_object.h"

//This is no more constant, as it may grow if there's a really big mess going on.
//Well, testing a better solution right now.
//const int MAX_REMOVES = 100;

g1_remove_manager_class g1_remove_man;


void g1_remove_manager_class::request_remove(g1_object_class * obj)
{
	//if (removes==(MAX_REMOVES-1))
	//	return; //Ignore request, will process it later.
	if (obj->get_flag(g1_object_class::MAP_OCCUPIED))
	{
		i4_warning("INTERNAL: Attempt to delete object while still on map");
		obj->unoccupy_location();
		obj->request_remove();
		return;
	}
#if 0
	if (!g1_global_id.check_id(obj->global_id))
	{
		i4_error("INTERNAL: Attempt to remove a piece with invalid id (name=%s)!", obj->name());
		return;
	}
#endif

	obj->flags |= g1_object_class::DELETED;

	if (obj->flags & g1_object_class::THINKING)
	{
		obj->stop_thinking();
	}
	remove_array.add(obj);
	//if (removes<MAX_REMOVES)
	//	{
	//	remove_list[removes] = obj;
	//	removes++;
	//	}
	//else
	//	{
	/*g1_object_class **larger_list=0;
	   MAX_REMOVES*=2;
	   larger_list=(g1_object_class**) I4_MALLOC(MAX_REMOVES*sizeof(g1_object_class*),
	   	"larger_remove_list");
	   I4_ASSERT(larger_list,"g1_remove_manager_class - Out of Memory");*/
	//	}


}

void g1_remove_manager_class::init()
{
	//remove_list =
	//	(g1_object_class**)I4_MALLOC( MAX_REMOVES*sizeof(g1_object_class*), "remove_list" );

	//I4_ASSERT(remove_list,
	//	"g1_remove_manager_class - Couldn't create removal list!");

	//removes = 0;
	remove_array.reallocate(100,100);
}


void g1_remove_manager_class::uninit()
{

	I4_TEST(remove_array.size()==0, "g1_remove_manager - Not all removals processed!");
	remove_array.uninit();

	//i4_free(remove_list);
}

int objs_sorter(g1_object_class * const * a,g1_object_class * const * b)
//no I do NOT know what exactly the above const's mean, but the compiler wants it like this
{
	if (*a>*b)
	{
		return 1;
	}
	else if(*a<*b)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}

void g1_remove_manager_class::process_requests()
{
	//I4_ASSERT(removes<MAX_REMOVES,
	//	"g1_remove_manager - Removes previously exceeded maximum removes");
	remove_array.sort(objs_sorter);
	for (w32 j=0; j<(w32)remove_array.size(); j++)
	{
		g1_object_class * remobj=remove_array[j];
		if (j>0&& remobj==remove_array[j-1])
		{
			continue;
		}            //skip if same object requested more than one delete
		i4_isl_list<g1_reference_class> *ref_list = &remove_array[j]->ref_list;

		while (!ref_list->empty())
		{
			i4_isl_list<g1_reference_class>::iterator i=ref_list->begin();
			ref_list->erase();

			i->ref = 0;
		}
		for (int k=0; k<g1_objs_in_view_dyn.size(); k++)
		{
			if (g1_objs_in_view_dyn[k]==remobj)
			{
				g1_objs_in_view_dyn[k]=0;
			}
		}
		delete remove_array[j];
	}
	remove_array.clear();

	//removes = 0;
}
