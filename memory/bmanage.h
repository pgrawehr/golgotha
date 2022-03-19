/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/

#ifndef BMANAGE_HH__
#define BMANAGE_HH__

#include "arch.h"
#include "memory/malloc.h"
class i4_file_class;  // file/file.hh

struct memory_node
{
	swptr size;
#ifdef i4_MEM_CHECK
	char * name;                   // name is allocated on regular heap
#endif                            // because it is used for debugging purposes
								  // and will probably be run on my linux box with VMM
	memory_node * next;
};


struct small_block
{
	swptr size;                   // size of blocks...
	wptr alloc_list;             // bit field saying weither each block is allocated or not.
	small_block * next;                    // next small block of same size
#ifdef i4_MEM_CHECK
	char * name[32];
#endif
} ;

// above 128 bytes is considered to be a big block and no hashing is done
#define JM_SMALL_SIZE 128

class i4_block_manager_class
{
public:

	void init(void * block, long Block_size);
	void *alloc(long size, char * name);
	void free(void * ptr);

	swptr largest_free_block();
	swptr available();
	swptr allocated();
	swptr pointer_size(void * ptr);
	void report(i4_file_class * fp);
	void inspect();
};

#endif
