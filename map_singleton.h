//All classes that derive from this have their think() method called
//once per g1_map_class::think_objects(). This is mainly for objects that need
//to manipulate the map but aren't on it themselves.
//these objects are called BEFORE any other thinking happens.
#ifndef G2_MAP_SINGLETON
#define G2_MAP_SINGLETON
#include "file/file.h"
#include "saver.h"
class g2_singleton
{
public:
	static g2_singleton *first;
	g2_singleton *next_sin;
	g2_singleton()
	{
		next_sin=first;
		first=this;
	};
	virtual ~g2_singleton()
	{
		g2_singleton *last=0, *i=first;

		for (; i && i!=this;)
		{
			last=i;
			i=i->next_sin;
		}

		if (last)
		{
			last->next_sin=next_sin;
		}
		else
		{
			first=next_sin;
		}

		next_sin=0;
	}
	virtual void think()=0;
	virtual i4_bool save(g1_saver_class *fp)=0;
	virtual i4_bool load(g1_loader_class *fp)=0;
};
#endif
