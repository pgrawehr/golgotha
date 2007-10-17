/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/

#ifndef HUMAN_HH
#define HUMAN_HH

#include "team_api.h"
#include "memory/que.h"
#include "memory/hashtable.h"

class special_command_entry
{
public:
	char commandname[40];
};

extern i4_hashtable<special_command_entry> command_lookup_table;

class g1_human_class :
	public g1_team_api_class
{

private:
	i4_float dragstartx,dragstarty,dragdestx,dragdesty;
	i4_float mouse_look_increment_x, mouse_look_increment_y;

	//The object that is selected on the map
	g1_typed_reference_class<g1_object_class> selected_object;

	//Building about to be built
	g1_object_class * prepared_building;
public:
	enum {
		DEFAULT,
		GOTO,
		ATTACK,
		OPTIONS,
		FOLLOW_PATH,
		SELECT_PATH,
		BACK_TO_PATH,
		SELECT,
		DESELECT,
		START_DRAG,
		END_DRAG,
		DRAGGING,
		ADD_TO_LIST,
		NOTIFY,
		REMOVE,
		BUILD_UNIT,
		BUILD_BUILDING,
		CHANGE_OWNER,
		RESEARCH,
		NUM_COMMANDS
	};
	enum {
		CLEAR_OLD,
		ADD_TO_OLD,
		SUB_FROM_OLD
	};


	g1_human_class(g1_loader_class * f);
	~g1_human_class();



	void send_selected_units(i4_float x, i4_float y);
	void attack_unit(g1_object_class * o, i4_float x, i4_float y);
	void clicked_on_object(g1_object_class * o);

	w32 show_selection(g1_object_controller_class * for_who,
					   i4_transform_class &transform, g1_draw_context_class * context);

	void draw_button_model(char * buttoncmd, i4_float posx, i4_float posy, i4_float posz, g1_object_class * forobj);
	w8 determine_cursor(g1_object_class * object_mouse_is_on);
	void player_clicked(g1_object_class * o, float gx, float gy,w32 command);
	int build_unit(g1_object_type type);
	bool prepare_to_build_building(g1_object_type type);
	bool is_building_prepared();
	void chancel_prepared_building();
	i4_bool select_path(float gx,float gy);
	void clear_selected();
	virtual void think();
	//char* ai_name() { return "Human"; }

	virtual void load(g1_loader_class * fp);
};


extern g1_human_class * g1_human;

#endif
