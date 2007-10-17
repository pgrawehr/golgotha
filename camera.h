/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/

#ifndef G1_CAMERA_HH
#define G1_CAMERA_HH

#include "math/num_type.h"
#include "math/vector.h"
#include "reference.h"
#include "g1_object.h"

class g1_loader_class;     // golg/saver.hh
class g1_saver_class;      // golg/saver.hh
class i4_transform_class;  // i4/math/transform.hh
class g1_object_class;     // golg/g1_object.hh

/// This enum defines different possibilities to set the camera location.
enum g1_view_mode_type
{
	G1_EDIT_MODE,        /// camera is set exactly by someone without outside influence
	G1_STRATEGY_MODE,    /// camera looks down from fixed height above ground
	G1_ACTION_MODE,      /// camera follow the stank from 1st person
	G1_FOLLOW_MODE,      /// camera follows the stank from 3rd person
	G1_CAMERA_MODE,      /// camera follows spline set down by the editor
	G1_WATCH_MODE,       /// camera wanders around looking at interesting things
	G1_CIRCLE_WAIT,      /// camera is circling an object/location until the user presses a key
	G1_MAXTOOL_MODE,     /// camera shows single object, context is maxtool
	G1_INTELLIGENT_MODE, /// unimplemented: camera behaves like in tomb raider
	G1_LOOK_OUT_MODE,    /// unimplemented: camera looks around from a fixed point
	G1_AUX_ACTION_MODE,  /// camera follows object from 1st person
	G1_AUX_FOLLOW_MODE,  /// camera follows object form 3rd person
};

enum g1_watch_type       // priorities for the camera
{
	G1_WATCH_INVALID,    // indicates camera event is not in use
	G1_WATCH_IDLE,       // lowest priority, vehicles moving
	G1_WATCH_EVENT,      // heli take off, bridge assembly, etc.
	G1_WATCH_FIRE,       // shots fired
	G1_WATCH_HIT,        // someone got hit
	G1_WATCH_EXPLOSION,  // someone blew up
	G1_WATCH_USER,       // user wants top look here
	G1_WATCH_FORCE       // force the camera here (base blown up)
};



class g1_camera_info_struct
{
public:
	i4_float gx, gy, gz;
	i4_float ground_x_rotate, ground_y_rotate;
	i4_float ground_rotate,    // rotation about game z
			 horizon_rotate, // rotation up/down
			 roll;
	i4_float scale_x,scale_y,scale_z;

	void defaults();
	g1_camera_info_struct()
	{
		defaults();
	}
	void load(g1_loader_class * fp);
	void save(g1_saver_class * fp);

};

struct g1_camera_event
{
	g1_typed_reference_class<g1_object_class> camera_at;
	g1_watch_type type;     // type of camera shot this is
	// object we are sort of tracking (0 if not tracking)
	g1_typed_reference_class<g1_object_class> follow_object;
	int min_time, max_time; // min/max times to sustain camera shot (in ticks)
	int time_elapsed;       // time that we've spent on this shot already (in ticks)

	g1_camera_event();
	void object_ids_changed(); // call after level is reloaded
};

extern int stereoport; //The default port address for lpt1

class g1_view_state_class
{
	friend li_object *g1_set_camera_params(li_object * o, li_environment * env);
	friend li_object *g1_edit_camera_params(li_object * o, li_environment * env);
	friend li_object *g1_toggle_glasses(li_object * o, li_environment * env);
public:
	enum {
		DATA_VERSION=1
	};
	enum {
		STEREO_NONE, STEREO_LEFT, STEREO_RIGHT, STEREO_BLACK
	} stereomode;

private:
	i4_3d_vector move_offset;  // next time the camera updates it will move this distance (not z)
	i4_float strategy_height_offset; //used to keep track of the height diff.
	i4_bool mode_changed;

private:
	struct circle_info
	{
		float zvel,
			  theta,  theta_vel,
			  dist, dist_vel;

		i4_3d_vector looking_at;
	} circle;


	g1_typed_reference_class<g1_object_class> camera1, camera2;

	g1_camera_event next_cam, current_cam;




	w32 follow_object_id;
	w32 ticks_to_watch_spot;

	g1_view_mode_type view_mode;
	g1_watch_type watch_type;

	g1_camera_info_struct start,      // when the camera is zooming from one location to another
						  end;

	// where is the camera between start and end? 1.0 = end, 0.0 = start
	float start_end_interpolate_fraction;

	// how much is added to the above fraction in each tick, calculated by how many ticks
	// you want the camera to take to get to it's destination
	float start_end_interpolate_fraction_step;

	// do not set this directly, it is calculated from start and end
	g1_camera_info_struct current;


	i4_bool start_invalid;    // when the game first starts, the start camera is invalid
public:

	i4_bool stereo_enabled()
	{
		if (stereomode==STEREO_NONE)
		{
			return i4_F;
		}
		else
		{
			return i4_T;
		}
	}

	void enable_stereo(i4_bool enable=i4_T)
	{
		if (enable)
		{
			stereomode=STEREO_LEFT;
		}
		else
		{
			stereomode=STEREO_NONE;
		}
	}
public:
	void defaults();

	void update_circle_mode();
	void update_follow_mode();
	void update_action_mode();
	void update_watch_mode();
	void update_camera_mode();
	void update_strategy_mode();
	void update_edit_mode();
	void use_next_cam();
public:
	i4_transform_class *get_transform();
	void get_camera_pos(i4_3d_vector &v);
	// returns the sqaured distance to the camera
	float dist_sqrd(const i4_3d_vector &v);

	void load(g1_loader_class * fp);
	void save(g1_saver_class * fp);

	g1_view_state_class();
	g1_camera_info_struct *get_camera();  // calculates current and returns a pointer to it
	void update();                 // should be called after every tick, updates end camera pos

	g1_view_mode_type get_view_mode()
	{
		return view_mode;
	}
	void set_view_mode(g1_view_mode_type mode)
	{
		if (view_mode!=mode)
		{
			mode_changed=i4_T;
		}
		view_mode=mode;
	}

	void calc_transform(i4_transform_class &t);

	void suggest_camera_mode(g1_view_mode_type mode,
							 w32 follow_object_global_id=0);

	bool is_following_object(w32 object_id)
	{
		return follow_object_id==object_id;
	}

	w32 get_following_object()
	{
		return follow_object_id;
	}

	void follow_object(w32 id)
	{
		follow_object_id=id;
	}

	// called by radar map
	void set_camera_position(float game_x, float game_y);

	void suggest_camera_event(g1_camera_event &event);

	void rotate(i4_float about_game_z, i4_float up_down);
	void pan(i4_float x, i4_float y, i4_float z);
	void roll(i4_float r);
	void zoom(float dist);
	void scale(float x, float y, float z);
};

g1_view_state_class *g1_current_view_state();

#endif
