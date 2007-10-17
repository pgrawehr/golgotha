//implementation of the randomizer classes

#include "pch.h"
#include "map.h"
#include "g1_object.h"
#include "transport/randomize.h"
#include "file/file.h"
#include "lisp/li_all.h"
#include "map_man.h"

g2_random_data::g2_random_data(w32 initialseed, const i4_const_str &file, w32 maxdata)
{
	i4_file_class * fp=i4_open(file,I4_READ|I4_NO_BUFFER);

	data=0;
	datasize=0;
	seed=initialseed;
	seed2=1;
	upper=65536;
	lower=0;
	char buf[500];
	i4_os_string(file,buf,500);
	char * rbuf;
	if (!fp)
	{
		i4_error("ERROR: Could not load random data file %s.",buf);
		return;
	}
	rbuf= (char *) malloc(fp->size()+1);
	fp->read(rbuf,fp->size());
	w32 i=maxdata,k,j=0;
	datasize=maxdata;
	data= new w32[datasize];
	char * index=rbuf;
	while ((i>0) && (index<(rbuf+fp->size())))
	{
		li_read_token(index,buf);
		k=0;
		sscanf(buf,"%i",&k);
		if (k)
		{
			data[j]=k;
		}
		else
		{
			data[j]=1; //don't add any zeroes to the array
		}
		i--;
		j++;
	}
	datasize=j; //just in case the file was shorter than exspected
	delete fp;
	delete rbuf;
}
#define RANDOM_DATA_VERSION 1
i4_bool g2_random_data::save(g1_saver_class * fp)
{
	fp->write_32(RANDOM_DATA_VERSION);
	fp->write_32(seed);
	fp->write_32(seed2);
	fp->write_32(upper);
	fp->write_32(lower);
	fp->write_32(datasize);
	fp->write(data,datasize*4);
	return i4_T;
}

i4_bool g2_random_data::load(g1_loader_class * fp)
{
	fp->read_32();
	seed=fp->read_32();
	seed2=fp->read_32();
	upper=fp->read_32();
	lower=fp->read_32();
	datasize=fp->read_32();
	delete [] data;
	data=new w32[datasize];
	fp->read(data,datasize*4);
	return i4_T;
}

g2_random_data::~g2_random_data()
{
	delete [] data;
}

w32 g2_random_data::rnd()
{
	seed2++;
	if (seed2>datasize)
	{
		seed2=0;
	}
	seed=data[seed2]+seed;
	if (seed&0xf)
	{
		seed++;
		seed2+=2;
	}
	return ((seed%(upper-lower))+lower);
};

g2_scramble_thinkers::g2_scramble_thinkers(w32 initialseed) :
	rnd(initialseed,"resource/randnum.txt",150)
{
	rnd.set_limits(0,10000);
};

g2_scramble_thinkers::~g2_scramble_thinkers()
{
};

void g2_scramble_thinkers::think()
{
	//do the magic.
	g1_map_class * map=g1_get_map();
	w32 s=map->think_que_dyn.size();

	rnd.set_limits(0,s);

	w32 num_exchanges=(s<=100) ? s : 99; //only exchange some of the objects
	w32 f,t;
	g1_object_class * xch;
	while(num_exchanges>0)
	{
		f=rnd.rnd();
		t=rnd.rnd();
		xch=map->think_que_dyn[f]; //exchange the f.th entry with the t.th.
		map->think_que_dyn[f]=map->think_que_dyn[t];
		map->think_que_dyn[t]=xch;
		num_exchanges--;
	}
};

i4_bool g2_scramble_thinkers::save(g1_saver_class * fp)
{
	return rnd.save(fp);
}

i4_bool g2_scramble_thinkers::load(g1_loader_class * fp)
{
	return rnd.load(fp);
}
