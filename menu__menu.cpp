// MENU.CPP
/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "pch.h"
#include "device/event.h"
#include "device/keys.h"
#include "device/key_man.h"

#include "window/win_evt.h"
#include "window/style.h"
#include "image/image.h"
#include "loaders/load.h"

#include "menu/menu.h"
#include "menu/menuitem.h"
#include "menu/menuitem.h"
#include "menu/textitem.h"
#include "menu/image_item.h"
#include "menu/key_item.h"
#include "menu/boxmenu.h"
#include "menu/pull.h"

class i4_depress_menu_item : public i4_object_message_event_class
{
  public :

  i4_menu_item_class *item;
  
  i4_depress_menu_item(i4_menu_class *menu, 
                       i4_menu_item_class *item)
    : i4_object_message_event_class(menu),item(item) {}


  virtual i4_event  *copy() 
  { 
    return new i4_depress_menu_item((i4_menu_class *)object,item); 
  }

} ;

void i4_menu_class::add_item(i4_menu_item_class *item)
{
  item->set_menu_parent(this);
  i4_parent_window_class::add_child(0,0,item);
}

int i4_menu_class::get_sub_menu(const i4_const_str &cmd_name, i4_menu_item_class **items, int maxitems)
	{
	win_iter it=children.begin();
	//we assume that all our children are of type i4_menu_item_class, otherwise BANG!!!
	i4_menu_item_class *item;
	char buf[256];
	i4_os_string(cmd_name,buf,256);
	int ret=0;
	for (;it!=children.end();it++)
		{
		item=(i4_menu_item_class*)&(it.operator *());
		if (item->send.press)
			if (i4_event::DO_COMMAND==item->send.press->event->type())
				{
				i4_do_command_event_class *cmd=(i4_do_command_event_class*)
					item->send.press->event;
				if (strcmp(buf,cmd->command)==0 && ret<maxitems)
					{
					items[ret]=item;
					ret++;
					}
				}
			else
				{
				i4_warning("get_sub_menu(): Strange event type encountered.");
				}
		}
	return ret;
	}



void i4_menu_class::note_reaction_sent(i4_menu_item_class *who,       // this is who sent it
                                       i4_event_reaction_class *ev,   // who it was to 
                                       i4_menu_item_class::reaction_type type)
{
  // if the item was pressed, send a delayed event to ourself to depress it
  if (type==i4_menu_item_class::PRESSED)
  {
    i4_depress_menu_item dn(this,who);
    i4_kernel.send_event(this,&dn);
    
    if (hide_on_pick)
      hide();
  }
}

void i4_menu_class::receive_event(i4_event *ev)
{
  if (deleted)
      {
      i4_error("INTERNAL: i4_menu_class::receive_event() Getting events after death. ");
      return;
      }

  if (ev->type()==i4_event::OBJECT_MESSAGE)
  {
    CAST_PTR(r,i4_depress_menu_item,ev);
    if (r->object==this)
      r->item->do_depress();
    else i4_parent_window_class::receive_event(ev);
  }
  else
  {
    if (ev->type()==i4_event::WINDOW_MESSAGE && focus_inform)
    {
      CAST_PTR(wev,i4_window_message_class,ev);
      if (wev->sub_type==i4_window_message_class::LOST_MOUSE_FOCUS)
      {
        CAST_PTR(lost, i4_window_lost_mouse_focus_class, ev);

        i4_menu_focus_event_class mlost(this, 
                                       i4_menu_focus_event_class::lost_focus,
                                       lost->lost_to);

        i4_kernel.send_event(focus_inform, &mlost );
      }
      else if (wev->sub_type==i4_window_message_class::GOT_MOUSE_FOCUS)
      {
        i4_menu_focus_event_class got(this, 
                                      i4_menu_focus_event_class::got_focus,
                                      0);

        i4_kernel.send_event(focus_inform, &got );
      }        
    }
    i4_parent_window_class::receive_event(ev);
  }
}

i4_menu_class::~i4_menu_class()
{
  deleted=i4_T;
}

// MENUITEM.CPP
/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/


i4_menu_item_class::i4_menu_item_class(const i4_const_str *idle_context_help,
                                       i4_graphical_style_class *_hint,
                                       w16 w, w16 h,
                                       i4_event_reaction_class *press,   
                                       i4_event_reaction_class *depress,
                                       i4_event_reaction_class *activate,
                                       i4_event_reaction_class *deactivate
                                       ) :
  i4_parent_window_class(w,h),
  hint(_hint)//Warning: Compiler bug (or feature?) if object member
  //has same name as constructor parameter, this initialization works,
  //but any further reference will use the parameter value, throwing
  //its value away after the constructor ends! The compiler doesn't even
  //generate a warning on this.
{
  if (idle_context_help)
    context_help=new i4_str(*idle_context_help);
  else
    context_help=0;
  if (!hint) hint=i4_current_app->get_style();
  send.press=press;
  send.depress=depress;
  send.activate=activate;
  send.deactivate=deactivate; 
  menu_parent=0;

  active=i4_F;
  pressed=i4_F;
  disabled=i4_F;
}

void i4_menu_item_class::send_event(i4_event_reaction_class *ev, reaction_type type)
{
  if (menu_parent)
    menu_parent->note_reaction_sent(this, ev, type);

  if (ev && ev->event && ev->handler_reference.get())
    i4_kernel.send_event(ev->handler_reference.get(), ev->event);
}

void i4_menu_item_class::receive_event(i4_event *ev)
{
  i4_parent_window_class::receive_event(ev);

  if (!disabled)
  {
    if (ev->type()==i4_event::MOUSE_BUTTON_UP || ev->type()==i4_event::MOUSE_BUTTON_DOWN)
      if (context_help_window.get())
        i4_kernel.delete_handler(context_help_window.get());


    if (ev->type()==i4_event::WINDOW_MESSAGE)
    {
      CAST_PTR(w,i4_window_message_class,ev);
      switch (w->sub_type)
      {
        case i4_window_message_class::GOT_KEYBOARD_FOCUS :
        case i4_window_message_class::GOT_MOUSE_FOCUS :
        {          
          do_activate();
          send_event(send.activate, ACTIVATED);
        } break;
        case i4_window_message_class::LOST_KEYBOARD_FOCUS :
        case i4_window_message_class::LOST_MOUSE_FOCUS :
        {
          if (context_help_window.get())
          {
            i4_window_class *w=context_help_window.get();
			w->call_stack_counter++;
            //The Context-Help-Window must not be removed
			//here. (Causes GPF if Mouse is on top of it)
            i4_kernel.delete_handler(w);
			w->call_stack_counter--;
			context_help_window=0;
          }

          do_deactivate();
          send_event(send.deactivate, DEACTIVATED);
        } break;

      }
    }
    else if (ev->type()==i4_event::IDLE_MESSAGE) 
      do_idle();

  }
}


void i4_menu_item_class::do_idle()
{
  if (!context_help_window.get() && context_help)
    context_help_window=hint->create_quick_context_help(x(), y()+height()+5, *context_help);
}

void i4_menu_item_class::do_activate()
{
  if (!active)
  {
    active=i4_T;
    request_redraw();
  }
}

void i4_menu_item_class::do_deactivate()
{
  if (active)
  {
    active=i4_F;
    request_redraw();
  }
}

void i4_menu_item_class::do_disable()
{
  if (!disabled)
  {
    disabled=i4_T;
    request_redraw();
  }
}

void i4_menu_item_class::do_undisable()
{
  if (disabled)
  {
    disabled=i4_F;
    request_redraw();
  }
}

void i4_menu_item_class::do_press()
{
  if (!pressed)
  {
    pressed=i4_T;
    request_redraw();
  }
}

void i4_menu_item_class::do_depress()
{
  if (pressed)
  {
    pressed=i4_F;
    request_redraw();
  }
}


i4_menu_item_class::~i4_menu_item_class()
{
  if (context_help_window.get())
    i4_kernel.delete_handler(context_help_window.get());

  if (context_help)
    delete context_help;

  if (send.press)
    delete send.press;   

  if (send.depress)
    delete send.depress;

  if (send.activate)
    delete send.activate;

  if (send.deactivate)
    delete send.deactivate;
}


// PULL.CPP
/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

i4_pull_menu_class::i4_pull_menu_class(i4_graphical_style_class *style, 
                                       i4_parent_window_class *root_window)
  : i4_menu_class(i4_F),
    menu_colors(*style->color_hint),
    root(root_window),
    style(style)
{
  active_top=0;
  active_sub=0;
  sub_focused=i4_F;
  watching_mouse=i4_F;

  menu_colors.selected_text_foreground=i4_read_color_from_resource("selected_menu_foreground");
  menu_colors.selected_text_background=i4_read_color_from_resource("selected_menu_background");
}

void i4_pull_menu_class::watch()
{
  if (!watching_mouse)
  {
    watching_mouse=i4_T;
    i4_kernel.request_events(this,     
                             i4_device_class::FLAG_KEY_PRESS |
                             i4_device_class::FLAG_KEY_RELEASE |
                             i4_device_class::FLAG_MOUSE_BUTTON_DOWN |
                             i4_device_class::FLAG_MOUSE_BUTTON_UP);

  }
}

void i4_pull_menu_class::unwatch()
{
  if (watching_mouse)
  {
    i4_kernel.unrequest_events(this,
                               i4_device_class::FLAG_KEY_PRESS |
                               i4_device_class::FLAG_KEY_RELEASE |
                               i4_device_class::FLAG_MOUSE_BUTTON_DOWN |
                               i4_device_class::FLAG_MOUSE_BUTTON_UP);

    watching_mouse=i4_F;
  }
}

i4_pull_menu_class::~i4_pull_menu_class()
{
  unwatch();
  hide_active();

  sub_menus.destroy_all();
}

void i4_pull_menu_class::remove_all_menus()
	{
	hide_active();
	sub_menus.destroy_all();
	}

void i4_pull_menu_class::show_active(i4_menu_item_class *who)
{
  win_iter name=children.begin(), sub=sub_menus.begin(), last=sub_menus.end();
  while (who!=&*name)
  {
    last=sub;
    ++name;
    ++sub;
  }

  sw32 px,py;

  py=who->y()+who->height();
  px=who->x();

  i4_menu_class *m=((i4_menu_class *)(&*sub));

  if (last==sub_menus.end())
    sub_menus.erase();
  else
    sub_menus.erase_after(last);


  m->show(root, px-root->x(), py-root->y());

  if (sub->height()+sub->y()>root->height())
  {
    m->hide();
    py=who->y()-sub->height();
    m->show(root, px-root->x(), py-root->y());
  }

   
  if (sub->width()+sub->x()>root->width())
  {
    m->hide();
    px=root->width()-sub->width();
    m->show(root, px-root->x(), py-root->y());
  }    

  active_top=who;
  active_sub=m;
  watch();
}

void i4_pull_menu_class::note_reaction_sent(i4_menu_item_class *who,       // this is who sent it
                                            i4_event_reaction_class *ev,   // who it was to 
                                            i4_menu_item_class::reaction_type type)
{
  i4_menu_class::note_reaction_sent(who, ev, type);

  if (type==i4_menu_item_class::PRESSED)
  {
    who->do_depress();
    if (!active_sub)
      show_active(who);
  } else if (type==i4_menu_item_class::ACTIVATED)
  {
    if (active_sub)
    {
      hide_active();
      show_active(who);
    }
      
    active_top=who;
  } else if (type==i4_menu_item_class::DEACTIVATED)
  {
    if (active_sub)
      who->do_activate();
  }
}

void i4_pull_menu_class::hide_active()
{
  if (active_sub)
  {
    active_sub->hide();

    if ((i4_window_class *)active_top==&*children.begin())
      sub_menus.insert(*active_sub);
    else
    {
      win_iter s=sub_menus.begin(), t=children.begin();
      ++t;
      for (;(*&t)!=active_top;)
      {
        ++s;
        ++t;
      }
      sub_menus.insert_after(s, *active_sub);
    }
    unwatch();
  }

  if (active_top)
    active_top->do_deactivate();

  active_top=0;
  active_sub=0;
}

void i4_pull_menu_class::receive_event(i4_event *ev)
{
  if (ev->type()==i4_event::OBJECT_MESSAGE)
  {
    CAST_PTR(lev, i4_menu_focus_event_class, ev);

    if (lev->focus_state==i4_menu_focus_event_class::lost_focus)
      sub_focused=i4_F;
    else
      sub_focused=i4_T;
  } 
  else if (ev->type()==i4_event::WINDOW_MESSAGE)
  {
    CAST_PTR(wev, i4_window_message_class, ev);
    if (wev->sub_type==i4_window_message_class::LOST_MOUSE_FOCUS)
      focused=i4_F;
    else if (wev->sub_type==i4_window_message_class::GOT_MOUSE_FOCUS)
      focused=i4_T;      
    
    i4_menu_class::receive_event(ev);
  }
  else if (ev->type()==i4_event::MOUSE_BUTTON_DOWN ||
           ev->type()==i4_event::MOUSE_BUTTON_UP)
  {
    if (!focused && !sub_focused)
      hide_active();

    if (focused)
      i4_menu_class::receive_event(ev);
  }
  else if (ev->type()==i4_event::KEY_PRESS)
  {
    CAST_PTR(kev, i4_key_press_event_class, ev);
    if (kev->key_code==I4_ESC)
      hide_active();
    else
      i4_menu_class::receive_event(ev);
  }  
  else if (focused)
    i4_menu_class::receive_event(ev);
}


void i4_pull_menu_class::parent_draw(i4_draw_context_class &context)
{
  i4_rect_list_class child_clip(&context.clip,0,0);
  child_clip.intersect_list(&undrawn_area);

  child_clip.swap(&context.clip);

  style->deco_neutral_fill(local_image, 0,0, width()-1, height()-1, context);


  //  local_image->clear(menu_colors.text_background,context);

  child_clip.swap(&context.clip);
  i4_parent_window_class::parent_draw(context);
}

void i4_pull_menu_class::add_sub_menu(i4_menu_item_class *name,
                                      i4_menu_class *sub_menu)
{
  add_item(name);
  name->set_menu_parent(this);
  sub_menu->notify_focus_change(this);

  sub_menus.insert_end(*sub_menu);

  reparent(local_image, parent);
}

int i4_pull_menu_class::get_sub_menu(const i4_const_str &cmd_name, i4_menu_item_class **items, int maxitems)
	{
	i4_isl_list<i4_window_class>::iterator it=sub_menus.begin();
	i4_menu_item_class *current;
	i4_menu_class *sub_menu;
	int ret1=0,ret2=0;
	for (;it!=sub_menus.end();it++)
		{
		sub_menu=(i4_menu_class*) &*it;
		//we assume that one submenu has only one entry with a given command
		ret1=sub_menu->get_sub_menu(cmd_name,&current,1);		
		if (ret1==1 && ret2<maxitems)
			{
			items[ret2]=current;
			ret2++;
			}
		}
	return ret2;
	};


void i4_pull_menu_class::reparent(i4_image_class *draw_area, i4_parent_window_class *parent)
{
  i4_parent_window_class::reparent(draw_area, parent);
  if (parent)
  {
    resize(parent->width(), 0);
    arrange_right_down();

    w32 max_h=0;
    for (win_iter c=children.begin(); c!=children.end(); ++c)
      if (c->y()-y()+c->height()>(sw32)max_h)
        max_h=c->y()-y()+c->height();

    resize(parent->width(), (w16)max_h);
  }
}

void i4_pull_menu_class::resize(w16 new_width, w16 new_height)
{
  i4_window_class::resize(new_width, new_height);
  arrange_right_down();
}



void i4_pull_menu_class::hide()
{
  if (parent)     
  {
    parent->remove_child(this);
    hide_active();
  }
  parent=0;
}

// TEXTITEM.CPP
/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/


void i4_text_item_class::change_text(const i4_const_str &new_st)
{
  delete text;
  text=new i4_str(new_st, (w16)new_st.length()+1);
  request_redraw(i4_F);
}

i4_menu_item_class *i4_text_item_class::copy() 
{
  return new i4_text_item_class(*text, hint, color, font, 
                                send.press ? send.press->copy() : 0,
                                send.depress ? send.depress->copy() : 0,
                                send.activate ? send.activate->copy() : 0,
                                send.deactivate ? send.deactivate->copy() : 0);
}


i4_text_item_class::i4_text_item_class(const i4_const_str &_text,
                                       i4_graphical_style_class *style,
                                       i4_color_hint_class *_color,
                                       i4_font_class *_font,
                                       i4_event_reaction_class *press,
                                       i4_event_reaction_class *depress,
                                       i4_event_reaction_class *activate,
                                       i4_event_reaction_class *deactivate,
                                       w16 pad_left_right,
                                       w16 pad_up_down)
      
  : i4_menu_item_class(0, style, 0,0, press,depress,activate,deactivate),
    color(_color),
    font(_font),
    text(new i4_str(_text,(w16)_text.length()+1)),
    pad_lr(pad_left_right)
{
  if (!color) color=style->color_hint;
  if (!font)  font=style->font_hint->normal_font;

  bg_color=style->color_hint->neutral();

  resize(font->width(_text)+pad_left_right*2, font->height(_text)+pad_up_down*2);
  
  // show the keyboard short cut for commands
  if (press && press->event && press->event->type()==i4_event::DO_COMMAND)
  {
    CAST_PTR(dev, i4_do_command_event_class, press->event);

    i4_key key;
    w16 mod;

    if (i4_key_man.get_key_for_command(dev->command_id, key, mod))
    {      
      i4_str *key_name=i4_key_name(key, mod);
      private_resize(width() + font->width(*key_name)+5, height());
      delete key_name;
    }
  }


}


void i4_text_item_class::parent_draw(i4_draw_context_class &context)
{
  local_image->add_dirty(0,0,width()-1,height()-1,context);

  i4_color fg,bg;

  if (active)
  {
    fg=color->selected_text_foreground;
    bg=color->selected_text_background;
  }
  else
  {
    fg=color->text_foreground;
    bg=color->text_background;
  }

  if (disabled)
	  {
	  fg=0x666666;//gray, this should always be ok
	  bg=color->text_background;
	  }

  if (!active)
  {
    if (bg_color==color->neutral())
    {
      for (i4_rect_list_class::area_iter c=context.clip.list.begin();c!=context.clip.list.end();++c)
        hint->deco_neutral_fill(local_image, c->x1, c->y1, c->x2, c->y2, context);
    }
    else local_image->clear(bg_color, context);

  }
  else
    local_image->clear(bg, context);

  font->set_color(fg);

  int dy=height()/2-font->largest_height()/2;
  font->put_string(local_image,
                   pad_lr,
                   dy,
                   *text,context);  
  

  // draw key name if there is one
  if (send.press && send.press->event && send.press->event->type()==i4_event::DO_COMMAND)
  {
    CAST_PTR(dev, i4_do_command_event_class, send.press->event);

    i4_key key;
    w16 mod;

    if (i4_key_man.get_key_for_command(dev->command_id, key, mod))
    {      
      i4_str *key_name=i4_key_name(key, mod);
      font->put_string(local_image, width() - pad_lr - 
                       font->width(*key_name) + 1, 
                       dy, *key_name, context);
	  delete key_name;
    }
  }


}


void i4_text_item_class::receive_event(i4_event *ev)
{
  //if this entry is disabled, NOTHING shall happen at all
  //This also includes that the menu should not go away.
  if (disabled)
	  return;
  if (ev->type()==i4_event::MOUSE_BUTTON_DOWN)
  {
    CAST_PTR(b,i4_mouse_button_down_event_class,ev);
    if (b->but==i4_mouse_button_down_event_class::LEFT)
    {        
      do_press();
      send_event(send.press, PRESSED);

      do_depress();
      send_event(send.depress, DEPRESSED);

    } else i4_menu_item_class::receive_event(ev);
  } else if (ev->type()==i4_event::KEY_PRESS)
  {
    CAST_PTR(k,i4_key_press_event_class,ev);
    if (k->key==I4_ENTER)
    {
      do_press();
      send_event(send.press, PRESSED);

      do_depress();
      send_event(send.depress, DEPRESSED);
    } else if (k->key==I4_TAB)
    {
      i4_window_message_class tab(i4_window_message_class::REQUEST_NEXT_KEY_FOCUS,this);
      i4_kernel.send_event(parent, &tab);      
    }
    else if (k->key==I4_LEFT)
    {
      i4_window_message_class l(i4_window_message_class::REQUEST_LEFT_KEY_FOCUS,this);
      i4_kernel.send_event(parent, &l);
    }
    else if (k->key==I4_RIGHT)
    {
      i4_window_message_class r(i4_window_message_class::REQUEST_RIGHT_KEY_FOCUS,this);
      i4_kernel.send_event(parent, &r);
    }
    else if (k->key==I4_UP)
    {
      i4_window_message_class u(i4_window_message_class::REQUEST_UP_KEY_FOCUS,this);
      i4_kernel.send_event(parent, &u);
    }
    else if (k->key==I4_DOWN)
    {
      i4_window_message_class d(i4_window_message_class::REQUEST_DOWN_KEY_FOCUS,this);
      i4_kernel.send_event(parent, &d);
    }
    else i4_menu_item_class::receive_event(ev);
  } else 
    i4_menu_item_class::receive_event(ev);
}

i4_checkbox_images_class checkbox_images_instance;

void i4_checkbox_images_class::init()
	{
	maxwidth=0;
	maxheight=0;
	checkbox=i4_load_image("bitmaps/editor/checkbox.bmp",0);
	radiobox=i4_load_image("bitmaps/editor/radiobox.bmp",0);
	if (!checkbox||!radiobox)
		{
		i4_error("FATAL: GUI: Could not load checkbox.bmp or radiobox.bmp.");
		return;
		}
	maxwidth=checkbox->height();//images are 3*height quadratic
	maxheight=checkbox->height();
	}

void i4_checkbox_images_class::uninit()
	{
	delete checkbox;
	checkbox=0;
	delete radiobox;
	radiobox=0;
	}

i4_checkbox_class::i4_checkbox_class(
			   const i4_const_str &_text,      
			   w32 _flags,
               i4_graphical_style_class *style,
         
               i4_color_hint_class *color_hint,
               i4_font_class *font,
         
               i4_event_reaction_class *press,
               i4_event_reaction_class *depress,
               i4_event_reaction_class *activate,
               i4_event_reaction_class *deactivate,
               w16 pad_left_right,
               w16 pad_up_down
			   )
			   :flags(_flags),
			   state(0),
			   i4_text_item_class(_text,
			   style,
			   color_hint,
			   font,
			   press,
			   depress,
			   activate,
			   deactivate,
			   //(_flags&TYPE_MASK==0)?pad_left_right:pad_left_right+10,
			   pad_left_right,
			   pad_up_down)
		{
		images = &checkbox_images_instance;
		if ((flags&TYPE_MASK)==CHECKBOX||(flags&TYPE_MASK)==RADIOBUTTON)
			{
			resize(width()+images->maxwidth,(height()>images->maxheight)?height():images->maxheight);
			}
		
		}

void i4_checkbox_class::parent_draw(i4_draw_context_class &context)
	{
	if (!images->checkbox) return; //can only happen if system is just beeing shut down.
	local_image->add_dirty(0,0,width()-1,height()-1,context);
	
	i4_color fg,bg;
	if ((flags&TYPE_MASK)==FONT_TOGGLE)
		{
		if (active)//Mark line blue
			{
			fg=color->selected_text_foreground;
			bg=color->selected_text_background;
			}
		else
			{
			fg=color->text_foreground;
			bg=color->text_background;
			}
		
		if (!active)
			{
			if (bg_color==color->neutral())
				{
				for (i4_rect_list_class::area_iter c=context.clip.list.begin();c!=context.clip.list.end();++c)
					hint->deco_neutral_fill(local_image, c->x1, c->y1, c->x2, c->y2, context);
				}
			else local_image->clear(bg_color, context);
			
			}
		else
			local_image->clear(bg, context);
		}
	else//not FONT_TOGGLE
		{
		hint->deco_neutral_fill(local_image,0,0,local_image->width(),local_image->height(),context);
		fg=color->selected_text_foreground;
		bg=color->selected_text_background;
		}
	font->set_color(fg);
	
	int dy=height()/2-font->largest_height()/2;
	short imw=(short)images->maxwidth;
	if ((flags&TYPE_MASK)==CHECKBOX)
		{
		images->checkbox->put_part(local_image,0,0,(short)imw*state,0,(short)(imw*state)+imw,imw,context);
		}
	else if ((flags&TYPE_MASK)==RADIOBUTTON)
		{
		images->radiobox->put_part(local_image,0,0,(short)imw*state,0,(short)(imw*state)+imw,imw,context);
		}
	else if ((flags&TYPE_MASK)==FONT_TOGGLE)
		{
		font->put_string(local_image,//This should print somewhat in bold letters
			pad_lr+2,
			dy,*text,context);
		}
	font->put_string(local_image,
		pad_lr+imw,
		dy,
		*text,context);  
	
	
	// draw key name if there is one
	if (send.press && send.press->event && send.press->event->type()==i4_event::DO_COMMAND)
		{
		CAST_PTR(dev, i4_do_command_event_class, send.press->event);
		
		i4_key key;
		w16 mod;
		
		if (i4_key_man.get_key_for_command(dev->command_id, key, mod))
			{      
			i4_str *key_name=i4_key_name(key, mod);
			font->put_string(local_image, width() - pad_lr - 
				font->width(*key_name) + 1, 
				dy, *key_name, context);
			delete key_name;
			}
		}
	}

void i4_checkbox_class::toggle_state()
	{
	if (flags&THREESTATES)
		{
		state=(state+1)%3;
		}
	else
		{
		state=(state+1)%2;
		}
	request_redraw();
	}

void i4_checkbox_class::receive_event(i4_event *ev)
	{
	if (ev->type()==i4_event::MOUSE_BUTTON_DOWN)
		{
		CAST_PTR(b,i4_mouse_button_down_event_class,ev);
	    if (b->but==i4_mouse_button_down_event_class::LEFT)
		{        
		//do_press();
		//send_event(send.press, PRESSED);
		toggle_state();
		//do_depress();
		//send_event(send.depress, DEPRESSED);

		}
		}
	if (ev->type()==i4_event::KEY_PRESS)
		{
		CAST_PTR(k,i4_key_press_event_class,ev);
		if (k->key==I4_SPACE)
			{
			do_press();
			send_event(send.press, PRESSED);
			toggle_state();
			do_depress();
			send_event(send.depress, DEPRESSED);
			}
		}
	i4_text_item_class::receive_event(ev);
	};


// IMAGE_ITEM.CPP
/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/


i4_image_item_class::i4_image_item_class(const i4_const_str *context_help,
                                         i4_image_class *normal_image,
                                         i4_graphical_style_class *style,
                                         i4_image_class *active_image,     // if 0, then image will brighten
                                         i4_bool delete_images_on_death,
                                         i4_event_reaction_class *press,
                                         i4_event_reaction_class *depress,
                                         i4_event_reaction_class *activate,
                                         i4_event_reaction_class *deactivate)

  : i4_menu_item_class(context_help, style, normal_image->width(), 
                       normal_image->height(), press,depress,activate,deactivate)
{
  I4_ASSERT(normal_image, "no normal");

  im=normal_image;

  del_im=delete_images_on_death;

  if (active_image)
  {
    act=active_image;
    del_act=delete_images_on_death;
  }
  else
  {
    del_act=i4_T;



    int w=im->width(), h=im->height();
    act=i4_create_image(w,h, im->pal);


    for (int y=0; y<h; y++)
      for (int x=0; x<w; x++)
      {
        w32 c=im->get_pixel(x,y);
        int r=((c&0xff0000)>>16)<<1;  if (r>255) r=255;
        int g=((c&0xff00)>>8)<<1;    if (g>255) g=255;
        int b=((c&0xff)>>0)<<1;      if (b>255) b=255;
        
        act->put_pixel(x,y, (r<<16)|(g<<8)|b);
      }



  }
  
  
}


i4_image_item_class::~i4_image_item_class()
{
  if (del_im)
    delete im;
  
  if (del_act)
    delete act;
}

void i4_image_item_class::parent_draw(i4_draw_context_class &context)
{
  if (active)
    act->put_image(local_image,0,0,context);
  else 
    im->put_image(local_image,0,0,context);

}

void i4_image_item_class::receive_event(i4_event *ev)
{
  if (ev->type()==i4_event::MOUSE_BUTTON_DOWN)
  {
    CAST_PTR(b,i4_mouse_button_down_event_class,ev);
    if (b->but==i4_mouse_button_down_event_class::LEFT)
    {        
      do_press();
      send_event(send.press, PRESSED);

      do_depress();
      send_event(send.depress, DEPRESSED);

    } else i4_menu_item_class::receive_event(ev);
  }
  else i4_menu_item_class::receive_event(ev);
}


i4_menu_item_class *i4_image_item_class::copy()
{
  return new i4_image_item_class(context_help,
                                 im->copy(), 
                                 hint,
                                 act->copy(), 
                                 i4_T,
                                 send.press ? send.press->copy() : 0,
                                 send.depress ? send.depress->copy() : 0,
                                 send.activate ? send.activate->copy() : 0,
                                 send.deactivate ? send.deactivate->copy() : 0);
}
// KEY_ITEM
/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/


static i4_key_accel_watcher_class i4_key_accel_watcher_instance;

enum { key_space=5 };

i4_key_item_class::~i4_key_item_class()
{
  delete text;
  if (use_key!=I4_NO_KEY)
    i4_key_accel_watcher_instance.unwatch_key(this, use_key, key_modifiers);
}

i4_key_item_class::i4_key_item_class(const i4_const_str &_text,
                                     i4_color_hint_class *color_hint,
                                     i4_font_hint_class *font_hint,
                                     i4_graphical_style_class *style,
                                     w16 key,
                                     w16 key_modifiers,
                                     w16 pad_left_right,
                                     w16 pad_up_down
                                     )
  : i4_menu_item_class(0,
                       style,
                       font_hint->normal_font->width(_text)+pad_left_right*2,
                       font_hint->normal_font->height(_text)+pad_up_down*2,
                       0,0,0,0),     
    key_modifiers(key_modifiers),
    color(color_hint),
    font(font_hint),
    text(new i4_str(_text,(w16)_text.length()+1)),
    pad_lr(pad_left_right),
    pad_ud(pad_up_down),
    use_key(key)
{
  key_focused=i4_F;
  valid=i4_T;
  
  w32 add_width=0;
  i4_font_class *fnt=font_hint->normal_font;

  if (key!=I4_NO_KEY)
  {
    i4_key_accel_watcher_instance.watch_key(this, key, key_modifiers);
    i4_str *key_name=i4_key_name(key, key_modifiers);
    add_width=fnt->width(*key_name)+key_space;
    delete key_name;
  }

  resize((w16)(fnt->width(_text) + add_width + pad_left_right*2),
         (w16)(fnt->height(_text) + pad_up_down*2));
}

void i4_key_item_class::receive_event(i4_event *ev)
{
  if (ev->type()==i4_event::MOUSE_BUTTON_DOWN)
  {
    CAST_PTR(b,i4_mouse_button_down_event_class,ev);
    if (b->but==i4_mouse_button_down_event_class::LEFT)
    {
      do_depress();
      send_event(send.depress, PRESSED);
      action();
    }
    else i4_menu_item_class::receive_event(ev);

  } else if (ev->type()==i4_event::KEY_PRESS)
  {
    CAST_PTR(k,i4_key_press_event_class,ev);

    if (k->modifiers & I4_MODIFIER_CTRL)
      k->modifiers |= I4_MODIFIER_CTRL;
    if (k->modifiers & I4_MODIFIER_ALT)
      k->modifiers |= I4_MODIFIER_ALT;
    if (k->modifiers & I4_MODIFIER_SHIFT)
      k->modifiers |= I4_MODIFIER_SHIFT;
      
    if (k->key_code==use_key && k->modifiers==key_modifiers)
      action();
    else if (k->key==I4_ENTER)
    {
      do_depress();
      send_event(send.depress, PRESSED);
      action();
    }
    else if (k->key==I4_TAB)
    {
      i4_window_message_class nf(i4_window_message_class::REQUEST_NEXT_KEY_FOCUS,this);
      i4_kernel.send_event(parent, &nf);
    }
    else if (k->key==I4_LEFT)
    {
      i4_window_message_class l(i4_window_message_class::REQUEST_LEFT_KEY_FOCUS,this);
      i4_kernel.send_event(parent, &l);
    }
    else if (k->key==I4_RIGHT)
    {
      i4_window_message_class r(i4_window_message_class::REQUEST_RIGHT_KEY_FOCUS,this);
      i4_kernel.send_event(parent, &r);
    }
    else if (k->key==I4_UP)
    {
      i4_window_message_class u(i4_window_message_class::REQUEST_UP_KEY_FOCUS,this);
      i4_kernel.send_event(parent, &u);
    }
    else if (k->key==I4_DOWN)
    {
      i4_window_message_class d(i4_window_message_class::REQUEST_DOWN_KEY_FOCUS,this);
      i4_kernel.send_event(parent, &d);
    }
    else 
      i4_menu_item_class::receive_event(ev);
  } 
  else if (ev->type()==i4_event::WINDOW_MESSAGE)
  {
    CAST_PTR(wev, i4_window_message_class, ev);
    if (wev->sub_type==i4_window_message_class::GOT_KEYBOARD_FOCUS)
      key_focused=i4_T;
    else if (wev->sub_type==i4_window_message_class::LOST_KEYBOARD_FOCUS)
      key_focused=i4_F;
    else if (wev->sub_type==i4_window_message_class::NOTIFY_RESIZE)
    {
      CAST_PTR(rev, i4_window_notify_resize_class, ev);
      if (rev->from()==parent)
      {
        w32 nw=rev->new_width-(x()-parent->x())*2;
        if (nw!=width())
          resize((w16)nw, height());
      }
    }

    i4_menu_item_class::receive_event(ev);
  }
  else 
    i4_menu_item_class::receive_event(ev);
}


i4_key_accel_watcher_class::i4_key_accel_watcher_class()
{
  initialized=i4_F;
  memset(user,0,sizeof(user));
  total=0;
}


i4_key_item_class **i4_key_accel_watcher_class::
   key_item_pointer_type::get_from_modifiers(w16 modifiers)
{
  int index=0;

  if (modifiers & I4_MODIFIER_SHIFT)
    index |= 1;

  if (modifiers & I4_MODIFIER_CTRL)
    index |= 2;
    
  if (modifiers & I4_MODIFIER_ALT)
    index |= 3;

  return &modkey[index];
}

void i4_key_accel_watcher_class::watch_key(i4_key_item_class *who, w16 key, w16 modifiers)
{
  if (!initialized)
  {
    i4_kernel.request_events(this,  
                             i4_device_class::FLAG_KEY_PRESS |
                             i4_device_class::FLAG_KEY_RELEASE);
    initialized=i4_T;
  }

  I4_TEST(key<I4_NUM_KEYS, "Key out of range");
  if (*user[key].get_from_modifiers(modifiers))
  {
    char name[80];
    i4_warning("key alread has function [%s]\n", i4_get_key_name(key,modifiers,name));
  }
  else total++;

  *user[key].get_from_modifiers(modifiers)=who;
}

void i4_key_accel_watcher_class::unwatch_key(i4_key_item_class *who, w16 key, w16 modifiers)
{
  if (*user[key].get_from_modifiers(modifiers)==who)
  {
    *user[key].get_from_modifiers(modifiers)=0;
    total--;
    if (total==0)
    {      
      i4_kernel.unrequest_events(this,  
                                 i4_device_class::FLAG_KEY_PRESS |
                                 i4_device_class::FLAG_KEY_RELEASE);
      initialized=i4_F;
    }
  }
}



void i4_key_accel_watcher_class::receive_event(i4_event *ev)
{
  if (ev->type()==i4_event::KEY_PRESS)
  {
    CAST_PTR(kev, i4_key_press_event_class, ev);

    // if the object has the keyboard focus, then it will get the key through
    // normal window channels
    i4_key_item_class *i=*(user[kev->key_code].get_from_modifiers(kev->modifiers));
    if (i && !i->has_keyboard_focus())      
      i4_kernel.send_event(i, ev);
  }
}



void i4_key_item_class::parent_draw(i4_draw_context_class &context)
{
  local_image->add_dirty(0,0,width()-1,height()-1,context);

  i4_color fg,bg;
  

  if (active)
  {
    fg=color->selected_text_foreground;
    bg=color->selected_text_background;
  }
  else
  {
    fg=color->text_foreground;
    bg=color->text_background;
  }

  i4_str *key_name;

  if (use_key!=I4_NO_KEY)
    key_name=i4_key_name(use_key, key_modifiers);
  else key_name=0;

  if (!active)
    hint->deco_neutral_fill(local_image, 0,0, width()-1, height()-1, context);
  else
    local_image->clear(bg, context);

  i4_font_class *fnt=font->normal_font;

  if (!valid)
  {
    fg=color->window.passive.dark;

    fnt->set_color(color->window.passive.bright);
    fnt->put_string(local_image, pad_lr+1, pad_ud+1, *text,context);

    if (key_name)
      fnt->put_string(local_image, width() - pad_lr - fnt->width(*key_name) + 1, 
                      pad_ud + 1,
                      *key_name, context);

  }

  if (valid || !active)
  {
    font->normal_font->set_color(fg);
    font->normal_font->put_string(local_image, pad_lr, pad_ud, *text,context);
    if (key_name)
      fnt->put_string(local_image, width() - pad_lr - fnt->width(*key_name), 
                      pad_ud, *key_name, context);

  }

  if (key_name)
    delete key_name;
}



// BOXMENU.CPP

/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/


void i4_box_menu_class::show(i4_parent_window_class *show_on, i4_coord _x, i4_coord _y)
{
  if (parent)
    i4_error("show called twice");

  win_iter m=children.begin();
  w16 hi=0,wi=0;
  i4_coord dy=2,dx=2;

  for (;m!=children.end();++m)
  {
    hi+=m->height();
    if (m->width()>wi)
      wi=m->width();

    m->private_move(dx-(m->x()-x()),dy-(m->y()-y()));
    dy+=m->height();
  }
    
  for (m=children.begin(); m!=children.end(); ++m)
    m->private_resize(wi, m->height());


  resize(wi+4,hi+4);

  show_on->add_child(_x,_y,this);
}

void i4_box_menu_class::hide()
{
  if (!parent)
    return ;
  
  parent->remove_child(this);
  parent=0;
}


void i4_box_menu_class::parent_draw(i4_draw_context_class &context)
{
  local_image->add_dirty(0,0,width()-1,height()-1,context);
    
  i4_color_hint_class::bevel *color;

  color=&style->color_hint->window.passive;
    
  i4_color b,d,m=color->medium;
  b=color->bright;
  d=color->dark;


  local_image->bar(0,0,width()-1,0,b,context);
  local_image->bar(0,0,0,height()-1,b,context);    
  local_image->bar(1,1,width()-2,1,m,context);
  local_image->bar(1,1,1,height()-2,m,context);

  local_image->bar(2,height()-2,width()-2,height()-2,d,context);
  local_image->bar(width()-2,2,width()-2,height()-2,d,context);
  local_image->bar(0,height()-1,width()-1,height()-1,style->color_hint->black,context);
  local_image->bar(width()-1,0,width()-1,height()-1,style->color_hint->black,context);

  local_image->bar(2,2,width()-3,height()-3,color->medium,context);  
}
