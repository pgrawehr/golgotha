//STL wrapper include file
//for convenience and to allow the stl to be replaced by own classes
#ifndef I4_STL_INCLUDE_H
#define I4_STL_INCLUDE_H
//#include <stl.h>
#undef new
#include <deque>
#include <algorithm>
//#include <complex>
//#include <list>
//#include <map> doesn't work (conflict with local namespace)
#include "memory/new.h"
using namespace std;

#endif
