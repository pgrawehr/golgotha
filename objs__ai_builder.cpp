/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/

#include "pch.h"
#include "team_api.h"
#include "player.h"

class ai_builder :
	public g1_team_api_class
{
public:
	ai_builder(g1_loader_class *f=0)
	{
	}

	virtual void think()
	{
		build_unit(g1_get_object_type("peon_tank"));
	}
};

class ai_neutral :
	public g1_team_api_class
{
public:
	ai_neutral(g1_loader_class *f=0)
	{
	};
	virtual void think()
	{
	};
};

//currently, doesn't do anything either, but this will perhaps change
class ai_remote_player :
	public g1_team_api_class
{
public:
	ai_remote_player(g1_loader_class *f=0)
	{
	};
	virtual void think()
	{
		//g1_player_man.get(team())->continue_wait=i4_F;
	};
};
g1_team_api_definer<ai_neutral> ai_neutral_def("ai_neutral");
g1_team_api_definer<ai_builder> ai_builder_def("ai_builder");
g1_team_api_definer<ai_remote_player> remote_player("remote_player");
