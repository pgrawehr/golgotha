
//this file belongs to the revival project only

#ifndef I4_DYNQUE_HH
#define I4_DYNQUE_HH

#include "arch.h"
#include "memory/array.h"

/** Implements a dynamic queue.
   This template implements a dynamic queue that can be used as a stack, queue or
   a priority queue.
   This should be implemented as a binominal heap, for maximum performance.
   let's see wheter I'll get the theory into practice sometime.
   to use as a first-in-first-out queue:
   que() to add an element
   deque() to remove an element
   front() to get (but not remove) head
   to use as a last-in-first-out (stack) queue:
   push() to add an element
   pop() to remove an element
   top() to get (without removing) the top of the stack
   to use as a priority queue:
   insert() to add an element
   deque() to remove an element
   front() to ask what's in front
   common functions:
   empty() true, if queue is empty
   full() always false
   reset() clears list
   set_sorter() sets the sort function for the given object type (needed for prio queue)

   Be aware that mixing these uses will result in unpredictable behaviour!
 */
template <class T, w32 SIZE=50, w32 GROW=50>
class i4_dynamic_que
{
public:
	typedef int (*compare_type)(const T * a, const T * b);
protected:
	i4_array<T> data;
	w32 head, tail;
	compare_type sort_fun;
public:

	void reset()
	{
		head=tail=0;
		data.uninit();
		data.set_used();
	}

	i4_dynamic_que() :
		data(SIZE,GROW)
	{
		reset();
		sort_fun=0;
	}

	i4_dynamic_que(compare_type s_fun) :
		data(SIZE,GROW)
	{
		reset();
		sort_fun=s_fun;
	};

	~i4_dynamic_que()
	{               /*data.uninit();*/
	}                                  //the compiler should add this automatically

	i4_bool empty()
	{
		return (i4_bool)(head==tail);
	}

	//i4_bool full() { return (i4_bool)((head==SIZE-1 && tail==0) || head+1==tail); }
	i4_bool full()
	{
		return i4_F;
	};                           //implication on a dynamic queue: cannot get full

	void set_sorter(compare_type s_fun)
	{
		sort_fun=s_fun;
	};

	w32 size() //returns the number of elements in the que
	{
		if (head>=tail)
		{
			return head-tail;
		}
		else
		{
			return (data.size()-(tail-head));
		}
	}
	T& operator [](int i) //the 0'th element is the one that is at the tail
	{
		return data[(i+tail)%data.size()];
	}


	i4_bool que(const T &object) //inserts at the end of the list
	{
		w32 next_head=(head+1);

		if (data.size()==0) //because we cannot set the size in the constructor
		{
			data.grow_bigger();
			data.set_used();
		}
		if (next_head==(w32)data.size())
		{
			next_head=0;
		}

		if (next_head==tail) //full: grow array (and be careful about the elements at
		//the beginning of the array!!!)
		{
			w32 oldsize=data.size(); //should be equal to max_size() here
			data.grow_bigger();
			data.set_used(); //to avoid the array_out_of_bounds warning
			w32 newsize=data.max_size(),i,putat=oldsize;
			for (i=0; i<next_head; i++)
			{
				data[putat]=data[i];
				if (++putat>=newsize)
				{
					putat=0;
				}

			}
			next_head=putat;
			head=next_head==0 ? newsize-1 : next_head-1;
		}

		data[head]=object;
		head=next_head;
		return i4_T;
	}

	i4_bool insert(const T &object) //inserts in order
	{
		w32 i;

		que(object);
		T tempobj;
		i=size()-1;
		I4_ASSERT(sort_fun,"INTERNAL: Unspecified usage of dynamic queue.");
		while((i>0) && sort_fun(&(operator [](i)),&(operator [](i-1)))<0)
		{
			tempobj=operator[](i);
			operator[](i)=operator[](i-1);
			operator[](i-1)=tempobj;
			i--;
		}

		return i4_T;
	};

	i4_bool deque(T &object)
	{
		if (empty())
		{
			return i4_F;
		}

		object=data[tail];
		tail++;
		if (tail==(w32)data.max_size())
		{
			tail=0;
		}
		return i4_T;
	}

	i4_bool front(T &object)
	{
		if (empty())
		{
			return i4_F;
		}

		object=data[tail];
		return i4_T;
	}
};

#endif
