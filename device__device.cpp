// DEVICE.CPP
/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/
#include "pch.h"
#include "memory/malloc.h"
#include "memory/growheap.h"
#include "device/device.h"
#include "device/event.h"
#include "device/kernel.h"
#include "device/processor.h"
#include "device/key_man.h"
#include "device/keys.h"

#include "error/error.h"
#include "error/alert.h"

#include "init/init.h"


#include "time/time.h"
#include "threads/threads.h"
#include "isllist.h"
#include "file/file.h"

#include <string.h>
#include <stdlib.h>
#include "memory/new.h"

//The following macro can be defined when you need to debug handler
//allocation/deallocation bugs.
//#define HANDLER_DEBUG

//#ifdef _DEBUG
//#undef new
//#define new DEBUG_NEW
//#undef THIS_FILE
//static char THIS_FILE[] = __FILE__;
//#endif

i4_device_class::i4_device_class()
{
	next=NULL;
}

void i4_device_class::send_event_to_agents(i4_event *ev, device_flags receive_types)
{
	i4_kernel.broadcast_event_type(ev, receive_types);
}

i4_event_handler_class::~i4_event_handler_class()
{
	I4_ASSERT(!thinking(), "i4_event_handler::thinking on destructor");

//#ifndef I4_RETAIL
//  if (!i4_is_initialized() && first)
//  {
//    i4_warning("event_handler has references at end of program");
//  }
//#endif


	// clear out all the references to ourself
	while (first)
	{
		first->ref=0;
		first=first->next;
	}
}


i4_event_reaction_class *i4_event_reaction_class::copy()
{
	i4_event *ev=event ? event->copy() : 0;

	return new i4_event_reaction_class(handler_reference.get(), ev);
}

i4_event_reaction_class::~i4_event_reaction_class()
{
	I4_ASSERT(i4_is_initialized() || handler_reference.get()==0,
			  "event reaction is global and has a reference at end of program");


	handler_reference.reference(0);

	I4_ASSERT(i4_is_initialized() || event==0,
			  "event reaction is global and has an event at end of program");

	if (event)
	{
		delete event;
	}
}

i4_event_reaction_class::i4_event_reaction_class(i4_event_handler_class *hand,
												 i4_event *event)
	: event(event)
{

	handler_reference.reference(hand);
}

i4_event_reaction_class::i4_event_reaction_class(i4_event_handler_class *hand,
												 w32 user_message_id)
{
	event=new i4_user_message_event_class(user_message_id);
	handler_reference.reference(hand);
}


void i4_event_handler_private_reference_class::destroy_ref()
{
	// see if we need to remove our reference from the object we were referencing
	if (ref)
	{
		i4_event_handler_private_reference_class *p, *last=0;
		p=ref->first;
		while (p && p!=this)
		{
			last=p;
			p=p->next;
		}

		I4_ASSERT(p, "destroy_ref : couldn't find reference");

		if (last)
		{
			last->next=next;
		}
		else
		{
			ref->first=next;
		}

		ref=0;
	}
}

// if who is 0, then reference is destroyed
void i4_event_handler_private_reference_class::reference(i4_event_handler_class *who)
{
	destroy_ref();
	if (who)
	{
		ref=who;
		next=who->first;
		who->first=this;
	}
}
// EVENT.CPP
/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/


i4_do_command_event_class::i4_do_command_event_class(char *_command, int command_id)
	: command_id(command_id)
{
	int len=strlen(_command);
	if (len>=sizeof(command))
	{
		//Bail out with fatal error here, might be vulnerable to buffer-overruns otherwise.
		i4_error("FATAL: Command too long");
	}
	strcpy(command, _command);
}

i4_do_command_event_class::i4_do_command_event_class(char *_command, int command_id, i4_time_class &time)
	: command_id(command_id),
	  time(time)
{
	int len=strlen(_command);
	if (len>=sizeof(command))
	{
		i4_error("FATAL: Command too long");
	}
	strcpy(command, _command);
}

i4_event *i4_do_command_event_class::copy()
{
	return new i4_do_command_event_class(command, command_id);
}



i4_end_command_event_class::i4_end_command_event_class(char *_command, int command_id, i4_time_class &time)
	: command_id(command_id),
	  time(time)
{
	int len=strlen(_command);
	if (len>=sizeof(command))
	{
		i4_error("FATAL: Command too long");
	}
	strcpy(command, _command);
}

i4_end_command_event_class::i4_end_command_event_class(char *_command, int command_id)
	: command_id(command_id)
{
	int len=strlen(_command);
	if (len>=sizeof(command))
	{
		i4_error("FATAL: Command too long");
	}
	strcpy(command, _command);
}
// KERNEL.CPP
/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/


enum {
	I4_SHOW_NONE,
	I4_SHOW_ALL,
	I4_SHOW_NON_TRIVIAL,           // excludes mouse move & window messages
};

int i4_show_events=I4_SHOW_NONE;

i4_kernel_device_class i4_kernel;


struct defered_event
{
	i4_event_handler_reference_class<i4_event_handler_class> send_to;
	defered_event *next;
	i4_event *ev_copy;
	defered_event(i4_event_handler_class *_send_to,
				  i4_event *ev) :
		ev_copy(ev->copy()),
		next(0) {
		send_to.reference(_send_to);
	}

	~defered_event() {
		delete ev_copy;
	}

};


struct event_handler_delete_node
{
	i4_event_handler_class *who;
	event_handler_delete_node *next;
	event_handler_delete_node(i4_event_handler_class *who)
		: who(who) {
	}
};


static defered_event *defered_list;



static i4_critical_section_class list_lock;
static i4_isl_list<defered_event> list;
typedef i4_isl_list<defered_event>::iterator def_iter;

static i4_isl_list<event_handler_delete_node> eh_delete_list;
static i4_time_class last_user_input;

// r includes event and who to send to
void i4_kernel_device_class::send(i4_event_reaction_class *r)
{
	if (r && r->handler_reference.get())
	{
		send_event(r->handler_reference.get(), r->event);
	}
}



#ifdef _DEBUG
void i4_kernel_device_class::show_pending()
{
	list_lock.lock();

	i4_isl_list<defered_event>::iterator i=list.begin();
	char buf[i4_event::MAX_NAME_BUFFER_SIZE];
	char buf2[i4_event::MAX_NAME_BUFFER_SIZE];
	for (; i!=list.end(); ++i)
	{
		i->ev_copy->name(buf);
		i->send_to->name(buf2);
		i4_warning("'%s' for '%s'",buf, buf2);
	}

	list_lock.unlock();
}
#endif

void i4_kernel_device_class::deque_events(i4_event_handler_class *for_who)
{
	list_lock.lock();

	i4_isl_list<defered_event>::iterator i=list.begin(), last=list.end(), q;
	for (; i!=list.end(); )
	{
		if (i->send_to.get()==for_who)
		{
			if (last==list.end())
			{
				list.erase();
			}
			else
			{
				list.erase_after(last);
			}


			q=i;
			++i;

			list_lock.unlock(); // in case deleted events send events
			delete &*q;
			list_lock.lock();
		}
		else
		{
			last=i;
			++i;
		}
	}

	list_lock.unlock();
}


void i4_kernel_device_class::send_event(i4_event_handler_class *send_to, i4_event *ev)
{

#ifndef I4_RETAIL
	char debug_buf[i4_event::MAX_NAME_BUFFER_SIZE];
	char debug_buf2[i4_event::MAX_NAME_BUFFER_SIZE];
#endif
	if (ev->when()!=i4_event::NOW)
	{
#ifndef I4_RETAIL
		if (i4_show_events==I4_SHOW_ALL ||
			(i4_show_events==I4_SHOW_NON_TRIVIAL &&
			 !(ev->type()==i4_event::MOUSE_MOVE || ev->type()==i4_event::WINDOW_MESSAGE)))
		{
			ev->name(debug_buf);
			send_to->name(debug_buf2);
			i4_warning("queing : '%s' to '%s'",debug_buf, debug_buf2);
		}
#endif
		defered_event *dv=new defered_event(send_to,ev);

		list_lock.lock();
		list.insert_end(*dv);
		list_lock.unlock();
	}
	else
	{
#ifndef I4_RETAIL
		if (i4_show_events==I4_SHOW_ALL ||
			(i4_show_events==I4_SHOW_NON_TRIVIAL &&
			 !(ev->type()==i4_event::MOUSE_MOVE || ev->type()==i4_event::WINDOW_MESSAGE)))
		{
			ev->name(debug_buf);
			send_to->name(debug_buf2);
			i4_warning("sending : '%s' to '%s'",debug_buf, debug_buf2);
		}
#endif

		send_to->call_stack_counter++;
		events_sent++;
		send_to->receive_event(ev);
		send_to->call_stack_counter--;
	}
}

i4_bool i4_kernel_device_class::process_events()       // returns true if an event was dispatched
{
	i4_bool ret=i4_F;
	for (i4_device_class *d=device_list; d; d=d->next)
	{
		if (d->process_events())
		{
			ret=i4_T;
		}
	}

	ret=(i4_bool)(flush_events() | ret);

	check_for_idle();

	return ret;
}

void i4_kernel_device_class::set_modal(i4_window_class *ref)
{
	modal_stack.push(ref);
}

i4_window_class *i4_kernel_device_class::modal()
{
	return modal_stack.top();
}

void i4_kernel_device_class::end_modal(i4_window_class *ref)
{
	if (modal_stack.pop()!=ref)
	{
		i4_warning("Modal Stack: Unbalanced push and pop operations.");

	}
	;
}

void i4_kernel_device_class::flush_handlers()
{
	list_lock.lock();
	// delete any event handlers that are qued for deletion
	i4_isl_list<event_handler_delete_node>::iterator i=eh_delete_list.begin(),
	last=eh_delete_list.end(), q;


	while (i!=eh_delete_list.end())
	{
		if (!i->who->thinking())
		{
			q=i;
			++i;

			if (last!=eh_delete_list.end())
			{
				eh_delete_list.erase_after(last);
			}
			else
			{
				eh_delete_list.erase();
			}

			list_lock.unlock();     // unlock because deleted object might send events
#ifdef DEBUG_HANDLER
			i4_warning("Executing postphoned deletion of handler 0x%x",q->who);
#endif
			delete q->who;
			delete &*q;

			list_lock.lock();
		}
		else
		{
			++i;
		}
	}

	list_lock.unlock();
}

i4_bool i4_kernel_device_class::flush_events()
{
	i4_bool ret=i4_F;

	// send any events that were queued
	while (list.begin()!=list.end())
	{
		list_lock.lock();
		i4_isl_list<defered_event>::iterator old=list.begin();
		list.erase();
		list_lock.unlock();

		// make sure event handler is still around..
		i4_event_handler_class *eh=old->send_to.get();
		if (eh)
		{
#ifdef _DEBUG
			i4_event *ev=old->ev_copy;
			char buf[i4_event::MAX_NAME_BUFFER_SIZE];
			char buf2[i4_event::MAX_NAME_BUFFER_SIZE];
			if (i4_show_events==I4_SHOW_ALL ||
				(i4_show_events==I4_SHOW_NON_TRIVIAL &&
				 !(ev->type()==i4_event::MOUSE_MOVE || ev->type()==i4_event::WINDOW_MESSAGE)))
			{
				old->ev_copy->name(buf);
				eh->name(buf2);
				i4_warning("sending : '%s' to '%s'",buf, buf2);
			}
#endif
			eh->call_stack_counter++;
			events_sent++;
			eh->receive_event(old->ev_copy);
			eh->call_stack_counter--;
		}

		delete &*old;

		ret=i4_T;
	}


	flush_handlers();

	return ret;
}


void i4_kernel_device_class::request_events(i4_event_handler_class *for_who, w32 event_types)
{
	int type=0;
	while (event_types)
	{
		if (event_types&1)
		{
			response[type]=new response_type(for_who, response[type]);
		}

		event_types>>=1;
		type++;
	}
}

// unrequest_events tells any devices sending event_types events to you to stop
void i4_kernel_device_class::unrequest_events(i4_event_handler_class *for_who, w32 event_types)
{
	int type=0;
	while (event_types)
	{
		if (event_types&1)
		{
			response_type *last=0, *p;
			for (p=response[type]; p && p->who!=for_who; p=p->next)
			{
				last=p;
			}



			if (!p)
			{
				//i4_error("unrequesting events & not installed");
				event_types>>=1;
				type++;
				continue; //unrequesting something that has newer been requested, just ignore.
			}
			else
			{
				//        for_who->dereference();
				if (response[type]->who==for_who)
				{
					response[type]=response[type]->next;
				}
				else
				{
					last->next=p->next;
				}

				delete p;
			}
		}
		event_types>>=1;
		type++;
	}
}

void i4_kernel_device_class::add_device(i4_device_class *device)       // returns 16 bits device id
{
	device->next=device_list;
	device_list=device;
}

void i4_kernel_device_class::remove_device(i4_device_class *device)
{
	i4_device_class *last=0, *p;
	for (p=device_list; p && p!=device; p=p->next)
	{
		last=p;
	}

	if (!p)
	{
		i4_error("remove device : device not found");
	}
	if (last)
	{
		last->next=device->next;
	}
	else
	{
		device_list=device->next;
	}
}


void i4_kernel_device_class::not_idle()
{
	last_user_input.get();
}

void i4_kernel_device_class::check_for_idle()
{
	if (can_send_idle)
	{
		i4_time_class now;
		if (now.milli_diff(last_user_input)>(sw16)milliseconds_before_idle_events_sent)
		{
			i4_user_idle_event_class uiev;
			broadcast_event_type(&uiev, i4_device_class::FLAG_IDLE);
			can_send_idle=i4_F;
		}
	}
}

void i4_kernel_device_class::set_milliseconds_before_idle_events_sent(w32 milli_seconds)
{
	milliseconds_before_idle_events_sent=milli_seconds;
	check_for_idle();
}

void i4_kernel_device_class::init()
{
	modal_stack.push(0);
}

void i4_kernel_device_class::uninit()
{
	modal_stack.uninit();
	flush_handlers();
}

i4_kernel_device_class::i4_kernel_device_class() :
	modal_stack(0,20)
{
	events_sent=0;
	//modal_stack.push(0);//the top of stack marks no modal window active
	can_send_idle=i4_F;
	milliseconds_before_idle_events_sent=1000;

	memset(response,0,sizeof(response));
	device_list=0;
}


void i4_kernel_device_class::broadcast_event_type(i4_event *ev, w32 event_type)
{
	if (event_type & (i4_device_class::FLAG_MOUSE_MOVE |
					  i4_device_class::FLAG_MOUSE_BUTTON_DOWN |
					  i4_device_class::FLAG_MOUSE_BUTTON_UP |
					  i4_device_class::FLAG_KEY_PRESS |
					  i4_device_class::FLAG_KEY_RELEASE))
	{
		can_send_idle=i4_T;
		last_user_input.get();
	}

	int type=0;
	while ((event_type&1)==0)
	{
		event_type>>=1;
		type++;
	}

	for (response_type *r=response[type]; r; r=r->next)
	{
		send_event(r->who, ev);
	}

}


void i4_kernel_device_class::delete_handler(i4_event_handler_class *handler)
{
	if (handler->thinking())
	{
#ifdef HANDLER_DEBUG
		i4_warning("Marking handler 0x%x for deletion. ",handler);
#endif
		i4_isl_list<event_handler_delete_node>::iterator i=eh_delete_list.begin();
		// This is cheap, since the delete list is usually quite small.
		// (around 3-4 items max)
		while (i!=eh_delete_list.end())
		{
			if (i->who==handler)
			{
				return; //already there, don't do anything.
			}
			++i;
		}
		eh_delete_list.insert(*(new event_handler_delete_node(handler)));
	}
	else
	{
		//i4_warning("need to fix this handler deletion thing, trey");
		/*
		   if (!valid_ptr(handler))
		   {
		   valid_ptr(handler);
		   //i4_warning("bad handler pointer");
		   delete handler;
		   handler=NULL;
		   }
		   else

		   	{
		   delete handler;
		   handler=NULL;
		   	}
		 */

		i4_isl_list<event_handler_delete_node>::iterator i=eh_delete_list.begin();

		while (i!=eh_delete_list.end())
		{
			if (i->who==handler)
			{
#ifdef HANDLER_DEBUG
				i4_warning("Attempting to bypass deletion handler for already queued item 0x%x, ignoring.",handler);
#endif
				return;
			}
			++i;
		}
#ifdef HANDLER_DEBUG
		i4_warning("Immediate delete of handler 0x%x.",handler);
#endif
		delete handler;
	}
}
// processor.CPP
/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/


void i4_win32_get_cpu_info(i4_cpu_info_struct *s);

void i4_get_cpu_info(i4_cpu_info_struct *s)
{
	if (!s)
	{
		return;
	}

	s->cpu_type  = i4_cpu_info_struct::NON_X86;
	s->cpu_flags = 0;

#if (__linux)
	s->cpu_type  = i4_cpu_info_struct::UNKNOWN_X86;
	s->cpu_flags = 0;
#endif

#if (_WINDOWS)
	i4_win32_get_cpu_info(s);
#endif
}
// win32_cpu.cpp
/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/

#ifdef _WINDOWS
w32 x86_can_do_cpuid()
{
	w32 result;

	_asm
	{
		pushfd                      //save EFLAGS

		pop eax
		test eax,0x00200000 //check ID bit (bit 21)
		jz set_21         //bit 21 is not set, so jump to set_21
		and  eax,0xffdfffff //clear bit 21
		push eax        //save new value in register

		popfd                     //store new value in flags
		pushfd

		pop eax
		test eax,0x00200000 //check ID bit

		jnz cpu_id_not_ok   //if bit 21 isnt clear,then jump to cpu_id_not_ok

		mov dword ptr [result],1 // return 1
		jmp done

set_21:
		or   eax,0x00200000 //set bit 21
		push eax         //store new value

		popfd                      //store new value in EFLAGS
		pushfd

		pop eax
		test eax,0x00200000 //if bit 21 is on
		jz cpu_id_not_ok     //then jump to cpu_id_ok

		mov dword ptr [result],1 // return 1
		jmp done

cpu_id_not_ok:

		mov dword ptr [result],0 //CPUID inst is not supported

done:
	}

	return result;
}

w32 x86_get_cpu_type()
{
	char name[32];

	_asm
	{
		mov eax,0

		__emit 0x0F //cpuid
		__emit 0xA2

		//store it
		mov byte ptr [name],bl
		mov byte ptr [name+1],bh

		ror ebx, 16

		mov byte ptr [name+2],bl
		mov byte ptr [name+3],bh
		mov byte ptr [name+4],dl
		mov byte ptr [name+5],dh

		ror edx,16

		mov byte ptr [name+6],dl
		mov byte ptr [name+7],dh
		mov byte ptr [name+8],cl
		mov byte ptr [name+9],ch

		ror ecx,16

		mov byte ptr [name+10],cl
		mov byte ptr [name+11],ch
		mov byte ptr [name+12],0
	}

	if (!strcmp(name,"AuthenticAMD") || !strcmp(name,"CentaurHauls"))
	{
		return i4_cpu_info_struct::AMD_X86;
	}
	else
	if (!strcmp(name,"GenuineIntel"))
	{
		return i4_cpu_info_struct::INTEL_X86;
	}
	else
	{
		return i4_cpu_info_struct::UNKNOWN_X86;
	}
}

w32 x86_get_cpu_flags(w32 cpu_type)
{
	unsigned int processor_sig;
	unsigned int extended_flags;

	_asm
	{
		mov eax,0x80000001  //the cpuid function we're requesting

		__emit 0x0F
		__emit 0xA2 //cpuid

		mov dword ptr [processor_sig],eax
		mov dword ptr [extended_flags],edx
	}

	//perhaps check the processor signature too?

	w32 flags = 0;

	if (cpu_type==i4_cpu_info_struct::AMD_X86)
	{
		if (extended_flags & (1<<4))
		{
			flags |= i4_cpu_info_struct::RDTSC;
		}

		if (extended_flags & (23<<31))
		{
			flags |= i4_cpu_info_struct::MMX;
		}

		if (extended_flags & (1<<31))
		{
			flags |= i4_cpu_info_struct::AMD3D;
		}
	}
	else
	if (cpu_type==i4_cpu_info_struct::INTEL_X86)
	{
		if (extended_flags & (1<<4))
		{
			flags |= i4_cpu_info_struct::RDTSC;
		}

		/*
		   if (extended_flags & (23<<31))
		   flags |= i4_cpu_info_struct::MMX;

		   if (extended_flags & (1<<31))
		   flags |= i4_cpu_info_struct::AMD3D;
		 */
	}

	return flags;
}

void i4_win32_get_cpu_info(i4_cpu_info_struct *s)
{
	_asm pusha

	if (!x86_can_do_cpuid())
	{
		s->cpu_type  = i4_cpu_info_struct::UNKNOWN_X86;
		s->cpu_flags = 0;
	}
	else
	{
		s->cpu_type  = x86_get_cpu_type();
		s->cpu_flags = x86_get_cpu_flags(s->cpu_type);
	}
	_asm popa
}
#endif
// KEY_MAN.CPP
/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/


i4_key_man_class i4_key_man;

i4_key_man_class::i4_key_man_class()
{
	loaded=0;
	active_list=0;
	context_list=0;
	command_list=0;
	char_heap=0;
	memset(keys, 0, sizeof(keys));
}


// end commands no longer appropriate for for the current modifiers
void i4_key_man_class::end_actives(int matches_key, i4_time_class &time)
{
	key_item *last=0;
	for (key_item *j=active_list; j; )
	{
		if (j->modifier_flags!=key_modifiers_pressed || j->key==matches_key)
		{
			key_item *q=j;
			j=j->next_active;

			if (last)
			{
				last->next_active=q->next_active;
			}
			else
			{
				active_list=active_list->next_active;
			}

			i4_end_command_event_class kcmd( (*command_list)[q->command_id], q->command_id, time);
			send_event_to_agents(&kcmd, FLAG_END_COMMAND);

			q->command_active=0;
		}
		else
		{
			last=j;
			j=j->next_active;
		}
	}
}


void i4_key_man_class::get_modifiers(int k_mod)
{
	if (k_mod)                   // turn left & right into same thing
	{
		if (k_mod & I4_MODIFIER_SHIFT)
		{
			k_mod=I4_MODIFIER_SHIFT;
		}
		if (k_mod & I4_MODIFIER_CTRL)
		{
			k_mod=I4_MODIFIER_CTRL;
		}
		if (k_mod & I4_MODIFIER_ALT)
		{
			k_mod=I4_MODIFIER_ALT;
		}

		// don't uses these modifiers
		//if (k_mod & (I4_MODIFIER_WINDOWS | I4_MODIFIER_CAPS | I4_MODIFIER_NUMLOCK))
		//  k_mod &= ~(I4_MODIFIER_WINDOWS | I4_MODIFIER_CAPS | I4_MODIFIER_NUMLOCK);
	}

	//if (context_list)
	//  k_mod &= ~((*context_list)[context_id].modifiers_taken);


	key_modifiers_pressed=k_mod;
}


void i4_key_man_class::add_active(i4_key_man_class::key_item *i, i4_time_class &time)
{
	if (i4_kernel.modal())
	{
		return;
	}                            //don't send any DO_COMMANDS if modal window active
	if (!i->command_active)
	{
		i->command_active=1;
		i->next_active=active_list;
		active_list=i;

		i4_do_command_event_class kcmd( (*command_list)[i->command_id], i->command_id, time);
		send_event_to_agents(&kcmd, FLAG_DO_COMMAND);
	}
}

void i4_key_man_class::end_all_commands()
{
	key_item *last=0;
	for (key_item *j=active_list; j; )
	{

		key_item *q=j;
		j=j->next_active;

		if (last)
		{
			last->next_active=q->next_active;
		}
		else
		{
			active_list=active_list->next_active;
		}
		i4_time_class t(0);
		i4_end_command_event_class kcmd( (*command_list)[q->command_id], q->command_id, t);
		send_event_to_agents(&kcmd, FLAG_END_COMMAND);

		q->command_active=0;

	}
	key_modifiers_pressed=0;
}

void i4_key_man_class::receive_event(i4_event *ev)
{
	if (!loaded)
	{
		return ;
	}

	if (ev->type()==i4_event::KEY_PRESS)
	{
		//i4_warning("Pressed a key, current state=%s",context_name(current_context()));
		CAST_PTR(kev, i4_key_press_event_class, ev);
		int old_modifiers=key_modifiers_pressed;

		get_modifiers(kev->modifiers);

		if (old_modifiers!=key_modifiers_pressed)
		{
			for (key_item *i=active_list; i; i=i->next_active)
			{
				for (key_item *j=keys[i->key]; j; j=j->next)
				{
					if (j!=i && j->modifier_flags==key_modifiers_pressed &&
						(j->context_mask&(1<<context_id)))
					{
						add_active(j, kev->time);
					}

				}
			}

			end_actives(-1, kev->time);
		}

		for (key_item *i=keys[kev->key_code]; i; i=i->next)
		{
			if ((key_modifiers_pressed == i->modifier_flags) && (i->context_mask& (1<<context_id)))
			{
				add_active(i, kev->time);
			}
		}

	}
	else if (ev->type()==i4_event::KEY_RELEASE)
	{
		CAST_PTR(kev, i4_key_press_event_class, ev);
		if ((kev->key==0)&&(kev->key_code==0)) //We lost keyboard focus->End all commands
		{
			end_all_commands();
			key_modifiers_pressed=0; //Clear all modifiers
			return;
		}
		int old_modifiers=key_modifiers_pressed;
		get_modifiers(kev->modifiers);

		if (old_modifiers!=key_modifiers_pressed)
		{
			for (key_item *i=active_list; i; i=i->next_active)
			{
				for (key_item *j=keys[i->key]; j; j=j->next)
				{
					if (j!=i && j->modifier_flags==key_modifiers_pressed &&
						(j->context_mask & (1<<context_id)))
					{
						add_active(j, kev->time);
					}

				}
			}

			end_actives(-1, kev->time);
		}


		end_actives(kev->key_code, kev->time);
	}
}

static i4_bool is_white(char *s)
{
	if (*s==' ' || *s=='\n' || *s=='\r' || *s=='\t')
	{
		return i4_T;
	}
	else
	{
		return i4_F;
	}
}

static void skip_white(char *&s)
{
	while (*s && is_white(s)) s++;

}

static i4_bool i4_go_key_start(char *&s)
{
	while (*s && *s!='(')
	{
		if (*s=='#')
		{
			while (*s && (*s!='\n' && *s!='\r'))
				s++;

		}
		else
		{
			s++;
		}
	}

	if (*s)
	{
		//This entirely skips the command, so the leading def_key
		//is newer actually interpreted at all.
		while (*s && *s!=' ') s++;

		skip_white(s);
		return i4_T;
	}
	else
	{
		return i4_F;
	}
}

static char get_char(char *&s)
{
	if (*s=='\\')
	{
		s+=2;
		if (s[-1]=='n')
		{
			return '\n';
		}
		if (s[-1]=='r')
		{
			return '\r';
		}
		if (s[-1]=='t')
		{
			return '\t';
		}
		if (s[-1]=='b')
		{
			return '\b';
		}
		if (s[-1]=='\\')
		{
			return '\\';
		}
	}
	else
	{
		s++;
		return s[-1];
	}
	return '\\';
}

static void i4_read_str(char *&s, char *buf)
{
	skip_white(s);
	if (s[0]=='"')
	{
		s++;
		while (*s && *s!='"')
			*(buf++)=get_char(s);

		*buf=0;
		s++;
	}
	else
	{
		*(buf++)=*(s++);
		while (*s && !is_white(s) && *s!=')')
			*(buf++)=get_char(s);

		*buf=0;
	}
}

int i4_key_man_class::acquire_modifiers_for_contexts(int context_mask, int mod, char *key_name)
{
	int c=context_mask, i=0, total=0, skip_this_key=0;
	while (c)
	{
		if (c&1)
		{
			if (((*context_list)[i].modifiers_used & mod))
			{
				i4_alert(i4gets("modifier_in_use"),200, key_name);
				return 0;
			}
			else
			{
				(*context_list)[i].modifiers_used |= mod;
				(*context_list)[i].modifiers_taken |= mod;
			}
		}

		c>>=1;
		i++;
	}

	return 1;
}

int i4_key_man_class::use_modifiers_for_contexts(int context_mask, int mod, char *key_name)
{
	int c=context_mask, i=0;

	while (c)
	{
		if (c&1)
		{
			if (((*context_list)[i].modifiers_taken & mod))
			{
				i4_alert(i4gets("modifier_in_use"),200, key_name);
				return 0;
			}
			else
			{
				(*context_list)[i].modifiers_used |= mod;
			}
		}

		c>>=1;
		i++;
	}

	return 1;
}

i4_bool i4_key_man_class::load(const i4_const_str &filename)
{
	check_init();
	i4_file_class *fp=i4_open(filename);
	if (!fp)
	{
		return i4_F;
	}

	int size=fp->size();
	void *mem=I4_MALLOC(size+1,"");
	fp->read(mem,size);
	delete fp;

	char *c=(char *)mem;
	c[size]=0;

	char tmp[256];

	int x=0;
	while (i4_go_key_start(c))
	{
		w16 mod;
		i4_key key;
		char key_name[256],cmd[256];
		int skip_key=0;

		x++;
		i4_read_str(c,key_name);
		if (!i4_find_key(i4_const_str(key_name), key, mod))
		{
			i4_alert(i4gets("no_key"),100, key_name, &filename);
			skip_key=1;
		}

		i4_read_str(c,cmd);
		int id=get_command_id(cmd);

		int context_mask=0;
		do
		{
			i4_read_str(c,tmp);
			if (tmp[0] && tmp[0]!=')')
			{
				context_mask|=(1<<get_context_id(tmp));
			}
		} while (tmp[0]!=')' && tmp[0]);

		if (key==0 && mod!=0 && !skip_key) // they want a CTRL-ALT-SHIFT type key
		{
			if (acquire_modifiers_for_contexts(context_mask, mod, key_name))
			{

				if (mod & I4_MODIFIER_CTRL)
				{
					keys[I4_CTRL_L]=new key_item(context_mask, id, I4_MODIFIER_CTRL, I4_CTRL_L, keys[I4_CTRL_L]);
					keys[I4_CTRL_R]=new key_item(context_mask, id, I4_MODIFIER_CTRL, I4_CTRL_R, keys[I4_CTRL_R]);
				}

				if (mod & I4_MODIFIER_ALT)
				{
					keys[I4_ALT_L]=new key_item(context_mask, id, I4_MODIFIER_ALT, I4_ALT_L, keys[I4_ALT_L]);
					keys[I4_ALT_R]=new key_item(context_mask, id, I4_MODIFIER_ALT, I4_ALT_R, keys[I4_ALT_R]);
				}

				if (mod & I4_MODIFIER_SHIFT)
				{
					keys[I4_SHIFT_L]=new key_item(context_mask, id, I4_MODIFIER_SHIFT, I4_SHIFT_L, keys[I4_SHIFT_L]);
					keys[I4_SHIFT_R]=new key_item(context_mask, id, I4_MODIFIER_SHIFT, I4_SHIFT_R, keys[I4_SHIFT_R]);
				}

				if (mod & I4_MODIFIER_WINDOWS)
				{
					keys[I4_COMMAND]=new key_item(context_mask, id, 0, I4_COMMAND, keys[I4_COMMAND]);
				}

				if (mod & I4_MODIFIER_CAPS)
				{
					keys[I4_CAPS]=new key_item(context_mask, id, 0, I4_CAPS, keys[I4_CAPS]);
				}

				if (mod & I4_MODIFIER_NUMLOCK)
				{
					keys[I4_NUM_LOCK]=new key_item(context_mask, id, 0, I4_NUM_LOCK, keys[I4_NUM_LOCK]);
				}
			}
		}
		else if (!skip_key)
		{
			if (use_modifiers_for_contexts(context_mask, mod, key_name))
			{
				// make sure the key isn't already assigned
				for (key_item *i=keys[key]; i; i=i->next)
				{
					if (i->modifier_flags==mod && (i->context_mask & context_mask))
					{
						i4_error("USER: attempting to assign command %s but key %s (%d) is already assigned to command %s",
								 (*command_list)[i->command_id], key_name, key, (*command_list)[id]);
					}

				}

				keys[key]=new key_item(context_mask, id, (w8)mod, key, keys[key]);
			}
		}
	}

	i4_free(mem);
	loaded=i4_T;
	return i4_T;
}


void i4_key_man_class::uninit()
{
	if (!command_list)
	{
		return;
	}

	i4_time_class now;
	while (active_list)
		end_actives(active_list->key, now);


	int i;
	for (i=0; i<I4_NUM_KEYS; i++)
	{
		while (keys[i])
		{
			key_item *ki=keys[i];
			keys[i]=keys[i]->next;
			delete ki;
		}
	}

	delete command_list;
	command_list=0;
	delete context_list;
	context_list=0;
	delete char_heap;
	char_heap=0;

	i4_kernel.unrequest_events(this,
							   i4_device_class::FLAG_KEY_PRESS | i4_device_class::FLAG_KEY_RELEASE);
}

char *i4_key_man_class::alloc_str(char *s)
{
	int l=strlen(s)+1;
	char *t=(char *)char_heap->malloc(l,"");
	memcpy(t,s,l);
	return t;
}

void i4_key_man_class::check_init()
{
	if (!context_list)
	{
		context_list = new i4_array<context>(32,32);
		command_list = new i4_array<char *>(32,32);
		char_heap = new i4_grow_heap_class(2048, 2048);

		i4_kernel.request_events(this,
								 i4_device_class::FLAG_KEY_PRESS | i4_device_class::FLAG_KEY_RELEASE);
	}
}

int i4_key_man_class::get_context_id(char *context_name)
{
	check_init();
	int s=context_list->size();
	for (int i=0; i<s; i++)
	{
		if (strcmp( (*context_list)[i].name, context_name)==0)
		{
			return i;
		}
	}

	if (context_list->size()==32)
	{
		i4_error("max contexts exceed with %s", context_name);
	}

	context *c=context_list->add();
	c->name=alloc_str(context_name);
	c->modifiers_taken=0;
	c->modifiers_used=0;

	return s;
}

int i4_key_man_class::get_command_id(char *command)
{
	check_init();
	int s=command_list->size();
	for (int i=0; i<s; i++)
	{
		if (strcmp( (*command_list)[i], command)==0)
		{
			return i;
		}
	}

	command_list->add(alloc_str(command));
	return s;
}


i4_bool i4_key_man_class::get_key_for_command(int command_id, i4_key &key, w16 &mod)
{
	for (int i=0; i<I4_NUM_KEYS; i++)
	{
		for (key_item *k=keys[i]; k; k=k->next)
		{
			if ((k->context_mask & (1<<context_id)) && (k->command_id == command_id))
			{
				key=i;
				mod=k->modifier_flags;
				return i4_T;
			}
		}
	}

	return i4_F;
}



void i4_key_matchup_class::add(char *command, int remap)
{
	matchup.insert(new command_matchup(i4_key_man.get_command_id(command), remap));
}

int i4_key_matchup_class::remap(int command_id)
{
	command_matchup f=command_matchup(command_id,0);
	command_matchup *m=matchup.find(&f);
	if (m)
	{
		return m->remap_id;
	}
	else
	{
		return -1;
	}
}
// KEYS.CPP
/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/


static char *i4_extended_key_names[]=
//{{{
{
	"Up",
	"Down",
	"Left",
	"Right",
	"Left Ctrl",
	"Right Ctrl",
	"Left Alt",
	"Right Alt",
	"Left Shift",
	"Right Shift",
	"Caps Lock",
	"Num Lock",
	"Home",
	"End",
	"Del",
	"F1",
	"F2",
	"F3",
	"F4",
	"F5",
	"F6",
	"F7",
	"F8",
	"F9",
	"F10",
	"F11",
	"F12",
	"F13",
	"F14",
	"F15",
	"Insert",
	"PageUp",
	"PageDown",
	"Command",
	"Pad 0",
	"Pad 1",
	"Pad 2",
	"Pad 3",
	"Pad 4",
	"Pad 5",
	"Pad 6",
	"Pad 7",
	"Pad 8",
	"Pad 9",
	"Pad =",
	"Pad /",
	"Pad *",
	"Pad -",
	"Pad +",
	"Pad Enter",
	"Pad ."
};
//}}}

static struct i4_key_alias_struct {
	i4_key key;
	char *name;
}
i4_key_alias[] =
{
	{ I4_BACKSPACE,    "Backspace" },
	{ I4_TAB,          "Tab"       },
	{ I4_ENTER,        "Enter"     },
	{ I4_ESC,          "Escape"    },
	{ I4_SPACE,        "Space"     }
};
const int i4_key_aliases=sizeof(i4_key_alias)/sizeof(i4_key_alias_struct);

static struct i4_modifier_map_struct {
	w16 modifier;
	char *name;
}
i4_modifier_map[] =
{
	{ I4_MODIFIER_CTRL,    "Ctrl"  },
	{ I4_MODIFIER_ALT,     "Alt"   },
	{ I4_MODIFIER_SHIFT,   "Shift" },
};
const int i4_modifiers = sizeof(i4_modifier_map)/sizeof(i4_modifier_map_struct);


char *i4_get_key_name(i4_key key, w16 modifiers, char *out)
//{{{
{
	char *buffer,*p;
	char sing[2];
	w32 i;

	if (key>255 && key<I4_NUM_KEYS)
	{
		// extended key name
		buffer = i4_extended_key_names[key-256];
	}
	else
	{
		// search standard key aliases
		i=0;
		while (i<i4_key_aliases && key!=i4_key_alias[i].key)
			i++;


		if (i<i4_key_aliases)
		{
			// found key alias
			buffer = i4_key_alias[i].name;
		}
		else
		{
			// not found, assume normal ascii key
			buffer = sing;
			buffer[0]=(key>='a' && key<='z') ? (key-'a'+'A') : key;
			buffer[1]=0;
		}
	}

	p=out;
	for (i=0; i<i4_modifiers; i++)
	{
		if (modifiers & i4_modifier_map[i].modifier)
		{
			strcpy(p,i4_modifier_map[i].name);
			p += strlen(p);
			*p++='+';
		}
	}
	strcpy(p,buffer);

	return out;
}
//}}}


i4_str *i4_key_name(i4_key key, w16 modifiers)
//{{{
{
	char buff[256];

	return i4_from_ascii(i4_get_key_name(key,modifiers,buff));
}
//}}}

i4_bool i4_find_key(const i4_str& name, i4_key &key, w16 &mod)
//{{{
{
	i4_str::iterator p(name.begin());
	char buf[256];
	w32 len=0;
	w32 i;

	// clear values
	mod = key = 0;
	int combo_key=0;

	// parse key names & modifiers
	buf[0]=0;
	while ((*p)!=0 && len<sizeof(buf)-1)
	{
		while (len<sizeof(buf)-1 && (*p) && *p!='+')
		{
			buf[len] = *p;
			++len;
			++p;
		}
		buf[len]=0;

		if (*p)
		{
			combo_key=1;
			++p;
			if (!(*p))
			{
				buf[len++]='+';
				buf[len]=0;
			}
			else
			{
				len = 0;
			}
		}

		// scan for modifiers
		for (i=0; i<i4_modifiers; i++)
		{
			if (!strcmp(buf, i4_modifier_map[i].name))
			{
				mod |= i4_modifier_map[i].modifier;
			}
		}
	}
	if (len>=sizeof(buf)-1 || len==0)
	{
		// no key name can be THAT long or THAT short
		return i4_F;
	}
	if (len>1)
	{
		// found a named key

		// search common key aliases
		i=0;
		while (i<i4_key_aliases && strcmp(buf, i4_key_alias[i].name))
			i++;


		if (i<i4_key_aliases)
		{
			key = i4_key_alias[i].key;
		}
		else
		{
			// search extended key names
			i=256;
			while (i<I4_NUM_KEYS && strcmp(buf,i4_extended_key_names[i-256]))
				i++;


			if (i<I4_NUM_KEYS)
			{
				key = (w16)i;
			}
			else if (combo_key || mod==0)
			{
				return i4_F;
			}
			else
			{
				return i4_T;
			}
		}
	}
	else
	{
		// use ascii value for key (uppercase)
		key=(buf[0]>='a' && buf[0]<='z') ? (buf[0]-'a'+'A') : buf[0];
	}

	return i4_T;
}
//}}}


enum
//{{{ Internal modifier key translation flags
{
	I4_SHIFT_L_FLAG = 1,
	I4_SHIFT_R_FLAG = 2,
	I4_CAPS_FLAG = 4,
	I4_SHIFT_FLAGS = 7,
	I4_NUM_LOCK_FLAG = 8,
	I4_CTRL_L_FLAG = 16,
	I4_CTRL_R_FLAG = 32,
	I4_CTRL_FLAGS = 48
};
//}}}


w16 i4_key_translate(i4_key raw_key, i4_bool press, w16 &state)
//{{{
{
	switch (raw_key)
	{
		case I4_SHIFT_L:
			if (press)
			{
				state |= I4_MODIFIER_SHIFT_L;
			}
			else
			{
				state &= ~I4_MODIFIER_SHIFT_L;
			}
			break;
		case I4_SHIFT_R:
			if (press)
			{
				state |= I4_MODIFIER_SHIFT_R;
			}
			else
			{
				state &= ~I4_MODIFIER_SHIFT_R;
			}
			break;

		case I4_CAPS:
			if (press)
			{
				state ^= I4_MODIFIER_CAPS;
			}
			break;

		case I4_NUM_LOCK:
			if (press)
			{
				state ^= I4_MODIFIER_NUMLOCK;
			}
			break;

		case I4_CTRL_L:
			if (press)
			{
				state |= I4_MODIFIER_CTRL_L;
			}
			else
			{
				state &= ~I4_MODIFIER_CTRL_L;
			}
			break;

		case I4_CTRL_R:
			if (press)
			{
				state |= I4_MODIFIER_CTRL_R;
			}
			else
			{
				state &= ~I4_MODIFIER_CTRL_R;
			}
			break;
//Actually dumb to hardcode the keyboard layout, but Windows doesn't seem to
			//have an usable function to avoid this. WM_CHAR is not usable as there's no
			//WM_CHARUP Message
		case '1':
			return (state & I4_MODIFIER_SHIFT) ? '!' : '1';

		case '2':
			return (state & I4_MODIFIER_SHIFT) ? '@' : '2';

		case '3':
			return (state & I4_MODIFIER_SHIFT) ? '#' : '3';

		case '4':
			return (state & I4_MODIFIER_SHIFT) ? '$' : '4';

		case '5':
			return (state & I4_MODIFIER_SHIFT) ? '%' : '5';

		case '6':
			return (state & I4_MODIFIER_SHIFT) ? '^' : '6';

		case '7':
			return (state & I4_MODIFIER_SHIFT) ? '&' : '7';

		case '8':
			return (state & I4_MODIFIER_SHIFT) ? '*' : '8';

		case '9':
			return (state & I4_MODIFIER_SHIFT) ? '(' : '9';

		case '0':
			return (state & I4_MODIFIER_SHIFT) ? ')' : '0';

		case '-':
			return (state & I4_MODIFIER_SHIFT) ? '_' : '-';

		case '=':
			return (state & I4_MODIFIER_SHIFT) ? '+' : '=';

		case '[':
			return (state & I4_MODIFIER_SHIFT) ? '{' : '[';

		case ']':
			return (state & I4_MODIFIER_SHIFT) ? '}' : ']';

		case '\\':
			return (state & I4_MODIFIER_SHIFT) ? '|' : '\\';

		case ';':
			return (state & I4_MODIFIER_SHIFT) ? ':' : ';';

		case '\'':
			return (state & I4_MODIFIER_SHIFT) ? '"' : '\'';

		case ',':
			return (state & I4_MODIFIER_SHIFT) ? '<' : ',';

		case '.':
			return (state & I4_MODIFIER_SHIFT) ? '>' : '.';

		case '/':
			return (state & I4_MODIFIER_SHIFT) ? '?' : '/';

		case '`':
			return (state & I4_MODIFIER_SHIFT) ? '~' : '`';

		case I4_KP0:
		case I4_KP1:
		case I4_KP2:
		case I4_KP3:
		case I4_KP4:
		case I4_KP5:
		case I4_KP6:
		case I4_KP7:
		case I4_KP8:
		case I4_KP9:
			if (((state & I4_MODIFIER_NUMLOCK)!=0) ^
				((state & I4_MODIFIER_SHIFT)!=0))
			{
				return raw_key - I4_KP0 + '0';
			}
			else
			{
				return raw_key;
			}


		default:
			if (raw_key>='A' && raw_key<='Z')
			{
				if (state & I4_MODIFIER_CTRL)
				{
					return raw_key - 'A' + 1;
				}
				else if (!(state & I4_MODIFIER_SHIFT))
				{
					return raw_key - 'A' + 'a';
				}
			}

	}
	return raw_key;
}
//}}}


//{{{ Emacs Locals
// Local Variables:
// folded-file: t
// End:
//}}}
