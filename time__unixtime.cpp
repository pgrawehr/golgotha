/**********************************************************************

   	Golgotha Forever - A portable, free 3D strategy and FPS game.
   	Copyright (C) 1999 Golgotha Forever Developers

   	Sources contained in this distribution were derived from
   	Crack Dot Com's public release of Golgotha which can be
   	found here:  http://www.crack.com

   	All changes and new works are licensed under the GPL:

   	This program is free software; you can redistribute it and/or modify
   	it under the terms of the GNU General Public License as published by
   	the Free Software Foundation; either version 2 of the License, or
   	(at your option) any later version.

   	This program is distributed in the hope that it will be useful,
   	but WITHOUT ANY WARRANTY; without even the implied warranty of
   	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   	GNU General Public License for more details.

   	You should have received a copy of the GNU General Public License
   	along with this program; if not, write to the Free Software
   	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

   	For the full license, see COPYING.

 ***********************************************************************/

#include "pch.h"
#include "time/time.h"
//#include <stdio.h>
//#include <stdlib.h>
//#include <sys/time.h>
//#include <unistd.h>

void i4_time_class::get()
{
	struct timezone tz={
		0,0
	};
	struct timeval tv;

	gettimeofday( (struct timeval *)&tv,&tz);

	time.unix_time.sec=tv.tv_sec;
	time.unix_time.usec=tv.tv_usec;
}


void i4_time_class::add_milli(sw32 milli_sec)
{

	sw32 new_usec;

	if (milli_sec>0)
	{
		if (milli_sec>1000)
		{
			int sec_add=milli_sec/1000;
			time.unix_time.sec+=sec_add;
			milli_sec-=sec_add*1000;
		}

		new_usec=time.unix_time.usec+milli_sec*1000;
		if (new_usec>1000*1000)
		{
			time.unix_time.sec++;
			new_usec-=1000*1000;
		}
	}
	else
	{
		if (milli_sec<-1000)
		{
			int sec_add=milli_sec/1000;
			time.unix_time.sec+=sec_add;
			milli_sec-=sec_add*1000;
		}

		new_usec=time.unix_time.usec-milli_sec*1000;
		if (new_usec<0)
		{
			new_usec+=1000*1000;
			time.unix_time.sec--;
		}
	}
	time.unix_time.usec=new_usec;
}


i4_bool i4_time_class::operator >(const i4_time_class &other) const
{
	return (other.time.unix_time.sec<time.unix_time.sec ||
			(other.time.unix_time.sec==time.unix_time.sec &&
			 other.time.unix_time.usec<time.unix_time.usec));
}


i4_bool i4_time_class::operator <(const i4_time_class &other) const
{
	return (other.time.unix_time.sec>time.unix_time.sec ||
			(other.time.unix_time.sec==time.unix_time.sec &&
			 other.time.unix_time.usec>time.unix_time.usec));
}



sw32 i4_time_class::milli_diff(const i4_time_class &past_time) const
{
	return (time.unix_time.usec-past_time.time.unix_time.usec)/1000 +
		   (time.unix_time.sec-past_time.time.unix_time.sec)*1000;
}

i4_time_class::i4_time_class(sw32 milli_sec)
{
	sw32 sec=milli_sec/1000;

	time.unix_time.sec=sec;
	milli_sec-=sec*1000;
	time.unix_time.usec=milli_sec*1000;
}


w64 i4_get_system_clock()
{
	struct timeval tv;
	struct timezone tz;

	gettimeofday(&tv, &tz);
	return( (w64)(tv.tv_sec)*1000000 + (w64)(tv.tv_usec) );
}


int i4_win_clocks_per_sec = -1;

int i4_get_clocks_per_second()
{
	if (i4_win_clocks_per_sec==-1)
	{
		w64 _start = i4_get_system_clock();

		i4_time_class now,start;
		while (now.milli_diff(start) < 1000) now.get();



		w64 end = i4_get_system_clock();

		i4_win_clocks_per_sec = end - _start;
	}

	return i4_win_clocks_per_sec;
}

void i4_sleep(int seconds)
{
	sleep(seconds);
}
void i4_milli_sleep(int milli_seconds)
{
	usleep(milli_seconds*1000);
}
