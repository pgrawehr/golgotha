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


#ifndef LINUX_SOUND_HH
#define LINUX_SOUND_HH

#include "sound/sound.h"
#include "math/fixed_point.h"
#include <pthread.h>

typedef i4_fixed_point<10> linux_sound_index;
enum {
	LINUX_SOUND_NUM_VOICE = 8
};

class linux_sample_class
{
public:
	w8 * data;
	linux_sound_index sample_rate;
	linux_sound_index size;
};

class linux_sound_class;

class linux_voice_class :
	public i4_voice_class
{
public:
	linux_sound_index index;
	linux_sound_index increment;
	w16 volume;
	sw16 pan;
	w16 left_vol,right_vol;
	i4_bool looping, active;
	linux_sample_class * sound;

	i4_stream_wav_player * stream_man;
	float hearable_distance;

	i4_stream_wav_player *stream_manager()
	{
		return stream_man;
	}
	void set_stream_manager(i4_stream_wav_player * s)
	{
		stream_man = s;
	}


	linux_voice_class();

	~linux_voice_class();

	virtual void stop();
	virtual void play();
	virtual i4_bool is_playing();
	virtual void set_frequency(i4_frequency freq);
	virtual void set_volume(i4_volume vol);
	virtual void set_pan(i4_pan pan);
	virtual void set_looping(i4_bool yes_no);
	virtual i4_frequency get_frequency();
	virtual i4_volume get_volume();
	virtual i4_pan get_pan();

	virtual void lock(w32 start_position, w32 size,
					  void *&block1, w32 &block1_size,
					  void *&block2, w32 &block2_size);

	virtual void unlock(void * block1, w32 block1_size,
						void * block2, w32 block2_size);

	virtual w32 get_sound_position();
	virtual void set_sound_position(w32 pos);

	virtual void set_hearable_distance(float x)
	{
		hearable_distance=x;
	};
	virtual float get_hearable_distance()
	{
		return hearable_distance;
	}

	virtual void set_3d_position(i4_float x, i4_float y, i4_float z,
								 i4_bool immediately);
	virtual void set_3d_velocity(i4_float x,i4_float y,i4_float z,
								 i4_bool immediately);

};

class linux_sound_class :
	public i4_sound_manager_class
{
	friend void *linux_sound_mixer(void * arg);
protected:
	int audio_fd;
	linux_sample_class * sound;
	linux_voice_class voice[LINUX_SOUND_NUM_VOICE];
	int thread_state;

	void start_thread();
	void stop_thread();

	void initialize_volume_table();

public:
	I4_SOUND_NAME("Linux Sound");

	virtual void init();
	virtual void uninit()
	{
	}

	void set_listener_velocity(i4_float x,i4_float y,i4_float z);
	void set_listener_position(i4_float x,i4_float y,i4_float z);

	i4_voice_class *alloc(w32 buffer_size, sound_parameters &description);

	i4_voice_class *duplicate_2d(i4_voice_class * voice);

	void set_listener_orientation(i4_float f_x,i4_float f_y,i4_float f_z,
								  i4_float u_x,i4_float u_y,i4_float u_z);

	virtual void free_voice(i4_voice_class * voice);

};

extern linux_sound_class linux_sound;

#endif
