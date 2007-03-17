/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/

#ifndef I4_PI_HH
#define I4_PI_HH

#include "math/num_type.h"
//these pseudo-functions have only float - precision!
#undef BE_VERY_SLOW

#ifdef BE_VERY_SLOW
inline i4_float i4_pi()
{
	return 3.141592654f;
}
inline i4_float i4_2pi()
{
	return 2.0*3.141592654f;
}
inline i4_float i4_deg2rad(i4_float deg)
{
	return deg/180.0f *i4_pi();
}
#else

#define i4_pi() (3.141592654f)
#define i4_2pi() (6.28318530718f)
#define i4_deg2rad(deg) (deg*1.74532925199E-2f)
#define i4_rad2deg(rad) (rad*57.2957795132f)

//this means pi/2
#define i4_pi_2() (1.5707963268f)
//pi/4
#define i4_pi_4() (0.7853981633f)
//3pi/4
#define i4_pi_3_4() (2.356194490f)
//3pi/2
#define i4_pi_3_2() (4.7123889803f)
//pi/8
#define i4_pi_8() (0.39269908170f)
#endif

#endif
