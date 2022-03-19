/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/

#ifndef __NUM_TYPE_HPP_
#define __NUM_TYPE_HPP_

#include "arch.h"

/*
   Use i4_float instead of float so we can
   substitute fixed point later on
   (would now only make sense on systems
   without a fpu)
 */

typedef float i4_float;
typedef float i4_angle;   // stored in radians
typedef double i4_double;  //for traffsim we need high precision
//since the map dimensions are huge.

inline i4_float i4_fabs(const i4_float &x)
{
	if (x<0)
	{
		return -x;
	}
	else
	{
		return x;
	}
}

#if defined WIN32 && !defined I4_64BITCPU
inline sw32 i4_f_to_i(i4_float f)
{
	w16 i4_f_to_i_control_word_1;
	w16 i4_f_to_i_control_word_2;
	sw32 res;

	__asm
	{
		fld f

		wait
		fnstcw i4_f_to_i_control_word_1
		wait

		mov ax,i4_f_to_i_control_word_1
		or     ah,0xC
		mov word ptr i4_f_to_i_control_word_2,ax
		fldcw i4_f_to_i_control_word_2
		fistp res
		fldcw i4_f_to_i_control_word_1
	}
	return res;
}

inline sw32 i4_s_to_i(double f)
{
	w16 i4_f_to_i_control_word_1;
	w16 i4_f_to_i_control_word_2;
	sw32 res;

	__asm
	{
		fld qword ptr [f]

		fnstcw i4_f_to_i_control_word_1

		mov ax,i4_f_to_i_control_word_1
		or     ah,0xC
		mov word ptr i4_f_to_i_control_word_2,ax
		fldcw i4_f_to_i_control_word_2
		fistp res
		fldcw i4_f_to_i_control_word_1
	}
	return res;
}

#else

#define i4_f_to_i(a) (sw32)(a)
#define i4_d_to_i(a) (sw32)(a)

#endif

inline i4_float i4_interpolate(i4_float from, i4_float to, i4_float frac)
{
	return (to - from)*frac + from;
}

#endif
