/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/
// Golgatha Map Object Class (Moving objects are derived from this)
//

#ifndef MAP_PIECE_HH
#define MAP_PIECE_HH

#include "g1_object.h"
#include "sound_man.h"
#include "player_type.h"
#include "range.h"
#include "objs/defaults.h"
#include "objs/model_draw.h"
#include "objs/model_collide.h"
#include "obj3d.h"
#include "global_id.h"
#include "sound/sfx_id.h"
#include "objs/vehic_sounds.h"
#include "path.h"
#include "map.h"

class g1_path_object_class;     // objs/path_object.hh
class g1_road_object_class;
class g2_link;

const i4_float VSPEED = 0.04f;
const i4_float FLY_HEIGHT = 1.5f;
const i4_float DROP_HEIGHT = 2.7f;
extern w32 g1_tick_counter;          // number of ticks in simulation model

class g1_map_piece_class : public g1_object_class
{
public:
  void fix_forward_link(g1_object_class *next) { next_object = next; }
  void fix_previous_link(g1_object_class *prev) { prev_object = prev; }
protected:
  friend class g1_path_object_class;
  friend class g1_road_object_class;

  g1_typed_reference_class<g1_object_class> next_object;
  g1_typed_reference_class<g1_object_class> prev_object;
  i4_float path_pos, path_len;
  i4_float path_cos, path_sin, path_tan_phi;
  i4_float stagger;
public:
  void unlink();
  void link(g1_object_class *origin);
  i4_bool can_enter_link(g1_path_object_class *from, g1_path_object_class *to);
  g2_link *link_on();  
  
  g1_object_class *get_next_object()
	  {
	  if (next_object.valid())
		  return next_object.get();
	  return 0;
	  }
  g1_object_class *get_prev_object()
	  {
	  if (prev_object.valid())
		  {
		  return prev_object.get();
		  }
	  return 0;
	  }
  g1_object_defaults_struct *defaults;

  i4_float           speed;          // current speed of the piece
  i4_float           vspeed;         // vertical speed of the piece
  i4_float           dest_x, dest_y; // if it has a destination, its point & direction is here
  i4_float           dest_z;         // if it has a destination height
  i4_float           dest_theta;
  w16                fire_delay;     // number of game ticks before we can fire again
  g1_id_ref         *path_to_follow;  // null terminated array of global id's
  g1_id_ref          next_path;

  // calculated values
  i4_float           tread_pan;      // accumulated tread texture pan based on speed
  i4_float           terrain_height; // terrain under vehicle
  i4_float           groundpitch, lgroundpitch;
  i4_float           groundroll, lgroundroll;
  i4_float           damping_fraction;
  w8                 ticks_to_blink;   // when taking damage (not saved)
  i4_float			 used_path_len;
  //returns the prefered solver
  virtual g1_map_solver_class *prefered_solver();
  solve_flags        solveparams;
  g1_map_solver_class *my_solver;//use this one if you know what it is
  // path information
  enum {NO_PATH, GAME_PATH, FINAL_POINT};
  g1_path_handle path;
  w8                 death;          // type of death desired

  i4_3d_vector       damage_direction; //inertia vector of whatever attacked last (or killed us)

  g1_typed_reference_class<g1_object_class> attack_target;  // the thing its targeting

  g1_map_piece_class(g1_object_type id, g1_loader_class *fp);
  void load(g1_loader_class *fp);
  void skipload(g1_loader_class *fp);

  virtual ~g1_map_piece_class();

  void add_team_flag();
  i4_bool alive() const { return health>0; }

  void hit_ground();      // when an object falls off a cliff and hits the ground
  void find_target(i4_bool unfog=i4_T);

  i4_bool find_target_now() const
	{
        return (((g1_tick_counter+global_id)&3)==0);
	}
  virtual void fire();
  virtual void damage(g1_object_class *obj, int hp, i4_3d_vector damage_dir);
  void lead_target(i4_3d_point_class &lead_point, i4_float shot_speed=-1);
  virtual void save(g1_saver_class *fp);
  /**
  Detection Range of this vehicle.
  */
  float detection_range() const { return defaults->detection_range; }
  /**
  Movement check.
  checks wheter this move is ok. (Collision etc.)
  might adjust dest_x and dest_y to allow a small movement
  in a slightly other direction than requested
  @param dest_x x coordinate of position I wanna go to, passed as reference, may be changed uppon return
  @param dest_y y coordinate of position, same limitations
  */
  i4_bool check_move(i4_float &dest_x,i4_float &dest_y);

  virtual void change_player_num(int new_player_num);
  virtual i4_bool check_collision(const i4_3d_vector &start, i4_3d_vector &ray)
  { return g1_model_collide_radial(this, draw_params, start, ray); }

  virtual void calc_world_transform(i4_float ratio, i4_transform_class *transform=0);
  
  void set_path(g1_id_ref *path);  // null terminated array of global id's

  g1_rumble_type rumble_type;
  void init_rumble_sound(g1_rumble_type type);
  virtual void init();

  virtual void think();
  virtual void draw(g1_draw_context_class *context);
  i4_bool check_turn_radius();

  virtual void request_remove();//remove me from map

  void advance_path();//go to the next path node, starting from current node
  virtual void return_to_path();//go to the next node, starting from actual pos
  virtual i4_bool deploy_to(float x, float y, g1_path_handle ph);//go there using path-solving methods
  void set_path(g1_path_class *path);//set a path specific for this unit
  w32 follow_path();

  virtual short get_max_health() {return defaults->health;};

  i4_bool suggest_move(i4_float &dist,
                       i4_float &dtheta,
                       i4_float &dx, i4_float &dy,
                       i4_float brake_speed=0.1,
                       i4_bool reversible=i4_T);

  i4_bool suggest_air_move(i4_float &dist,
                           i4_float &dtheta,
                           i4_3d_vector &d);
  //checks the health to make sure the object is still alive
  //if health is <0, the object is removed from the map, and i4_F returned
  i4_bool check_life(i4_bool remove_if_dead=i4_T);
  
  /** 
    Get Info on Terrain.
    Quite expensive function to calculate terrain heights, possible non-flat surface
    constraints and other stuff needed for proper movement. 
    */
  void get_terrain_info();

  /** 
    Virtually derived from parent.
    */
  i4_bool occupy_location()
  {
    int ret=g1_object_class::occupy_location();
    if (ret) 
      get_terrain_info(); 
    return ret;
  }

  /**
    The grab_old functions are used for the frame interpolator.
    Call this method if the object has changed it's location drastically from it's previous one
    */    
  virtual void grab_old()
  {
    g1_object_class::grab_old();
    lgroundpitch = groundpitch;
    lgroundroll  = groundroll;
  }

  /** 
    Standard movement function. 
    Important: This method is not virtual, it's just an example how to do it.
    @param x_amount Amount of movement in x direction (in world units)
    @param y_amount Movement in y direction
    */
  i4_bool move(i4_float x_amount, i4_float y_amount);
  
  /**
    Static cast Method. 
    This method can be used to check wheter a given object instance is a child of 
    g1_map_piece_class (instances of g1_map_piece_class itself cannot exist)
    @return 0 if cannot cast, obj casted to g1_map_piece_class* otherwise
    */
  static g1_map_piece_class *cast(g1_object_class *obj)

	  {
	  if (!obj || !(obj->get_type()->get_flag(g1_object_definition_class::TO_MAP_PIECE)))
		  return 0;
          return (g1_map_piece_class*)obj;
	  };

  virtual i4_bool can_attack(g1_object_class *who);//also used to ask: can we still attack this guy?
  i4_bool in_range(g1_object_class *o) const
	  {
	  float r=detection_range();
	  r*=r;
	  float d=(o->x-x)*(o->x-x) + (o->y-y)*(o->y-y);
          return (d<=r);
	  };

  li_object *message(li_symbol *message, li_object *message_param, li_environment *env);
};

#endif

