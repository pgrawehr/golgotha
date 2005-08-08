/* \file 
Transport simulation loading, saving and thinking code.
(c) 2002 Patrick Grawehr
*/

#include "pch.h"
#include "transport/transport.h"
#include "file/file.h"
#include "lisp/li_all.h"
#include "map.h"
#include "math/random.h"
#include "objs/path_object.h"
#include "objs/road_object.h"
#include "status/status.h"
#include "objs/model_id.h"
#include "map_vars.h"
#include "objs/car.h"
#include "map_man.h"

extern g1_object_type car_type;


// NODE MANAGER METHODS
// ********************

node *node_manager::get_node(node_id id)
	{
	node *n=0;
	if (!use_active)
		n=(node *)nodes.get(id);
	else
		{
		i4_error("ERROR: node_manager::get_node() invalid on active map");
		return 0;
		}
	if (!n)
		i4_error("ERROR: Invalid Node requested.");
	return n;
	}

obj_ref_t *node_manager::get_obj(node_id id)
	{
	if (!use_active)
		return 0;
	return (obj_ref_t *) nodes.get(id);
	}

node *node_manager::add_node(node *newnode)
	{
	nodes.insert_if_single(newnode->id,newnode);
	return newnode;
	};

void node_manager::remove_node(node *rem)
	{
	nodes.remove(rem->id);
	}

void node_manager::remove_node(node_id nid)
	{
	nodes.remove(nid);//don't need to delete anything
	};

i4_bool node_manager::load_nodes(const i4_const_str &filename)
	{
	i4_file_class *f=i4_open(filename,I4_READ|I4_NO_BUFFER);
	i4_bool ret=i4_F;
	if (f)
		{
		ret=load_nodes(f);
		delete f;
		}
	else
		i4_error("ERROR: Could not load nodes file.");
	return ret;
	};

void node_manager::reset()
	{
	nodes.reset(i4_F);//all objects are on map, so they get deleted on their own
	//road_nodes.reset(i4_T);
	}

node_manager::~node_manager()
	{
	reset();
	}

i4_bool node_manager::load_nodes(i4_file_class *fp)
	{
	char *buf;
	char *index,*field;
	long size=fp->size();
	w32 r;
	w32 id=0;
	i4_status_class *stat=i4_create_status("Loading Nodes...");
	float x=0,y=0,h=0;
	node *n;
	reset();
	buf=new char[fp->size()+1];
	fp->read(buf,fp->size());
	buf[size]=0;
	index=buf;
	//r=sscanf(buf,"\t\t\t\t\n%n",&index);
	r=1;
	while ((*index)!='\n') index++;
	index++;//we still point at the '\n'
	while (r&&(index<buf+size))
		{
		field=index;
		if (stat)
			stat->update((float)((float)(index-buf))/(float)size);
		r=sscanf(index,"%d\t%f\t%f\t%f",&id,&x,&y,&h);
		while ((*index)!='\n' && (*index!=0)) index++;
		index++;
		if (r)
			{
			if (minx>x)
				minx=x;
			if (miny>y)
				miny=y;
			if (maxx<x)
				maxx=x;
			if (maxy<y)
				maxy=y;
			n=new node(id,x,y,h);
			add_node(n);
			}
		}
	i4_warning("Loaded %d nodes. Map extends: (%f,%f) to (%f,%f).",
		nodes.entries(),minx,miny,maxx,maxy);
	delete [] buf;
	delete stat;
	return i4_T;

	}

i4_bool node_manager::save_nodes(const i4_const_str &filename)
	{
	i4_file_status_struct retstat;
	if (i4_get_status(filename,retstat)==i4_T)
		{
		i4_warning("Skipping nodes on save: Target file exists.");
		return i4_F;//don't do anything if target file exists.
		}
	i4_file_class *f=i4_open(filename,I4_WRITE);
	i4_bool ret=i4_F;
	if (f)
		{
		ret=save_nodes(f);
		delete f;
		}
	return ret;
	}

i4_bool node_manager::save_nodes(i4_file_class *fp)
	{
	char buf[200];
	char *head="ID\tEAST\tNORTH\tELEVAT\tNOTES\n";
	w32 len=strlen(head);
	fp->write(head,len);
	i4_hashtable<void>::iterator it=nodes.get_iterator();
	while (it.get())
		{
		g1_road_object_class *ro=(g1_road_object_class*)it.get();
		w32 east=(ro->x/scalingx)-offsetx;
		w32 north=(ro->y/scalingy)-offsety;
		sprintf(buf,"%i\t%i\t%i\t%i\t  \n",ro->nid,east,north,0);
		fp->write(buf,strlen(buf));
		it++;
		}
	return i4_T;
	}

node_manager::node_manager():nodes(1000,0)
	{
		minx=1e37f;miny=1e37f;maxx=-1e37f;maxy=-1e37f;
		use_active=i4_F;
		scalingx=1;
		scalingy=1;
		offsetx=0;
		offsety=0;

	};

i4_bool g2_node_collect_fun(g1_object_class *obj)
	{
	if (obj->id==g1_get_object_type("road_object"))
		{
		return i4_T;
		}
	else
		return i4_F;
	}

void node_manager::collect()
	{
	reset();
	g1_object_class **buf;
	g1_road_object_class *r;
	sw32 t;
	t=g1_get_map()->make_object_list(buf,g2_node_collect_fun);
	for (int i=0;i<t;i++)
		{
		r=(g1_road_object_class *)buf[i];
		nodes.insert_if_single(r->nid,r);
		}
	}

void node_manager::activate(g1_map_class *map)
	{
	if (use_active)
		return;
	
	offsetx=0-minx+3;offsety=0-miny+3;
	scalingx=(map->width()-6)/(maxx-minx);
	scalingy=(map->height()-6)/(maxy-miny);
	i4_hashtable<void>::iterator i=nodes.get_iterator();
	g1_road_object_class *road;
	g1_object_type road_type=g1_get_object_type("road_object");
	
	node *nod;
	w32 toprocess=nodes.entries();
	w32 processed=0;
	float xp,yp,hp;
	while(nod=(node*)i.get())
		{
		road=(g1_road_object_class*)g1_create_object(road_type);
		node_id mnid=nod->get_id();//to be shure this doesn't get scrambled up
		xp=(nod->xpos()+offsetx)*scalingx;
		yp=(nod->ypos()+offsety)*scalingy;
		hp=0.3f;//should actually average over the terrain height.
		road->setup(mnid,
			xp,
			yp,
			hp);
		if (road->get_flag(g1_object_class::DELETED))
			{
			i4_error("INTERNAL: Scaling of roadmap to game map failed, created object off map");
			//return;
			}
		i++;//before replace, since else we bomb...
		nodes.replace(mnid,road,i4_F);
		processed++;
		delete nod;
		
		}
	
	//nodes.reset(i4_T);
	use_active=i4_T;
	i4_hashtable<g2_link>::iterator l=g2_link_man()->links.get_iterator();
	g2_link *cl;
	g1_road_object_class *startnode,*endnode;
	while(cl=l.get())
		{
		node_id f=cl->from;
		node_id t=cl->to;
		startnode=(g1_road_object_class *) nodes.get(f);
		endnode=(g1_road_object_class *) nodes.get(t);
		startnode->add_link(G1_ALLY,endnode,cl->id);
		endnode->add_link(G1_ENEMY,startnode,cl->id);
		l++;
		}
	//g1_factory_class *fact=map->find_object_by_id(g1_get_object_type("mainbasepad"),G1_ALLY);
	//fact->set_start(startnode);//must be connected to _some_ node such that the radar shows something
	map->solvehint=0;
	float wscale=(scalingx+scalingy)/2;
	//li_float_class_member model_sc("world_scaling");
	//g1_map_vars.vars()->set(model_sc,new li_float(wscale));
	li_set_value("world_scaling",new li_float(wscale),0);
	g1_model_list_man.scale_models(wscale);//means the same here (by default)
	};

// LINK MANAGER METHODS
// ********************

g2_link *link_manager::get_link(link_id id)
	{
	return links.get(id);
	}

g2_link *link_manager::add_link(g2_link *n)
	{
	if (!links.insert_if_single(n->id,n))
		{
		delete n;//do not add link => drop it;
		n=0;
		}
	return n;
	}

void link_manager::remove_link(g2_link *rem)
	{
	links.remove(rem->id);
	nman->get_node(rem->from)->remove_out_link(rem->id);
	nman->get_node(rem->to)->remove_in_link(rem->id);
	}

i4_bool link_manager::load_links(const i4_const_str &filename,g1_loader_class *mainfp)
	{
	//if (g2_act_man()==0)//is this a fresh load or just a restore?
	//	{
		i4_file_class *f=i4_open(filename,I4_READ|I4_NO_BUFFER);
		i4_bool ret=i4_F;
		if (f)
			{
			ret=load_links(f);
			delete f;
			}
		else
			{
			i4_error("ERROR: Could not load links file");
			return i4_F;
			}
		if (mainfp)
			load_qualitydata(mainfp);
	//	}
	/*i4_filename_struct fnamest;
	i4_split_path(filename,fnamest);
	char newext[20];
	w32 d=(g2_act_man())?g2_act_man()->day:0;
	if (d>=10)
		sprintf(newext,"day%i.qudata",d);
	else
		sprintf(newext,"day0%i.qudata",d);

	char qudataname[MAX_PATH*2];
	sprintf(qudataname,"%s/%s_%s",fnamest.path,fnamest.filename,newext);
	i4_file_class *qufile=i4_open(qudataname,I4_READ);
	if (qufile)
		{
		load_qualitydata(qufile);
		delete qufile;
		}
	*/
	return ret;
	};

void link_manager::reset()
	{
	links.reset(i4_T);
	}

link_manager::~link_manager()
	{
	reset();
	}

i4_bool link_manager::load_links(i4_file_class *fp)
	{
	char *buf;
	char *index;
	char token[200];
	long size=fp->size();
	w32 r;
	link_id id=0;
	node_id from=0,to=0;
	w32 lanes=0;
	float length=0;
	float cap=0;
	float freespd=0;
	char *skip=0;
	g2_link *n;
	reset();
	buf=new char[fp->size()+1];
	fp->read(buf,fp->size());
	buf[size]=0;
	index=buf;
	//r=sscanf(buf,"\t\t\t\t\n%n",&index);
	r=1;
	i4_status_class *stat=i4_create_status("Loading links...");
	li_environment *env=0;
	while ((*index)!='\n') index++;
	li_last_line=0;
	//strcpy(li_last_file,"The_links_file");
	index++;//we still point at the '\n'
	while (r&&(index<(buf+size-2)))//do not depend on wether there is a newline at the end
		{
		if (stat)
			stat->update((float)((float)(index-buf))/(float)size);
		//field=index;
		/*r=sscanf(index,"%d\t",&id);
		while ((*index)!='\t' && (*index!=0) ) index++;
		index++;
		while ((*index)!='\t' && (*index!=0) ) index++;
		index++;
		r=sscanf(index,"%d\t%d\t%s\t%d\t%s\t%s\t%s\t%s\t%s\t%f\t%s\t%s\t%s\t%f\t%s\t%s\t%s\t%s\t%f",
			&from,&to,&skip,&lanes,&skip,&skip,&skip,&skip,&skip,&length,
			&skip,&skip,&skip,&cap,&skip,&skip,&skip,&skip,&freespd);
		while ((*index)!='\n' && ((*index)!=0)) index++;
		index++;*/
		li_read_token(index,token);
		id=li_get_int(li_read_number_in_radix(10,token),env);
		li_read_token(index,token);//name
		li_read_token(index,token);
		from=li_get_int(li_read_number_in_radix(10,token),env);
		li_read_token(index,token);
		to=li_get_int(li_read_number_in_radix(10,token),env);
		li_read_token(index,token);//permlanesa
		li_read_token(index,token);//permlanesb
		lanes=li_get_int(li_read_number_in_radix(10,token),env);
		int sk=0;
		for (;sk<6;sk++)
			{
			li_read_token(index,token);
			}
		sscanf(token,"%f",&length);
		li_read_token(index,token);//grade
		li_read_token(index,token);//packa
		li_read_token(index,token);//packb
		li_read_token(index,token);//capa
		
		li_read_token(index,token);//capb
		cap=li_get_int(li_read_number_in_radix(10,token),env);
		li_read_token(index,token);
		li_read_token(index,token);
		li_read_token(index,token);
		li_read_token(index,token);
		sscanf(token,"%f",&freespd);
		while ((*index)!='\n') index++;
		index++;
		if (r)
			{
			n=new g2_link(id,from,to,length,freespd,cap);
			
			add_link(n);
			if (g2_act_man()==0)
				{//its a new load
				nman->get_node(from)->add_out_link(id);
				nman->get_node(to)->add_in_link(id);
				}
			}
		}
	i4_warning("Loaded %d links.",links.entries());
	delete [] buf;
	delete stat;
	return i4_T;

	}

i4_bool link_manager::save_links(const i4_const_str &filename, g1_saver_class *mainfp)
	{
	/*
	i4_filename_struct fnamest;
	i4_split_path(filename,fnamest);
	char newext[20];
	w32 d=g2_act_man()->day;
	if (d>=10)
		sprintf(newext,"day%i.qudata",d);
	else
		sprintf(newext,"day0%i.qudata",d);

	char qudataname[MAX_PATH*2];
	sprintf(qudataname,"%s/%s_%s",fnamest.path,fnamest.filename,newext);
	i4_file_class *qufile=i4_open(qudataname,I4_WRITE);
	if (qufile)
		{
		save_qualitydata(qufile);
		delete qufile;
		}
	*/
	if (mainfp)
		save_qualitydata(mainfp);
	i4_file_status_struct retstat;
	if (i4_get_status(filename,retstat)==i4_T)
		{
		i4_warning("Skipping links on save: Target file exists.");
		return i4_F;//don't do anything if target file exists.
		}
	i4_file_class *f=i4_open(filename,I4_WRITE);
	i4_bool ret=i4_F;
	if (f)
		{
		ret=save_links(f);
		delete f;
		}
	return ret;
	}

i4_bool link_manager::save_links(i4_file_class *fp)
	{
	char buf[1000];
	char *head="ID\tNAME\tNODEA\tNODEB\tPERMLA\tPERMLB"
		"\tPCKTSA\tPCKTSB\tPCKTSA\tPCKTSB\tTWOWAYT\tLENGTH"
		"\tGRADE\tBACKA\tBACKB\tCAPA\tCAPB\tSLMTA\tSLMTB\tSPDA"
		"\tSPDB\tFUNCT\tTHRUA\tTHRUB\tCOLOR\tVEHICLE\tNOTES\n";
	char *dat="%i\t%s\t%i\t%i\t%i\t%i"
		"\t0\t0\t0\t0\t0\t%f"
		"\t0\t0\t0\t%i\t%i\t%f\t%f\t%f"
		"\t%f\tOTHER\t%i\t%i\t%i\t%s\t%s\n";
	w32 len=strlen(head);
	fp->write(head,len);
	i4_hashtable<g2_link>::iterator it=links.get_iterator();
	while (it.get())
		{
		sprintf(buf,dat,it.get()->get_id(),"[NIL]",it.get()->get_from(),it.get()->get_to(),
			1,1,//lanes
			it.get()->get_length(),
			it.get()->get_capacity(),it.get()->get_capacity(),
			it.get()->get_freespeed(),it.get()->get_freespeed(),it.get()->get_freespeed(),it.get()->get_freespeed(),
			0,0,0,"AUTO","[NONE]"
			);
		fp->write(buf,strlen(buf));
		it++;
		}
	return i4_T;
	}

#define QUALITY_DATA_VERSION 2
#define QUALITY_DATA_STRING "Transport simulation quality data V2"

i4_bool link_manager::save_qualitydata(g1_saver_class *fp)
	{
	i4_hashtable<g2_link>::iterator it=links.get_iterator();
	//char *head="Golgotha transport simulation link quality data.\n"\
	//	"This file contains binary information only.\n\32";
	//fp->write(head,strlen(head)+1);//also write the terminal 0
	fp->mark_section(QUALITY_DATA_STRING);
	fp->write_32(QUALITY_DATA_VERSION);//Version info
	fp->write_32(links.entries());
	int i,s;
	while (it.get())
		{
		s=it.get()->quality.size();
		fp->write_32(it.get()->get_id());
		fp->write_32(s);
		for (i=0;i<s;i++)
			{
			fp->write_float(it.get()->quality[i].acctimes);
			fp->write_float(it.get()->quality[i].quratio);
			fp->write_16(it.get()->quality[i].samples);
			fp->write_float(it.get()->quality[i].start);
			}
		it++;
		}
	return i4_T;
	}

i4_bool link_manager::load_qualitydata(g1_loader_class *fp)
	{
	//while (fp->read_8()!=0);
	if (!fp->goto_section(QUALITY_DATA_STRING))
		{
		i4_warning("ERROR: Found transport simulation data inside the level file but no link quality data!");
		return i4_F;
		}
	w32 version=fp->read_32();
	if (version!=QUALITY_DATA_VERSION) 
		return i4_F;
	w32 numlinks=fp->read_32();
	w32 it,k,kmax;
	link_id l_id;
	g2_link_quality q;
	for (it=0;it<numlinks;it++)
		{
		l_id=fp->read_32();
		kmax=fp->read_32();
		links.get(l_id)->quality.uninit();
		for (k=0;k<kmax;k++)
			{
			q.acctimes=fp->read_float();
			q.quratio=fp->read_float();
			q.samples=fp->read_16();
			q.start=fp->read_float();
			links.get(l_id)->quality.add(q);
			}
		}
	return i4_T;
	}


// LINK METHODS
// ************
int qdtime_compare(const g2_link_quality *a, const g2_link_quality *b)
	{
	if (a->start<b->start)
		return -1;
	else if (a->start>b->start)
		return 1;
	else return 0;
	}

void g2_link::mark_bad(double daytime, double extime, double usedtime)
	{
	if (usedtime<=1) return;
	float dt=floor(daytime/TTIMES_SAMPLE_TIME);
	sw32 i=0;
	w32 s=quality.size();
	g2_link_quality q;
	q.samples=1;
	q.start=dt;
	i=quality.binary_search(&q,&qdtime_compare);
	if (i==-1)
		{
		q.acctimes=(float)usedtime;
		q.quratio=(float)length/freespeed;
		//quality.add_at(q,i);//FATAL BUG: i is -1 -> memory corruption unavoidable
		quality.add(q);
		quality.sort(&qdtime_compare);//some kind of overkill
		return;
		}
	quality[i].acctimes+=usedtime;
	quality[i].samples++;//so we'll be able to average over these
	//number of vehicles
	}

i4_float g2_link::quratio(double starttime,i4_float pathlen)
	{
	starttime=starttime+(pathlen*1.1);//an estimate of
	//when we are going to cross this link
	float dt=floor(starttime/TTIMES_SAMPLE_TIME);
	sw32 i=-1;
	w32 s=quality.size();
	g2_link_quality q;
	q.samples=1;
	q.start=dt;
	if (s)
		i=quality.binary_search(&q,&qdtime_compare);
	if (i==-1)
		{
		return length/freespeed;
		}
	return quality[i].quratio;
	}
	//number of vehicles

// ACTIVITIES MANAGER METHODS
// **************************
i4_bool Nodes_Active=i4_F;

i4_bool act_manager::load_activities(const i4_const_str &filename)
	{
	i4_file_class *f=i4_open(filename,I4_READ|I4_NO_BUFFER);
	i4_bool ret=i4_F;
	if (f)
		{
		ret=load_activities(f);
		delete f;
		}
	else
		i4_error("ERROR: Could not load activities file");
	return ret;
	}

int g2_car_start_time_sorter(g2_car_object_class *const *a,g2_car_object_class *const *b)
	{
	if (((*a)->start_time)<((*b)->start_time))
		{
		return -1;
		}
	else if (((*a)->start_time)>((*b)->start_time))
		return 1;
	else if (((*a)->car_id)<((*b)->car_id))//this id is unique (hopefully)
		return -1;
	else 
		return 1;
		
	}

int g2_car_old_order_sorter(g2_car_object_class *const *a, g2_car_object_class *const *b)
	{
	if (((w32)((*a)->vars)) < ((w32)((*b)->vars)))
		return -1;
	else
		return 1;
	}

void act_manager::sort_car_list(int (*sortfun)(g2_car_object_class* const *a,g2_car_object_class* const *b))
	{
	i4_isl_list<g2_car_object_class>::iterator reader=g2_car_object_list.begin();
	i4_array<g2_car_object_class *> sorthelp(1000,1000);
	while(reader!=g2_car_object_list.end())
		{
		sorthelp.add(reader.operator ->());//that function is not for this, I know...
		reader++;
		g2_car_object_list.erase();//the list should be empty at the end
		}
	sorthelp.sort(sortfun);
	//g2_car_object_list.destroy_all(); //but the object MUST NOT be deleted!
	//so destroying is a VERY BAD idea :-(
	int writer=sorthelp.size()-1;
	for(;writer>=0;writer--)
		{
		g2_car_object_list.insert(*sorthelp[writer]);
		}
	//last=(sorthelp[0]);
	sorthelp.uninit();
	};

i4_bool act_manager::load_activities(i4_file_class *fp)
	{
	int i;
	char token[20][20];
	char *buf,*index;
	int sz=fp->size();
	i4_status_class *status=i4_create_status("Loading activities...");
	buf=(char*) malloc(sz+10);
	fp->read(buf,sz);
	buf[sz]=0;
	index=buf;
	w32 last_id=0,cur_id=0;
	w32 start_time,end_time;
	float start_hour,end_hour;
	w32 start_link,end_link;
	
	g2_car_object_class *last_car=0,*cur_car=0;
	daytime=3600*24;//we haven't seen anything that starts earlier
	day=1;//first day of simulation
	w32 numcars=0;
	while(index<(buf+sz))
		{
		if (status)
			status->update((float)((float)(index-buf))/(float)sz);
		for (i=0;i<20;i++)
			{
			li_read_token(index,token[i]);
			}
		while ( (*index) !='\n') index++;
		index++;
		cur_id=li_get_int(li_read_number_in_radix(10,token[0]),0);
		if (cur_id<=last_id)//we already have created this car
			{
			if (cur_id==last_id)
				{
				sscanf(token[4],"%f",&end_hour);//was erroneously accessing field 12
				end_time=(w32) (end_hour*3600);
				end_link=li_get_int(li_read_number_in_radix(10,token[19]),0);
				cur_car->dest_link=end_link;
				cur_car->required_arrival_time=end_time;
				}
			else
				{
				i4_error("WARNING: More than one trip per traveler is not implemented yet.");
				}
			}
		else
			{
			sscanf(token[12],"%f",&start_hour);
			start_time=(w32) (start_hour*3600);
			if (start_time<daytime)
				daytime=start_time;
			start_link=li_get_int(li_read_number_in_radix(10,token[19]),0);
			cur_car=(g2_car_object_class *) g1_create_object(car_type);
			if (!cur_car)
				{
				i4_error("SEVERE: Could not create a car object. Out of Memory?");
				return i4_T;
				}
			cur_car->start_link=start_link;
			cur_car->start_time=start_time;
			cur_car->car_id=cur_id;
			last_id=cur_id;
			numcars++;
			}
		
		}

	//maxwindowsize=100;
	maxwindowsize=g2_node_man()->num_nodes()*2;//an estimate.
	if (maxwindowsize<80) 
		maxwindowsize=80;
	curwindowsize=0;
	if (status)
		status->update("Sorting...",100);
	//first solution of sorting
	sort_car_list(g2_car_start_time_sorter);

	//second solution (slowwwww...)
	/*
	i4_isl_list<g2_car_object_class>::iterator sorter=g2_car_object_list.begin(),temp1,last,
		ende=g2_car_object_list.end();//i4_isl_list<>::end() returns always 0
	i4_bool finished=i4_F;
	g2_car_object_class *item;
	while (!finished)
		{
		finished=i4_T;
		sorter=g2_car_object_list.begin();
		last=0;
		while(sorter!=ende)
			{
			temp1=sorter;
			temp1++;
			if (temp1!=g2_car_object_list.end() && sorter->start_time>temp1->start_time)
				{
				finished=i4_F;
				item= &(sorter.operator *());//to make it clear
				//g2_car_object_list.find_and_unlink(item);
				if (last!=ende)
					g2_car_object_list.erase_after(last);
				else
					g2_car_object_list.erase();
				g2_car_object_list.insert_after(temp1,*item);
				}
			last=sorter;
			sorter++;
			}
		}
		*/
	nextstart=g2_car_object_list.begin();
	oldestwaiting=g2_car_object_list.begin();
	i4_warning("We loaded activities for %i cars. The simulation starts %f seconds after midnight.",numcars,daytime);
	delete status;
	free(buf);
	return i4_T;
	}

act_manager::act_manager():scramble(1)
	{
	};


//this is mainly a method of act_manager because it inherits friendship
i4_bool act_manager::save(g1_saver_class *fp)
	{
	node_manager *nman=g2_node_man();
	link_manager *lman=g2_link_man();
	union
		{
		w32 tdouble[2];
		double d;
		};
	li_string_class_member nodes("traffic_sim_nodes");
	li_string_class_member links("traffic_sim_links");
	li_string_class_member veh("traffic_sim_vehicles");
	li_string_class_member acts("traffic_sim_acts");
	li_symbol_class_member can_load("traffic_sim");
	li_class_context ctx(g1_map_vars.vars());
	if (can_load()==li_nil)
		{
		return i4_F;
		}
	fp->mark_section(TRANSIMS_DATA_STRING);
	//fp->start_version(TRANSIMS_DATA_VERSION);
	fp->write_32(TRANSIMS_DATA_VERSION);
	fp->write_float(daytime);
	fp->write_32(day);
	fp->write_32(maxwindowsize);
	fp->write_32(curwindowsize);
	if (nextstart!=g2_car_object_list.end())
		fp->write_32(nextstart->car_id);//these alone are persistent
	else 
		fp->write_32(0);
	if (oldestwaiting!=g2_car_object_list.end())
		fp->write_32(oldestwaiting->car_id);
	else
		fp->write_32(0);
	fp->write_counted_str(nodes());
	fp->write_counted_str(links());
	nman->save_nodes(nodes());
	//Well... I think those 0's and 1's should be exchanged if run on 
	// a little endian architecture.
	w32 zero=i4_bigend,one=i4_litend;
	d=nman->maxx;
	fp->write_32(tdouble[zero]);
	fp->write_32(tdouble[one]);
	d=nman->maxy;
	fp->write_32(tdouble[zero]);
	fp->write_32(tdouble[one]);
	d=nman->minx;
	fp->write_32(tdouble[zero]);
	fp->write_32(tdouble[one]);
	d=nman->miny;
	fp->write_32(tdouble[zero]);
	fp->write_32(tdouble[one]);
	d=nman->offsetx;
	fp->write_32(tdouble[zero]);
	fp->write_32(tdouble[one]);
	d=nman->offsety;
	fp->write_32(tdouble[zero]);
	fp->write_32(tdouble[one]);
	d=nman->scalingx;
	fp->write_32(tdouble[zero]);
	fp->write_32(tdouble[one]);
	d=nman->scalingy;
	fp->write_32(tdouble[zero]);
	fp->write_32(tdouble[one]);
	lman->save_links(links(),fp);
	i4_isl_list<g2_car_object_class>::iterator it=g2_car_object_list.begin();
	while (it!=g2_car_object_list.end())
		{
		if (!it->linked)
			{
			fp->write_32(0xCABED);//it follows an object 
			it->save(fp);
			}
		else
			{
			fp->write_32(0xBAADFEED);//at this position belongs the given object
			fp->write_32(it->car_id);
			}
		it++;
		}
	fp->write_32(0xABCDEF);//end of list
	scramble.save(fp);
	//fp->end_version();
	return i4_T;
	}

extern node_manager *node_man;
extern link_manager *link_man;
extern act_manager *act_man;

i4_bool act_manager::load(g1_loader_class *fp)
	{
	li_class_context ctx(g1_map_vars.vars());
	li_string_class_member nodes("traffic_sim_nodes");
	li_string_class_member links("traffic_sim_links");
	li_string_class_member veh("traffic_sim_vehicles");
	li_string_class_member acts("traffic_sim_acts");
	li_symbol_class_member can_load("traffic_sim");
	union 
		{
		w32 tdouble[2];
		double d;
		};
	if (can_load()==li_nil)
		{
		return i4_F;
		}
	
	w16 version,data_size=0;
//	fp->get_version(version,data_size);
	version=fp->read_32();//cannot use get_version() since 
	//it doesn't allow recursion
	daytime=fp->read_float();
	day=fp->read_32();
	maxwindowsize=fp->read_32();
	curwindowsize=fp->read_32();
	w32 tempnext,tempoldest;
	tempnext=fp->read_32();
	tempoldest=fp->read_32();
	i4_str *s=fp->read_counted_str();//we don't need these right now
	delete s;
	s=fp->read_counted_str();
	delete s;
	node_man=new node_manager();
	node_man->use_active=i4_T;
	w32 zero=i4_bigend,one=i4_litend;
	tdouble[zero]=fp->read_32();
	tdouble[one]=fp->read_32();
	node_man->maxx=d;
	tdouble[zero]=fp->read_32();
	tdouble[one]=fp->read_32();
	node_man->maxy=d;
	tdouble[zero]=fp->read_32();
	tdouble[one]=fp->read_32();
	node_man->minx=d;
	tdouble[zero]=fp->read_32();
	tdouble[one]=fp->read_32();
	node_man->miny=d;
	tdouble[zero]=fp->read_32();
	tdouble[one]=fp->read_32();
	node_man->offsetx=d;
	tdouble[zero]=fp->read_32();
	tdouble[one]=fp->read_32();
	node_man->offsety=d;
	tdouble[zero]=fp->read_32();
	tdouble[one]=fp->read_32();
	node_man->scalingx=d;
	tdouble[zero]=fp->read_32();
	tdouble[one]=fp->read_32();
	node_man->scalingy=d;

	node_man->collect();
	link_man=new link_manager(node_man);
	link_man->load_links(links(),fp);
	i4_isl_list<g2_car_object_class>::iterator it=g2_car_object_list.begin();
	g1_object_type ctype=g1_get_object_type("car");
	fp->set_remap(0);
	w32 marker=fp->read_32();
	w32 caridpos=0;
	w32 permpos=1;
	i4_isl_list<g2_car_object_class>::iterator tempit;
	while (marker!=0xABCDEF)
		{
		switch (marker)
			{
			case 0xCABED:
				{
				g2_car_object_class *o=(g2_car_object_class *)
					g1_object_type_array[ctype]->create_object(ctype,fp);
				o->global_id=g1_global_id.alloc(o);
				//we are abusing this field for sorting
				//It's contents can be invalid outside the router.
				//it is a pointer value, but who cares?
				//Uh, I'd like to use another value since we have no
				//idea what the gc will do if he finds something like this...
				o->vars=(li_class*)permpos;
				
				}
				break;
			case 0xBAADFEED:
				{
				caridpos=fp->read_32();
				tempit=g2_car_object_list.begin();
				while (tempit!=g2_car_object_list.end() 
					&& tempit->car_id!=caridpos)
					tempit++;
				if (tempit==g2_car_object_list.end())
					i4_warning("ERROR: Car %i does not exist on load.",caridpos);
				tempit->vars=(li_class*)permpos;
				tempit->get_terrain_info();//must be restored manually
				//since the objects occupy_location() doesn't call this.
				}
				break;
			default:
				{
				i4_error("WARNING: Unrecognized sequence found in file.");
				}
			}
		permpos++;
		marker=fp->read_32();
		}
	sort_car_list(g2_car_old_order_sorter);
	nextstart=g2_car_object_list.begin();
	oldestwaiting=g2_car_object_list.begin();
	//for some reason this doesn't work :-(
	//But I don't see another way of restoring these pointers
	//oldestwaiting seems to get ahead of nextstart.
	//That just CANNOT work as intended since the start_times
	//have changed during simulation and therefore the above
	//sort_car_list() gives a new permutation of the cars,
	//but we need the old one! (Actually except if the saved state
	//is from the end of an iteration).
	//->We need to save the permutation of the objects in the list.

	//while (nextstart!=g2_car_object_list.end() && nextstart->linked)
	w32 pos=0;
	while (nextstart!=g2_car_object_list.end() && nextstart->car_id!=tempnext) 
		{
		nextstart->vars=0;//reset these
		nextstart++;
		pos++;
		
		};
	tempit=nextstart;
	while (tempit!=g2_car_object_list.end())//continue until we reach the end
		{
		tempit->vars=0;
		tempit++;
		}
	w32 difpos=pos;
	while (oldestwaiting!=g2_car_object_list.end() && oldestwaiting->car_id!=tempoldest) 
		{
		oldestwaiting++;
		difpos--;
		}
	//I4_ASSERT(difpos>=0,"INTERNAL: Iterators on linked list violate required invariant.");
	scramble.load(fp);
	//fp->end_version(I4_LF);
	return i4_T;
	}

char *stoh(double d)
	{
	static char buf[10];
	int hour=(int) (d/3600);
	//int min=((((int) d - (hour*3600)) / 3600) * 60);
	int min=0;
	min= (int) d%3600;
	min= (min * 60) / 3600;//not assoziative!
	sprintf(buf,"%i:%s%i",(int)(d/3600),min>=10?"":"0",min);
	return buf;
	}

//int dropofftime=(1+3600*24);

void act_manager::think()
	{
	daytime=daytime+1;//actual increment will depend on simulation speed.
	//g2_car_object_class *c=0;
	if ((((int)daytime)%(3600/4))==0)
		{
		i4_warning("It's now %s. ",stoh(daytime));
		}
	//if ((int)daytime>dropofftime) //for debugging purposes
	//	return;
	//i4_bool dayover=i4_T;//will stay true at the end if everybody has
	//reached his destination and we can start the next day
	/*i4_isl_list<g2_car_object_class>::iterator cit=g2_car_object_list.begin();
	for (;cit!=g2_car_object_list.end();cit++)
		{
		if (cit->start_time<=daytime && cit->linked==i4_F && 
			cit->arrival_time==g2_car_object_class::TIME_NOTYET)
			{
			cit->start((int)daytime,cit->dest_link);
			}
		if (cit->arrival_time==g2_car_object_class::TIME_NOTYET)
			dayover=i4_F;
		//todo: add code to log congestion
		//to also do: Have some g2_mix_think class that 
		//scrambles up the order in wich the cars think.
		}
		*/
	i4_isl_list<g2_car_object_class>::iterator oldnext,oldold;
	oldnext=nextstart;
	while (nextstart != g2_car_object_list.end() && 
		nextstart->start_time <= daytime && curwindowsize<maxwindowsize)
		{
		nextstart->start((int)daytime,nextstart->dest_link);
		nextstart++;
		curwindowsize++;
		}
	oldold=oldestwaiting;
	while (oldestwaiting!=oldnext && curwindowsize>0)
		{
		if (!oldestwaiting->linked)
			{//already started?
			oldestwaiting->start((int)daytime,oldestwaiting->dest_link);
			}
		oldestwaiting++;
		//curwindowsize--;
		}
	oldestwaiting=oldold;
	while (oldestwaiting!=nextstart && oldestwaiting->linked) 
		{
		oldestwaiting++;
		curwindowsize--;
		}
	g1_object_class *acar=0;
	if (oldestwaiting==g2_car_object_list.end())
		acar=g1_get_map()->find_object_by_id(car_type,1);//save some time
	//if (acar && !acar->get_flag(g1_object_class::THINKING))
	//	{
	//	i4_warning("There is a non-thinking car on the map. ID %i.",acar->global_id);
	//	}
	if (oldestwaiting==g2_car_object_list.end() && !acar)
		{
		//i4_error("USER: The %i.th day has been simulated completelly.",day);
		//li_call("File/Save As");//save everything of the old day
		//this doesn't work. Probably a problem because the call is
		//postphoned until the next iteration
		i4_warning("On day %i, the latest car arrived at %s.",day,stoh(daytime));
		nextstart=g2_car_object_list.begin();
		oldestwaiting=g2_car_object_list.begin();
		day++;
		i4_hashtable<g2_link>::iterator i=g2_link_man()->links.get_iterator();
		g2_link *cl;
		w32 j;
		float tqual,extime;
		while (cl=i.get())
			{
			extime=cl->get_length()/cl->get_freespeed();
			for (j=0;j<cl->quality.size();j++)
				{
				//the quality for tomorrow
				tqual=(float)(cl->quality[j].acctimes/cl->quality[j].samples);

				//extime is for shure smaller than tqual since 
				//we record only cars that complain about having too long.
				//i4_float qtemp=1.0;//MODIFY
				//pow(tqual-extime,1.05);
				cl->quality[j].quratio=tqual;//just the time we expect to need
				cl->quality[j].acctimes=extime;
				cl->quality[j].samples=1;//(we add one sample to avoid a div by zero tomorrow)
				}
			i++;
			}//for all links
		i4_isl_list<g2_car_object_class>::iterator git=g2_car_object_list.begin();
		scramble.rnd.set_limits(0,100);
		double total_time_used=0;
		while(git!=g2_car_object_list.end())
			{
			total_time_used+=(double) (git->arrival_time - git->start_time);
			if (scramble.rnd.rnd()<10)
				{
				git->set_flag(g1_object_class::SCRATCH_BIT,1);//set the scratch bit
				//to indicate that he should choose a new way
				w32 oldstarttime=git->start_time;
				w32 newstarttime=0;
				newstarttime=git->calc_new_start_time(oldstarttime,
						git->arrival_time,git->required_arrival_time);
				git->start_time=newstarttime;
				}
			if (git->path_to_follow)
				{
				//if it doesn't choose a new path, he will copy the old
				//back here.
				i4_free(git->path_to_follow);
				git->path_to_follow=0;
				}
			if (!git->last_route)
				{
				i4_warning("Car %i doesn't have a last route. What happened to him?",git->car_id);
				}
			git++;
			}
		i4_warning("The total amount of time used is %lf.",total_time_used);
		sort_car_list(g2_car_start_time_sorter);
		nextstart=g2_car_object_list.begin();
		oldestwaiting=g2_car_object_list.begin();
		
		daytime=nextstart->start_time;
		i4_warning("Tomorrow, simulation starts at %s.",stoh(daytime));
		}//if the day is over
	}

