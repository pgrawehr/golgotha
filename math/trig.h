/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef I4_TRIG_HH
#define I4_TRIG_HH

#include <math.h>

inline i4_float i4_atan2(i4_float y, i4_float x)
{
  return (float)atan2(y,x);
}

//Can't use this with name fsqrt, will potentially give conflicts with
//the runtime library (had an infinite recursion on sgi, because the math
//library itself makes a call to fsqrt)

inline i4_float i4_fsqrt(float f)
	{
	return (float) sqrt(f);
	}


inline i4_float i4_fceil(float f)
	{
	return (float)ceil(f);
	}

inline i4_float i4_ffloor(float f)
	{
	return (float)floor(f);
	};

inline i4_float i4_flog(float f)
	{
	return (float)log(f);
	};

inline i4_float i4_fract(float f)
{
	return f-i4_ffloor(f);
}


#endif
