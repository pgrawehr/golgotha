/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

// Golgatha Generic game object class
//
// The g1_object_class is the base type for all non-building/ground objects in the game.


#ifndef G1_OBJECT_HH
#define G1_OBJECT_HH

#include "arch.h"
#include "math/num_type.h"
#include "g1_limits.h"
#include "math/transform.h"
#include "image/context.h"

#include "obj3d.h"
#include "reference.h"
#include "draw_context.h"
#include "error/error.h"
#include "player_type.h"
#include "objs/model_id.h"
#include "objs/model_draw.h"
#include "path.h"
#include "range.h"
#include "solvemap.h"
#include "global_id.h"


class g1_loader_class;
class g1_saver_class;
class g1_poly_list;
class g1_saver_class;
class g1_loader_class;
class i4_graphical_style_class;
class i4_event_handler_class;         // defined in device/device.hh
class li_object;
class li_symbol;
class li_environment;
class li_class;
class g1_path_object_class;
class g1_object_definition_class;
class g1_screen_box;
struct g1_object_defaults_struct;     // objs/defaults.hh
class g1_damage_map_struct;            // objs/defaults.hh
class g1_team_icon_ref;
class g1_player_piece_class;

//g1_mini_object stuff
class g1_mini_object
{
public:
  void draw(g1_draw_context_class *context, 
            i4_transform_class *transform,
            g1_screen_box *bound_box,
            g1_player_type player,
            i4_transform_class *use_this_transform=0, //if you want to supply a local space transform
            i4_bool pass_world_space_transform=i4_T,  // if you don't want lighting pass i4_F
            i4_bool use_lod_model=i4_F);
            
  i4_3d_vector offset;       // tells where the center of mini_object model is (where to rotate)
  i4_3d_vector loffset;

  i4_float x,y;              // location in game map in game coordinates
  i4_float lx,ly;            // last tick position (use to interpolate frames)
	
  i4_float h;                // height above (or below ground)
  i4_float lh;               // last tick height

  i4_3d_vector rotation;     // contains rotation information
  i4_3d_vector lrotation;
  
  g1_model_id_type defmodeltype; //model used by this mini object. can be overridden with draw()
  g1_model_id_type lod_model;

  w16 frame;          // current animation frame
  w16 animation;      // current animation number in model
  
  void calc_transform(i4_float ratio, i4_transform_class *trans);
  // calculates transform to parent system

  void position(const i4_3d_vector &pos) { lx=x=pos.x; ly=y=pos.y; lh=h=pos.z; }
  
  void grab_old()
  //be sure this is called by any parent object grab_old()'s
  {
    lx=x;
    ly=y;
    lh=h;
    lrotation = rotation;    
    loffset = offset;
  }
};



class g1_object_chain_class
{
public:
  g1_object_class       *object;
  g1_object_chain_class *next;
  w32 offset; //should be expanded to 32 bit to support larger maps
	//but could give problems. No, it seems there was another bug.
  inline g1_object_chain_class *next_solid();  // defined after g1_object_class
  g1_object_chain_class *next_non_solid() { return next; }  
};


enum g1_radar_type
{
  G1_RADAR_NONE,
  G1_RADAR_VEHICLE,
  G1_RADAR_STANK,
  G1_RADAR_WEAPON,
  G1_RADAR_BUILDING,
  G1_RADAR_PATH_OBJECT
};

/** The type of object ids */
typedef sw16 g1_object_type;

/** The base class for all objects.
This class is the base for all objects that can be on the map.
It is pure virtual, so no object can be of its actual type. 
For every map tick (1/10th of a second) the think() method is called.
For every image frame, draw() is called. 
*/

class g1_object_class
{
protected:
  friend class g1_reference_class;
  friend class g1_map_class;
  friend class g1_map_cell_class;
  friend class g1_remove_manager_class;

  /** several aged load functions to load old maps */
  void load_v2(g1_loader_class *fp);
  void load_v3(g1_loader_class *fp);
  void load_v4(g1_loader_class *fp);
  void load_v5(g1_loader_class *fp);
  void load_v6(g1_loader_class *fp);
  void load_v7(g1_loader_class *fp);
  void load_v8(g1_loader_class *fp);
  void load_v9(g1_loader_class *fp);

  //! a list of all objects that reference us, so we can remove the reference when we delete ourself.
  //! Don't modify this unless you know really what you're doing.
  //! @see g1_reference_class
  i4_isl_list<g1_reference_class> ref_list;
  
  //! list of squares this object is on (max of 4), if value is 0xffff then it is not a reference.
  //! the last one holds the center of the object, if necessary (ie, the object takes 
  //! up more than 1 square and his corners are far enough apart)
  //! Hint: There are cases where this can become larger than 4.
  i4_array<g1_object_chain_class> occupied_squares;

  //! This is for internal use only. 
  g1_object_chain_class *new_occupied_square()
  {
    g1_object_chain_class *ret = occupied_squares.add();
    ret->object = this; 
    return ret;
  }
  //! Occupies the map.
  //! Version for large objects.
  i4_bool occupy_location_model(const g1_model_draw_parameters& draw_params);
  //! Version for cases where you know what you're doing.
  i4_bool occupy_location_corners();
  //! For small objects only.
  i4_bool occupy_location_center();     // only adds the center of the object
  //! Similar to the first one.
  i4_bool occupy_location_model_extents(const g1_model_draw_parameters &draw_params);
public:

  // generic type flags for run time object caster, defaults to can't cast
  /*enum  //this would get confusions with the type manager
  {
    TO_MAP_PIECE      = (1<<0),
    TO_PLAYER_PIECE   = (1<<1),
    TO_DYNAMIC_OBJECT = (1<<2),
    TO_PATH_OBJECT    = (1<<3),
    TO_DECO_OBJECT    = (1<<4),
  };*/

  //! A virtual function that always returns 0.
  virtual w32 private_safe_to_cast() const { return 0; }

  w32 global_id;                ///< unique to the object, used to reference via networking and lisp code

  g1_object_type id;            ///< this is the object's 'type' id

  i4_float x;                 ///< location in game map in game coordinates
  i4_float y;				  ///< location, y component.
  i4_float lx;               ///< last tick position (used to interpolate frames)
  i4_float ly;				 ///< last tick, y component.

  i4_float h;                   ///< height above (or below) ground
  i4_float lh;                  ///< last tick height

  i4_float theta;               ///< facing direction in the x-y game plane
  i4_float ltheta;              ///< last tick theta (yaw)

  i4_float pitch;               ///< pitch (y-z)
  i4_float lpitch;              ///< last tick pitch

  i4_float roll;                ///< roll (x-z)
  i4_float lroll;               ///< last tick roll

  sw16 health;                  ///< the health its got left

  li_class *vars;               ///< lisp variables used for dialog's.  These load and save automatically.

  g1_player_type player_num;    ///< The player this object belongs to.
  g1_radar_type radar_type;	    ///< The type of radar image for this unit. Used if radar_image is 0;
  g1_team_icon_ref *radar_image;///< An image to show in the radar for this unit.

  /** Changes the owner of an unit. */
  virtual void change_player_num(int new_player_num);  // don't change player_num directly
  /** Gets the team this player belongs to. 
  Internally, player number and team are not the same, since
  players could play together, but this is rarelly used right now.
  */
  g1_team_type get_team() const;
  
  /** Some very important object flags */
  enum flag_type
  {
    SELECTED      =1<<0,   ///< if object has been selected by player
    THINKING      =1<<1,   ///< if object is in a level's think list
 
    SHADOWED      =1<<2,   ///< if object should have shadows drawn for it
    MAP_INVISIBLE =1<<3,   ///< if object is invisible
    DELETED       =1<<4,   ///< if object has been removed, but remove has not been processed yet
    IN_TUNNEL     =1<<5,   ///< if the object is in a tunnel

    SELECTABLE    =1<<6,   ///< if the user can select the object in the game
    TARGETABLE    =1<<7,   ///< if object can be attacked

    GROUND        =1<<8,   ///< This object is on the ground
    UNDERWATER    =1<<9,   ///< This object is in the water
    AERIAL        =1<<10,  ///< This object is flying. 

    HIT_GROUND    =1<<11,  ///< We can attack things on the ground
    HIT_UNDERWATER=1<<12,  ///< We can attack submarines
    HIT_AERIAL    =1<<13,  ///< can attack objects in the air

    BLOCKING      =1<<14,  ///< if object blocks passage of other objects

    SCRATCH_BIT   =1<<15,  ///< use this for temporary calculations, but set it back to 0 when done
    MAP_OCCUPIED  =1<<16,  ///< makes sure you don't call occupy_location/unoccupy_location twice
    
    DANGEROUS     =1<<17,  ///< object should be killed

    RADAR_REMOVE  =1<<18,  ///< object needs to be removed from radar
    CAN_DRIVE_ON  =1<<19,  ///< if objects can drive over / stand on you
    ON_WATER      =1<<20,  ///< if objects is on water
	SPECIALTARGETS=1<<21,  ///< if object has special targets (i.e own units)
	EXT_GLOBAL_ID =1<<22,  ///< uses special way to find global id
	USES_POSTTHINK=1<<23,  ///< Nonzero if post_think() method should be called.
	BLOCKS_MAP    =1<<24,  ///< if this object is on the blockage maps.

	//the following flags are particularly important in networking mode
	NEEDS_SYNC    =1<<25,  ///< Nonzero if the object has thought something that cannot be quessed 
						   ///< on the remote machine
	LAST_SYNC     =1<<26,  ///< Has synced last frame (perhaps requires resending)
	LOST_SYNC     =1<<27,  ///< Is known to have lost sync. 
	NEWER_SYNC    =1<<28   ///< This object cannot get syncronized as the remote has different id.
  };

  //! This Flag is used internally upon load.
  enum { SAVE_FLAGS= SELECTED | THINKING };//This mask is applied on object creation from a file only

  //! Where all the flags go.
  w32 flags;
  //! Some convenience functions to modify flags.
  int get_flag(int x) const { return flags & x; }
  void set_flag(int x, int value) { if (value) flags|=x; else flags&=(~x); }  
  i4_bool selected() const { return (w8)flags & SELECTED; }
  i4_bool out_of_bounds(i4_float x, i4_float y) const;
  i4_bool moved() const { return (lx!=x || ly!=y); }
  i4_bool valid() const { return get_flag(MAP_OCCUPIED)!=0; }

  //! The type class of the object. 
  g1_object_definition_class *get_type();     // inlined below

  void mark_as_selected()
	  {
	  set_flag(SELECTED,1);
	  };
  void mark_as_unselected()
	  {
	  set_flag(SELECTED,0);
	  }

  //! Should return the size of the object (if it were a sphere)
  virtual i4_float occupancy_radius() const;

  // called when the object is on a map cell that is about to be drawn
  //virtual void predraw() {}

  //! called when the object is on a map cell that is drawn.
  //! This function is called exactly once per frame when the object
  //! is actually visible. Under most circumstances, this function
  //! should not change the internal state of the object. 
  virtual void draw(g1_draw_context_class *context);

  //! called for each object after everything else has been drawn and in editor mode
  virtual void editor_draw(g1_draw_context_class *context);

  //! Really seldomly used function
  virtual void note_stank_near(g1_player_piece_class *s) { ; }

  //! called every game tick.
  //! A game tick is always 1/10th of a second.
  //! You can safely assume that no other object is modifying any data
  //! while you think. 
  virtual void think() = 0;

  //! If you have to think about something after all other units have thought.
  //! Only called if the corresponding flag is set. 
  virtual void post_think() {;}

  /** The constructor of g1_object_class.
  A constructor of an object creates an object of the given type.
  If fp is nonzero, the object is loaded from the given file. 
  There must not be any other constructors for any of the derived classes.
  The constructor should not call occupy_location().
  */
  g1_object_class(g1_object_type id, g1_loader_class *fp);

  //!a mostly empty function
  virtual void validate() {}
  //!saves this object to the given file. Can safely assume fp!=0
  virtual void save(g1_saver_class *fp);

  //! Synchronization function.
  //! Does the same as the constructor, but doesn't reset the other data
  //! Can safely assume fp!=0 and data_version==latest.
  //! An object that doesn't have a save() doesn't need this either.
  virtual void load(g1_loader_class *fp);

  //! Skips the corresponding amount of data in the file stream.
  //! Used if the given object does not need to be synced 
  //! i.e if we received a packet that contains older data than the last 
  //! one for a particular object.
  virtual void skipload(g1_loader_class *fp);

  //! Request that your think() method is called the next tick.
  //! Can safely be called more than once in sequence, even without a
  //! performance penalty.
  //! @see think()
  void request_think();

  //! Request to get removed the next tick.
  //! Call unoccupy_location() right before.
  virtual void request_remove();

  //! Call to add object to a map (adds to cell x,y where object is standing).
  //! May decide (via virtual override) which occupy_xxx version is used.
  //! @return i4_F if an error occured (usually happens if location is not on map)
  virtual i4_bool occupy_location();

  //! call to remove an object from a map.
  //! Always call unoccupy_location before moving the object!
  virtual void unoccupy_location();

  //! Try to go to the indicated location.
  //! If ph is not 0, uses the given path.
  //! @return i4_F if cannot go there. This base implementation always 
  //!         returns i4_F.
  virtual i4_bool deploy_to(float x, float y, g1_path_handle ph) 
	  {
	  if (ph) g1_path_manager.free_path(ph);
	  return i4_F; 
	  }

  //! Called if this unit should take damage.
  //! Can be overriden, ie if your object should be invulnerable.
  virtual void damage(g1_object_class *who_is_hurting,//we got some damage
                      int how_much_hurt, i4_3d_vector damage_dir);

  //! Check for collisions.
  //! @param start (x,y,h) pos of position vector
  //! @param ray Direction vector.
  //! @return i4_T if the ray starting from start intersects us.
  virtual i4_bool check_collision(const i4_3d_vector &start, 
                                  i4_3d_vector &ray);

  enum {NOTIFY_DAMAGE_KILLED=1};

  //! Notifies us that we just applied damage to another unit.
  //! Only usefull for weapons that support this kind of info.
  virtual void notify_damage(g1_object_class *obj, sw32 hp)//informs firing object that the target was hit
	  {};
  
  //! Calculate some transformations.
  //! calculates the transform from object coordinates to the world and stores it in 
  //! 'transform'.  if transform is null, it stores it into the object's internal storage.
  virtual void calc_world_transform(i4_float ratio, i4_transform_class *transform=0);
  
  i4_transform_class *world_transform; ///<calculated and linearly allocated at draw time

  //! Contains information about the geometry of the object. 
  g1_model_draw_parameters draw_params;
  //! The mini-objects. 
  //! Mini objects are "parts" of another object that can move independently
  g1_mini_object *mini_objects;
  //! The number of mini_objects.
  w16 num_mini_objects;

  void allocate_mini_objects(int num,char *reason)
  {
    mini_objects = (g1_mini_object *)I4_MALLOC(sizeof(g1_mini_object)*num,reason);
    memset(mini_objects,0,sizeof(g1_mini_object) * num);
    num_mini_objects = num;
  }

  virtual void grab_old();     // grab info about the current tick for interpolation

  //  to be consistant, every g1_object should have an init
  virtual void init()  {}

  //! use this instead of defaults->max_health.
  virtual short get_max_health();
  //! returns usually 0, but not always.
  virtual short get_min_health() {return 0;}
  //! show in editor if mouse cursor stays still on object
  virtual i4_str *get_context_string();  

  //! The virtual destructor. 
  virtual ~g1_object_class();

  //! Return true if you can attack the object pointed to.
  virtual i4_bool can_attack(g1_object_class *who) { return i4_F; }

  //! Remove yourself from the think-list. 
  //! This is an expensive function and should only be used if needed.
  //! You usually only call this one just before you call request_remove().
  //! Otherwise it's usually better just not to call request_think() any more.
  virtual void stop_thinking();

  //! Get some info about pos.
  float height_above_ground();      // calls map->terrain_height

  //! The name of the object type. 
  //! Use some descriptive name, as it's not only used internally!
  //! Avoid using spaces in the name.
  char *name();
  
  //! Passes some messages around.
  virtual li_object *message(li_symbol *message_name,
                             li_object *message_params, 
                             li_environment *env) { return 0; }

  //! Called for every object when you click 'ok' in the vars edit dialog for an object.
  //! Carefully read what this means! This method is called for EVERY object
  //! when the user clicks on ok in the edit dialog of ANY object. 
  //! Therefore, you usually do something like if (who==this) in here.
  virtual void object_changed_by_editor(g1_object_class *who, li_class *old_values) {}
  virtual int get_chunk_names(char **&list) { return 0; }

  enum {
	  FO_NONE=0,
	  FO_GOAHEAD=1,
	  FO_KILLSELF=2,
	  FO_ENDPATH=3,
	  FO_GROUP_ARBITRARY=4,
	  FO_GROUP_BOX=5,
	  FO_GROUP_ARROW=6,
	  FO_GROUP_LINE=7,
	  FO_GROUP_ONPAD=8,
	  FO_ENUM_FORCE_DWORD=0x7fffffff
	  };
  //! Called when an object reaches the end of a path or a formation is requested.
  //! If where is 0, the function tries to find the best spot itself.
  //! @param formationcode Some FO_XXX constant.
  //! @param where The spot where a formation should be built.
  //! @return i4_T if the unit could be added to the formation.
  virtual i4_bool enter_formation(int formationcode, g1_object_class *where);

  //moved these here to allow any object to be a factory under some
  //circumstances (needed i.e for road_object)
  virtual g1_path_object_class *get_start()
	  {return 0;};
  virtual void set_start(g1_path_object_class *start){};
  //! Returns i4_T if this object can build objects of the given type.
  virtual i4_bool build(int type)
	  {return i4_F;};//can't build anything
  //! Returns the color of the path used for objects of this building.
  virtual w32 get_path_color()
	  {return 0xffa00000;}
  //! Returns the color of the path used for objects of this building if selected.
  virtual w32 get_selected_path_color()
	  {return 0xffff0000;}
};


inline g1_object_chain_class *g1_object_chain_class::next_solid()
{
  if (!next || !next->object->get_flag(g1_object_class::BLOCKING))
    return 0;
  else return next;
}

//! This enum contains the reasons why a build might fail. 
//! Be aware that you cannot always depend on these, since build requests
//! are usually queued. If inserting into the queue works, but later on
//! we discover that we can't build anyway (i.e to few money) the build
//! is postphoned. 
enum eBuildError
{
  G1_BUILD_OK=0,                       //< successful build
  G1_BUILD_WAIT,                       //< needs more time to build
  G1_BUILD_TOO_EXPENSIVE,              //< needs more money to build
  G1_BUILD_OCCUPIED,                   //< space to build is occupied
  G1_BUILD_NO_SPACE,                   //< no space to build
  G1_BUILD_INVALID_OBJECT,             //< invalid object or object parameters
  G1_BUILD_ALREADY_EXISTS,             //< unique object already exists
  G1_BUILD_PLAYBACK,                   //< playback in progress, can't build manually
};

class g1_object_definition_class;

//! This is the call to add a new object_type to the game.
g1_object_type g1_add_object_type(g1_object_definition_class *def);

//! This is the call to find the object_type of a specified object
g1_object_type g1_get_object_type(const char *name);
g1_object_type g1_get_object_type(li_symbol *name);

//! Remove an object from the list.
//! Remove probably doesn't need to be used during a normal game, but can be useful for
//! adding and removing a dll during a running session instead of restarting the game.
//! It is automatically called for each dynamic object of a map on unloading or
//! reloading of the level. 
void g1_remove_object_type(g1_object_type type);

//! Object definition class. 
//! This class defines an object to golgotha, primarly how to create such an object and
//! load it from disk.  The object then has virtual functions that allow it do things like
//! think, save, and draw.  Objects can be created at anytime in the game, and should be
//! able to be linked in through dlls or created dynamically using lisp code. 
//! In the current implementation, you should avoid adding dynamic objects while
//! a level is loaded.
class g1_object_definition_class
{
protected:
  typedef void (*function_type)(void);
  typedef void (*special_function_type)(g1_object_definition_class *type);
  char *_name;
  function_type init_function, uninit_function;
  
  g1_damage_map_struct          *damage;     // loaded from scheme/balance.scm

public:
    //! Object type flags.
    //! These flags define some properties of the object type. 
    //! The members that start with TO_ are used for casting. 
    //! If you have an instance of g1_object_class whose type has 
    //! TO_MAP_PIECE set, you know that it is actually an instance
    //! of g1_map_piece_class or a subclass of that one. 
  enum 
  {
    EDITOR_SELECTABLE = (1<<0),      //< object shows up in editor
    DELETE_WITH_LEVEL = (1<<1),      //< delete this type with the map objects
    MOVABLE           = (1<<2),      //< The object can move
    TO_MAP_PIECE      = (1<<3),      //< Can be cast to g1_map_piece_class. See above.
    TO_PLAYER_PIECE   = (1<<4),
    TO_DYNAMIC_OBJECT = (1<<5),
    TO_PATH_OBJECT    = (1<<6),
    TO_DECO_OBJECT    = (1<<7),
	TO_FACTORY        = (1<<8),
	TO_BUILDING       = (1<<9),
    HAS_ALPHA         = (1<<10),      //< if object has alpha polys it will draw after non-alpha objects
	TO_MOVABLE_DYNAMIC_OBJECT = (1<<11),
    TO_TRIGGER        = (1<<12)
  };

  special_function_type special_init_function;//used for dynamically created objects
  w32                        var_class;   // class type to create for object's vars
  li_class                   *vars;       // variables specific to the type
  g1_object_defaults_struct  *defaults;   // loaded from scheme/balance.scm
  
  g1_damage_map_struct        *get_damage_map();
  
  w32 flags;
  int get_flag(int x) const { return flags & x; }
  void set_flag(int x, int value) { if (value) flags|=x; else flags&=(~x); }  

  g1_object_type type;

  g1_object_definition_class(char *_name,
                             w32 type_flags = EDITOR_SELECTABLE,
                             function_type _init = 0,
                             function_type _uninit = 0);

  virtual ~g1_object_definition_class() { g1_remove_object_type(type); }

  // create_object should return a new initialized instance of an object, if fp is null then
  // default values should be supplied, otherwise the object should load itself from the file
  virtual g1_object_class *create_object(g1_object_type id,
                                         g1_loader_class *fp) = 0;

  // the name on an object should be unique, so you might want to be creative,  the name
  // is used to match up the object types in save files since object type id's are dynamically
  // created
  const char *name() { return _name; }

  i4_bool editor_selectable() { return (w8)flags & EDITOR_SELECTABLE; }

  virtual void init();
  virtual void uninit() { if (uninit_function) (*uninit_function)();}
  
  virtual void save(g1_saver_class *fp) { ; }  // save info about type
  virtual void load(g1_loader_class *fp) { ; } // load info about type

  // this is called when the object is right-clicked, the dialog is added to a draggable frame
  virtual i4_window_class *create_edit_dialog();

};

void g1_apply_damage(g1_object_class *kinda_gun_being_used,
                     g1_object_class *who_pulled_the_trigger,
                     g1_object_class *whos_on_the_wrong_side_of_the_gun,
                     const i4_3d_vector &direction_hurten_is_commin_from);

// increase this number if you change the load/save structure of g1_object_class
#define G1_OBJECT_FORMAT_VERSION 1 


// this table has an array of pointers to object definitions
// this is used by the border frame to find object with build info so it 
// can add buttons for them
extern g1_object_definition_class *g1_object_type_array[G1_MAX_OBJECT_TYPES];
extern g1_object_type g1_last_object_type;  // largest object number assigned

// this is prefered way to create new object in the game
inline g1_object_class *g1_create_object(g1_object_type type)
{ 
  if (g1_object_type_array[type])
	  {
	  g1_object_class *o=g1_object_type_array[type]->create_object(type, 0);
	  o->request_think();
	  //cannot do this from here as we don't want to sync _every_ object.
	  //This particularly applies to explosion objects which will be
	  //generated separatelly on every system (with different IDS, unfortunatelly). 
	  //o->set_flag(g1_object_class::NEEDS_SYNC,1);
	  o->set_flag(g1_object_class::NEWER_SYNC,1);//All objects created solely with this 
	  //method will be created automatically by the remote machine using 
	  //a different id. (Because we exspect the remote objects to think
	  //exactly like the local ones)
      return o;
	  }
  else 
    return 0;
}

inline g1_object_definition_class *g1_object_class::get_type() { return g1_object_type_array[id]; }

void g1_initialize_loaded_objects(); 
void g1_uninitialize_loaded_objects();

#endif


