/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "pch.h"
#include "window/window.h"
#include "window/colorwin.h"
#include "window/style.h"
#include "gui/text_input.h"
#include "gui/text.h"
#include "gui/list_pick.h"
#include "gui/button.h"
#include "gui/image_win.h"
#include "gui/smp_dial.h"
#include "image/image.h"
#include "loaders/load.h"
#include "app/app.h"
#include "lisp/lisp.h"
#include "lisp/li_init.h"
//#include "maxtool/sdk_inc/max.h"
#include "maxtool/m1_info.h"
#include "maxtool/max_object.h"
#include "menu/textitem.h"
#include "memory/new.h"



class m1_mount_window_class : public i4_color_window_class
{
private:
		sw16 list_ystart;
protected:
  typedef struct _mount_entry{
		w32 index;
		i4_const_str *caption;//the generated caption visible to the user
		i4_const_str *name;//the name part
		i4_3d_vector v;//the actual data
		_mount_entry():v(0,0,0){index=0;caption=0;name=0;};
		//~mount_entry(){if (caption) delete caption};
		}mount_entry;
  i4_array<mount_entry> mount_array;
  i4_array<i4_window_class *> items;//the references to the gui-objects.
  i4_window_class *msg;
  i4_list_pick *list;
  i4_graphical_style_class *style;
  w32 sel_elem;
public:
  void name(char* buffer) { static_name(buffer,"m1_mount_window_class"); }
  void add_item(int &h, int item)
  {
    
  }
  

  void update()
	  {
	  m1_poly_object_class *obj = m1_info.obj;
	if (!obj) return;
	mount_entry *m=0;
	sw16 sizex,sizey;
	
	if (msg) 
		{
		remove_child(msg);
		i4_kernel.delete_handler(msg);
		}
	msg=0;
	int i;
	for (i=0;i<mount_array.size();i++)
		{
		delete mount_array[i].caption;
		}
	if (list)//is 0 if just added first mount point
		{
		sizex=list->width();
		sizey=list->height();
		i4_kernel.delete_handler(list);//don't delete children, we will do.
		}
	else
		{
		sizex=width()-2;
		sizey=height()-list_ystart-2;
		}
	
	for (i=0;i<items.size();i++)
		{
		i4_kernel.delete_handler(items[i]);
		}
	list=0;
	mount_array.clear();
	items.clear();
	//i4_window_class *mw=0;
	if (obj->num_mounts==0)
		{
		msg=new i4_text_window_class("You've deleted all mounts",style);
		add_child(1,list_ystart,msg);
		}
	else
		{
		for(i=0;i<obj->num_mounts;i++)
			{
			m=mount_array.add();
			m->index=i;
			m->name=obj->mount_names[i];
			m->v.x=obj->mount_store[i].x;
			m->v.y=obj->mount_store[i].y;
			m->v.z=obj->mount_store[i].z;
			i4_const_str pattern("%i %S ( %f , %f , %f )");
			m->caption=pattern.sprintf(150,i,m->name,m->v.x,m->v.y,m->v.z);
			items.add(new i4_text_item_class(*m->caption,
                                      style, style->color_hint, style->font_hint->small_font,
                                      new i4_event_reaction_class(this, i)));
			
			
			}
		//avoid that the window becomes too large
		//if (x2>200) x2=200;
		list=new i4_list_pick(sizex,
			sizey,
			obj->num_mounts,&items[0],
			i4_list_pick::LB_SCROLLSELF,
			0x003090,i4_F);
		add_child(1,list_ystart,list);
		//list->request_redraw();
		}
	request_redraw();
	  }
  
  void init()
  //{{{
  {
    sw16 x1=0,y1=1,x2=20,y2=20;
	
	i4_image_class *add_icon=0;
	i4_image_window_class *icon=0;
	i4_button_class *add=0;

	add_icon=i4_load_image("bitmaps/editor/mountadd.bmp",0);
	icon=new i4_image_window_class(add_icon,i4_T,i4_T);
	//icon will get deleted on destructor of window
	add=new i4_button_class(&i4gets("mountpoint_add"),
		icon,
		style,new i4_event_reaction_class(this,0x80000001));
	add->set_popup(i4_T);
	add_child(x1,y1,add);
	x1+=add->width();

	add_icon=i4_load_image("bitmaps/editor/mountedt.bmp",0);
	icon=new i4_image_window_class(add_icon,i4_T,i4_T);
	//icon will get deleted on destructor of window
	add=new i4_button_class(&i4gets("mountpoint_edit"),
		icon,
		style,new i4_event_reaction_class(this,0x80000002));
	add->set_popup(i4_T);
	add_child(x1,y1,add);
	x1+=add->width();

	add_icon=i4_load_image("bitmaps/editor/mountdel.bmp",0);
	icon=new i4_image_window_class(add_icon,i4_T,i4_T);
	//icon will get deleted on destructor of window
	add=new i4_button_class(&i4gets("mountpoint_remove"),
		icon,
		style,new i4_event_reaction_class(this,0x80000003));
	add->set_popup(i4_T);
	add_child(x1,y1,add);
	x1+=add->width();

	y1=add->height()+2;//only required once
	list_ystart=y1;//save this for later use.
	m1_poly_object_class *obj = m1_info.obj;
	if (!obj) return;
	mount_entry *m=0;
	//i4_window_class *mw=0;
	if (obj->num_mounts==0)
		{
		msg=new i4_text_window_class("No mount points defined.",style);
		add_child(0,y1,msg);
		y2+=y1+msg->height()+40;
		x2=200;
		list=0;
		}
	else
		{
		for(int i=0;i<obj->num_mounts;i++)
			{
			m=mount_array.add();
			m->index=i;
			m->name=obj->mount_names[i];
			m->v.x=obj->mount_store[i].x;
			m->v.y=obj->mount_store[i].y;
			m->v.z=obj->mount_store[i].z;
			i4_const_str pattern("%i %S ( %f , %f , %f)");
			m->caption=pattern.sprintf(150,i,m->name,m->v.x,m->v.y,m->v.z);
			items.add(new i4_text_item_class(*m->caption,
                                      style, style->color_hint, style->font_hint->small_font,
                                      new i4_event_reaction_class(this, i)));
			
			y2+=items[i]->height();
			
			if (x2<items[i]->width())
				{
				x2=items[i]->width();
				}
			}
		//avoid that the window becomes too large
		//if (x2>200) x2=200;
		if (y2>200) y2=200;
		if (y2<100) y2=100;
		x2+=10;
		if (x2<200) x2=200;
		list=new i4_list_pick(x2,
			y2-y1,
			obj->num_mounts,&items[0],
			i4_list_pick::LB_SCROLLSELF,
			0x003090,i4_F);
		add_child(0,y1,list);
		}
	resize(x2,y2);
  }
  //}}}

  m1_mount_window_class(i4_graphical_style_class *style)
    : i4_color_window_class(400,200,style->color_hint->neutral(),style),
	mount_array(10,10),style(style),items(10,10)
  
  {
    msg=0;
	sel_elem=0;
    
    init();
  }

  ~m1_mount_window_class()
	  {
	  /*
	  if (msg)
		  {
		  msg->call_stack_counter++;
		  i4_kernel.delete_handler(msg);
		  msg->call_stack_counter--;
		  }
	  msg=0;*/
	  int i;
	  for (i=0;i<mount_array.size();i++)
		  {
		  //items[i]->call_stack_counter++;
		  i4_kernel.delete_handler(items[i]);//both arrays have same size
		  //items[i]->call_stack_counter--;
		  delete mount_array[i].caption;
		  }
	  mount_array.uninit();
	  items.uninit();
	  
	  //i4_color_window_class::~i4_color_window_class();
	  }

  virtual void receive_event(i4_event *ev)
  //{{{
  {
    switch (ev->type())
    {
		case i4_event::USER_MESSAGE:
			{
			CAST_PTR(uev, i4_user_message_event_class, ev);
			if ((w32)uev->sub_type<(w32)items.size())
				{
				((i4_text_item_class *)items[sel_elem])->bg_color=style->color_hint->neutral();
				items[sel_elem]->request_redraw();
				sel_elem=uev->sub_type;
				((i4_text_item_class *)items[sel_elem])->bg_color=style->color_hint->window.active.medium;
				request_redraw(i4_T);
				//User has clicked on an entry of the list-box
				}
			else if (uev->sub_type==0x80000001)//add button
				{
				if (li_call("add_mount")==li_nil) return;
				/*mount_entry *m;
				m=mount_array.add();
				m->index=mount_array.size()-1;
				m->name=m1_info.obj->mount_names[m->index];
				m->v=m1_info.obj->mount_store[m->index];
				i4_const_str pattern("%i %S (%f,%f,%f)");
				m->caption=pattern.sprintf(150,m->index,m->name,m->v[0],m->v[1],m->v[2]);
				items.add(new i4_text_item_class(*m->caption,
                                      style, style->color_hint, style->font_hint->small_font,
                                      new i4_event_reaction_class(this, m->index)));
				list->update(m->index,&items[0]);*/
				update();
			
				}
			else if (uev->sub_type==0x80000002)//edit button
				{
				if (mount_array.size()==0) return;
				m1_poly_object_class *obj=m1_info.obj;
				i4_str newname("newname");
				if (i4_input_box("Name","Mount Name",*obj->mount_names[sel_elem],
					newname,MSG_OK+MSG_CANCEL)==MSG_CANCEL) return;
				obj->change_mount_name(sel_elem,newname);
				//Display the 3 Elements of the vector in order
				i4_str strvectout("out");
				i4_const_str afl("( %f , %f , %f )");
				i4_3d_vector &v=obj->mount_store[sel_elem];
				i4_str *strvectin=afl.sprintf(100,v.x,
					v.y,v.z);
				
				if (i4_input_box("Vector","Enter components of vector",*strvectin,
					strvectout,MSG_OK+MSG_CANCEL)==MSG_CANCEL)
					{
					update();//the name might still have changed
					return;
					}
				char buf[200];
				i4_float x=0,y=0,z=0;
				i4_os_string(strvectout,buf,200);
				if (sscanf(buf,"( %f , %f , %f )",&x,&y,&z)<3)
					{
					i4_message_box("Invalid Input","You entered something that cannot be interpreted as a vector.",MSG_OK);
					return;
					}
				v.x=x;
				v.y=y;
				v.z=z;
				update();
				}
			else if (uev->sub_type==0x80000003)//remove button
				{
				//mount_entry *m;
				if (mount_array.size()==0) return;
				if (i4_message_box("Delete Mount","Are you shure you want to remove this mount point?",MSG_YESNO)==MSG_NO)
					return;
				m1_info.obj->remove_mount(sel_elem);
				sel_elem=0;
				update();
				//m=mount_array[sel_elem];

				}
			else
				i4_color_window_class::receive_event(ev);
			request_redraw();
			}


      default:
        i4_color_window_class::receive_event(ev);
        break;
    }
  }
  //}}}
};

static i4_event_handler_reference_class<m1_mount_window_class> m1_mount_dialog;

li_object *m1_show_mount(li_object *o, li_environment *env)
//{{{
{
  if (!m1_info.obj) return 0;
  if (m1_mount_dialog.get())
    m1_mount_dialog->init();
  else
  {
    i4_graphical_style_class *style = i4_current_app->get_style();
    m1_mount_dialog = new m1_mount_window_class(style);
    
    style->create_mp_window(-1,-1, 
                            m1_mount_dialog->width(),
                            m1_mount_dialog->height(),
                            i4gets("mount_window_title"))
      ->add_child(0,0,m1_mount_dialog.get());
  }
  return 0;
}
//}}}
/*li_object *m1_update_mount(li_object *o, li_environment *env)
	{//update window if it is visible
	if (m1_mount_dialog.get())
		m1_mount_dialog->init();
	return 0;
	}
*/
li_automatic_add_function(m1_show_mount, "m1_edit_mount_points");
//li_automatic_add_function(m1_update_special, "update_edit_mount_points");

//{{{ Emacs Locals
// Local Variables:
// folded-file: t
// End:
//}}}
