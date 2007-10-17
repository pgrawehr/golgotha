/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/

#ifndef I4_ARRAY_HH
#define I4_ARRAY_HH


#include "error/error.h"
#include "memory/malloc.h"
#include "search.h"
#include <stdlib.h>

//! Our dynamic array (vector) class.
//! This template class manages an array of objects.
//! It automatically expands when add() is called and not enough elements are
//! available.  It also has an array reference operator allowing for transparent
//! range checking. This class is comparable to stl::vector<T> in functionality.
//! \par Warning1
//! Don't use this class with reference types that need their
//! destructor called when erasing elements.
//! Use either i4_class_array or pointer to classes and delete manually
//! \par Warning2
//! Don't cast i4_class_arrays to i4_arrays! The functions are
//! non-virtual!
template <class T>
class i4_array
{
protected:
	T * entry;
	int used,entries,grow;
public:
	//! Sets used to entries.
	//! Use with care
	void set_used()
	{
		used=entries;
	}
	//! Adjust growth.
	//! If the growth is set to zero, an exception will be thrown if a resize would be needed.
	void set_grow(int _new_grow)
	{
		grow=_new_grow;
	};
	int size() const
	{
		return used;
	}
	int max_size() const
	{
		return entries;
	}
	int capacity() const
	{
		return entries;
	}
	bool empty() const
	{
		return used==0;
	}
	typedef int iterator; //an iterator for this kind of array is just an int
	T& operator[](int i) const
	{
		I4_ASSERT(i>=0 && i<used, "INTERNAL: i4_array<T>::operator[]() Bad array reference");
		return entry[i];
	}

	T& operator()(int i) const
	{
		I4_ASSERT(i>=0 && i<used, "INTERNAL: i4_array<T>::operator()() Bad array reference");
		return entry[i];
	}

	int begin()
	{
		return 0;
	}
	int end()
	{
		return used;
	}
	void reallocate(int _entries, int _grow = -1)
	{
		if (_grow>=0)
		{
			grow = _grow;
		}

		entries = _entries;
		T * new_entry = (T *)I4_REALLOC(entry, sizeof(T)*entries, "grow array");
		I4_ASSERT(new_entry, "SEVERE: i4_array::resize(): Out of memory.");
		entry = new_entry;
	}

	//! Resize the array (Ensure the array contains a specified number of elements).
	//! This also sets the used to entries
	void resize(int _entries, int _grow =-1)
	{
		reallocate(_entries,_grow);
		used=_entries;
	}

	//! Array constructor.
	//! For security reasons, the array does not have a parameterless default constructor.
	//! The problem is that this would lead people to forget that they'd have to clean up
	//! after array usage in a class.
	i4_array(int entries, int grow = 0) :
		entries(entries),
		grow(grow),
		entry(0),
		used(0)
	{
		if (entries>0)
		{
			entry = (T *)I4_MALLOC(sizeof(T)*entries,"grow array");
			I4_ASSERT(entry, "SEVERE: i4_array::i4_array(): Out of memory.");
		}
		else
		{
			entry = 0;
		}
	}

	//! Copy constructor.
	i4_array(const i4_array<T> &tocopy)
	{
		entry=0;
		used=0;
		entries=0;
		grow=tocopy.grow;
		if (tocopy.used>0)
		{
			entries=tocopy.entries;
			entry=(T *)I4_MALLOC(sizeof(T)*entries,"copied grow array");
			memcpy(entry,tocopy.entry,sizeof(T)*used); //copy all the used elements (rest will be left uninitalized)
			I4_ASSERT(entry, "SEVERE: i4_array::copy.ctor(): Out of memory");
			used=tocopy.used;
		}
	}

	i4_array& operator=(const i4_array& assign)
	{
		if (entry)
		{
			i4_free(entry);
		}
		entry=0;
		used=0;
		entries=0;
		grow=assign.grow;
		if (assign.used>0)
		{
			entries=assign.entries;
			used=assign.used;
			int elemsize=sizeof(T);
			entry=(T *)I4_MALLOC(elemsize*entries,"re-assigned grow array");
			//copy all the used elements (rest will be left uninitalized)
			memcpy(entry,assign.entry,elemsize*used);
			I4_ASSERT(entry, "SEVERE: i4_array::operator=(): Out of memory");

		}
		return *this;
	}

	void uninit()     // frees memory (use clear just to reset)
	{
		if (entry)
		{
			i4_free(entry);
		}
		entry=0;
		used=0;
		entries=0;
	}

	~i4_array()
	{
		uninit();
	}

	void grow_bigger()
	{
		if (grow)
		{
			entries += grow;

			T * new_entry = (T *)I4_REALLOC(entry, sizeof(T)*entries, "grow array");

			I4_ASSERT(new_entry, "SEVERE: i4_array::grow_bigger(): Out of memory");

			entry = new_entry;
		}
		else
		{
			I4_ASSERT(0, "INTERNAL i4_array::grow_bigger(): Out of entries in static array.");
		}
	}

	T *add_at(int ref)
	{
		if (used==entries)
		{
			grow_bigger();
		}

		for (int i=used; i>ref; i--)
		{
			entry[i] = entry[i-1];
		}
		used++;
		return entry+ref;
	}

	T *add()
	{
		if (used==entries)
		{
			grow_bigger();
		}

		T * ret=entry+used;
		used++;
		return ret;
	}

	T *add_many(int num)
	{
		while (used+num>entries)
			grow_bigger();



		T * ret=entry+used;
		used+=num;
		return ret;
	}

	int add_at(T item, int ref)
	{
		T * q=add_at(ref);

		*q=item;
		return ref;
	}

	int add(T item)
	{
		T * q=add();

		*q=item;
		return used-1;
	}


	int add_array(const i4_array& tab,int ref = -1)
	{
		if (ref<0)
		{
			ref += used+1;
		}

		I4_ASSERT(ref>=0 && ref<=used,"INTERNAL: i4_array::add_array() bad item referenced");

		if (used+tab.size() >= entries)
		{
			if (grow)
			{
				if (used+tab.size() >= entries+grow)
				{
					entries = used+tab.size();
				}
				else
				{
					entries += grow;
				}

				T * new_entry = (T *)realloc(entry, sizeof(T)*entries);

				I4_ASSERT(new_entry, "INTERNAL: i4_array::add_array() out of memory");

				entry = new_entry;
			}
			else
			{
				I4_ASSERT(0, "INTERNAL: i4_array::add_array() out of entries");
			}
		}

		int i;

		for (i=used-1; i>ref; i--)
		{
			entry[i+tab.size()] = entry[i];
		}
		for (i=0; i<tab.size(); i++)
		{
			entry[ref+i] = tab.entry[i];
		}

		used+=tab.size();

		return ref;
	}

	void remove(int ref)
	{
		I4_ASSERT(ref>=0 && ref<used, "INTERNAL: i4_array<T>::remove() Bad item deletion");

		used--;
		for (int i=ref; i<used; i++)
		{
			entry[i] = entry[i+1];
		}
	}

	void clear()
	{
		used = 0;
	}

	void sort(int (* compar)(const T *, const T *))
	{
		typedef int (*compare_type)(const void * x, const void * y);

		qsort(entry, used, sizeof(T), (compare_type)compar);
	}

	int binary_search(const T * find, int (* compar)(const T * a, const T * b))
	{
		if (size()==0)
		{
			return -1;
		}

		w32 res;

		if (i4_base_bsearch(find, res, entry, sizeof(T), (w32)size(), (i4_bsearch_compare_function_type)compar))
		{
			return res;
		}
		else
		{
			return -1;
		}
	}

	void push_back(T& x)
	{
		add(x);
	}

	void push_front(T& x)
	{
		add_at(x,0);
	}
	void pop_back()
	{
		remove(used-1);
	}

};

/*
   //PG: Too complicated. If I really need this, I should find a way
   //to make the stl vector class really portable...
   template<typename T>
   class class_type_allocator
   {
   	public:
   	void destr(T* elem){elem->~T();}
   	//duh, now we're using really strange things like placement new...
   	void alloc(T*& elem){new (elem) T());}
   };

   template<typename T>
   class ptr_type_allocator
   {
   	public:
   	void destr(T* elem){delete elem;}
   	void alloc(T*& elem){elem=new T();}
   };
   //this class can also be used for members which need their destructors
   //called automatically


   template<typename T, class allocator=class_type_allocator<T> >
   class i4_class_array:public i4_array<T>
   {
   private:
   	allocator alloc;
   public:
   i4_class_array(int entries, int grow=0):i4_array<T>(entries,grow){};
   T* add()
   	  {
   	  if (used==entries)
   		grow_bigger();

   	  T *ret=entry+used;
   	  alloc.alloc(ret);
   	  used++;
   	  return ret;
   	  }
   T *add_at(int ref)
   {
   	if (used==entries)
   	  grow_bigger();
   	alloc.alloc(&entry[used]);
   	for (int i=used; i>ref; i--)
   	  entry[i] = entry[i-1];
   	used++;
   	return entry+ref;
   }
   void push_back(T item)
   {
   	T *q=add();
 *q=item;
   }

   void pop_back()
   {
   	used--;
   	alloc.destr(&entry[used]);
   	//entry[used].T::~T();
   }

   void erase(int en)
   {
   	 alloc.destr(&entry[en]);
   	 remove(en);
   };
   void clear()
   {
   	 for(int i=0;i<used;i++)
   	 {
   	   alloc.destr(&entry[i]);
   	 }
   	 used=0;
   }
   void remove(int ref)
   {
   	I4_ASSERT(ref>=0 && ref<used, "INTERNAL: i4_array<T>::remove() Bad item deletion");
   	alloc.destr(&entry[ref]);
   	used--;
   	for (int i=ref; i<used; i++)
   	  entry[i] = entry[i+1];
   }
   ~i4_class_array()
   {
   	clear();
   	uninit();
   }
   };
 */

#endif
