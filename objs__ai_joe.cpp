
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
#include "team_api.h"
#include "memory/array.h"
#include "math/random.h"
#include "math/angle.h"
#include "math/trig.h"
#include "time/profile.h"
#include "objs/stank.h"
#include "objs/goal.h"
#include "objs/convoy.h"
#include "g1_rand.h"
#include "player.h"


char *unit_name_joe[] = 
{
  "stank",                      //0
  "peon_tank",                  //1
  "electric_car",               //2
  "rocket_tank",                //3
//  "",
//  "",
  "helicopter",                 //4
  "jet",                        //5
  "engineer",                   //6
  "trike"                       //7
};
const int num_unit_types=sizeof(unit_name_joe)/sizeof(char*);
w16 unit_type_joe[num_unit_types];

char *goodies_name[] = 
{
  "takeover_pad",               //1
  "floorpad"                 //2
  "mainbasepad"
};
const int num_goodies_types=sizeof(goodies_name)/sizeof(char*);
w16 goodies_type[num_goodies_types];

static i4_bool goodies_filter(g1_object_class *who)
	{
	for (int i=0;i<num_goodies_types;i++)
		{
		if (goodies_type[i]==who->id)
			return i4_T;
		}
	return i4_F;
	}


char *formation_joe[] =
{
  "      "
  "      "
  "121212"
  "      "
  "1 63 1"
  "  33  ", // stock

  "222222"
  "      "
  "      "
  "      "
  "      "
  "      ", // electric brigade

  "  11  "
  " 1221 "
  "123321"
  "      "
  "      "
  "      ", // hole puncher

  "12221 "
  "23632 "
  "      "
  "      "
  "      "
  "      ", // armored convoy

  "      "
  "      "
  "1 1 1 "
  " 1 1 1"
  "7 7 7 "
  " 7 7 7", // gene's checker

  "  1   "
  " 161  "
  "1   1 "
  "      "
  "      "
  "      ", // peon V

  "  4   "
  " 4 4  "
  "      "
  "      "
  "      "
  "      ", // heli hell

  "111111"
  " 3663 "
  " 3333 "
  "      "
  "      "
  "      ", // rockets R us

  "22    "
  "66    "
  "      "
  "      "
  "      "
  "      ", // small band

  "22    "
  "36    "
  "      "
  "      "
  "      "
  "      ", // yet another small band

  "33    "
  "66    "
  "      "
  "      "
  "      "
  "      ", // yet another small band

  "6     "
  "      "
  "      "
  "      "
  "      "
  "      ", // long shot

  "2222  "
  "3333  "
  "      "
  "      "
  "      "
  "      ", // ouch!
};

const int num_formations=sizeof(formation_joe)/sizeof(char*);

static const float xoffs[6]={-1.5f,-1.0f,-0.5f,0.5f,1.0f,1.5f}; //not really good...
static const float yoffs[6]={-1.5f,-1.0f,-0.5f,0.5f,1.0f,1.5f};
enum eDir
{
  N, E, S, W
};

sw16 dirx[] = {-1, 1 , -1, 1};
sw16 diry[] = {-1, -1,  1, 1};


typedef struct _temp_id_struct
	{
	w32 id;
	}id_struct;

//i4_profile_class pf_ai_joe_setup_production("ai_joe_setup_production");
i4_profile_class pf_ai_joe_setup_new_pieces("ai_joe_setup_new_pieces");
//i4_profile_class pf_ai_joe_cleanup("ai_joe_cleanup");
//i4_profile_class pf_ai_joe_next_spot("ai_joe_next_spot");
i4_profile_class pf_ai_joe_guide_hero("ai_joe_guide_hero");
i4_profile_class pf_ai_joe_think("ai_joe_think");


class ai_joe : public g1_team_api_class
{
public:
  enum eBuildMode 
  { INIT, INITWAIT, IDLE, BUILDWAIT, BUILD, WAITFORPIECE, FORMANDMOVE, RESURRECT, COLLECT_GOALS };

  
  float bdiffx,bdiffy;

  w8 dir1, dir2; //actually of type eDir.

  //i4_array<unit_class> troop;
  //Directly using typed references inside i4_arrays doesn't work for
  //two reasons: First, the constructors and destructors are not correctly
  //called, second, the objects could be moved around in memory. 
  //Smart pointers cannot work then. 
  i4_array<g1_typed_reference_class<g1_convoy_class> * > convoys;
  i4_array<sw32> time;

  g1_typed_reference_class<g1_convoy_class> building;
  g1_typed_reference_class<g1_object_class> where;
  i4_array<g1_typed_reference_class<g1_object_class> *> goals;

  int form, place, formation_size;
  int build_count;
  int build_wait, retry;
  eBuildMode build_mode;

  w16 goal_type;
  w16 convoy_type;

  //int reload;  // supertank is going to reload ammo

  ai_joe(g1_loader_class *f=0) 
	: convoys(40,40), time(40,40), goals(40,40)
  {
    build_mode = INIT;
	formation_size=1;
    form = -1;
    place = 0;
	build_count=0;
	build_wait=1;
	retry=1;
  }

  void clear_unitlists()
	  {
	  for (int i=0;i<convoys.size();i++)
		  {
		  delete convoys[i];//this deletes the reference classes;
		  }
	  convoys.clear();
	  for (int j=0;j<goals.size();j++)
		  {
		  delete goals[j];
		  }
	  goals.clear();
	  }

  ~ai_joe()

  {
    clear_unitlists();
    convoys.uninit();
	time.uninit();
	goals.uninit();
  }


  void setup_production(i4_float _prodx,i4_float _prody)
  {
//    pf_ai_joe_setup_production.start();
    // setup around production site at location

//    prodx = _prodx;
//    prody = _prody;

    int
      dx = int(_prodx - map_width()/2),
      dy = int(_prody - map_height()/2);

    int num=0;

    num |= (dx<0)  ?0:8;
    num |= (dy<0)  ?0:4;
    num |= (dx<dy) ?0:2;
    num |= (dx<-dy)?0:1;

    switch (num)
    {
      case  5:  //N quadrant
      case 13:  dir1 = S;  break;
      case 15:  //E quadrant
      case 11:  dir1 = W;  break;
      case 10:  //S quadrant
      case  2:  dir1 = N;  break;
      case  0:  //W quadrant
      case  4:  dir1 = E;  break;
    }
    dir2 = (dir1+1)%4;

    reload=0;
//    pf_ai_joe_setup_production.stop();
  }

  virtual void init()
  {
    int i;


	clear_unitlists();

    for (i=0; i<num_unit_types; i++)
      unit_type_joe[i] = object_type(unit_name_joe[i]);
    for (i=0; i<num_goodies_types; i++)
      goodies_type[i] = object_type(goodies_name[i]);
    goal_type = object_type("goal");
	convoy_type=object_type("convoy");

    build_mode = INIT;
    form = -1;
    place = 0;
	bdiffx=0;
	bdiffy=0;

    dir1 = w8(g1_rand(20)%4);
    dir2 = ((dir1+1)%4);
  }

  void cleanup()
  {
	clear_unitlists();
  }


  int next_spot()
  //{{{ sets next formation spot.. returns false if no more left
  {
//    pf_ai_joe_next_spot.start();
    place++;
    while ((place<6*6) && (formation_joe[form][place]==' '))
      place++;

    if (place==6*6)
    {
//      pf_ai_joe_next_spot.stop();
	  form=-1;
      return 0;
    }

    //int x=(place%6)*2-5, y=5-(place/6)*2;

    //bdiffx = (5 + dirx[dir1]*y + dirx[dir2]*x)/2;
    //bdiffy = (5 + diry[dir1]*y + diry[dir2]*x)/2;
	bdiffx=xoffs[place%6]*dirx[dir1];
	bdiffy=yoffs[place/6]*diry[dir1];


//    pf_ai_joe_next_spot.stop();
    return 1;
  }
  //}}}


  void guide_units()
	  //{{{
	  {
	  int i;
	  for (i=0; i<time.size(); i++)
		  {
		  if (!convoys[i]->valid())//an entire convoy was killed
			  {
			  convoys.remove(i);
			  time.remove(i);
			  i--;
			  }
		  
		  }
	  float tx,ty;
	  for (i=0; i<time.size(); i++)
		  {
		  if (time[i]>0)
			  time[i]--;
		  else 
			  {
			  //int went=0;
			  
			  if (goals.size()>0)
				  {
				  //tx = g1_rand(70)%(map_width()-10) + 5.5f;
				  //ty = g1_rand(80)%(map_height()-10) + 5.5f;
				  //tx = goal[i<goal.size()?i:0].x();
				  //ty = goal[i<goal.size()?i:0].y();
				  //PG: This should be a bit more random
				  tx = goals[i%goals.size()]->get()->x;
				  ty = goals[i%goals.size()]->get()->y;
				  if (deploy_unit(convoys[i]->operator*().global_id, tx,ty))
					  {
					  time[i]=g1_rand(9)%500+500;
					  }
				  else
					  {
					  time[i]=5;//retry later
					  }
				  
				  }
			  else
				  {
				  tx = g1_rand(90)%(map_width()-10) + 5.5f;
				  ty = g1_rand(100)%(map_height()-10) + 5.5f;
				  if (deploy_unit(convoys[i]->operator*().global_id, tx,ty))
					  {
					  time[i]=g1_rand(5)%500+500;
					  }
				  else 
					  {
					  time[i]=15;
					  }
				  
				  
				  }
			  }
		  }
	  }
  //}}}

  virtual void object_built(w32 id)
  //{{{
  {
    g1_team_api_class::object_built(id);
  }
  
  virtual void object_added(g1_object_class *which, g1_object_class *toobj)
	  {
	  g1_typed_reference_class<g1_convoy_class> *newconv;
	  if (where.get()!=toobj)
		  {
		  //We have multiple build spots, and the last one used
		  //differs from the current, just send the current convoy away
		  //and create a new one.
		  if (building.valid())
			  {
			  newconv=new g1_typed_reference_class<g1_convoy_class>(building.get());
			  convoys.add(newconv);
			  time.add(100);//immediatelly suggest a new way for this convoy.
			  building=0;
			  };
		  where=toobj;
		  }
	  g1_convoy_class *myconv=0;
	  if (!building.valid())
		  {
		  myconv=(g1_convoy_class*)g1_create_object(convoy_type);
		  myconv->setup(which);
		  myconv->change_player_num(team());//otherwise, deploy will 
		  //refuse to do its job.
		  building=myconv;
		  }
	  else
		  {
		  myconv=building.get();
		  myconv->add_object(which);
		  }
	  float point[4];
	  point[0]=bdiffx+toobj->x;
	  point[1]=bdiffy+toobj->y;
	  point[2]=which->x;
	  point[3]=which->y;
	  g1_path_handle phn=g1_path_manager.alloc_path(2,point);
	  //we assume the path is straigt. 
	  which->deploy_to(bdiffx+toobj->x,bdiffy+toobj->y,phn);//it's a single unit
	  if (!next_spot())
		  {
		  newconv=new g1_typed_reference_class<g1_convoy_class>(building.get());
		  convoys.add(newconv);
		  time.add(100);
		  building=0;
		  build_mode=COLLECT_GOALS;
		  chooseform();
		  }
	  }
  /*
	  {
    object_class obj(object(id));
    unit_class u(unit(id));
    int i,j,k;

    if (obj.alive())
    {
      if (obj.id()==goodies_type[0])
        goal.add(obj);
      else if (u.alive())
      {
        for (i=0; i<num_unit_types && obj.type()!=unit_type_joe[i]; i++) ;

        if (i<num_unit_types)
        {
          k=troop.add(u);
		  //It seems I must completelly recode this
          for (j=0; j<group.size() && group[j].id()!=convoy_id(u); j++) ;

          if (j==group.size() && !u.built())
          {
			
            //group.add(unit_class(convoy_id(u)));
		    
		    //desired behaviour: Create a convoy out of all
		  //units lined up in troop and clear troop.
			group.add(u);
            time.add(0);
          }
		  else if (k==0)
			  {
			  setup_production(troop[k].x(),troop[k].y());
			  next_spot();
			  
			  }
		  else
			  {
			  next_spot();
			  }
        }
      }
    }
  }
  */
  void chooseform()
	  {
	  if (form<0)
		{
		// determine next formation
		form = g1_rand(99)%num_formations;
		place = -1;//will be increased just again
		next_spot();
		build_count = 0;
		}
	  }
  virtual void think()
  //{{{
  {
    pf_ai_joe_think.start();
	w16 unit_to_build;

    //cleanup();
	chooseform();

    guide_units();

    guide_hero();

    switch (build_mode)
    {
      case INIT:
        //{{{
      {
       
        build_mode = INITWAIT;
        build_wait = 50;
      } break;
      //}}}
      case INITWAIT:
        //{{{
        if (--build_wait<=0)
          build_mode = IDLE;
        break;
        //}}}
      case IDLE:
        //{{{
      {
          build_mode = BUILD;
      } break;
        //}}}
      case BUILDWAIT:
        //{{{
        if (--build_wait<=0)
          build_mode = BUILD;
        break;
        //}}}
      case BUILD:
        //{{{
        //if (build_unit(unit_type[formation[form][place] - '0'], bx, by) == G1_BUILD_OK)
		  unit_to_build=unit_type_joe[formation_joe[form][place]-'0'];
		  if (build_unit(unit_to_build)==G1_BUILD_OK)
			  {
			  build_count++;
			  //place++;
			  
        
			  /*int i=troop.add(unit(p));
			  setup_production(troop[i].x(), troop[i].y());
              int c=group.add(convoy(built_convoy_id()));
              time.add(-1);

              bx = prodx - dirx[dir1]*3;
              by = prody - diry[dir1]*3;
              send_convoy(built_convoy_id(), bx, by);*/
			  build_mode=BUILDWAIT;
			  build_wait=15;
			  }
		  else
			  {
              build_mode = BUILDWAIT;
              build_wait = 20;
			  }
		  if (formation_joe[form][place]==' ')
			  {
			  next_spot();
			  }
                  
        break;
	  case COLLECT_GOALS:
		  {
		  int numgoals;
		  g1_object_class **buf;
		  numgoals=g1_get_map()->make_object_list(buf,goodies_filter);
		  int n;
		  for (n=0;n<goals.size();n++)
			  delete goals[n];
		  goals.clear();
		  for (n=0;n<numgoals;n++)
			  {
			  goals.add(new g1_typed_reference_class<g1_object_class>(buf[n]));
			  }
		  build_mode=BUILDWAIT;
		  build_wait=10;
		  }
		  break;
        //}}}
      case WAITFORPIECE:
        //{{{
        // wait for object_build or build_error to complete
        break;
        //}}}
      case FORMANDMOVE:     // jc:fixme
        build_mode=INITWAIT;
        build_wait=g1_rand(103)%500+100;
        break; 

      case RESURRECT:
        //guide_hero() assures that there exist always a commander
        build_unit(unit_type_joe[0]);
		//g1_player_man.get(team())->continue_wait=i4_F;
        build_wait=20;
        build_mode=INITWAIT;
        break;
    }
    
    pf_ai_joe_think.stop();
  }
  
  enum {JOE_DATA_VERSION=5};
  void save(g1_saver_class *fp)
	  {
	  fp->start_version(JOE_DATA_VERSION);
	  fp->write_reference(building);
	  fp->write_32(convoys.size());
	  for (int i=0;i<convoys.size();i++)
		  {
		  fp->write_reference(*convoys[i]);
		  fp->write_32(time[i]);
		  }
	  fp->write_format("ff11444444",&bdiffx,&bdiffy,&dir1,&dir2,
		  &form,&place,&formation_size,&build_count,&build_wait,&retry);

	  fp->end_version();
	  }

  void load(g1_loader_class *fp)
	  {
	  w16 data_version=0,size=0;
	  clear_unitlists();
	  fp->get_version(data_version,size);
	  g1_typed_reference_class<g1_convoy_class> *aref;
	  if (data_version==JOE_DATA_VERSION)
		  {
		  fp->read_reference(building);
		  int k=fp->read_32();
		  for (int i=0;i<k;i++)
			  {
			  aref=new g1_typed_reference_class<g1_convoy_class>();
			  fp->read_reference(*aref);
			  convoys.add(aref);
			  time.add(fp->read_32());
			  }
		  fp->read_format("ff11444444",&bdiffx,&bdiffy,&dir1,&dir2,
			&form,&place,&formation_size,&build_count,&build_wait,&retry);

		  }
	  else
		  {
		  fp->seek(fp->tell()+size);
		  }
	  build_mode=COLLECT_GOALS;
	  fp->end_version(I4_LF);
	  }
};

g1_team_api_definer<ai_joe> ai_joe_def("ai_joe");


// AI joe is now chosen as the default through (g1_set_deault_ai "joe") in scheme/start.scm


