#include "pch.h"

/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

// This is a windows-only file.
#include "error/error.h"
#include "error/alert.h"

#include "sound/dsound/direct_sound.h"
#include "sound/dsound/ds_error.h"

//This sound library is not used any more.
//#include "sound/dsound/a3d.h"

#include "loaders/wav_load.h"
#include "string/string.h"
#include "file/file.h"
#include "main/win_main.h"
#include "time/profile.h"
#include "main/main.h"
#include "lisp/lisp.h"
#include "lisp/li_init.h"

#include <process.h>
//#ifdef _DEBUG
//#define new DEBUG_NEW
//#endif

sw32 play_count = 0;
sw32 stop_count = 0;
sw32 total_sounds = 0;

//global class declaration
direct_sound_class i4_direct_sound_class_instance;

static sw16 i4_direct_sound_volume_table[I4_SOUND_VOLUME_LEVELS] = 
{
  // Volume ramp
  //{{{ Note:
  //  Generated with:
  //
  // perl -e 'printf "  %6d, ", -10000; for ($i=1; $i<64; $i++)
  //   { printf( "%6d, ", 1000*log($i/63)); if ($i%10 == 9) { print "\n  "; }}; print "\n"'
  //}}}

  -10000,  -4143,  -3449,  -3044,  -2756,  -2533,  -2351,  -2197,  -2063,  -1945, 
   -1840,  -1745,  -1658,  -1578,  -1504,  -1435,  -1370,  -1309,  -1252,  -1198, 
   -1147,  -1098,  -1052,  -1007,   -965,   -924,   -885,   -847,   -810,   -775, 
    -741,   -709,   -677,   -646,   -616,   -587,   -559,   -532,   -505,   -479, 
    -454,   -429,   -405,   -381,   -358,   -336,   -314,   -292,   -271,   -251, 
    -231,   -211,   -191,   -172,   -154,   -135,   -117,   -100,    -82,    -65, 
     -48,    -32,    -16,      0
};


//}}}

dsound_buffer_class::dsound_buffer_class(IDirectSoundBuffer *_pDSB, 
                                         DWORD _flags, w32 _buffer_size)
{
  pDSB         = _pDSB;
  p3DSB        = 0;
  pNotify      = 0;
  flags        = _flags;
  sound_length = _buffer_size;
  stream_man   = 0;

  hearable_distance = 20;

  default_frequency = get_frequency();
  set_sound_position(0);
  
  total_sounds++;
} 

dsound_buffer_class::~dsound_buffer_class()
{  
  if (is_playing()) stop();

  if (p3DSB)
  {
    p3DSB->Release();
    p3DSB = 0;
  }
  
  if (pDSB)
  {
    pDSB->Release();
    pDSB = 0;
  }
  
  if (pNotify)
  {
    pNotify->Release();
    pNotify = 0;
  }

  total_sounds--;
}


void direct_sound_class::init() 
{ 
  initialized=i4_F;
  
  // this assigns the global i4_sound_class pointer to us
  i4_sound_manager_class::init();
  
  lpDirectSound = 0;
  //lpA3D         = 0;
  lpListener    = 0;
  lpPrimary     = 0;
  enabled=i4_T;
}

void direct_sound_class::uninit()
{
  if (lpListener)
  {
    lpListener->Release();
    lpListener = 0;
  }
  
  if (lpPrimary)
  {
    lpPrimary->Release();
    lpPrimary = 0;
  }

  //if (lpA3D)
  //{
  //  lpA3D->Release();
  //  lpA3D = 0;
  //}

  if (lpDirectSound)
  {
    lpDirectSound->Release();
    lpDirectSound = 0;
  }
  initialized=i4_F;
  enabled=i4_T;
  i4_sound_manager_class::uninit();
  //CoUninitialize();
}

void dsound_buffer_class::lock(w32 start_position, w32 size, 
                               void *&block1, w32 &block1_size,
                               void *&block2, w32 &block2_size)
	{
	if (!pDSB || !i4_sound_man->is_enabled())
		{
		block1_size=0;
		block2_size=0;
		block1=block2=0;
		return;
		}
		
	
	DWORD b1s, b2s;
	
	HRESULT r=pDSB->Lock(start_position, size,
		&block1, &b1s,
		&block2, &b2s,
		0);
	
	if (!i4_dsound_check(r))
		{
		i4_warning("dsound_buffer_class::lock() failed");
		if (r==DSERR_BUFFERLOST)
			{
			i4_dsound_check(pDSB->Restore());
			r=pDSB->Lock(start_position,size,&block1,&b1s,
				&block2,&b2s,0);
			if (!i4_dsound_check(r))
				{
				block1_size=0;//Caller can test this to avoid crash.
				block2_size=0;
				block1=block2=0;
				if (!i4_sound_man->is_enabled()) return;
				i4_error("There's a problem with the DirectSound device (cannot lock buffer), try restarting your system.");
				pDSB->Release();//Ignore Problem: Continue w/o sound
				pDSB=NULL;
				//don't show this warning twice.
				i4_sound_man->disable_sound();
				
				
				return;
				}
			}
		
		}
	
	
	block1_size=b1s;
	block2_size=b2s;
	}

void dsound_buffer_class::unlock(void *block1, w32 block1_size,
                                void *block2, w32 block2_size)
{
  if (!pDSB)
    return;

  pDSB->Unlock(block1, block1_size,
               block2, block2_size);
}

void dsound_buffer_class::set_sound_position(w32 pos)
{
  if (!pDSB)
    return;
  
  pDSB->SetCurrentPosition(pos);  
}

w32 dsound_buffer_class::get_sound_position()
{
  if (!pDSB)
    return 0;

  w32 play_cursor,write_cursor;
    
  pDSB->GetCurrentPosition(&play_cursor,&write_cursor);
  
  return play_cursor;
}

i4_profile_class pf_dsound_play("dsound::play()");

void dsound_buffer_class::play()
{
  //play_count++;
  if (!pDSB) return;

  pf_dsound_play.start();
    
  HRESULT res = pDSB->Play(0,0,flags);
  i4_dsound_check(res);
    //i4_warning("dsound_buffer_class::play() failed");

  pf_dsound_play.stop();
  
  return;
}

void dsound_buffer_class::stop()
{
  //stop_count++;
  if (!pDSB)
    return;
  
  HRESULT res = pDSB->Stop();
  if (!i4_dsound_check(res))
    i4_warning("dsound_buffer_class::stop() failed");
}


void dsound_buffer_class::set_frequency(i4_frequency freq)
{
  if (!pDSB)
    return;
  
  HRESULT res = pDSB->SetFrequency(freq);
  
  if (!i4_dsound_check(res))
    i4_warning("dsound_buffer_class::set_frequency() failed");
}

i4_frequency dsound_buffer_class::get_frequency()
{
  if (!pDSB)
    return 0;
  
  DWORD f;
  pDSB->GetFrequency(&f);
  
  return f;
}


void dsound_buffer_class::set_volume(i4_volume vol)
{
  if (!pDSB)
    return;

  if (vol < 0)
    vol = 0;
  else
  if (vol >= I4_SOUND_VOLUME_LEVELS)
    vol = I4_SOUND_VOLUME_LEVELS-1;
  
  HRESULT res = pDSB->SetVolume(i4_direct_sound_volume_table[vol]);
  if (!i4_dsound_check(res))
    i4_warning("dsound_buffer_class::set_volume() failed");
}

i4_volume dsound_buffer_class::get_volume()
{
  if (!pDSB)
    return 0;
  
  //returns a direct_sound volume level, should return an i4_volume level
  LONG v;
  pDSB->GetVolume(&v);
  
  return v;
}


void dsound_buffer_class::set_pan(i4_pan pan)
{
  if (!pDSB || p3DSB) //dont call set_pan on 3d sounds, dummy
    return;  

  HRESULT res = pDSB->SetPan(pan);
  if (!i4_dsound_check(res))
    i4_warning("dsound_buffer_class::set_pan() failed");
}

i4_pan dsound_buffer_class::get_pan()
{
  if (!pDSB)
    return 0;

  LONG p;
  pDSB->GetPan(&p);

  return p;
}


void dsound_buffer_class::set_looping(i4_bool loop)
{
  if (loop)
    flags |= DSBPLAY_LOOPING;
  else
    flags &= (~DSBPLAY_LOOPING);
}

i4_bool dsound_buffer_class::is_playing()
{
  if (!pDSB)
    return i4_F;
  
  DWORD returned_status=0;

  pDSB->GetStatus(&returned_status);

  if (returned_status & DSBSTATUS_PLAYING)
    return i4_T;

  return i4_F;
}

i4_voice_class *direct_sound_class::duplicate_2d(i4_voice_class *voice)
{
  if (!voice)
    return 0;

  dsound_buffer_class *v = (dsound_buffer_class *)voice;

  if (!v->pDSB)
    return 0;

  IDirectSoundBuffer *new_pDSB=0;

  HRESULT res = lpDirectSound->DuplicateSoundBuffer(v->pDSB, &new_pDSB);

  if (!i4_dsound_check(res))
    return 0;

  dsound_buffer_class *new_voice = new dsound_buffer_class(new_pDSB, v->flags, v->sound_length);
  
  return new_voice;
}

i4_voice_class *direct_sound_class::duplicate_3d(i4_voice_class *voice)
{
  i4_voice_class *new_voice = duplicate_2d(voice);
  
  //if 3d sound is not active just return a new 2d buffer
  if (!use_3d_sound || !new_voice)
    return new_voice;
    
  dsound_buffer_class *v = (dsound_buffer_class *)new_voice;

  HRESULT res = v->pDSB->QueryInterface(IID_IDirectSound3DBuffer,(void **)&v->p3DSB);

  if (!i4_dsound_check(res))
  {
    i4_warning("query for 3d sound buffer failed");
    delete v;
    return 0;
  }
    
  res = v->p3DSB->SetMode(DS3DMODE_NORMAL,DS3D_DEFERRED);

  if (!i4_dsound_check(res))
  {
    i4_warning("DS3D sound buffer setup failed");
    delete v;
    return 0;
  }        
  
  return v;
}

void direct_sound_class::free_voice(i4_voice_class *voice)
{
  if (!voice)
    return;

  delete (dsound_buffer_class *)voice;
}


i4_bool direct_sound_class::setup()
{  
  if (initialized) return i4_T;  
  if (!enabled) return i4_F;
  enabled=i4_T;  
  //CoInitialize(0);  //winmain does this for us

  HRESULT res;

  use_3d_sound=i4_F;

  /*for (int i=1; i<(int)i4_global_argc; i++)
  {
    if (i4_global_argv[i]=="-3dsound")
      use_3d_sound=i4_T;
  }


  if (use_3d_sound)
  {
    res = CoCreateInstance(CLSID_A3d, NULL, CLSCTX_INPROC_SERVER,  
				     IID_IDirectSound, (VOID **)&lpDirectSound);
		
    if (i4_dsound_check(res))
    {
      res = lpDirectSound->Initialize((LPGUID)&(GUID_NULL));
    
      if (i4_dsound_check(res))
      {                  
        res = lpDirectSound->QueryInterface(IID_IA3d,(void **)&lpA3D);

        if (i4_dsound_check(res))
        {
          res = lpA3D->SetResourceManagerMode(A3D_RESOURCE_MODE_DYNAMIC);
          if (i4_dsound_check(res))
          {
            i4_warning("A3d Sound Manager setup successful");
          }
          else
          {
            i4_warning("A3d::SetResourceManagerMode Failed, using normal sound");
            use_3d_sound = i4_F;
          }
        }
        else
        {
          i4_warning("QueryInterface::A3d Failed, using normal sound");
          use_3d_sound = i4_F;
        }
      }
      else
      {
        i4_warning("lpDirectSound::Initialize Failed, using normal sound");
        use_3d_sound = i4_F;
      }      
    }
    else
    {
      i4_warning("CoCreateInstance::CLSID_A3d Failed, using normal sound");
      use_3d_sound = i4_F;
    }    
  }
  */
  if (i4_win32_startup_options.use2dsound ==TRUE)
  {    
    res = CoCreateInstance(CLSID_DirectSound, NULL, CLSCTX_INPROC_SERVER,  
				     IID_IDirectSound, (VOID **)&lpDirectSound);

    if (i4_dsound_check(res))
    {
      res = lpDirectSound->Initialize( (LPGUID)&(GUID_NULL) );    
      if (i4_dsound_check(res))
      {
        i4_warning("DirectSound Sound (2d) Manager setup successful");
      }
      else
      {
        i4_warning("CoCreateInstance::CLSID_DirectSound Failed, no sound");
		lpDirectSound->Release();
		lpDirectSound=0;enabled=i4_F;
        return i4_F;
      }      
    }
    else
    {
      i4_warning("lpDirectSound::Initialize Failed, no sound");
	  enabled=i4_F;
      return i4_F;
    }
  }
  else 
	  {
	  i4_warning("DirectSound not initialized due to user 'no sound' setting.");
	  enabled=i4_F;
	  return i4_F;
	  }
	// is DSSCL_EXCLUSIVE needed?
  if (!i4_dsound_check(lpDirectSound->SetCooperativeLevel(i4_win32_window_handle, DSSCL_EXCLUSIVE)))
  {
    i4_warning("i4_sound_manager_class::setup() - couldn't get exclusive sound");
	enabled=i4_F;
    return i4_F;
  }
  
  //get the primary buffer
  DSBUFFERDESC dsBD;
  ZeroMemory(&dsBD, sizeof(DSBUFFERDESC));
  dsBD.dwSize = sizeof(DSBUFFERDESC);
  
  dsBD.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_LOCHARDWARE|DSBCAPS_GETCURRENTPOSITION2|DSBCAPS_CTRLVOLUME;
  if (i4_win32_startup_options.use3dsound ==TRUE) use_3d_sound=i4_T; 
	else use_3d_sound=i4_F;
  if (use_3d_sound) dsBD.dwFlags |= DSBCAPS_CTRL3D;
    
  if (!i4_dsound_check(lpDirectSound->CreateSoundBuffer(&dsBD, &lpPrimary, 0)))
  {
    i4_warning("DirectSound Setup - couldn't access primary buffer in hardware");
    dsBD.dwFlags &= (~DSBCAPS_LOCHARDWARE);
    
    if (!i4_dsound_check(lpDirectSound->CreateSoundBuffer( &dsBD, &lpPrimary, 0)))
    {      
      if (use_3d_sound)
      {
        i4_warning("Direct Sound Setup - couldn't create primary buffer as 3D");
            
        use_3d_sound = i4_F;
        dsBD.dwFlags &= (~DSBCAPS_CTRL3D);
      
        if (!i4_dsound_check(lpDirectSound->CreateSoundBuffer( &dsBD, &lpPrimary, 0)))
        {
          i4_error("ERROR: Direct Sound Setup - couldn't allocate primary buffer. Check your soundcard-driver.");
		  enabled=i4_F;
          return i4_F;
        }
      }
      else
      {
        i4_warning("Direct Sound Setup - couldn't create primary buffer");
		enabled=i4_F;
        return i4_F;
      }
    } 
	
  }
      else 
		{
		i4_warning("DirectSound Manager (3D) Sound setup successful.");
		}
  //set the output format (22khz 16bit stereo)
  PCMWAVEFORMAT pcmwf;
  //DWORD blah;  
  ZeroMemory(&pcmwf, sizeof(PCMWAVEFORMAT));
  pcmwf.wf.wFormatTag = WAVE_FORMAT_PCM;
  pcmwf.wf.nChannels  = 2;
  pcmwf.wf.nSamplesPerSec = 22050;
  pcmwf.wf.nBlockAlign = 4;
  pcmwf.wf.nAvgBytesPerSec = 22050*4;
  pcmwf.wBitsPerSample = 16;
  
  if (!i4_dsound_check(lpPrimary->SetFormat((LPWAVEFORMATEX)&pcmwf)))
  {
    i4_warning("Unable to set the Primary Buffer Output Format");    
  }  
  
  //create a listener object
  if (use_3d_sound)
  {    
    if (!i4_dsound_check(lpPrimary->QueryInterface(IID_IDirectSound3DListener,(void **)&lpListener)))
    {
      i4_warning("listener create failed");
	  enabled=i4_F;
      return i4_F;
    }

    if (!i4_dsound_check(lpListener->SetPosition(0,0,0,DS3D_IMMEDIATE)))
    {
      i4_warning("listener position set failed");
    }

    if (!i4_dsound_check(lpListener->SetVelocity(0,0,0,DS3D_IMMEDIATE)))
    {
      i4_warning("listener velocity set failed");
    }

    if (!i4_dsound_check(lpListener->SetOrientation(0,0,1,0,1,0,DS3D_IMMEDIATE)))
    {
      i4_warning("listener orientation set failed");
    }
    
    if  (!i4_dsound_check(lpListener->SetDistanceFactor(3,DS3D_IMMEDIATE)))
    {
      i4_warning("listener distance factor set failed");
    }
   
    if (!i4_dsound_check(lpListener->SetDopplerFactor(5,DS3D_IMMEDIATE)))
    {
     i4_warning("listener doppler factor set failed");
    }    
  }  
  //lpPrimary->Play(0,0,DSBPLAY_LOOPING);
  enabled=i4_T;
  initialized = i4_T;
  return i4_T;
}

//Called exactly once per frame, so we use it for checking the status of 
//the primary sound buffer
void direct_sound_class::commit_3d_changes()
{
  w32 primarylost=0;
  if (!lpPrimary) 
	  return;
  lpPrimary->GetStatus(&primarylost);
  if (primarylost&DSBSTATUS_BUFFERLOST)
	  {
	  lpPrimary->Restore();
	  //lpPrimary->Play(0,0,DSBPLAY_LOOPING);
	  /*PCMWAVEFORMAT pcmwf;
    
	  ZeroMemory(&pcmwf, sizeof(PCMWAVEFORMAT));
      pcmwf.wf.wFormatTag = WAVE_FORMAT_PCM;
      pcmwf.wf.nChannels  = 2;
      pcmwf.wf.nSamplesPerSec = 22050;
      pcmwf.wf.nBlockAlign = 4;
      pcmwf.wf.nAvgBytesPerSec = 22050*4;
      pcmwf.wBitsPerSample = 16;
  
      if (!i4_dsound_check(lpPrimary->SetFormat((LPWAVEFORMATEX)&pcmwf)))
		  {
          i4_warning("Unable to Reset the Primary Buffer Output Format");    
		  }
		  */
	  }

  if (!use_3d_sound)
    return;

  //i4_warning("num play() calls: %d",play_count);
  //i4_warning("num stop() calls: %d",stop_count);
  //i4_warning("total sounds:     %d",total_sounds);
  play_count=0;
  stop_count=0;
  
  if (!lpListener)
    return;

  if (!i4_dsound_check(lpListener->CommitDeferredSettings()))
  {
    i4_warning("unable to commit 3d changes");
  }
}

const i4_float distance_scale = 1.f;

i4_profile_class pf_d3d_set_position("d3d_set_position");

void dsound_buffer_class::set_3d_position(i4_float x, i4_float y, i4_float z, i4_bool immediately)
{
  if (p3DSB)
  {
    DWORD apply_time;
  
    if (immediately) apply_time = DS3D_IMMEDIATE;
    else             apply_time = DS3D_DEFERRED;

    pf_d3d_set_position.start();
    HRESULT res = p3DSB->SetPosition(x*distance_scale,y*distance_scale,z*distance_scale,apply_time);
    pf_d3d_set_position.stop();
    
    if (!i4_dsound_check(res))
    {
      i4_warning("3d setPosition failed");
    }
    
    set_volume(I4_SOUND_VOLUME_LEVELS-1);


  }
  else
  {
    i4_3d_vector cam = i4_direct_sound_class_instance.listener_position;
    i4_transform_class *trans = &(i4_direct_sound_class_instance.listener_transform);
    
    i4_3d_vector delta = i4_3d_vector(x - cam.x, y - cam.y, z - cam.z);
    
    i4_float dist    = (float)sqrt(delta.x*delta.x + delta.y*delta.y + delta.z*delta.z);
    if (dist>hearable_distance)
      set_volume(0);
    else
      set_volume((long)((hearable_distance - dist)*(I4_SOUND_VOLUME_LEVELS-1)
                 /hearable_distance));
    
    delta.normalize();
    delta *= 0.1f;

    i4_float pan = (float)DSBPAN_RIGHT*(trans->x.dot(delta));

    if (pan > DSBPAN_RIGHT)
      pan = DSBPAN_RIGHT;
    else
    if (pan < DSBPAN_LEFT)
      pan = DSBPAN_LEFT;
    
    set_pan((long)pan);        
  }
}
  
void dsound_buffer_class::set_3d_velocity(i4_float x, i4_float y, i4_float z,i4_bool immediately)
{
  if (!p3DSB)
    return;
  
  DWORD apply_time;
  
  if (immediately) apply_time = DS3D_IMMEDIATE;
  else             apply_time = DS3D_DEFERRED;
  
  if (!i4_dsound_check(p3DSB->SetVelocity(x,y,z,apply_time)))
  {
    i4_warning("3d setPosition failed");
  }
}

void direct_sound_class::set_listener_position(i4_float x,i4_float y,i4_float z)
{
  listener_position = i4_3d_vector(x,y,z);
  
  if (use_3d_sound && lpListener)
  {
    if (!i4_dsound_check(lpListener->SetPosition(x*distance_scale,y*distance_scale,z*distance_scale,DS3D_DEFERRED)))
    {
      i4_warning("listener Position set failed");
    }    
  }  
}

void direct_sound_class::set_listener_velocity(i4_float x,i4_float y,i4_float z)
{
  if (!lpListener)
    return;

  if (!i4_dsound_check(lpListener->SetVelocity(x,y,z,DS3D_DEFERRED)))
  {
    i4_warning("listener velocity set failed");
  }
}

void direct_sound_class::set_listener_orientation(i4_float f_x,i4_float f_y,i4_float f_z,
                                                  i4_float u_x,i4_float u_y,i4_float u_z)
{
  if (use_3d_sound)
  {
    if (!lpListener)
      return;

    if (!i4_dsound_check(lpListener->SetOrientation(f_x,f_y,f_z,u_x,u_y,u_z,DS3D_DEFERRED)))
    {    
      i4_warning("listener Orientation set failed");
    }
  }
  else
  {
    i4_3d_vector front = i4_3d_vector(f_x,f_y,f_z);
    i4_3d_vector up    = i4_3d_vector(u_x,u_y,u_z);
    i4_3d_vector side;
    side.cross(up,front);
    side.normalize();
    
    listener_transform.identity();
    listener_transform.x = side;
    listener_transform.y = up;
    listener_transform.z = front;
  }
}

i4_voice_class *direct_sound_class::alloc(w32 buffer_size, sound_parameters &desc)
{
  if (!initialized) 
  {
	
    if (!setup())
      return 0;
  }

  IDirectSoundBuffer *pDSB=NULL;
  LPDIRECTSOUND3DBUFFER *p3DSB=NULL;
  
  DSBUFFERDESC dsBD;
  WAVEFORMATEX wfx;
  /*PCMWAVEFORMAT pcmwf;
      
  ZeroMemory(&pcmwf, sizeof(PCMWAVEFORMAT));
  pcmwf.wf.wFormatTag = WAVE_FORMAT_PCM;
  pcmwf.wf.nChannels = (w16)desc.channels;
  pcmwf.wf.nSamplesPerSec = desc.frequency;

  pcmwf.wf.nBlockAlign = (w16)(desc.sample_size * desc.channels);
  pcmwf.wf.nAvgBytesPerSec = desc.frequency * pcmwf.wf.nBlockAlign;
  pcmwf.wBitsPerSample = (w16)(desc.sample_size*8);*/
      
  wfx.cbSize=sizeof(wfx);
  wfx.nBlockAlign=(w16)(desc.sample_size*desc.channels);
  wfx.nAvgBytesPerSec=desc.frequency*wfx.nBlockAlign;
  wfx.nChannels=(w16)desc.channels;
  wfx.nSamplesPerSec=desc.frequency;
  wfx.wBitsPerSample=(w16)desc.sample_size*8;
  wfx.wFormatTag=WAVE_FORMAT_PCM;
  ZeroMemory(&dsBD,sizeof(DSBUFFERDESC));
  dsBD.dwSize = sizeof(DSBUFFERDESC);  
  dsBD.dwBufferBytes = buffer_size;
  dsBD.lpwfxFormat = &wfx;//(LPWAVEFORMATEX)&pcmwf;

  dsBD.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY|DSBCAPS_GETCURRENTPOSITION2|DSBCAPS_LOCSOFTWARE;

  if (use_3d_sound && desc.capable_3d)
    dsBD.dwFlags |= DSBCAPS_CTRL3D|DSBCAPS_MUTE3DATMAXDISTANCE;
  else
    dsBD.dwFlags |= DSBCAPS_CTRLPAN;

  if (!desc.streaming)
    dsBD.dwFlags |= DSBCAPS_STATIC;//We won't be able to restore the buffer.

  if (!i4_dsound_check(lpDirectSound->CreateSoundBuffer(&dsBD, &pDSB, 0)))
  {
    i4_warning("direct_sound_class:: couldn't alloc sound buffer");
    return 0;  
  }
  
  w32 play_flags=0;
  
  if (desc.looping)
    play_flags |= DSBPLAY_LOOPING;

  dsound_buffer_class *new_voice = new dsound_buffer_class(pDSB, play_flags, buffer_size);
  
  if (use_3d_sound && desc.capable_3d && desc.streaming)
  {
    HRESULT res = pDSB->QueryInterface(IID_IDirectSound3DBuffer,(void **)&new_voice->p3DSB);
    
    res = new_voice->p3DSB->SetMode(DS3DMODE_NORMAL,DS3D_DEFERRED);
  }

  return new_voice;
}

li_object *enablesound(li_object *o, li_environment *env)
	{
	i4_sound_man->enable_sound();
	return 0;
	}


li_object *disablesound(li_object *o, li_environment *env)
	{
	i4_sound_man->disable_sound();
	return 0;
	}

li_object *togglesound(li_object *o, li_environment *env)
	{
	i4_sound_man->toggle_sound();
	return 0;
	}

li_object *setsound(li_object *o, li_environment *env)
	{// (setsound x) will enable sound if x>=1, else it is disabled
	if (!o) return 0;
	if (li_is_number(li_first(o,env),env))
		{
			int set=li_get_int(li_first(o,env),env);
			if (set>=1) i4_direct_sound_class_instance.enable_sound();
			else i4_direct_sound_class_instance.disable_sound();
			return li_first(o,env);

		}
	return 0;
	}



li_automatic_add_function(setsound,"set_sound");
li_automatic_add_function(enablesound,"enable_sound");
li_automatic_add_function(disablesound,"disable_sound");
li_automatic_add_function(togglesound,"toggle_sound");


