//The header for a car object
//car objects are always managed by the roads!
#ifndef G2_CAR_OBJECT_INCLUDE
#define G2_CAR_OBJECT_INCLUDE
#include "objs/map_piece.h"

class g2_car_object_class:public g1_map_piece_class
	{
	friend class act_manager;
	public: 
		//g1_mini_object     *back_wheels;
		//g1_mini_object     *front_wheels;
		//w32 *route_quality;
		g1_id_ref *last_route;
		w32 route_len;
		w32 start_time;
		w32 required_arrival_time;
		w32 arrival_time;
		w32 car_id;
		i4_bool linked;
		link_id start_link, dest_link;
		w32 act_link_start_time;
	public:
		enum {CAR_DATA_VERSION=2};
		g2_car_object_class *next; //for i4_isl_list template
		enum {TIME_UNKNOWN=0xffffffff,
			TIME_NOTYET=0xfffffffe};
		g2_car_object_class(g1_object_type id, g1_loader_class *fp);
		void request_remove();
		void save(g1_saver_class *fp);
		void think();
		i4_bool occupy_location();
		void unoccupy_location();
		i4_bool can_attack(g1_object_class *who){return i4_F;}
		virtual void draw(g1_draw_context_class *context, i4_3d_vector& viewer_position);
		li_object *message(li_symbol *message_name,
                                      li_object *message_params, 
                                      li_environment *env);

		//the following starts the voyage of this vehicle
		//it returns false if car cannot start now (link full, engine failure...)
		i4_bool start(w32 current_time,link_id for_dest);
		i4_str *get_context_string();
		w32 calc_new_start_time(w32 olds,w32 reqarr, w32 actarr); 
		double calc_util(w32 ttrip,w32 tearly,w32 tlate);
		
		
	};

extern i4_isl_list<g1_path_object_class> g1_path_object_list;
extern i4_isl_list<g2_car_object_class> g2_car_object_list;
#endif
