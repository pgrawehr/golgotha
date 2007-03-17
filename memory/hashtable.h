/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/
//This fille belongs to the revival project
//Implements a hashtable for objects.
//Hashtables have under optimal circumstances
//insert() in O(1)
//get() in O(1)
//remove() in O(1)
//For non-cs - people: This means that insert, get, remove take a constant
//amount of time, regardless of the size of the table.
//drawback: next() and prev() are very slow. This implementation doesn't
//give these functions. next() is only used for duplicate keys.
//
//Warning 1: This code has not yet been tested throughfully
//Warning 2: This code is "as is" not thread-safe.

#ifndef HASHTABLE_H
#define HASHTABLE_H
#include "arch.h"
#include "string/string.h"
#include "memory/array.h"
#include "checksum/checksum.h"
#include "string/str_checksum.h"

//to disable the warning that operator-> is not allowed on void
//#pragma warning(disable:4284)

template <class T>
class i4_hashtable
{
public:
	enum {
		NONE=0,
		KEYADRESS=1,    //Use this flag, if the key value is an address.
		//this prevents the use of the lowest 2 bits for hashing.
		MASTERONLY=2,
		FORCESLAVES=4,
		INVALID=8,
		KEYSONLY=16,    //use this flag if the keys are the entries themselves
		//sensefull only if the array should just be deleted
		WAITFORINIT=32     //specify this if instance is created by global
					 //constructor. Use init() before actual usage.
	};
public:
	class iterator;
	friend class iterator;
	i4_hashtable(w32 master_size, w32 flags) :
		master_size(master_size),
		//master_table(0,master_size), //must actually not grow (would need reordering the entire table)
		//but we must not init an i4_array before i4_init()
		//sub_tables(0,master_size),
		sub_size(master_size),
		master_flags(flags)
	{
		//master_table=new i4_hashtable_entry[master_size+1];
		master_table=new LP_HASHTABLE_ENTRY[master_size+1];
		current=0;
		actual_entries=0;
		for (w32 i=0; i<master_size; i++)
		{
			master_table[i]=new i4_hashtable_entry();
			master_table[i]->flags=INVALID;
		}
	};
protected:
	typedef T *entry_type;
	class i4_hashtable_entry;
	i4_hashtable_entry **master_table;
	//i4_array<i4_hashtable<T> *> sub_tables;//not used yet
	w32 master_size;    //for general case, we need to use a real MOD instead of AND
	w32 sub_size;
	w32 master_flags;
	i4_hashtable_entry *current;    //temporary value for next();
	class i4_hashtable_entry
	{
protected:
		w32 key;
		entry_type entry;
		w32 flags;
		i4_hashtable_entry *next;
		friend class i4_hashtable<T>;
		friend class iterator;
		i4_hashtable_entry()
		{
			key=0;
			entry=0;
			flags=0;
			next=0;
		}
		i4_hashtable_entry(w32 k,entry_type e, w32 f)
		{
			key=k;
			entry=e;
			flags=f;
			next=0;
		};
		~i4_hashtable_entry()
		{
		};                      //don't do something weird here
		void set_next(i4_hashtable_entry *n)
		{
			next=n;
		};
		i4_hashtable_entry *get_next()
		{
			return next;
		};

	};
public:
	class iterator //used to iterate over all entries (out-of-any-order-iterator)
	//remark: doesn't work if modifications (insertions or deletions) are done within the iteration
	{
protected:
		w32 current_slot;
		i4_hashtable_entry *cu;
		i4_hashtable<T> *_this;
		void find_next_nonempty_slot()
		{
			/*P: cu==0; */
			do
			{
				current_slot++;
			} while (current_slot<_this->master_size && _this->master_table[current_slot]->flags==INVALID);
			if (current_slot>=_this->master_size)
			{
				return;
			}
			cu=_this->master_table[current_slot];
		}
		void advance()
		{
			if (_this==0)
			{
				//should become the end() - iterator
				return;
			}
			else if (current_slot==0 && cu==0)    //first element
			{
				if (_this->master_table[0]->flags!=INVALID)
				{
					cu=_this->master_table[0];
					return;
				}
				find_next_nonempty_slot();
			}
			else if (cu)
			{
				cu=cu->next;
				if (!cu)
				{
					find_next_nonempty_slot();
				}
			}
			else
			{
				return;
			}              //end of iteration
		}
public:
		iterator(i4_hashtable<T> *ref)
		{
			_this=ref;
			current_slot=0;
			cu=0;
			advance();
		}
		iterator()
		{
			_this=0;
			current_slot=0;
			cu=0;
		}
		entry_type get()
		{
			if (cu)
			{
				return cu->entry;
			}
			else
			{
				return 0;
			}

		}
		iterator &operator++()
		{
			advance();
			return *this;
		};
		iterator &operator++(int)
		{
			advance();
			return *this;
		};
		entry_type operator*()
		{
			return get();
		}
		entry_type operator->()
		{
			return get();    //cannot return 0
		}         //why does this give warnings?
		i4_bool operator!=(const iterator &p) const
		{
			return get()!=p.get();
		}
		i4_bool operator==(const iterator &p) const
		{
			return get()==p.get();
		}
	};
	typedef i4_hashtable_entry *LP_HASHTABLE_ENTRY;
protected:
	//i4_array<i4_hashtable_entry *> master_table;

	w32 get_slot(w32 key)
	{
		if (master_flags&KEYADRESS)
		{
			return (key>>2)%master_size;
		}
		return key%master_size;
	};

public:
	iterator get_iterator()
	{
		return iterator(this);
	};
	iterator begin()
	{
		return iterator(this);
	}
	iterator end()
	{
		return iterator(0);
	}
	entry_type operator[](w32 key)
	{
		return get(key);
	}
	w32 entries()
	{
		return actual_entries;
	}
protected:
	w32 actual_entries;
public:


	void init()
	{
		actual_entries=0;

	};

	i4_hashtable() :
		master_size(128),

		//sub_tables(0,128),
		sub_size(128),
		master_flags(0)
	{
		//master_table=new i4_hashtable_entry[master_size+1];
		master_table=new LP_HASHTABLE_ENTRY[master_size+1];
		current=0;
		actual_entries=0;
		for (w32 i=0; i<master_size; i++)
		{
			master_table[i]=new i4_hashtable_entry();
			master_table[i]->flags=INVALID;
			master_table[i]->entry=0;
			master_table[i]->next=0;
		}

	};

	~i4_hashtable()
	{
		//if deletion of the entries is needed, call reset(i4_T) before destruction
		reset();
		for (w32 i=0; i<master_size; i++)
		{
			delete master_table[i];
		}
		delete[] master_table;
	};

	//replaces old entry with new. Works only if table doesn't contain duplicates
	void replace(w32 key, T *newobj,i4_bool deleteold)
	{
		T *act=remove(key);
		if (deleteold)
		{
			delete act;
		}
		insert(key,newobj);
	}

	void insert(w32 key, T *o) //will insert duplicates
	{
		actual_entries++;
		w32 s=get_slot(key);
		//Should add some "increase table size" function
		if (master_table[s]->flags==INVALID)
		{
			master_table[s]->flags=0;
			master_table[s]->key=key;
			master_table[s]->entry=o;
			master_table[s]->next=0;
			return;
		}
		i4_hashtable_entry *e=master_table[s],*last;
		last=0;
		while (e->next&&e->key<=key) //sort buckets increasing
		{
			last=e;
			e=e->next;
		}
		i4_hashtable_entry *f=new i4_hashtable_entry(key,o,0);
		if (e->key>key)
		{
			//we'll insert after this entry
			if (last)
			{
				last->next=f;
				f->next=e;
			}
			else
			{
				f->next=e;
				master_table[s]=f;
			}
			return;
		}
		e->next=f;
	};

	w32 string_hash(const i4_const_str &st)
	{
		return i4_str_checksum(st);
	}
	void insert(const i4_const_str &st, T *o) //generates key itself
	{
		insert(string_hash(st),o);
	};

	i4_bool insert_if_single(w32 key, T *o) //won't do anything if key already present
	{
		//return i4_T if really inserted
		w32 s=get_slot(key);
		if (master_table[s]->flags==INVALID)
		{
			master_table[s]->flags=0;
			master_table[s]->key=key;
			master_table[s]->entry=o;
			master_table[s]->next=0;
			actual_entries++;
			return i4_T;
		}
		i4_hashtable_entry *e=master_table[s],*last;
		last=0; //needed if using second element;
		while (e->next&&e->key<key) //sort buckets increasing
		{
			last=e;
			e=e->next;
		}
		if (e->key==key)
		{
			return i4_F;
		}
		i4_hashtable_entry *f=new i4_hashtable_entry(key,o,0);
		if (e->key>key)
		{
			//we'll insert before this entry
			if (last)
			{
				last->next=f;
				f->next=e;
			}
			else
			//comes here if we need to insert the element as the
			//first of the list
			{
				//Precondition: e==master_table[s]
				f->next=e;
				master_table[s]=f;

				//Although clearer to read, this solution has the disad-
				//vantage of not holding the integrity constraint
				//i.e a pointer to an element might not be consistent
				//f->key=e->key;//exchange elements
				//f->next=e->next;
				//f->entry=e->entry;
				//e->key=key;
				//e->entry=o;
				//e->next=f;
			}
			actual_entries++;
			return i4_T;
		}
		//comes here only if e->key<key and e->next==0
		e->next=f;
		actual_entries++;
		return i4_T;
	};

	T *next(w32 key) //returns next element with same key as last
	//must not have called delete() or something in between
	{
		if (current)
		{
			if (current->next&&current->key==current->next->key)
			{
				current=current->next;
				return current->entry;
			}
		}
		current=0;
		return 0;
	};

	T *get(w32 key) //gets element for key
	{
		w32 s=get_slot(key);
		i4_hashtable_entry *e=master_table[s];
		if (e->flags&INVALID)
		{
			return 0;
		}
		while (e->key<key)
		{
			e=e->next;
			if (e==NULL)
			{
				current=0;
				return 0;
			}
		}
		if (e->flags!=INVALID&&e->key==key)
		{
			current=e;
			return e->entry;
		}
		current=0;
		return 0;
	};

	void reset(i4_bool delete_referenced_values=i4_F) //clears out the table
	//if the parameter is true, the referenced instances will be deleted
	{
		actual_entries=0;
		for (w32 i=0; i<master_size; i++)
		{
			i4_hashtable_entry *s=master_table[i],*t=0;
			if (delete_referenced_values)
			{
				delete s->entry;
			}
			t=s;
			s->flags=INVALID;
			s->entry=0;
			s->key=0;

			s=s->next; //don't delete the head of the list
			t->next=0;
			while (s)
			{
				if (delete_referenced_values)
				{
					delete s->entry;
				}
				t=s;
				s=s->next;
				delete t;
			}
		}

	};

	T *remove(w32 key) //removes first occurence of key and returns it
	{
		w32 s=get_slot(key);
		i4_hashtable_entry *e=master_table[s],*last=0;
		if (e->flags&INVALID)
		{
			return 0;
		}
		while (e->key<key)
		{
			last=e;
			e=e->next;
		}
		if (e->flags!=INVALID&&e->key==key)
		{
			current=0;
			T *o=e->entry;

			if (e==master_table[s])
			{

				if (e->next)
				{
					//actually, we should delete e, but we delete the next element instead
					//e->key=e->next->key;
					//e->flags=e->next->flags;
					//e->entry=e->next->entry;
					last=e->next;
					//e->next=e->next->next;
					//delete last;
					//delete &master_table[s];
					//master_table[s]=e->next;
					master_table[s]=last;
					delete e;
				}
				else
				{
					e->entry=0;
					e->flags=INVALID;
				}

			}
			else
			{
				last->next=e->next;
				delete e;
			}

			actual_entries--;
			return o;
		}
		current=0;
		return 0;
	};

	void removeall(w32 key, i4_bool delete_referenced_values) //removes all occurences of key
	{
		T *o;
		actual_entries=0;
		while((o=remove(key))!=0)
		{
			if (delete_referenced_values)
			{
				delete o;
			}
		}
	};
};

#endif
