//This is a new file of the revival project. 
//It contains the new definitions for triggerable objects.

#include "g1_object.h"

class g1_trigger_class : public g1_object_class
{
public:
  //enum trigger_type { ANYONE, TEAM_MATES, ENEMY, SUPERTANKS, ENEMY_SUPERTANKS } ;
  //int type;
    enum teams {EVERYBODY, FRIEND, ENEMY};
    int team_type;
  
  // this matches the symbol name to our enum for faster use in C
  // we leave it in symbol for in the editor easy/understandable editing
protected:
  void get_trigger_type();
public:
  g1_trigger_class(g1_object_type id, g1_loader_class *fp);

  void object_changed_by_editor(g1_object_class *who, li_class *old_vars);
  
  
  // is the object in question one we should be triggered by?
  i4_bool triggable_object(g1_object_class *o);

  // send our trigger message to all the object our list 
  void send_to_trigger_objects(li_symbol *sym,g1_object_class *who);

  

protected:
   virtual void note_enter_range(g1_object_class *who);

   virtual void note_leave_range(g1_object_class *who);
   
public:
  void think() ;
  static g1_trigger_class* cast(g1_object_class *obj)
        {
        if (!obj || !(obj->get_type()->get_flag(g1_object_definition_class::TO_TRIGGER)))
		  return 0;
          return (g1_trigger_class*)obj;
        }

};

//A trigger that has two states
class g1_switch_class: public g1_trigger_class
    {
    public:
        g1_switch_class(g1_object_type id,
            g1_loader_class *fp);
        void save(g1_saver_class *fp);
        void load(g1_loader_class *fp);
        void skipload(g1_loader_class *fp);
        void note_enter_range(g1_object_class *who);
        void note_leave_range(g1_object_class *who);
        w32 get_selection_flags()
            {
            return SEL_ENEMYCANSENDCMD;
            }
    
    };

//A trigger that has two states and is activated manually.
class g1_toggable_switch_class: public g1_switch_class
    {
    public:
        g1_toggable_switch_class(g1_object_type id,
            g1_loader_class *fp);
        li_object* message(li_symbol *message_name, 
            li_object *message_params, li_environment *env);
        void note_enter_range(g1_object_class *who);
        void note_leave_range(g1_object_class *who);
    };

//A trigger that can send complex messages for lisp scripts. 
class g1_extended_trigger_class: public g1_trigger_class
    {
    public:
        g1_extended_trigger_class(g1_object_type id,
            g1_loader_class *fp);
    };

