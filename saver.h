/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_SAVER_HH
#define G1_SAVER_HH
#ifndef G1_GLOBAL_ID_HH
#error You must include global_id.h before this file (or instead)
#endif
#include "file/file.h"
#include "memory/growarry.h"
#include "path.h"
#include "reference.h"
#include "loaders/dir_load.h"
#include "loaders/dir_save.h"
#include "lisp/li_types.h"
//#include "global_id.h"

class g1_path_saver_class;
class g1_object_class;


struct g1_saver_section_type
{
  w16 section_id;
  w32 section_offset;
  g1_saver_section_type(w16 section_id, w32 section_offset)
    : section_offset(section_offset), section_id(section_id) {}
};
class g1_global_id_manager_class;//circular references here
//class g1_global_id_manager_class::remapper;
class g1_saver_class : public i4_saver_class
{
  g1_object_class **ref_list;
  w32 t_refs;
  g1_global_id_manager_class::remapper *remap;

public: 
  g1_saver_class(i4_file_class *out, i4_bool close_on_delete=i4_T);
  ~g1_saver_class();

  virtual void set_helpers(g1_object_class **reference_list, w32 total_references);
  virtual i4_bool write_reference(const g1_reference_class &ref);
  virtual i4_bool write_global_id(w32 id);
};

//this class does quite the same as the above, except it writes
//current global_ids instead of the complicated way of regetting the ids
//for old files.
class g1_realtime_saver_class: public g1_saver_class
	{
	public:
		g1_realtime_saver_class(i4_file_class *Out, i4_bool Close_on_delete=i4_T);
		~g1_realtime_saver_class();
  void set_helpers(g1_object_class **reference_list, w32 total_references);
  i4_bool write_reference(const g1_reference_class &ref);
  i4_bool write_global_id(w32 id);
	};

class g1_loader_class : public i4_loader_class
{
  g1_object_class **ref_list;
  w32 t_refs;
  g1_reference_class *first_ref;
  w32 *id_remap;

public: 
  li_type_number *li_remap;    // used to load lisp objects

  i4_bool references_were_loaded() { return first_ref!=0; }

  g1_loader_class(i4_file_class *in, i4_bool close_on_delete=i4_T, i4_bool use_buffer=i4_T);

  virtual void set_remap(w32 total_refs);
  virtual void end_remap();

  virtual void set_helpers(g1_object_class **reference_list, w32 total_references);

  virtual void read_reference(g1_reference_class &ref);
  virtual void skip_reference();//call this if you don't want to load a saved reference
  virtual w32 read_global_id();

  // called by level loader after all objects have been loaded
  virtual void convert_references();

  ~g1_loader_class();
};

class g1_realtime_loader_class:public g1_loader_class
	{
	public:
		g1_realtime_loader_class(i4_file_class *In, i4_bool Close_on_delete=i4_T, i4_bool use_buffer=i4_T);
		//as these are all virtual, there's no drawback in not inlining them
	virtual void set_remap(w32 total_refs);
	virtual void end_remap();
    virtual void set_helpers(g1_object_class **reference_list, w32 total_references);

  virtual void read_reference(g1_reference_class &ref);
  virtual void skip_reference();
  virtual w32 read_global_id();

  // called by level loader after all objects have been loaded
  virtual void convert_references();

  ~g1_realtime_loader_class();
		
	};

// returns NULL if file is corrupted
g1_loader_class *g1_open_save_file(i4_file_class *in,
                                   i4_bool close_on_delete_or_fail=i4_T);

#endif



