/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/
#include "pch.h"
#include "global_id.h"
//#include "saver.h"
#include "checksum/checksum.h"
#include "memory/malloc.h"
#include "g1_object.h"

// Saver methods

g1_saver_class::g1_saver_class(i4_file_class * out, i4_bool close_on_delete)
	: i4_saver_class(out, close_on_delete),
	  remap(0)
{
	t_refs=0;
	state=DIRECTORY_CREATE;
	current_offset=out->tell();
}

g1_saver_class::~g1_saver_class()
{
	g1_global_id.free_remapping(remap);

}

void g1_saver_class::set_helpers(g1_object_class * * reference_list, w32 total_references)
{
	ref_list=reference_list;
	t_refs=total_references;
	if (remap)
	{
		g1_global_id.free_remapping(remap);
	}
	remap=0;
	remap = g1_global_id.alloc_remapping();
	for (w32 i=0; i<t_refs; i++)
	{
		(*remap)[ref_list[i]->global_id] = i;
	}
}

i4_bool g1_saver_class::write_global_id(w32 id)
{
	if (state==DIRECTORY_CREATE)
	{
		current_offset+=2;
		return i4_T;
	}
	else
	{
		w16 found=0xffff;
		if (g1_global_id.check_id(id))
		{
			found = (w16)(*remap)[id];
		}

		out->write_16(found);

		if (found==0xffff)
		{
			return i4_F;
		}
	}

	return i4_T;
}

i4_bool g1_saver_class::write_reference(const g1_reference_class &ref)
{
	if (state==DIRECTORY_CREATE)
	{
		current_offset+=4;
		return i4_T;
	}
	else
	{
		w16 found=0;

		if (ref.ref)
		{
			for (w32 i=0; !found && i<t_refs; i++)
			{
				if (ref.ref==ref_list[i])
				{
					found=(w16)i+1;
				}
			}

			out->write_16(found);
		}
		else
		{
			out->write_16(0);
		}

		// blank for notifee
		out->write_16(0);
	}

	return i4_T;
}

// Realtime saver methods

g1_realtime_saver_class::g1_realtime_saver_class(i4_file_class * Out, i4_bool Close_on_delete)
	: g1_saver_class(Out, Close_on_delete)
{
	state=RAW_WRITE; //directly write raw data
	Out->write_32(i4_check_sum32("GOLGRAWDATA",11)); //hint for the loader
};

void g1_realtime_saver_class::set_helpers(g1_object_class * * reference_list, w32 tot)
{    //ignore
};

i4_bool g1_realtime_saver_class::write_global_id(w32 id)
{
	out->write_32(id); //easy...
	//out->write_32(g1_global_id.get(id)->id);//also write the type of the object
	//in case we need to create it afterwards
	//dumb: not needed, since we write a global id only when saving an object
	//not a reference
	return i4_T;
}

i4_bool g1_realtime_saver_class::write_reference(const g1_reference_class &ref)
{
	if (ref.ref)
	{
		out->write_32(ref.ref->global_id);
		out->write_16(ref.ref->id);
	}
	else
	{
		out->write_32(0);
	}
	return i4_T;
}

g1_realtime_saver_class::~g1_realtime_saver_class()
{
};


// Loader methods

g1_loader_class::g1_loader_class(i4_file_class * in, i4_bool close_on_delete, i4_bool use_buffer)
	: i4_loader_class(in, close_on_delete,use_buffer),
	  id_remap(0)
{
	ref_list=0;
	first_ref=0;
	li_remap=0;
}

g1_loader_class::~g1_loader_class()
{
	if (id_remap)
	{
		g1_global_id.claim_freespace();
		i4_free(id_remap);
	}
}

void g1_loader_class::set_remap(w32 total_references)
{
	t_refs=total_references;
	if (id_remap)
	{
		i4_free(id_remap);
	}
	id_remap=0;
	if (total_references==0)
	{
		return;
	}
	id_remap = (w32 *)I4_MALLOC(total_references*sizeof(*id_remap), "loader_remapping");
	for (int i=0; i<(int)total_references; i++)
	{
		id_remap[i] = g1_global_id.alloc(0);
	}
}

void g1_loader_class::end_remap()
{
	for (int i=0; i<(int)t_refs; i++)
	{
		if (g1_global_id.preassigned(id_remap[i]))
		{
			g1_global_id.free(id_remap[i]);
		}
	}
}

void g1_loader_class::set_helpers(g1_object_class * * reference_list, w32 total_references)
{
	ref_list=reference_list;
	t_refs=total_references;
}

w32 g1_loader_class::read_global_id()
{
	w32 i=in->read_16();

	if (i==0xffff || !id_remap)
	{
		return g1_global_id.invalid_id();
	}
	else
	{
		return id_remap[i];
	}
}

void g1_loader_class::read_reference(g1_reference_class &ref)
{
	w16 index=in->read_16();

	ref.ref=0;                          // clear out hi bits
	ref.ref=(g1_object_class *)index;

	// read blank for notifee
	in->read_16();

	if (ref.ref)  // if non-0 reference we need to modify it later, so add to list
	{
		ref.next=first_ref;
		first_ref=&ref;
	}
	else
	{
		ref.next=0;
	}
}

void g1_loader_class::skip_reference()
{
	in->read_16();
	in->read_16();
}


void g1_loader_class::convert_references()
{
	g1_reference_class * f,* next;

	for (f=first_ref; f;)
	{
		next=f->next;
		g1_object_class * r;

		//PG: Bugfix: The following line doesn't work as exspected on a big
		//endian architecture. Should include an endianess test in the
		//bootstrap code.
		//w16 ref=*((w16 *)&f->ref);
		w32 ref32=*((w32 *)&f->ref);
		w16 ref=(w16)ref32;

		if (ref==0)
		{
			r=0;
		}
		else
		{
			ref--;
			if (ref<t_refs)
			{
				r=ref_list[ref];
			}
			else
			{
				i4_warning("bad reference in file");
				r=0;
			}
		}

		f->ref=0;

		f->reference_object(r);

		f=next;
	}
	first_ref=0;
}

//Realtime loader class methods

g1_realtime_loader_class::g1_realtime_loader_class(i4_file_class * In, i4_bool Close_on_delete, i4_bool use_buffer)
	: g1_loader_class(In,Close_on_delete,use_buffer)
{

}

g1_realtime_loader_class::~g1_realtime_loader_class()
{

}

void g1_realtime_loader_class::convert_references()
{
};

void g1_realtime_loader_class::set_helpers(g1_object_class * * reflist,w32 tot)
{
};

void g1_realtime_loader_class::end_remap()
{
};

void g1_realtime_loader_class::set_remap(w32 tot)
{
};

w32 g1_realtime_loader_class::read_global_id()
{
	return in->read_32();
}

void g1_realtime_loader_class::skip_reference()
{
	w32 gid=in->read_32();

	if (gid)
	{
		in->read_16();
	}
}

void g1_realtime_loader_class::read_reference(g1_reference_class &ref)
{
	w32 gid=in->read_32(); //read in global id
	w16 typeofref=0;

	if (gid)
	{
		g1_object_class * o=g1_global_id.checked_get(gid);
		typeofref=in->read_16();
		if (o)
		{
			ref.reference_object(o);
		}
		else
		{
			//the object being referenced doesn't yet exist.
			//we assume the corresponding data comes soon
			o=g1_object_type_array[typeofref]->create_object(typeofref,0);
			o->x=2; //place
			o->y=2;
			o->h=0;
			o->occupy_location();
			o->grab_old();
			i4_warning("Needed to create object %i as %s with no info.",
					   gid,g1_object_type_array[typeofref]->name());
			//No sync required, either, as we expect to _be_ synced
			//o->request_think();//don't think before the location is ok
			ref.reference_object(o);
		}
	}
	else
	{
		ref.reference_object(0);
	}
}





g1_loader_class *g1_open_save_file(i4_file_class * in, i4_bool close_on_delete_or_fail)
{
	g1_loader_class * l=new g1_loader_class(in, close_on_delete_or_fail);

	if (l->error())
	{
		delete l;
		return 0;
	}

	return l;
}
