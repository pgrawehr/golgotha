//Everything you need to randomize some aspects of the game

#ifndef TRANSPORT_RANDOMIZE_H
#define TRANSPORT_RANDOMIZE_H
#include "map_singleton.h"


//this class creates quite good but reproduceable random numbers
//between lower and upper limits. The difference between lower
//and upper should not be larger than the greatest number in the 
//random seeds file.
//This file has the format of having one random number on each line.
//You get such a file i.e. from www.random.org with the setting columns=1
class g2_random_data
	{
	protected:
		w32 seed;
		w32 seed2;
		w32 *data;
		w32 datasize;
		w32 lower, upper;
	public:
		g2_random_data(w32 initialseed, const i4_const_str &file, w32 maxdata=100);
		void set_limits(w32 _lower, w32 _upper)
			{
			lower=_lower;
			upper=_upper;
			};
		void get_limits(w32 &_lower, w32 &_upper)
			{
			_lower=lower;
			_upper=upper;
			}
		~g2_random_data();
		w32 rnd();
		i4_bool save(g1_saver_class *fp);
		i4_bool load(g1_loader_class *fp);

	};
/*if an instance of this class exists, the think que will be
randomly scrambled once every few ticks
This avoids artefacts that come from pure sequential update:
Some units are always priviledged over others (i.e due to position
on map or age)
This changes the behaviour to random sequential update.
The ideal solution would actually be a paralell update, but the
engine is currently not supporting that without huge modifications.
*/

class g2_scramble_thinkers:public g2_singleton
	{
	public:
		g2_random_data rnd;
		g2_scramble_thinkers(w32 initialseed);
		~g2_scramble_thinkers();
		void think();
		i4_bool save(g1_saver_class *fp);
		i4_bool load(g1_loader_class *fp);
	};



#endif
