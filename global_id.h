/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_GLOBAL_ID_HH
#define G1_GLOBAL_ID_HH

#include "arch.h"
#include "g1_limits.h"
#include "init/init.h"
#include "file/file.h"
#include "file/ram_file.h"


class g1_object_class;

class g1_global_id_reset_notifier
{
public:
  g1_global_id_reset_notifier *next;
  static g1_global_id_reset_notifier *first;
  g1_global_id_reset_notifier();
  virtual void reset() = 0;
  ~g1_global_id_reset_notifier();
};

//having these enums global saves much typing... 
//As they are always preseted. it's seldom that we run into trouble
enum {
	  ID_NET_DISABLED=0,
	  ID_NET_ENABLED=1,//Allocations have to be handled specially here
	  ID_NET_SERVER=2, //This is set if the local id manager is the global one
	  ID_NET_CLIENT=4, //This is set if we are a slave only
	  ID_NET_READY=8
	  };

class g1_global_id_manager_class:public i4_init_class
{
public:
	friend class g1_object_marker_class;
  enum 
  { 
    ID_INCREMENT=(1<<(G1_MAX_OBJECTS_BITS+1)),
    ID_MASK=ID_INCREMENT-1,
	ID_DELETEPROPAGATE=1
  };//A global id can newer be smaller than ID_INCREMENT
  //An entry smaller than ID_INCREMENT is either invalid (0)
  //or an offset in the free list or means something special.
  //Actually, I'm not shure, why that +1 above is needed.
  //I think, it would not be needed, but who cares?

  enum
	  {
	  INACTIVE=0,
	  ACTIVE=1
	  };

  class client_info
	  {
	  public:
	  w32 num_ids_assigned;
	  i4_temp_file_class *lastpacket;//in case this needs resending
	  //w8 *buf;
	  w32 flags;
	  w32 lastacked;//number of the packet that was last acknowledged
	  w32 lastnumber;//the last packet number used (the one that should 
	  //be acked next)
	  client_info()
		  {
		  lastpacket=0;
		  //buf=0;
		  cleanup();
		  }
	  void cleanup()
		  {
		  num_ids_assigned=0;
		  delete lastpacket;
		  lastpacket=0;
		  //buf=0;
		  flags=0;
		  lastacked=0;
		  lastnumber=0;
		  }
	  ~client_info()
		  {
		  delete lastpacket;
		  lastpacket=0;
		  }
	  };
  
private:

  //w32 obj_id[G1_MAX_OBJECTS];
  w32 *obj_id;
  //g1_object_class *obj[G1_MAX_OBJECTS];
  g1_object_class **obj;
  w32 first_free; 
  w32 num_reserved;
  w32 netflags;
  w32 num_local_ids;
  w32 lastrequpacketid;
  w32 lastrecepacketid;
  sw32 timeout;
  client_info client[G1_MAX_PLAYERS];
public:
  g1_global_id_manager_class();
  ~g1_global_id_manager_class();

  void init();
  void uninit();

  void enable_networking(w32 mode);//should only be disabled by resetting

  int networking_enabled()
	  {
	  return netflags&ID_NET_ENABLED;
	  }

  i4_bool poll(); //should be called at least once a frame in network mode

  int is_ready()//returns false if waiting for required adresses from server
	  {
	  return netflags&ID_NET_READY;
	  }

  int receive_packet(i4_file_class *pkt); //called by the network object
  
  void claim_freespace();
  void free_objects();

  i4_bool check_id(w32 id) const 
	  { 
	  return ((id==obj_id[id&ID_MASK]) && ((w32)obj[id&ID_MASK]>=ID_INCREMENT )); /*&& ((id & ID_MASK) < num_reserved)*/ 
	  //the second test is actually only required, if the game is played over the network
	  //we haven't ever used an id above num_reserved, therefore, the third test is useless
	  }

  i4_bool preassigned(w32 id) const;
  void assign(w32 id, g1_object_class *for_who);
  w32 alloc(g1_object_class *for_who);
  void free(w32 id);
  g1_object_class *get(w32 id) { return obj[id&ID_MASK]; }
  g1_object_class *checked_get(w32 id) { return check_id(id) ? get(id) : 0; }
  void check_size();//grows the obj and obj_id arrays if needed
  w32 invalid_id() { return 0; }

  class remapper
  {
  protected:
    friend class g1_global_id_manager_class;
    g1_global_id_manager_class *gid;
    w32 *map;
    // remapping functions
    remapper(g1_global_id_manager_class *gid);
    ~remapper();
  public:
    w32 &operator[](w32 id) { return map[id&ID_MASK]; }
  };
  remapper* alloc_remapping() { return new remapper(this); }
  void free_remapping(remapper* mapper) { delete mapper; }

  void debug(w32 flag_pass=0xffffffff);
};

extern g1_global_id_manager_class g1_global_id;
class g1_loader_class;
class g1_saver_class;

//This file must be included after the global id manager is defined
#include "saver.h" 
//#include "g1_object.h"
// a convient way to access objects through global id's
class g1_id_ref
{
public:
  w32 id;
  g1_object_class *get() const { return g1_global_id.checked_get(id); }
  i4_bool valid() { return g1_global_id.check_id(id); }
  g1_id_ref() { id=0; }
  g1_id_ref(w32 id) : id(id) {}
  g1_id_ref(g1_object_class *o);
/*	  {
	  if (o->get_flag(g1_object_class::EXT_GLOBAL_ID))
	  {
		id=g1_global_id.invalid_id();
	  }
      else
	    id=o->global_id;
	  }
	  */
  g1_object_class *operator->() const { return get(); }
  g1_id_ref& operator=(const g1_id_ref &r) { id=r.id;  return *this; }
  g1_id_ref& operator=(w32 _id) { id=_id;  return *this; }
  g1_id_ref& operator=(g1_object_class *o);
/*	  {
	  if (o)
        id=o->global_id;
      else
        id=g1_global_id.invalid_id();

      return *this;
	  }
	  */
  void save(g1_saver_class *fp)
	  {
	  fp->write_global_id(id);
	  };
  void load(g1_loader_class *fp)
	  {
	  id=fp->read_global_id();
	  }
  static void skip(g1_loader_class *fp)
	  {
	  fp->read_global_id();
	  }
};

#endif
