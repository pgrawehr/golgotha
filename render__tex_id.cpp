/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/
#include "pch.h"
#include "render/tex_id.h"
#include "string/string.h"
#include "file/file.h"
#include "string/str_checksum.h"
#include "render/r1_res.h"
#include "memory/hashtable.h"


w32 r1_get_file_id(const i4_const_str &fname) //retrieves the integer from an integer-form filename
{
	int x;
	char st[30], * s;

	s=st;

	i4_const_str::iterator l=fname.begin();
	while (l!=fname.end() && l.get().ascii_value()!='.')
	{
		*(s++)=l.get().ascii_value();
		++l;
	}

	*s=0;

	if (sscanf(st,"%x",&x))
	{
		return x;
	}
	else
	{
		return 0;
	}
}

r1_texture_ref * r1_texture_ref::first=0;

i4_str *r1_texture_id_to_filename(w32 id,
								  const i4_const_str &out_dir)
{
	return i4_const_str("%S/%x.gtx").sprintf(100,
											 &out_dir,
											 id);

}

typedef struct _id_hash_table_entry {
	w32 id;
	char name[100];
	struct _id_hash_table_entry * next;
} ID_HASH_TABLE_ENTRY,* LPID_HASH_TABLE_ENTRY;

#define hash_table_size 512 /*must be power of 2*/
#define table_and_value (hash_table_size-1)
//format of this hash-table: static table of pointers to a list with the same hash-values
//no problems here, as there won't be any deletions in this table

//The hash-Table has only an advantage over a binary tree if
//(entries/hash_table_size)<log2(entries)
//Currently, we have 2080 textures: ~4 textures per entry
//versus tree depth of 11
LPID_HASH_TABLE_ENTRY id_hash_table[hash_table_size];

i4_bool r1_loaded_dir=i4_F;

void r1_truncate_texture_file(void)
//Initialize texture lookup-hash-table
{
	//i4_file_class *fp=i4_open(HASH_CODE_FILE,I4_WRITE|I4_NO_BUFFER);
	//delete fp;
	ZeroMemory(id_hash_table,sizeof(id_hash_table));
	r1_loaded_dir=i4_F;
}

void r1_cleanup_texture_lookup_table(void)
{
	LPID_HASH_TABLE_ENTRY h,h2;

	for (int i=0; i<hash_table_size; i++)
	{
		h=id_hash_table[i];
		while(h)
		{

			h2=h->next;
			delete h;
			h=h2;
		}
	}
	r1_loaded_dir=i4_F;
}


void r1_register_texture_name(const i4_const_str &full_filename,w32 id)
{
	i4_filename_struct fn;
	w32 index=id&table_and_value;
	LPID_HASH_TABLE_ENTRY h2=id_hash_table[index],h3=NULL;

	while(h2)
	{
		if(h2->id==id) //already present->skip
		{

			return;
		}
		h3=h2; //remember last entry of list
		h2=h2->next;
	}
	i4_split_path(full_filename,fn);
	LPID_HASH_TABLE_ENTRY h=new ID_HASH_TABLE_ENTRY;
	h->id=id;
	strcpy(h->name,fn.filename);
	h->next=0;

	if (h3)
	{
		h3->next=h;
	}
	else
	{
		id_hash_table[index]=h;
	}
	return;
	/*i4_file_class *fp=i4_open(HASH_CODE_FILE,I4_APPEND|I4_NO_BUFFER);
	   if (!fp)
	   	return;
	   fp->write_32(id);
	   char buf[100];
	   char buf2[100];
	   i4_os_string(full_filename,buf,100);
	   FillMemory(buf2,100,' ');
	   wsprintf(buf2,"%s %x",buf,id);
	   fp->write(buf2,100);
	   delete fp;*/
}



w32 r1_get_texture_id(const i4_const_str &full_filename) //returns i4_str_checksum(filename)
//don't call for strings that are already ids! Would generate a new id of the id.
{
	if (!full_filename.null())
	{
		i4_filename_struct fn;
		i4_split_path(full_filename, fn);
		w32 id=i4_str_checksum(fn.filename);
		r1_register_texture_name(full_filename,id);
		return id;
	}
	else
	{
		return 0;
	}
}


i4_const_str *r1_get_texture_name(w32 id)
{
	w32 index=id&table_and_value;

	LPID_HASH_TABLE_ENTRY h2=id_hash_table[index];

	while(h2)
	{
		if(h2->id==id)
		{

			return new i4_const_str(h2->name);
		}
		h2=h2->next;
	}
	//not found: Scan entire textures\ folder and add any file found.
	if (r1_loaded_dir)
	{
		return 0;
	}                           //if already searched the directory once
	//don't proceed. Would scan directory many times per frame(!)
	i4_directory_struct ds;
	i4_get_directory("textures",ds,i4_F,NULL);
	r1_loaded_dir=i4_T;
	int i=0;
	for (; i<(int)ds.tfiles; i++)
	{
		r1_get_texture_id(*(ds.files[i]));
	}
	h2=id_hash_table[index];
	while(h2)
	{
		if(h2->id==id)
		{

			return new i4_const_str(h2->name);
		}
		h2=h2->next;
	}
	return NULL; //still not found
	/*i4_file_class *fp=i4_open(HASH_CODE_FILE,I4_READ|I4_NO_BUFFER);
	   if (!fp) return NULL;
	   char buf[100];
	   w32 thisid=0;
	   while (!fp->eof())
	   	{
	   	thisid=fp->read_32();
	   	fp->read(buf,100);
	   	if (thisid==id)
	   		{
	   		int b=0;
	   		while(buf[b]!=' ')
	   			b++;
	   		buf[b]=0;
	   		return new i4_const_str(buf);
	   		};
	   	}
	   delete fp;
	   return NULL;*/
}

r1_texture_ref::~r1_texture_ref()
{
	if (this==first)
	{
		first=first->next;
	}
	else
	{
		r1_texture_ref * p, * last=0;
		for (p=first; p!=this;)
		{
			last=p;
		}
		p=p->next;
		last->next=next;
	}
}
