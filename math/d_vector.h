/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/

#ifndef __DVECTOR_HPP_
#define __DVECTOR_HPP_

#include "math/num_type.h"
#include "math/vector.h"
#include <math.h>

//#ifdef _WINDOWS
//#define G1_WIN32_VECTOR_ASM //should be changed for double here
//#endif


template <class Coord>
class i4_dvector3_template
{
public:
	Coord x,y,z;

	Coord& operator()(int pos) const
	{
		return ((Coord *)&x)[pos];
	}
	Coord& operator[](int pos) const
	{
		return ((Coord *)&x)[pos];
	}

	i4_dvector3_template()
	{
	}
	i4_dvector3_template(const i4_dvector3_template& p) :
		x(p.x),
		y(p.y),
		z(p.z)
	{
	}
	i4_dvector3_template(const i4_3d_vector &p) :
		x(p.x),
		y(p.y),
		z(p.z)
	{
	}
	i4_dvector3_template(Coord _x,Coord _y,Coord _z) :
		x(_x),
		y(_y),
		z(_z)
	{
	}
	i4_dvector3_template& set(Coord px,Coord py,Coord pz)
	{
		x = px;
		y = py;
		z = pz;

		return *this;
	}
	i4_dvector3_template& operator=(const i4_dvector3_template& p)
	{
		x = p.x;
		y = p.y;
		z = p.z;

		return *this;
	}

	i4_dvector3_template& operator+=(const i4_dvector3_template& b)
	{
		x += b.x;
		y += b.y;
		z += b.z;

		return *this;
	}
	i4_dvector3_template& operator-=(const i4_dvector3_template& b)
	{
		x -= b.x;
		y -= b.y;
		z -= b.z;

		return *this;
	}

	i4_dvector3_template& operator*=(Coord b)
	{
		x *= b;
		y *= b;
		z *= b;

		return *this;
	}

	i4_dvector3_template operator*(Coord b)
	{
		i4_dvector3_template tmp;
		tmp.x = x * b;
		tmp.y = y * b;
		tmp.z = z * b;

		return tmp;
	}


	i4_dvector3_template operator+(Coord b)
	{
		i4_dvector3_template tmp;
		tmp.x = x + b;
		tmp.y = y + b;
		tmp.z = z + b;

		return tmp;
	}

	i4_dvector3_template operator+(i4_dvector3_template v)
	{
		i4_dvector3_template tmp;
		tmp.x = x + v.x;
		tmp.y = y + v.y;
		tmp.z = z + v.z;

		return tmp;
	}

	i4_dvector3_template operator*(i4_dvector3_template v)
	{
		i4_dvector3_template tmp;
		tmp.x = x * v.x;
		tmp.y = y * v.y;
		tmp.z = z * v.z;

		return tmp;
	}

	i4_dvector3_template& operator/=(Coord b)
	{
		x /= b;
		y /= b;
		z /= b;

		return *this;
	}

	i4_dvector3_template& interpolate(const i4_dvector3_template& from, const i4_dvector3_template& to,
									  i4_float ratio)
	{
		x = i4_interpolate(from.x,to.x,ratio);
		y = i4_interpolate(from.y,to.y,ratio);
		z = i4_interpolate(from.z,to.z,ratio);
		return *this;
	}

	Coord length() const
	{
		return sqrt(dot(*this));
	}
	void normalize()
	{
		*this /= length();
	}

	i4_dvector3_template& reverse()
	{
		x=-x;
		y=-y;
		z=-z;

		return *this;
	}

	Coord dot(const i4_dvector3_template& b) const
	{
/*#ifdef G1_WIN32_VECTOR_ASM
   	i4_float dotret;
   	_asm
   	{
   	  mov     ecx, b
   	  mov     eax, this

   	  ;optimized dot product; 15 cycles
   	  fld dword ptr   [eax+0]     ;starts & ends on cycle 0
   	  fmul dword ptr  [ecx+0]     ;starts on cycle 1
   	  fld dword ptr   [eax+4]     ;starts & ends on cycle 2
   	  fmul dword ptr  [ecx+4]     ;starts on cycle 3
   	  fld dword ptr   [eax+8]     ;starts & ends on cycle 4
   	  fmul dword ptr  [ecx+8]     ;starts on cycle 5
   	  fxch            st(1)       ;no cost
   	  faddp           st(2),st(0) ;starts on cycle 6, stalls for cycles 7-8
   	  faddp           st(1),st(0) ;starts on cycle 9, stalls for cycles 10-12
   	  fstp dword ptr  [dotret]    ;starts on cycle 13, ends on cycle 14
   	}
   	return dotret;
 #else*/
		return x*b.x + y*b.y + z*b.z;
//#endif
	}

	i4_dvector3_template& cross(const i4_dvector3_template& a,
								const i4_dvector3_template& b)
	{
		x = a.y*b.z - a.z*b.y;
		y = a.z*b.x - a.x*b.z;
		z = a.x*b.y - a.y*b.x;
		return *this;
	}
};


class i4_d2d_vector
{
public:
	i4_double x,y;

	i4_d2d_vector()
	{
		;
	}

	i4_d2d_vector(const i4_2d_vector &from)
	{
		x=from.x;
		y=from.y;
	};

	i4_d2d_vector(i4_double x, i4_double y) :
		x(x),
		y(y)
	{
	}

	i4_double length() const
	{
		return sqrt(x*x+y*y);
	}
	i4_double dot(const i4_d2d_vector &b)
	{
		return x*b.x + y*b.y;
	}
	i4_double perp_dot(const i4_d2d_vector &b)
	{
		return x*b.y-y*b.x;
	}

	i4_d2d_vector& operator-=(const i4_d2d_vector& b)
	{
		x-=b.x;
		y-=b.y;
		return *this;
	}
	i4_d2d_vector& operator+=(const i4_d2d_vector& b)
	{
		x+=b.x;
		y+=b.y;
		return *this;
	}
	i4_d2d_vector& operator*=(const i4_d2d_vector& b)
	{
		x*=b.x;
		y*=b.y;
		return *this;
	}
	i4_d2d_vector& operator/=(const i4_d2d_vector& b)
	{
		x/=b.x;
		y/=b.y;
		return *this;
	}

	i4_d2d_vector operator+(const i4_d2d_vector& b)
	{
		return i4_d2d_vector(x+b.x, y+b.y);
	}
	i4_d2d_vector operator-(const i4_d2d_vector& b)
	{
		return i4_d2d_vector(x-b.x, y-b.y);
	}
	i4_d2d_vector operator*(const i4_d2d_vector& b)
	{
		return i4_d2d_vector(x*b.x, y*b.y);
	}
	i4_d2d_vector operator/(const i4_d2d_vector& b)
	{
		return i4_d2d_vector(x*b.x, y*b.y);
	}

	void normalize()
	{
		i4_double ool=1.0/length();
		x*=ool;
		y*=ool;
	}
};


typedef i4_dvector3_template<i4_double>  i4_d3d_vector;

#endif
