/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/
/* This is new Code */

//there should always be exactly one instance that implements this interface
//present in the system
//The capabilities of this class depend on the particular inplementation
//It is intended to play one single file at a time.
#include "init/init.h"
#include "device/device.h"

class i4_movie_engine_class: public i4_event_handler_class
	{
	
	public:
		enum {
			MOVIE_SUCCESS=0,
			MOVIE_OPENFAILED,
			MOVIE_PLAYBACKFAILED,
			MOVIE_SYNCFAILURE,
			MOVIE_PLAYBACKINPROGRESS,
			MOVIE_UNKNOWNERROR,
			MOVIE_FATALERROR
			};
		
	virtual w32 play(const i4_const_str &file)=0;//wait until playback has finished (useful for movies)
	virtual w32 background_play(i4_const_str &file, i4_event_reaction_class* notify_ev)=0;//return immediatelly
	virtual void background_stop()=0;
	virtual void receive_event(i4_event *ev)
		{
		i4_event_handler_class::receive_event(ev);
		};
	};



extern i4_movie_engine_class *i4_movie;