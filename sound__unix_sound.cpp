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
 

#include "error/error.h"
#include "error/alert.h"
#include "sound/linux/linux_sound.h"
#include "loaders/wav_load.h"
#include "string/string.h"
#include "file/file.h"
#include "time/profile.h"
#include "init/init.h"
#include "main/main.h"
#if 0
#include <linux/soundcard.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#endif
//this library is not yet complete. It is therefore currently disabled
//The initialization will always fail


// per OSS directions
#define BUF_SIZE 4096
unsigned char audio_buffer[BUF_SIZE];

enum { LINUX_SOUND_BUFFER_BITS = 11 };
enum { LINUX_SOUND_SAMPLE_SPEED = 11025 };

// Thread State Enumeration
enum
{
  LINUX_SOUND_UNINITIALIZED,
  LINUX_SOUND_RUNNING,
  LINUX_SOUND_REQUEST_STOP,
  LINUX_SOUND_STOPPED,
};

static sw16 volume_table[I4_SOUND_VOLUME_LEVELS][256];
static sw32 mix_buffer[1<<LINUX_SOUND_BUFFER_BITS];
static sw32 output_buffer[1<<LINUX_SOUND_BUFFER_BITS];
linux_sound_class linux_sound;

w32 _buffer_size;
sw32 total_sounds = 0;

linux_voice_class::linux_voice_class()
{
  sound_length = _buffer_size;
  stream_man = 0;

  hearable_distance = 20;

  default_frequency = get_frequency();
  set_sound_position(0);

  total_sounds++;

}

linux_voice_class::~linux_voice_class()
{
  if(is_playing()) stop();

  total_sounds--;
}

i4_bool linux_voice_class::is_playing()
{
  if(active)
    return i4_T;

  return i4_F;

}
  

void linux_voice_class::play()
{
  index = 0;
  active = 1;
}

void linux_voice_class::stop()
{
  active=0;
}


void linux_voice_class::set_frequency(i4_frequency freq)
{
  linux_sound_index f(freq);
  
  f.value /= LINUX_SOUND_SAMPLE_SPEED;
  increment = f;
}


void linux_voice_class::set_volume(i4_volume _vol)
{
  volume = _vol;

  left_vol  = (pan<0) ? ((volume < -pan)? 0 : volume + pan) : volume;
  right_vol = (pan<0) ? volume : ((volume < pan) ? 0 : volume - pan);

}


void linux_voice_class::set_pan(i4_pan _pan)
{
  pan = _pan;

  left_vol  = (pan<0) ? ((volume < -pan)? 0 : volume + pan) : volume;
  right_vol = (pan<0) ? volume : ((volume < pan) ? 0 : volume - pan);
}

void linux_voice_class::set_looping(i4_bool yes_no)
{
}

void linux_voice_class::set_3d_position(i4_float x, i4_float y, i4_float z,
                     i4_bool immediately)
{

}

void linux_voice_class::set_3d_velocity(i4_float x,i4_float y,i4_float z,
                     i4_bool immediately)
{

}

i4_frequency linux_voice_class::get_frequency()
{
  return 0;
}

i4_volume linux_voice_class::get_volume()
{
  return 0;
}

i4_pan linux_voice_class::get_pan()
{
  return 0;
}

void linux_voice_class::set_sound_position(w32 pos)
{

}

w32 linux_voice_class::get_sound_position()
{
  return 0;
}



void linux_voice_class::lock(w32 start_position, w32 size,
                             void *&block1, w32 &block1_size,
			     void *&block2, w32 &block2_size)
{

}

void linux_voice_class::unlock(void *block1, w32 block1_size,
                               void *block2, w32 block2_size)
{

}



void linux_sound_class::free_voice(i4_voice_class *voice)
{
  delete voice;
}

void linux_sound_class::start_thread()
{
  if (thread_state == LINUX_SOUND_UNINITIALIZED)
  {
    pthread_t handle;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_create(&handle, &attr, linux_sound_mixer, 0);

    thread_state = LINUX_SOUND_RUNNING;
  }
}


void linux_sound_class::stop_thread()
{
  if (thread_state != LINUX_SOUND_UNINITIALIZED)
  {
    if (thread_state == LINUX_SOUND_RUNNING)
      thread_state = LINUX_SOUND_REQUEST_STOP;

    while (thread_state != LINUX_SOUND_STOPPED && thread_state != LINUX_SOUND_UNINITIALIZED)
      sched_yield();

    thread_state = LINUX_SOUND_UNINITIALIZED;
  }
}


void linux_sound_class::initialize_volume_table()
{
  for (int level=0; level<I4_SOUND_VOLUME_LEVELS; level++)
    for (int i=0; i<256; i++)
      volume_table[level][i] = (i-128) * level;
}


void linux_sound_class::init()
{
  int i;
  
  enabled=i4_T;
//We fail here. 
//make the 1 to a 0 to load the sound modules. But you have to write 
//the code bellow first.
#if 1
  i4_warning("WARNING: The sound output has been disabled by hardcoding.");
  enabled=i4_F;
  return;
#else
  int open_mode = O_WRONLY;
  for (i=0; i<i4_global_argc; i++)
    if (i4_global_argv[i] == "-nosound")
      return ;

  if ((audio_fd = open("/dev/dsp", open_mode, 0)) == -1)
  {
    i4_warning("OSS: Unable to open /dev/dsp, sound effects disabled\n");
    return;
  }

  i4_warning("OSS: opened dsp\n");

  // 2 fragments of 2^LINUX_SOUND_BUFFER_BITS bytes
  i = 0x00020000|LINUX_SOUND_BUFFER_BITS;
  if((ioctl(audio_fd, SNDCTL_DSP_SETFRAGMENT, &i)) == -1)
  {
    i4_warning("OSS: Unable to set fragments.. This can't be good\n");
    return;
  }

  i4_warning("OSS: set fragments\n");

  i = AFMT_S16_LE;     // OSS: 16bit stereo, little endian
  if (ioctl(audio_fd, SNDCTL_DSP_SAMPLESIZE, &i) == -1)
  {
    i4_warning("OSS: 16bit stereo failed, sound effects disabled\n");    
    close(audio_fd);
    switch (i)
    {
      case AFMT_U8: i4_warning("OSS: need AFMT_U8\n");
                    break;
      case AFMT_S8: i4_warning("OSS: need AFMT_S8\n");
                    break;
      case AFMT_U16_LE: i4_warning("OSS: need AFMT_U16_LE\n");
                    break;
    }
    return;
  }

  i4_warning("OSS: set sample\n");

  i = 1;  // stereo!  ( 0 == mono ) 
  if(ioctl(audio_fd, SNDCTL_DSP_STEREO, &i) == -1)
  {
    i4_warning("OSS: setting stereo failed.  That can't be good.\n");
    close(audio_fd);
    if(i != 1)
      i4_warning("OSS: device doesn't support stereo. damn. \n");
    return;
  }

  i4_warning("OSS: set stereo\n");

  i = LINUX_SOUND_SAMPLE_SPEED; 
  if (ioctl(audio_fd, SNDCTL_DSP_SPEED, &i)<0)
  {
    i4_warning("OSS: dsp_speed 11025 failed\n");    
    close(audio_fd);
    return;
  }

  i4_warning("OSS: set speed\n");

  thread_state = LINUX_SOUND_UNINITIALIZED;

  initialize_volume_table();

  start_thread();
#endif
  i4_sound_manager_class::init();
}

void linux_sound_class::set_listener_velocity(i4_float x,i4_float y,i4_float z)
{

}

void linux_sound_class::set_listener_position(i4_float x,i4_float y,i4_float z)
{

}

void linux_sound_class::set_listener_orientation(i4_float f_x,i4_float f_y,
                        i4_float f_z, i4_float u_x,i4_float u_y,i4_float u_z)
{

}


i4_voice_class *linux_sound_class::alloc(w32 buffer_size, sound_parameters &desc)
{

  int i=0; 

//  if (!sound[sound_id].data)
//    return 0;

  while (i<LINUX_SOUND_NUM_VOICE && voice[i].sound)
    i++;

  if (i<LINUX_SOUND_NUM_VOICE)
  {
    voice[i].looping = desc.looping;
    voice[i].index = 0;
    voice[i].active = 0;

    voice[i].set_frequency(desc.frequency);
    voice[i].set_volume(desc.volume);
    voice[i].set_pan(desc.pan);

//    voice[i].sound = &sound[sound_id];

    return &voice[i];
  }
  else

    return 0;
}

i4_voice_class *linux_sound_class::duplicate_2d(i4_voice_class *voice)
{

}


void *linux_sound_mixer(void *arg)
{
#if 0
  w16 voc,i;

  while (linux_sound.thread_state == LINUX_SOUND_RUNNING)
  {
    memset(mix_buffer, 0, sizeof(mix_buffer));
    memset(output_buffer, 0, sizeof(output_buffer));
    for (voc=0; voc<LINUX_SOUND_NUM_VOICE; voc++)
    {
      linux_voice_class& v(linux_sound.voice[voc]);

      if (v.sound && v.active)
      {
        for (i=0; i<1<<(LINUX_SOUND_BUFFER_BITS-2); i++)
        {
          mix_buffer[i] += volume_table[v.left_vol][ v.sound->data[ w32(v.index) ] ];
          output_buffer[i] += volume_table[v.right_vol][ v.sound->data[ w32(v.index) ] ];
          v.index += v.increment;

          if (v.index >= v.sound->size)
          {
            if ( !v.looping /** && (v.complete == 0 || v.complete(&v)) **/  )
            {
              v.sound = 0;
              break;
            }
            else
              while (v.index >= v.sound->size)
                v.index -= v.sound->size;
          }
        }
      }
    }
    for (i=0; i<1<<(LINUX_SOUND_BUFFER_BITS-2); i++)
    {
      sw32 val;

      val = output_buffer[i];
      val = (w16)((val<-32768) ? -32768 : ( (val>32767)? 32767 : val ));
      output_buffer[i] = val;
      val = mix_buffer[i];
      val = (w16)((val<-32768) ? -32768 : ( (val>32767)? 32767 : val ));
      output_buffer[i] |= val<<16;
    }
    write(linux_sound.audio_fd, output_buffer, 1<<LINUX_SOUND_BUFFER_BITS);
    sched_yield();
  }

  linux_sound.thread_state = LINUX_SOUND_STOPPED;

#endif
  pthread_exit(0);
}
