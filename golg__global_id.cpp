/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/
#include "pch.h"
#include "global_id.h"
#include "error/error.h"
#include "memory/malloc.h"
#include "saver.h"
#include "lisp/li_all.h"
#include "g1_object.h"
#include "player.h"
#ifdef NETWORK_INCLUDED
#include "net/client.h"
#include "net/server.h"
#endif

#include <string.h>

sw32 g1_cur_num_map_objects;
#define PREASSIGN_SIG ((g1_object_class *)0xbadfeed)
//The following must be smaller than ID_INCREMENT to fall past the checked_get() test
#define REMASSIGN_SIG(remno) ((g1_object_class *) ID_INCREMENT - G1_MAX_PLAYERS + remno - 2)
#define GETREMNUM(addr) ((w32)(addr - ID_INCREMENT+G1_MAX_PLAYERS+2))
#define MAXIDPACKETSIZE 150

g1_global_id_manager_class::g1_global_id_manager_class()
{
  obj_id=0;
  obj=0;
  first_free=0;
  num_reserved=0;
  //init();
  //claim_freespace();
}

g1_global_id_manager_class::~g1_global_id_manager_class()
	{

	}

i4_bool g1_global_id_manager_class::preassigned(w32 id) const
{
  return obj[id&ID_MASK]==PREASSIGN_SIG;
}

g1_global_id_reset_notifier *g1_global_id_reset_notifier::first=0;

g1_global_id_reset_notifier::g1_global_id_reset_notifier()
{
  next=first;
  first=this;
}


g1_global_id_reset_notifier::~g1_global_id_reset_notifier()
{
  if (first==this)
    first=first->next;
  else
  {
    g1_global_id_reset_notifier *p;
    for (p=first; p->next!=this; p=p->next);
    p->next=next;
  }
}


void g1_global_id_manager_class::init()
{
  // initialize ids
  if (num_reserved>0)
	  {
	  uninit();
	  };
  netflags=ID_NET_DISABLED;//Also means "not ready"
  num_reserved=4096;
  obj_id=(w32*) malloc(num_reserved*sizeof(w32));
  memset(obj_id,0,num_reserved*sizeof(w32));
  for (w32 i=0; i<num_reserved; i++)
    obj_id[i] = ID_INCREMENT+i;

  //sizeof(pointertype) != sizeof(w32) on 64Bit architectures!
  obj=(g1_object_class**) malloc(num_reserved*sizeof(g1_object_class*));
  memset(obj, 0, num_reserved*sizeof(g1_object_class*));
  first_free=num_reserved;

  g1_cur_num_map_objects=0;
  g1_global_id_reset_notifier *p;
  for (p=g1_global_id_reset_notifier::first;p; p=p->next)
	  {
      p->reset();
	  }
  claim_freespace();
}

void g1_global_id_manager_class::enable_networking(w32 mode)
	{
#ifdef NETWORK_INCLUDED
	if (mode==ID_NET_SERVER)
		{
		netflags=ID_NET_SERVER|ID_NET_ENABLED;//but not yet ready.
		num_local_ids=0;
		for (w32 i=0;i<num_reserved;i++)
			{
			if (obj_id[i]==invalid_id())
				num_local_ids++;
			}
		netflags|=ID_NET_READY;
		}
	else
		{
		netflags=ID_NET_CLIENT|ID_NET_ENABLED;
		num_local_ids=0;
		first_free=num_reserved;//nothing available right now
		}
	lastrequpacketid=0;//0 means nothing to ack right now
	lastrecepacketid=0;
	timeout=0;
#endif
	}

i4_bool g1_global_id_manager_class::poll()
	{
#ifdef NETWORK_INCLUDED
	if (!networking_enabled()||netflags & ID_NET_SERVER)
		return i4_T;
	if (num_local_ids<400&&timeout<=0)
		{
		//Need new ids soon, so send a request
		PACKET(sp,r);
		r.write_8(G1_PK_ID_NEEDIDS);
		if (lastrequpacketid%2==0)
			lastrequpacketid++;//request packets have always odd numbers
		r.write_32(lastrequpacketid);
		r.write_32(g1_player_man.local_player);
		i4_network_send(sp,&r,0);
		timeout=20;
		}
	if (num_local_ids<100)
		{
		netflags = netflags & ~ID_NET_READY; //We must have ids right now 
		}
	timeout--;
	return i4_T;
#else
	return i4_T;
#endif
	};

int g1_global_id_manager_class::receive_packet(i4_file_class *pkt)
	{
#ifdef NETWORK_INCLUDED
	w32 msg=pkt->read_8();
	i4_temp_file_class *f;
	w32 ackid,from,anid,idm;
	switch (msg)
		{
		case G1_PK_ID_NEEDIDS:
			{
			//only the server can get such packets under normal conditions
			ackid=pkt->read_32();
			from=pkt->read_32();
			if (client[from].flags&ACTIVE)
				{
				if (ackid==client[from].lastacked)//an aged packet
					break;
				}
			else
				{
				client[from].cleanup();
				client[from].flags=ACTIVE;
				}
			if (client[from].lastpacket)//has not acked this one, resend
				{
				i4_network_send(client[from].lastpacket->get_buffer(),
					client[from].lastpacket,from);
				}
			else //Need to generate a new packet. Very complicated, sucks.
				{
				//f is just a shortcut
				f=client[from].lastpacket=new i4_temp_file_class(1000,1000);
				f->write_8(G1_PK_ID_HEREAREIDS);
				f->write_32(ackid);
				f->write_32(from);
				for (int i=0;i<MAXIDPACKETSIZE;i++)
					{
					anid=alloc(REMASSIGN_SIG(from));//reserve this id for a remote pc
					i4_warning("Sending ID %i to player %i",anid,from);
					f->write_32(anid);
					}
				f->write_32(invalid_id());//marks the end of the packet
				i4_network_send(f->get_buffer(),f,from);
				//well, that wasn't that complicated as i expected
				}
			}break;
		case G1_PK_ID_HEREAREIDS:
			{
			ackid=pkt->read_32();
			from=pkt->read_32();
			if (from!=g1_player_man.local_player)
				{
				i4_warning("Got a HEREAREIDS packet that's not for me!");
				return 0;
				};
			if (ackid==lastrequpacketid)
				{
				lastrequpacketid++;
				//mark the received ids as to be used next
				anid=pkt->read_32();
				while (anid)
					{
					idm = anid & ID_MASK;
					while(idm>=num_reserved)
						{
						check_size();
						};
					*((w32*)(&obj[idm]))=first_free;
					first_free=idm;
					obj_id[idm]=anid;//it's this id we will have to use
					num_local_ids++;//we just got a new id to use
					anid=pkt->read_32();
					};
				netflags |=ID_NET_READY;
				}
			PACKET(sp,r);
			r.write_8(G1_PK_ID_IDACK);
			r.write_32(ackid);
			r.write_32(from);
			i4_network_send(sp,&r,0);
			}break;
		case G1_PK_ID_IDACK:
			{
			ackid=pkt->read_32();
			from=pkt->read_32();
			if (client[from].lastpacket)
				{
				client[from].lastacked=ackid;
				delete client[from].lastpacket;
				client[from].lastpacket=0;
				}
			}break;
		}
	return 0;
#else 
	return 0;
#endif

	}

void g1_global_id_manager_class::uninit()
	{
	::free(obj);
	::free(obj_id);
	obj=0;
	obj_id=0;
	num_reserved=0;
	
	for (int i=0;i<G1_MAX_PLAYERS;i++)
		{
		delete client[i].lastpacket;
		client[i].lastpacket=0;
		}
	netflags=ID_NET_DISABLED;
	}

//this function is newer used (I think it's better like this...)
void g1_global_id_manager_class::free_objects()
{
  w32 i = first_free,j=0;

  while (i<num_reserved)
  {
    j = *((w32*)(&obj[i]));                       // next one
    obj[i] = 0;
    i = j;
  }

  for (i=0; i<num_reserved; i++)
    if (obj[i])
    {
      delete obj[i];
      obj[i]=0;
    }

  //could shrink the memory usage here, but since we already have it,
  //I don't think this will be of much use
  first_free=num_reserved;
}

void g1_global_id_manager_class::check_size()
	{
	w32 old_res=num_reserved;
	num_reserved*=2;
	obj=(g1_object_class**)realloc(obj,num_reserved*sizeof(g1_object_class*));
	obj_id=(w32*)realloc(obj_id,num_reserved*sizeof(w32));
	if (!obj || !obj_id)
		{
		i4_error("FATAL: global_id manager: Out of memory!");
		return;
		}
	
	for (w32 i=old_res;i<num_reserved;i++)
		{
		obj_id[i]=ID_INCREMENT+i;
		//it is now no more guaranteed that newer objects have higher ids!
		}
	memset(obj+old_res,0,(num_reserved-old_res)*sizeof(g1_object_class*));
	claim_freespace();
	}

void g1_global_id_manager_class::claim_freespace()
{
  if (netflags&ID_NET_CLIENT)//the client doesn't have to claim anything
	  {
	  return;
	  }
  int i=num_reserved;
  while (i>0)
  {
    i--;
    if (!obj[i])
    {
      *((w32*)(&obj[i]))=first_free;
      first_free=i;
    }
  }
  if (first_free>=num_reserved && num_reserved<G1_MAX_OBJECTS)
	  {
	  check_size();
	  }
}

void g1_global_id_manager_class::assign(w32 id, g1_object_class *for_who)
{
  
  w32 index = id&ID_MASK;
  if (id==0)//we loaded an object without an id, so stay without.
	  return;
  while (index>=num_reserved)
	  {
	  check_size();
	  }
  if (obj_id[index]!=id)
	  {
	  //As network-client, this might happen if we remote-create an object
	  //outside our own id range (which SHOULD happen)
      i4_warning("assigning a possibly invalid id!");
	  obj_id[index]=id;
	  }

  //In network mode, this is just normal: We must make a copy of a remote object with the same id 
  //but we already dispatched that id.
  if (obj[index]!=PREASSIGN_SIG)
    i4_warning("assigning a previously assigned id!");

  obj[index] = for_who;
  //return i4_T;
}

w32 g1_global_id_manager_class::alloc(g1_object_class *for_who)
{
  if (netflags&ID_NET_CLIENT)
	  {
	  //return one of the ids that are reserved for us
	  w32 id=first_free;
	  first_free = (*((w32*)(&obj[id]))); 
  
	  obj[id] = (for_who) ? for_who : PREASSIGN_SIG;
	  g1_cur_num_map_objects++;
	  num_local_ids--;

      return obj_id[id];
	  //return invalid_id();
	  }
  if (first_free>=num_reserved)
  {
    claim_freespace();
    if (first_free>=num_reserved)
		{
        i4_error("SEVERE: alloc object id : too many objects.");
		}
  }

  w32 id=first_free;
  first_free = (*((w32*)(&obj[id]))); 
  
  obj[id] = (for_who) ? for_who : PREASSIGN_SIG;
  g1_cur_num_map_objects++;

  return obj_id[id];
}

void g1_global_id_manager_class::free(w32 id)
{
  //if (!check_id(id))
  //{
  //  i4_warning("free object id : bad id");
  //  return;
  //}
  if (id==invalid_id()) return;

  id &= ID_MASK;

#if 1
  // forces use of all ids first
  obj[id]=0;
#else
  *((w32*)(&obj[id]))=first_free;
  first_free=id;
#endif
  obj_id[id] += ID_INCREMENT;
  g1_cur_num_map_objects--;
}

g1_global_id_manager_class::remapper::remapper(g1_global_id_manager_class *gid) : gid(gid)
{
  map = (w32 *)I4_MALLOC(G1_MAX_OBJECTS*sizeof(w32), "global_id_remapping"); 
}

g1_global_id_manager_class::remapper::~remapper() 
{ 
  i4_free(map); 
}

g1_global_id_manager_class g1_global_id;

//The following functions have been inlined for better speed
/*
void g1_id_ref::save(g1_saver_class *fp)
{
  fp->write_global_id(id);
}

void g1_id_ref::load(g1_loader_class *fp)
{
  id=fp->read_global_id(); 
}
*/
//void g1_id_ref::skip(g1_loader_class *fp)
//	{
//	fp->read_global_id();
//	}

//Cannot inline these due to circular reference problems
g1_id_ref::g1_id_ref(g1_object_class *o)
{
  if (o->get_flag(g1_object_class::EXT_GLOBAL_ID))
	  {
	  id=g1_global_id.invalid_id();
	  }
  else
	  id=o->global_id;
}


g1_id_ref& g1_id_ref::operator=(g1_object_class *o)
{
  if (o)
    id=o->global_id;
  else
    id=g1_global_id.invalid_id();

  return *this;
}

void g1_global_id_manager_class::debug(w32 flag_pass)
{
  w32 invalid[(G1_MAX_OBJECTS+31)/32];
  w32 num_free=0;

  memset(invalid,0,sizeof(invalid));

  w32 i = first_free;
  while (i<num_reserved)
  {
    invalid[i>>5] |= (1<< (i&31));                // mark bit
    i = *((w32*)(&obj[i]));                       // next one
    num_free++;                                   // count
  }


  for (i=0; i<num_reserved; i++)
    if ((invalid[i>>5] & (1<<(i&31)))==0)
      if (!obj[i])
      {
        invalid[i>>5] |= (1<< (i&31));
        num_free++;
      }
      else
        if (obj[i]->get_flag(flag_pass))
          i4_debug->printf("%d: [%s] team:%d [%c%c%c%c]\n", 
                           i, 
                           obj[i]->name(), 
                           obj[i]->player_num,
                           obj[i]->get_flag(g1_object_class::MAP_OCCUPIED)?'M':'m',
                           obj[i]->get_flag(g1_object_class::THINKING)?'T':'t',
                           obj[i]->get_flag(g1_object_class::SCRATCH_BIT)?'S':'s',
                           obj[i]->get_flag(g1_object_class::DELETED)?'D':'d'
                           );

  i4_debug->printf("Valid Objects: %d  Free Spaces: %d\n", G1_MAX_OBJECTS-num_free, num_free);

}

void g1_list_objects(w32 flag_pass=0xffffffff)
{
  g1_global_id.debug(flag_pass);
}

LI_HEADER(g1_list_objects)
	{
	g1_list_objects();
	return li_nil;
	};

li_automatic_add_function(li_g1_list_objects,"g1_list_objects");
