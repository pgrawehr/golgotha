/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/
//This fille belongs to the revival project
//This code is copied from the Microsoft MFC-Help.
//
#ifndef STACK_H
#define STACK_H
#include "arch.h"
#include "memory/array.h"

template <class T, int i> 
class i4_static_stack
//a stack with fixed size
{
protected:
    T StackBuffer[i];
    int cItems;
public:
    i4_static_stack( void ) {cItems=i;};
    void push( const T item );
    T pop( void );
	i4_bool isempty()
		{return cItems==i;}
};

template <class T, int i> void i4_static_stack< T, i >::push( const T item )
{
    if( cItems > 0 )
     StackBuffer[--cItems] = item;
    else
		i4_error("SEVERE: Stack structure overflow\n");
    return;
}

template <class T, int i> T i4_static_stack< T, i >::pop( void )
{
    if( cItems < i )
     return StackBuffer[cItems++];
    else

     i4_error("SEVERE: Stack structure underflow\n");
}

template <typename T>
class i4_stack:
		public i4_array<T>
{
	public:
	i4_stack(int entries,int grow=20):i4_array<T>(entries,grow){};
    T *push()
		{
        return i4_array<T>::add();
		};

	int push(T item)
		{
		return add(item);
		};

	T& top()
		{
		return i4_array<T>::entry[i4_array<T>::used-1];
		};

	T pop()
		{
		I4_ASSERT(i4_array<T>::used>0,"FATAL: i4_stack: stack underflow");
		T r=i4_array<T>::entry[i4_array<T>::used-1];
		i4_array<T>::used--;
		return r;
		};
};


#endif
