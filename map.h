/**********************************************************************
  \file
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
  This is map.h, the declaration section of the game map.
***********************************************************************/

#ifndef __MAP_HPP_
#define __MAP_HPP_


#include "arch.h"
#include "error/error.h"
#include "g1_limits.h"
#include "player_type.h"
#include "time/time.h"
#include "g1_vert.h"
#include "g1_object.h"
#include "reference.h"
#include "global_id.h"
#include "path_api.h"
#include "map_cell.h"
#include "map_vert.h"
#include "map_collision.h"
#include "critical_graph.h"
#include "critical_map.h"
#include "memory/dynque.h"
#include "memory/array.h"
//#include "editor/dialogs/path_win.h"

//class g1_map_cell_class;
//class g1_map_vertex_class;
class g1_draw_context_class;
class g1_obj_conscell_class;
class g1_solid_class;
class g1_bullet_class;
class g1_solid_class;
class g1_saver_class;
class g1_loader_class;
class g1_path_manager_class;
class g1_visible_projection;
class g1_quad_object_class;
class g1_movie_flow_class;
class g1_light_object_class;
class i4_str;
class i4_polygon_class;
struct g1_visible_cell;
class i4_pal_handle_class;
class g1_map_solver_class;
class g1_astar_map_solver_class;
class g1_path_manager_class;
class g1_critical_graph_class;
class g1_critical_map_maker_class;
class g1_breadth_first_map_solver_class;
class g1_breadth_first_graph_solver_class;


class g1_takeover_pad_class;
//class g1_critical_graph_class;
typedef g1_typed_reference_class<g1_takeover_pad_class> g1_takeover_pad_ref;

/*! these bit flags are passed into save & load for a map
 this can be used to merge in sections of another map or reduce the amount
 of undo info need to be saved and loaded
 */
enum {
  G1_MAP_CELLS=(1<<0), ///<The map cells
  G1_MAP_VERTS=(1<<1), ///<The map vertices (these two are always loaded together)
  G1_MAP_OBJECTS=(1<<2),///<the instances of the objects
  G1_MAP_MOVIE=(1<<3), ///<Some movie defined on the map
  G1_MAP_PLAYERS=(1<<4),///<Player info (including ai)
  G1_MAP_TICK=(1<<5),///<The tick time (actually only one word)
  G1_MAP_GRAPH=(1<<6),///<The connectivity-Graph
  G1_MAP_LIGHTS=(1<<7),///<Lights information
  G1_MAP_SKY=(1<<8),///<Information about the sky
  G1_MAP_VARS=(1<<9),///<Global map vars
  G1_MAP_CRITICAL_POINTS=(1<<10),///<Critical Points for routing
  G1_MAP_VIEW_POSITIONS=(1<<11),///<Editor Camera position
  G1_MAP_RES_FILENAME=(1<<12),///<Matching resource file name (.scm) [ignored]
  G1_MAP_CRITICAL_DATA=(1<<13),///<Criticals
  G1_MAP_TEXTURES=(1<<14),///<loads the textures
  G1_MAP_MODELS=(1<<15),///<loads the models from the gmod-files
  G1_MAP_TRANSPORT=(1<<16),///<Transport info
  G1_LAST_THING, 
  G1_MAP_ALL=(G1_LAST_THING-1)*2-1,///<Everything

  G1_MAP_SELECTED_VERTS=(1<<14)  ///< not part of G1_MAP_ALL because G1_MAP_VERTS encompases
};

/*! These flags determine the type of routing desired for this object*/
enum {
	  SF_NONE=0,///<not yet determined
	  SF_OK=1,///<this one assures that at least one bit is set if the flagword is valid
	  SF_GRADE1=(1<<1),
	  SF_GRADE2=(2<<1),
	  SF_GRADE3=(3<<1),
	  SF_GRADE4=(4<<1),
	  SF_GRADEMASK=0xfe,
	  
	  SF_USE_PATHS=(1<<9),
	  SF_USE_ASTAR=(1<<10),
	  SF_USE_MAP=(1<<11),
	  SF_USE_GRAPH=(1<<12),
	  SF_USE_ANY=(SF_USE_PATHS|SF_USE_ASTAR|SF_USE_MAP|SF_USE_GRAPH),
	  SF_FORCE_PATHS=(1<<13),      //only one of this group allowed
	  SF_FORCE_ASTAR=(1<<14),
	  SF_FORCE_MAP=(1<<15),
	  SF_FORCE_GRAPH=(1<<16),
	  SF_FORCEMASK=(SF_FORCE_PATHS|SF_FORCE_ASTAR|SF_FORCE_MAP|
	    SF_FORCE_GRAPH),
	  SF_NEWER_USEPATHS=(1<<17),
	  SF_NEWER_USEGRAPH=(1<<18),
	  SF_NEWER_USEMAP=(1<<19),
	  SF_NEWER_USEASTAR=(1<<20),
	  SF_NEWER_USEMASK=(SF_NEWER_USEPATHS|SF_NEWER_USEGRAPH|
		SF_NEWER_USEMAP|SF_NEWER_USEASTAR|(1<<21)),//do not move units at all?
	  SF_NEWER_USEANY=(1<<21),

	  SF_HAS_GRAPH=(1<<22),
	  SF_HAS_PATHS=(1<<23),
      
	  SF_BOUND_FORCE=(1<<24),///<if this is true, the units are bound to the path and are destroyed if they reach the end
	  SF_BOUND_DEFAULT=(1<<25),///<They follow the path if nothing else is commanded
	  SF_BOUND_UNBOUND=(1<<26),///<They wait until commanded, but can be sent along paths
	  SF_BOUND_FORCEUNBOUND=(1<<27),///<They cannot be sent along paths

	  SF_BOUND_ANYWAY=(1<<28)///<They go any direction they like (G1_ENEMY/G1_ALLY have only internal meaning)
	  
	  };

typedef w32 solve_flags;
#define GRADE(sflags) (((sflags&SF_GRADEMASK)>>1)-1)


enum {//warning: must take care of intersections with G1_MAP* constants
  G1_RECALC_RADAR_VIEW         = (1<<0),
  G1_RECALC_PAD_LIST           = (1<<1),
  G1_RECALC_BLOCK_MAPS         = (1<<2), 
  G1_RECALC_CRITICAL_DATA      = (1<<3),
  G1_RECALC_WATER_VERTS        = (1<<4),  // determines which verts are drawn with wave motion
  G1_RECALC_STATIC_LIGHT       = (1<<5)  // determines how much directional light hits each cell
};

typedef i4_bool (*OBJ_FILTER_TYPE) (g1_object_class *obj);

/*! This class contains the data for the current map. 
There should currently only be one instance of it active.
The global instance can always be obtained by calling g1_get_map().
*/
class g1_map_class
{
private:
  friend class g1_map_view_class;
  friend class g1_editor_class;

  friend i4_bool g1_load_level(const i4_const_str &filename,
                               int reload_textures_and_models,
                               w32 exclude_flags);

  friend class g1_critical_map_maker_class;  
  friend class g1_object_controller_class;

  void save_objects(g1_saver_class *out);

  w32 recalc;       ///< bits telling what need recalculating set by add_undo in editor
  w32 w,h;

  g1_map_cell_class *cells;       ///< 2d array (w*h) of tile type, rotation, blocking status
  g1_map_vertex_class *verts;    ///< 2d array ((w+1)*(h+1)) of height and lighting info of corners
  
  i4_bool block_map_inited;
  g1_block_map_class block[G1_GRADE_LEVELS];    ///< blockage maps for different slopes
  g1_collision_map_class collide; ///< collision map for objects

  i4_array<g1_object_class *> map_objects; ///< used for collecting objects
  
  void delete_map_view();

  g1_object_class **load_objects(g1_loader_class *fp, w32 &tobjs);

  // returns sections actually loaded
  w32 load(g1_loader_class *fp, w32 sections);

  i4_bool load_sky(g1_loader_class *fp);
  void save_sky(g1_saver_class *fp);

  void save_critical_map(g1_saver_class *f);
  void load_critical_map(g1_loader_class *f);

  g1_astar_map_solver_class *astar_solver;
  g1_path_manager_class *path_manager;
  g1_critical_graph_class *critical_graph;
  g1_critical_map_maker_class map_maker;
  g1_breadth_first_map_solver_class *solvemap[G1_GRADE_LEVELS];
  g1_breadth_first_graph_solver_class *solvegraph;
  
  
  

  // this is a hook so the level editor can draw selected verts
  typedef void (*cell_draw_function_type)(sw32 x, sw32 y, void *context);
  cell_draw_function_type post_cell_draw;
  void *post_cell_draw_context;

  i4_str *filename;

  void init_lod();
  void calc_map_lod(g1_object_controller_class *);

  i4_bool movie_in_progress;

public:
  // only use this if you know what you are doing
  void change_map(int w, int h, g1_map_cell_class *cells, g1_map_vertex_class *vertex);
  
  //enum { THINK_QUE_SIZE=G1_MAX_OBJECTS };
  //g1_object_class *think_que[THINK_QUE_SIZE];
  //i4_array<g1_object_class *> think_que_dyn;
  //was formerly private, is now public to allow flexible changes
  //to the think que.
  //Be shure to know what you are doing and state clearly what you do!
  i4_dynamic_que<g1_object_class *,8096,8096> think_que_dyn;
  w32 think_head, think_tail;
  i4_str *sky_name;

  void recalc_static_stuff();//update the map data structures

  void remove_from_think_list(g1_object_class *o);//do not think any more

  void mark_for_recalc(w32 flags) { recalc |= flags; }//will recalc on next frame

  //gets block map for this grade
  g1_block_map_class *get_block_map(w8 grade) const { return const_cast<g1_block_map_class*>(&block[grade]); }

  //get astar solver (usefull for stank only)
  g1_astar_map_solver_class *get_astar_solver() {return astar_solver;}

  //usefull way solver
  g1_breadth_first_map_solver_class *get_breadth_solver(w8 grade)
	  {return solvemap[grade];};

  //faster way solver (uses precalculated data)
  g1_breadth_first_graph_solver_class *get_graph_solver()
	  {return solvegraph;};

  //get the map maker reference
  g1_critical_map_maker_class *get_map_maker(){return &map_maker;};

  //calculates the prefered solver for the given map (dependent on user settings)
  g1_map_solver_class *get_prefered_solver();

  //has this map a fast graph solving algo?
  i4_bool has_graph();

  //has it paths?
  i4_bool has_paths();
  solve_flags solvehint;

  i4_bool playing_movie() { return movie_in_progress; }  

  i4_const_str get_filename();
  void set_filename(const i4_const_str &fname);
    

  void set_post_cell_draw_function(cell_draw_function_type fun, void *context) 
  { 
    post_cell_draw=fun; 
    post_cell_draw_context=context;
  }


  i4_bool start_movie();
  void stop_movie();
  i4_bool advance_movie_with_time();

  g1_movie_flow_class *current_movie;

  g1_movie_flow_class *get_current_movie() { return current_movie; }

  w32 get_tick();
  i4_time_class tick_time;  

  w16 width()  const { return (w16)w; }
  w16 height() const { return (w16)h; }

  class range_iterator
  {
  protected:
    sw32 left, right, top, bottom, ix, iy;
    g1_map_cell_class *cell;
    g1_object_chain_class *chain;
    w32 object_mask_flags, type_mask_flags;
  public:
    void begin(float x, float y, float range);
    void mask(w32 _object_mask_flags, w32 _type_mask_flags=0xffffffff)
    {
      object_mask_flags = _object_mask_flags;
      type_mask_flags = _type_mask_flags;
    }
    void safe_restart();
    i4_bool end();
    void next();

    g1_object_class *get() const;

    g1_object_class *operator*() { return get(); }
    range_iterator& operator++() { next(); return *this;}
    range_iterator& operator++(int) { next(); return *this;}
  };

  //! Gets all objects that are within range of the given location.
  //! The objects are filtered with object_mask_flags and type_mask_flags.
  //! Be aware that the function might return some objects that are farther
  //! away than range specifies.
  sw32 get_objects_in_range(float x, float y, float range, 
                            g1_object_class *dest_array[], w32 array_size,
						    w32 object_mask_flags=0xffffffff, w32 type_mask_flags=0xffffffff);
  //! Gets objects in range of the given location satisfying an user definable condition.
  sw32 get_objects_in_range_fn(float x, float y, float range,
	  g1_object_class *dest_array[], w32 array_size,
	  OBJ_FILTER_TYPE fn);
  //!gets a pointer to the cell at a given location
  //g1_map_cell_class *cell(w16 x, w16 y) const { return cells + y*w + x; };
  template <typename T>
  g1_map_cell_class *cell(T x, T y) const { return cells + ((w32)y)*w + ((w32)x); }
  g1_map_cell_class *cell(w32 offset) const { return cells + offset; };
  g1_map_vertex_class *vertex(w16 x, w16 y) const { return verts + y*(w+1) + x; };

  //! Call this to receive the coordinates from an offset
  void cell_inv(w16 &x, w16 &y, w32 offset) const;

  /*void request_think(g1_object_class *obj)
  {
    think_que[think_head]=obj;

    think_head++;
    if (think_head>=THINK_QUE_SIZE)
      think_head=0;
    
    if (think_head==think_tail)
      i4_error("g1_map_class::request_think - thinkers exceeded maximum");
  }*/

  //! Requests that an object is added to the think list.
  //! Usually, you will have to call the same method for the object,
  //! not for the map.
  //! @see g1_object_class::request_think()
  void request_think(g1_object_class *obj)
	  {
	  think_que_dyn.que(obj);
	  think_head++;
	  }

  void request_remove(g1_object_class *obj);

  void add_object(g1_object_chain_class &c, w32 x, w32 y);
  void remove_object(g1_object_chain_class &c);


  void remove_object_type(g1_object_type type);

  //! The constructor for a new map.
  g1_map_class(const i4_const_str &fname);

  void make_block_maps();

  void update_block_maps(w16 x, w16 y,g1_object_class *changingobj, i4_bool added);

  void draw(g1_draw_context_class *context,
            i4_float player_x, 
            i4_float player_y,
            i4_float player_z,
            i4_angle player_angle);

  void draw_cells(g1_draw_context_class  *context,
                  g1_visible_cell *cell_list,
                  int t_visible_cells);


  void fast_draw_cells(g1_draw_context_class  *context);
//                        g1_visible_cell *cell_list,
//                        int t_visible_cells);


/*! Makes the objects think
* First calls the think methods of all classes derived from g2_singleton
* and then calls the think method of each object of the map that is still
* in the think queue. Calls the post_think methods afterwards.
*/
  void think_objects();


  /*! check so see if an object can move to the position x,y blocking is not checked
   against solids on the same team.
   returns 1 if hit object, -1 if hit building, and 0 if nothing
  */
  int check_non_player_collision(g1_player_type player_num,
                                 const i4_3d_vector &point,
                                 i4_3d_vector &ray,
                                 g1_object_class*& hit) const;
  
  int check_collision(i4_float x, i4_float y, 
								  i4_float occupancy_radius,
								  i4_float &dx, i4_float &dy,
								  sw32 _ix, sw32 _iy,
								  g1_object_class *_this,
								  g1_object_class*& hit) const;

  int check_poly_collision(i4_float x, i4_float y, 
	  i4_float occupancy_radius, i4_float &dx, i4_float &dy, 
	  g1_object_class *_this, g1_object_class *&hit);

  int check_terrain_location(i4_float x, i4_float y, i4_float z,
                             i4_float occupancy_radius,
                             w8 grade, w8 dir) const;

  /*! Saves the given sections of the map data to the given file
  \param out A g1_saver_class (a file pointer) where the data is written to. Must not be 0.
  \param sections The sections that should be written
  */
  void save(g1_saver_class *out, w32 sections);

  /*! This method is mainly used to check if the map contains still an object of the given type.
  \param object_id An objects type id.
  \param prefered_team A player id. If possible, the function returns an object that belongs
  to the given player.
  */
  g1_object_class *find_object_by_id(w32 object_id, g1_player_type prefered_team);

  // this is not stank specific!
  //i4_bool find_path(i4_float start_x, i4_float start_y,
  //                  i4_float dest_x, i4_float dest_y,
  //                  i4_float *points, w16 &t_nodes); 

  //! all objects in this area will receive damage falling off with distance
  void damage_range(g1_object_class *obj,
                    i4_float x, i4_float y, i4_float z, 
                    i4_float range, w16 damage, i4_float falloff=0);

  //! Returns the critical graph of this map (if any)
  g1_critical_graph_class *get_critical_graph() { return critical_graph; };

  /*! Returns the height above sea level in default units at the given location
  */
  i4_float terrain_height(i4_float x, i4_float y) const;

  //! Almost same as terrain_height(), but also checks for tunnels
  i4_float map_height(i4_float x, i4_float y, i4_float z) const;

  void calc_terrain_normal(i4_float x, i4_float y, i4_3d_vector &normal);
  void calc_pitch_and_roll(i4_float x, i4_float y, i4_float z, i4_float &pitch, i4_float &roll);

  void calc_height_pitch_roll(g1_object_class *for_obj, i4_float x, i4_float y, i4_float z,
                              i4_float &height, i4_float &pitch, i4_float &roll);
                              

  sw32 make_object_list(g1_object_class **buffer, sw32 buf_size);  // returns total entries

  //returns total added and a pointer to an array of pointers
  //the buffer will be valid till the next call or some remove_man.process_requests() 
  //or similar functions
  sw32 make_object_list(g1_object_class **&buffer, OBJ_FILTER_TYPE obj_filter);
  sw32 make_selected_objects_list(w32 *buffer, sw32 buf_size);  // return total added (saved id's)

  void change_vert_height(sw32 x, sw32 y, w8 new_height);

  // how much light illuminates an object at this position
  void get_illumination_light(i4_float x, i4_float y, i4_float &r, i4_float &g, i4_float &b);

  void reload();

  // returns the total number of cells that can be seen
  int calc_visible(i4_transform_class &t,
                   i4_polygon_class *area_poly,
                   g1_visible_cell *buffer, w32 buf_elements,
                   i4_float xscale, i4_float yscale);           // how much to scale x & y members of vertexes


  i4_float min_terrain_height(w16 x, w16 y);

  ~g1_map_class();
} ;


#endif


