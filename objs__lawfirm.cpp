#include "pch.h"

/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/

#include "player.h"
#include "objs/def_object.h"
#include "lisp/li_class.h"
#include "lisp/li_init.h"
#include "object_definer.h"
#include "objs/map_piece.h"
#include "controller.h"
#include "image_man.h"
#include "saver.h"
#include "objs/miscobjs.h"
#include "objs/bases.h"
#include "objs/shrapnel.h"
#include "math/random.h"

static g1_team_icon_ref radar_iml("bitmaps/radar/lawfirm.tga");

static li_int_class_member income_rate("income_rate"),
counter("counter"), reset_time("reset_time");
static li_float_class_member commision("commision");

class g1_lawfirm_class :
	public g1_building_class
{
public:
	enum {
		DATA_VERSION=1
	};
	g1_lawfirm_class(g1_object_type id,
					 g1_loader_class * fp);
	void think();
	void change_player_num(int new_player_num);
	void save(g1_saver_class * fp);
	void skipload(g1_loader_class * fp);
	void load(g1_loader_class * fp);
};

g1_object_definer<g1_lawfirm_class>
g1_lawfirm_def("lawfirm",g1_object_definition_class::EDITOR_SELECTABLE|
			   g1_object_definition_class::TO_BUILDING);

g1_lawfirm_class::g1_lawfirm_class(g1_object_type id,
								   g1_loader_class * fp) :
	g1_building_class(id,fp)
{
	radar_image=&radar_iml;
	radar_type=G1_RADAR_BUILDING;
	set_flag(BLOCKING|
			 TARGETABLE|
			 SELECTABLE|
			 GROUND,1);
	draw_params.setup("lawyerbuilding2x3");
	w16 ver, data_size;
	if (fp)
	{
		fp->get_version(ver,data_size);    //actally, we don't exspect any data for us
		fp->seek(fp->tell() + data_size);
		fp->end_version(I4_LF);
	}
	else
	{
		ver=0;
	}


}

void g1_lawfirm_class::save(g1_saver_class * fp)
{
	g1_building_class::save(fp);
	fp->start_version(DATA_VERSION);
	fp->end_version();
}

void g1_lawfirm_class::load(g1_loader_class * fp)
{
	g1_building_class::load(fp);
	w16 v,ds;
	fp->get_version(v,ds);
	fp->end_version(I4_LF);
}

void g1_lawfirm_class::skipload(g1_loader_class * fp)
{
	g1_building_class::skipload(fp);
	w16 v,ds;
	fp->get_version(v,ds);
	fp->end_version(I4_LF);
};

void g1_lawfirm_class::think()
{
	if (!death.think())
	{
		return;
	}
	if (health<=0)
	{
		g1_shrapnel_class * shrapnel = NULL;

		short shrapnel_type = g1_get_object_type("shrapnel");
		shrapnel = (g1_shrapnel_class *)g1_create_object(shrapnel_type);
		if (shrapnel)
		{
			i4_float rh = (i4_rand()&0xFFFF)/((i4_float)0xFFFF) * 0.2f;
			shrapnel->setup(x, y, h + rh, 10, i4_T);
		}

		unoccupy_location();
		request_remove();
		return;
	}
	if (!counter())
	{
		counter()=reset_time();

		int take_away_total=0;

		for (int i=0; i<G1_MAX_PLAYERS; i++)
		{
			if (i!=player_num)
			{
				int take_away=income_rate();
				if (g1_player_man.get(i)->money() - take_away<0)
				{
					take_away=g1_player_man.get(i)->money();
				}

				g1_player_man.get(i)->money() -= take_away;
				take_away_total+=take_away;
				char msg1[70];
				sprintf(msg1,"A lawyer sued us for $%d.",take_away);
				g1_player_man.show_message(msg1,0xff0000,i);
			}
		}

		if (take_away_total>0)
		{
			sw32 tk=(sw32)(take_away_total * commision()); //we exspect commision to be <1
			g1_player_man.get(player_num)->money() += tk;
			//if (g1_current_controller.get())
			//    g1_current_controller->add_spin_event("powerup_lawyer", 0);
			//this cannot be used since it happens every 2seconds
			char msg[200];
			sprintf(msg,"Our lawfirm just won a lawsuite. We got $%d.",tk);
			g1_player_man.show_message(msg,0x00ff00,player_num);
		}

	}
	else
	{
		counter()--;
	}

	request_think();

}

void g1_lawfirm_class::change_player_num(int new_team)
{
	if (new_team!=player_num)
	{

		if (new_team==g1_player_man.local_player)
		{
			if (g1_current_controller.get())
			{
				g1_current_controller->add_spin_event("powerup_lawyer", 0);
			}

		}

		if (player_num==g1_player_man.local_player)
		{
			if (g1_current_controller.get())
			{
				g1_current_controller->add_spin_event("powerup_lawyer", 1);
			}

		}
	}
	g1_building_class::change_player_num(new_team);
}



//li_automatic_add_function(g1_lawfirm_think, "lawfirm_think");
