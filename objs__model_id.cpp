#include "pch.h"

/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/

#include "objs/model_id.h"
#include "string/string.h"
#include "error/alert.h"
#include "error/error.h"
#include "load3d.h"
#include "obj3d.h"
#include "global_id.h"
#include "render/r1_api.h"
#include "status/status.h"
#include "time/profile.h"
#include "lisp/lisp.h"

i4_profile_class pf_load_models("load_models");
g1_model_list_class g1_model_list_man;

g1_model_ref * model_references=0;


g1_model_ref::g1_model_ref(char * name)
{
	next = model_references;
	model_references = this;
	set_name(name);
}

void g1_model_ref::set_name(char * _name)
{
	name=_name;
	value=g1_model_list_man.find_handle(name);
}

g1_model_ref::~g1_model_ref()
{
	//   i4_debug->printf("cleaning up for %s\n", name);

	//   for (g1_model_ref *m=model_references; m; m=m->next)
	//     i4_debug->printf("%s ", m->name);
	//   i4_debug->printf("\n");


	if (model_references==this)
	{
		model_references = next;
	}
	else
	{
		g1_model_ref * p;

		for (p = model_references; p->next && p->next!=this; p=p->next)
		{
			;
		}

		if (!p->next)
		{
			i4_error("model reference not in list");
		}
		else
		{
			p->next = p->next->next;
		}
	}
}


i4_grow_heap_class * g1_object_heap=0;


int g1_model_info_compare(const void * a, const void * b)
{
	return strcmp(((g1_model_list_class::model_info *)a)->name_start,
				  ((g1_model_list_class::model_info *)b)->name_start);
}

void g1_model_list_class::free_array()
{
	for (int act=0; act<total_models; act++)
	{
		array[act].model->~g1_quad_object_class();
		array[act].model=0;
	}
}

void g1_model_list_class::cleanup()
{
	free_array();
	if (g1_object_heap)
	{
		delete g1_object_heap;
	}

	if (name_buffer)
	{
		delete name_buffer;
	}

	if (array)
	{
		i4_free(array);
		array=0;
	}
}

static i4_profile_class pf_model_load_open("models:open");

void g1_model_list_class::scale_models(i4_float to)
{
	for (int act=0; act<total_models; act++)
	{
		array[act].model->scale(1/model_scaling); //restore to original
		array[act].model->scale(to);
	}
	model_scaling=to;
}

void g1_model_list_class::reset(i4_array<i4_str *> &model_names, r1_texture_manager_class * tmap)
{
	free_array();
	if (g1_object_heap)
	{
		g1_object_heap->clear();
	}

	if (name_buffer)
	{
		name_buffer->clear();
	}

	if (array)
	{
		i4_free(array);
		array=0;
	}

	pf_load_models.start();

	//"Loading models..."
	i4_status_class * stat=i4_create_status(i4gets("loading_models"));

	total_models=model_names.size();
	array=(model_info *)I4_MALLOC(total_models * sizeof(model_info), "model list");

	int actual_total=0;
	g1_quad_object_loader_class loader(g1_object_heap);
	char nbuf[MAX_PATH];
	for (int i=0; i<model_names.size(); i++)
	{
		if (stat)
		{
			stat->update(i/(float)model_names.size());
		}

		//The default scale.
		model_scaling=0.1f;

		pf_model_load_open.start();
		//A string of the form "objects/%s.gmod"
		li_object * fmt=li_get_value("object_format", 0);
		char * n=li_string::get(fmt,0)->value();
		sprintf(nbuf,n,model_names[i]->c_str());

		i4_file_class * in_file=i4_open(nbuf);
		if (in_file)
		{
			g1_loader_class * fp=g1_open_save_file(in_file);
			pf_model_load_open.stop();
			if (fp)
			{
				array[actual_total].model=loader.load(fp, *model_names[i], tmap);

				if (array[actual_total].model)
				{
					i4_filename_struct fn;
					i4_split_path(*model_names[i], fn);

					array[actual_total].model->scale(model_scaling);


					// copy the name into the name buffer
					int len=strlen(fn.filename)+1;
					char * c=(char *)name_buffer->malloc(len, "name");
					strcpy(c, fn.filename);

					array[actual_total].name_start=c;
					actual_total++;
				}
				delete fp;

			}
			else
			{
				i4_alert(i4gets("old_model_file"),200, model_names[i]);
			}

		}
		else
		{
			pf_model_load_open.stop();
			i4_alert(i4gets("file_missing"), 200, model_names[i]);
		}

	}

	delete stat;

	total_models=actual_total;
	qsort(array, total_models, sizeof(model_info), g1_model_info_compare);


	// reset the model_reference values
	for (g1_model_ref * mi=model_references; mi; mi=mi->next)
	{
		mi->value=find_handle(mi->name);
	}

	pf_load_models.stop();

}

void g1_model_list_class::add_model(const i4_str& model_name, r1_texture_manager_class * tmap)
{
	//Ensure we're running.
	if (total_models==0 || array==NULL)
	{
		return;
	}

	total_models++;
	array=(model_info *)I4_REALLOC(array,total_models * sizeof(model_info), "model list extension");

	int actual_total=total_models-1;
	g1_quad_object_loader_class loader(g1_object_heap);
	char nbuf[MAX_PATH];


	//A string of the form "objects/%s.gmod"
	li_object * fmt=li_get_value("object_format", 0);
	char * n=li_string::get(fmt,0)->value();
	sprintf(nbuf,n,model_name.c_str());

	i4_file_class * in_file=i4_open(nbuf);
	if (in_file)
	{
		g1_loader_class * fp=g1_open_save_file(in_file);
		pf_model_load_open.stop();
		if (fp)
		{
			array[actual_total].model=loader.load(fp, model_name, tmap);

			if (array[actual_total].model)
			{
				i4_filename_struct fn;
				i4_split_path(model_name, fn);

				array[actual_total].model->scale(model_scaling);


				// copy the name into the name buffer
				int len=strlen(fn.filename)+1;
				char * c=(char *)name_buffer->malloc(len, "name");
				strcpy(c, fn.filename);

				array[actual_total].name_start=c;
				actual_total++;
			}
			delete fp;

		}
		else
		{
			i4_alert(i4gets("old_model_file"),200, model_name);
		}

	}
	else
	{
		pf_model_load_open.stop();
		i4_alert(i4gets("file_missing"), 200, model_name);
	}


	total_models=actual_total;
	qsort(array, total_models, sizeof(model_info), g1_model_info_compare);


	// reset the model_reference values
	for (g1_model_ref * mi=model_references; mi; mi=mi->next)
	{
		mi->value=find_handle(mi->name);
	}
}

w16 g1_model_list_class::find_handle(char * name) const
{
	if (!name || !total_models)
	{
		return 0;
	}

	sw32 lo=0,hi=total_models-1,mid;

	mid=(lo+hi+1)/2;
	for(;;)
	{
		int comp=strcmp(array[mid].name_start,name);
		if (comp==0)
		{
			return (w16)mid;
		}
		else if (comp<0)
		{
			lo=mid+1;
		}
		else
		{
			hi=mid-1;
		}

		w32 last_mid=mid;
		mid=(hi+lo)/2;

		if (last_mid==(w32)mid)
		{
			//i4_warning("Unable to find model %s, using default", name);
			//return 0;
			if (strstr(name,"_lod")!=0) //it's only a lod-model that was not found
			{
				return 0; //indicate that we don't have one.
			}
			if (comp>0)
			{
				i4_warning("Unable to find model %s, using nearest match %s",name,array[mid-1].name_start);
				return (w16)(mid-1);
			}
			else
			{
				i4_warning("Unable to find model %s, using nearest match %s",name,array[mid].name_start);
				return (w16)mid;
			}

		}
	}
	return 0;
}

void g1_model_list_class::init()
{
	name_buffer=new i4_grow_heap_class(2048,1024);
	g1_object_heap=new i4_grow_heap_class(1000*1024,2048); //increment was zero, what about increasing it?
	//this is required if large objects are on the map
	model_scaling=1.0f;
	total_models=0;
	array=0;
}

void g1_model_list_class::uninit()
{
	cleanup();
}
