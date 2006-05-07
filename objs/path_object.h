/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_PATH_OBJECT_HH
#define G1_PATH_OBJECT_HH

#include "g1_object.h"
#include "memory/array.h"
#include "global_id.h"
#include "transport/transport.h"

class g1_map_piece_class;

class g1_path_object_class : public g1_object_class
{
public:
  //enum { MAX_LINKS=8 };
  enum bridge_status_type { NOT_BRIDGABLE,
                            HAS_BRIDGE,
                            NO_BRIDGE };
  /*enum 
	  {
	  DEFAULT=0,
	  COMPLEX=1,
	  ARBITRARY=2,
	  NEWERUSE=4,
	  JUSTDEPLOY=8 //if the paths just indicate deploy points
	  }
  */


  bridge_status_type get_bridge_status();
  int bomb_warning_level();

  // vehicles should use this to determine which path to follow
  w32 last_selected_tick[G1_MAX_TEAMS];
  g1_path_object_class *next;  // maintains a list of path objects in the level
  
  class link_class
  {
  public:
    g1_id_ref object, path;
	//g1_path_object_class *first;//copy of next from above;
    g1_object_class *get_object() const { return object.get(); }
	g1_path_object_class *get_path();
	link_id link_desc;
  };
  i4_array<link_class> link;
  int link_index[G1_MAX_TEAMS+1];

  virtual void validate();
  virtual void save(g1_saver_class *fp);
  g1_path_object_class(g1_object_type id, g1_loader_class *fp);    
  virtual i4_bool occupy_location();
  virtual void unoccupy_location();

  virtual void draw(g1_draw_context_class *context, i4_3d_vector& viewer_position);
  virtual void editor_draw(g1_draw_context_class *context);
  virtual void think() {}  
  virtual i4_float occupancy_radius() const { return 0; }
  virtual li_object *message(li_symbol *message_name,
                             li_object *message_params, 
                             li_environment *env);

  int total_links(g1_team_type team);//how many links for team attached?
  int total_links();//ignore team association
  g1_path_object_class *get_link(g1_team_type team, int index) const
  { return link[link_index[team] + index].get_path(); }
  g1_path_object_class *get_link(int index) const
	  {
	  return link[index].get_path();
	  }
  link_id get_link_id(int index) const
	  {
	  return link[index].link_desc;
	  }
  link_id get_link_id(g1_team_type team, int index) const
	  {return link[link_index[team] + index].link_desc;}
  g1_object_class *get_object_link(g1_team_type team, int index) const
  { return link[link_index[team] + index].get_object(); }
  g1_object_class *get_object_link(int index) const
	  {
	  return link[index].get_object();
	  }

  link_id link_to(g1_team_type team, g1_path_object_class *obj);//returns the link id for
  //the link from here to there (one hop only)

  //this one only returns links for the given team
  g1_path_object_class *get_recent_link(g1_team_type team, g1_path_object_class *last_used);
  //this one needs the team only for the last_selected_tick[] array
  g1_path_object_class *get_recent_road(g1_team_type team, g1_path_object_class *last_used);
  
  int get_path_index(g1_team_type team, g1_path_object_class *obj) const;
  int get_path_index(g1_path_object_class *obj) const;
  int get_object_index(g1_object_class *obj) const;

  i4_bool remove_link(g1_team_type type, g1_path_object_class *o);
  void add_link(g1_team_type type, g1_path_object_class *o, link_id lid=0);

  virtual void request_remove();

  int total_controlled_objects();
  g1_object_class *get_controlled_object(int object_num);
  void add_controlled_object(g1_object_class *o);
  void remove_controlled_object(g1_object_class *o);

  static g1_path_object_class *cast(g1_object_class *obj)
  //{{{
  {
    if (!obj || !(obj->get_type()->get_flag(g1_object_definition_class::TO_PATH_OBJECT)))
      return 0; 
    return (g1_path_object_class *) obj;
  }
  //}}}

  // returns the total destinations found (banks & etc that are attached to the path)
  // uses "this" as the starting point
  int find_path_destinations(g1_object_class **list,
                             int list_size,               // max buffer size
                             g1_team_type type);             // ALLY or ENEMY

  virtual int find_path(g1_team_type type, g1_path_object_class *dest, 
                g1_path_object_class **stack, int stack_size);

  // this find path doesn't use a destination it follows most recently clicked nodes
  virtual int find_path(g1_team_type type, g1_path_object_class **stack, int stack_size);

  void change_player_num(int new_player_num); 
  
  g1_path_object_class* find_next(g1_team_type type, g1_path_object_class *dest);

  virtual bool repair(int how_much);
};




// this maintains a list of path_objects that are on the current_map
extern i4_isl_list<g1_path_object_class> g1_path_object_list;

void g1_calc_path_object_colors();

#endif

//{{{ Emacs Locals
// Local Variables:
// folded-file: t
// End:
//}}}
