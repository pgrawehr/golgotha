/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "pch.h"

#include "string/string.h"
#include "file/get_filename.h"

#include "main/win_main.h"
#include "window/style.h"
#include "window/window.h"
#include "window/colorwin.h"
#include "window/wmanager.h"
#include "gui/text_input.h"
#include "status/status.h"
#include "gui/create_dialog.h"
#include "gui/browse_tree.h"
#include "gui/text.h"
#include "gui/list_pick.h"
#include "menu/textitem.h"
#include "gui/image_win.h"
#include "gui/button.h"
#include "gui/deco_win.h"
#include "gui/list_box.h"
#include "gui/smp_dial.h"
#include "lisp/lisp.h"

/*
static int filename_sorter(i4_window_class *const *a, i4_window_class *const *b)
    {
    i4_text_item_class *itema,*itemb;
    itema=(i4_text_item_class *)*a;
    itemb=(i4_text_item_class *)*b;
    if (itema->get_text()<itemb->get_text())
        return 1;
    else if (itema->get_text()>itemb->get_text())
        return -1;
    else return 0;
    }
*/
static int filename_sorter(i4_str *const *a, i4_str *const *b)
    {
    i4_str *itema=*a;
    i4_str *itemb=*b;
    if (*itema<*itemb)
        return -1;
    else if (*itema>*itemb)
        return 1;
    else return 0;
    }


class i4_get_save_load_dialog : public i4_color_window_class
{
  i4_graphical_style_class *style;
  w32 ok_id, cancel_id;
  i4_event_handler_class *tell_who;
  i4_const_str title_name,start_dir,file_mask,mask_name;
  i4_list_pick *filepick,*dirpick;
  i4_deco_window_class *deco1,*deco2;
  i4_str cur_dir;
  w32 sel_elem;
  i4_text_window_class *text_win;
  i4_text_input_class *text_input_win;
  i4_list_box_class *listbox;
  w32 flags;
public:
    typedef int (*compare_type)(const void *x, const void *y);
  i4_parent_window_class *mp_window;

  enum { OK   =0x80000000, 
      CANCEL  =0x80000001, 
      CLOSE   =0x80000002,
      LISTBOX =0x80000003,
      DIRFLAG =0x40000000 };

  enum savedialogflags
      {
      DEFAULT = 0,
      LOAD    = 0,
      SAVE    = 1,
      CONFIRMOVERWRITE = 2
      };


//don't use references here, as local stays in memory while caller will not.
  i4_get_save_load_dialog(const i4_const_str _title_name,
                     const i4_const_str _start_dir,
                     const i4_const_str _file_mask,
                     const i4_const_str _mask_name, 
                     i4_graphical_style_class *style,
                     w32 ok_id, w32 cancel_id,
                     i4_event_handler_class *tell_who,
                     w32 _flags)
    : i4_color_window_class(0,0, style->color_hint->neutral(), style),
      style(style),
      ok_id(ok_id),
      cancel_id(cancel_id),
      tell_who(tell_who),
      title_name(_title_name),
      start_dir(_start_dir),
      file_mask(_file_mask),
      mask_name(_mask_name),
      cur_dir(_start_dir),
      flags(_flags)
  {    
    mp_window=0;
    text_input_win=0;
    cur_dir.insert(cur_dir.begin(),"./");
    simplify_dir(cur_dir);
    //i4_create_dialog(i4gets("get_savename_dialog"), 
    //                 this,
    //                 style,
    //                 &ti,
    //                 &start_dir,
    //                 this, OK,
    //                 this, CANCEL);
    i4_directory_struct files;
    i4_get_directory(cur_dir,files,i4_F,0);
    if (files.tfiles>0)
        qsort(files.files,files.tfiles,4,(compare_type)filename_sorter);
    i4_array<i4_window_class*> items(200,250);
    w32 i,j;
    for (i=0;i<files.tfiles;i++)
        {
        if (matches_filter(*files.files[i],file_mask))
            {
            //this is required because i grows much faster than j and
            //therefore the wrong entry would get the message. 
            j=items.size();
            items.add(new i4_text_item_class(*files.files[i],
                style,
                style->color_hint,
                style->font_hint->small_font,
                new i4_event_reaction_class(this,j)));
            }
        }
    //items.sort(filename_sorter);
    //be aware that the list might be empty. Must not
    //calculate &items[0] for an empty list.
    filepick=new i4_list_pick_cpitems(200,250,
        items.size(),items.size()?&items[0]:0,
        i4_list_pick::LB_SCROLLSELF,
        0);
    items.clear();
    for (i=0;i<files.tdirs;i++)
        {
        items.add(new i4_text_item_class(*files.dirs[i],
            style,
            style->color_hint,
            style->font_hint->small_font,
            new i4_event_reaction_class(this,i|DIRFLAG)));
        }
    //items.sort(filename_sorter);
    add_drives_to_list(items);
    //This list is actually never empty (windows systems have
    //at least one disk), but for safety..
    dirpick=new i4_list_pick_cpitems(200,250,
        items.size(),items.size()?&items[0]:0,
        i4_list_pick::LB_SCROLLSELF,
        0);
    //i4_parent_window_class *p=new i4_browse_window_class(style,
    //    new i4_text_window_class(start_dir,style),
    //    pick,i4_T,i4_T);
    w32 l=0,t=0,r=0,b=0;
    style->get_in_deco_size(l,t,r,b);
    deco1=new i4_deco_window_class(200,250,
        i4_T,style);
    deco2=new i4_deco_window_class(200,250,
        i4_T,style);
    deco1->add_child(l,t,dirpick);
    deco2->add_child(l,t,filepick);
    add_child(0,0,deco1);
    add_child(deco1->width(),0,deco2);
    sel_elem=0;

    i4_window_class *ok, *cancel;

    if (style->icon_hint->ok_icon && style->icon_hint->cancel_icon)
    {
        ok=new i4_image_window_class(style->icon_hint->ok_icon);
        cancel=new i4_image_window_class(style->icon_hint->cancel_icon);
    }
    else
    {
        ok=new i4_text_window_class(i4gets("ok"), style);
        cancel=new i4_text_window_class(i4gets("cancel"), style);
    }

    resize_to_fit_children();

    int x=0,maxh=filepick->height();
    i4_button_class *okb=new i4_button_class(0, ok, style, 
                                           new i4_event_reaction_class(this, OK));
    i4_button_class *cancelb=new i4_button_class(0, cancel, style, 
                                               new i4_event_reaction_class(this, CANCEL));
    x=width()/2-okb->width()/2-cancelb->width()/2;
    if (x<0) x=0;

    if (flags&SAVE)
        {
        text_win=new i4_text_window_class(cur_dir,style);
        add_child(5,maxh+4,text_win);
        maxh=maxh+4+text_win->height();
        text_input_win=new i4_text_input_class(style,"",
            width()-4,MAX_PATH);
        add_child(5,maxh+4,text_input_win);
        maxh=maxh+4+text_input_win->height();
        }
    else
        {
        text_win=new i4_text_window_class(cur_dir,style);
        add_child(5,maxh+4,text_win);
        maxh=maxh+4+text_win->height();
        text_input_win=0;
        }

    listbox=new i4_list_box_class(width()-4,
      style,i4_current_app->get_window_manager(),
      new i4_event_reaction_class(this,LISTBOX));
    i4_str filter;
    filter=mask_name;
    filter+=i4_str(" (");
    filter+=file_mask;
    filter+=i4_str(")");
    listbox->add_item(new i4_text_item_class(filter,style));
    listbox->set_current_item(0);
    filter=i4_str("All files (*.*)");
    listbox->add_item(new i4_text_item_class(filter,style));
    add_child(2,maxh+2,listbox);
    maxh=maxh+4+listbox->height();
    add_child(x, maxh+1, okb);
    add_child(x+okb->width(), maxh+1, cancelb);

    resize_to_fit_children();
    }

void receive_event(i4_event *ev)
 {
 CAST_PTR(uev, i4_user_message_event_class, ev);
 
 if (ev->type()==i4_event::USER_MESSAGE)
     {
     CAST_PTR(uev, i4_user_message_event_class, ev);
     if (uev->sub_type==OK)          // tell who-ever what the user typed
         {
         if (flags&SAVE)
             {
             i4_str sf;
             sf=*text_input_win->get_edit_string();
             
             if ((sf.length()==0)||
                 (sf.find_first_of(";:/\\?*")!=-1))
                 {
                 i4_message_box("Error",
                     "Invalid file name specified. The name must not contain anything like ;:/\\?*");
                 return;
                 }
             //Todo: Insert extension, if none given. But yields to
             //trouble if multiple were given. 
             sf.insert(sf.begin(),'/');
             sf.insert(sf.begin(),cur_dir);
             if (flags&CONFIRMOVERWRITE)
                 {
                 i4_file_class *openfile=i4_open(sf,I4_READ);
                 if (openfile)
                     {
                     if (i4_message_box("File exists.",
                         "A file with the given name exists already. Overwrite?",
                         MSG_YES|MSG_NO)==MSG_NO)
                         {
                         delete openfile;
                         return;
                         }
                     }
                 delete openfile;
                 }
             i4_file_open_message_class opm(ok_id,new i4_str(sf));
             i4_kernel.send_event(tell_who, &opm);
             }
         else
             {
             i4_str f;
             f=((i4_text_item_class*)filepick->get_item(sel_elem))->get_text();
             f.insert(f.begin(),'/');
             f.insert(f.begin(),cur_dir);
             i4_file_open_message_class o(ok_id,new i4_str(f));
             i4_kernel.send_event(tell_who, &o);
             }
         }
     else if (uev->sub_type==LISTBOX)
         {
         i4_text_item_class *tex=(i4_text_item_class*)listbox->get_current_item();
         i4_str fil=tex->get_text();
         fil.erase(0,fil.find_first_of("(")+1);
         fil.erase(fil.find_last_of(")"),1);
         file_mask=fil;
         update_file_view(0xFFFFFFFF);
         return;
         }
     else if ((w32)uev->sub_type<(w32)filepick->num_items())
         {
         //((i4_text_item_class *)items[sel_elem])->bg_color=style->color_hint->neutral();
         //items[sel_elem]->request_redraw();
         //sel_elem=uev->sub_type;
         //((i4_text_item_class *)items[sel_elem])->bg_color=style->color_hint->window.active.medium;
         //request_redraw(i4_T);
         ((i4_text_item_class*)filepick->get_item(sel_elem))->bg_color=
             style->color_hint->neutral();
         filepick->get_item(sel_elem)->request_redraw();
         sel_elem=uev->sub_type;
         ((i4_text_item_class*)filepick->get_item(sel_elem))->bg_color=
             style->color_hint->window.active.medium;
         filepick->get_item(sel_elem)->request_redraw();
         return;
         }
     else if ((w32)uev->sub_type<(dirpick->num_items()|DIRFLAG))
         {
         //choose new directory
         update_file_view((w32)uev->sub_type);
         
         return;
         }
     else 
         {
         i4_user_message_event_class c(cancel_id); // tell whoever the user canceled
         i4_kernel.send_event(tell_who, &c);
         }
     
     style->close_mp_window(mp_window);   // close ourself
     } 
 else i4_parent_window_class::receive_event(ev);
 }

void update_file_view(w32 entry)
    {
    
    if (entry!=0xFFFFFFFF)
        {
        i4_str d;
        entry&=~DIRFLAG;
        d=((i4_text_item_class*)dirpick->get_item(entry))->get_text();
        cur_dir.insert(cur_dir.end(),'/');
        cur_dir.insert(cur_dir.end(),d);
        simplify_dir(cur_dir);
        }
    i4_directory_struct files;
    i4_get_directory(cur_dir,files,i4_F,0);
    if (files.tfiles>0)
        qsort(files.files,files.tfiles,4,(compare_type)filename_sorter);
    i4_array<i4_window_class*> items(200,250);
    sel_elem=0;
    w32 i,j;
    for (i=0;i<files.tfiles;i++)
        {
        if (matches_filter(*files.files[i],file_mask))
            {
            j=items.size();
            items.add(new i4_text_item_class(*files.files[i],
                style,
                style->color_hint,
                style->font_hint->small_font,
                new i4_event_reaction_class(this,j)));
            }
        }
    //must not sort this list here, sine the reaction id's wouldn't 
    //match afterwards. 
    //items.sort(filename_sorter);
    filepick->update(items.size(),items.size()?&items[0]:0);
    filepick->request_redraw(i4_T);
    items.clear();
    for (i=0;i<files.tdirs;i++)
     {
     items.add(new i4_text_item_class(*files.dirs[i],
         style,
         style->color_hint,
         style->font_hint->small_font,
         new i4_event_reaction_class(this,i|DIRFLAG)));
     }
    //items.sort(filename_sorter);
    add_drives_to_list(items);
    dirpick->update(items.size(),items.size()?&items[0]:0);
    dirpick->request_redraw(i4_T);
    
    text_win->set_text(new i4_str(cur_dir));
    text_win->resize_to_fit_text();

    text_win->request_redraw();
    }

void simplify_dir(i4_str &dir)
    {
    int k=dir.find_first_of(":");
    if (k!=-1)
        {
        dir.erase(0,k-1);
        //return;
        }
    i4_str *newdir=i4_full_path(dir);
    dir=*newdir;
    delete newdir;
    
    }

protected:
i4_str get_filter(const i4_str &filter, int n) const
    {
    char str[250];
    memset(str,0,250);
    strncpy(str,filter.c_str(),248);
    int k=0;
    if (n==0)
        {
        k=filter.find_first_of(";");
        return k<1?filter:i4_str(str,k);
        }
    int i=0;
    char *pstr=str;
    while (strlen(pstr)>0)
        {
        if (pstr[i]==';')
            {
            pstr[i]=0;
            }
        pstr++;
        }
    //no more filters
    if (strlen(str)==0)
        return "";
    pstr=str;
    i=0;
    while (n>0&&i<250)
        {
        if (pstr[i]==0)
            n--;
        pstr++;
        }
    if (i==250)
        return "";
    return i4_str(pstr);
    }

i4_bool matches_filter(const i4_const_str &file, 
                       const i4_const_str &filter) const
    {
    if (filter=="*.*")
        return i4_T;
    i4_bool hasmorefilters=i4_T;
    int n=0;
    while (hasmorefilters)
        {
        i4_str afilt;
        afilt=get_filter(filter,n);
        if (afilt=="")
            return i4_F;
        afilt.erase(0,1);//remove the "*"
        if (file.strstr(afilt)!=file.end())
            return i4_T;
        n++;
        }
    return i4_F;
    
    }

// For windows, we need to add the drives to the
// directory list, or we'll never find them.
// for unix, this method currently does nothing.
void add_drives_to_list(i4_array<i4_window_class*> &l)
    {
#ifdef _WINDOWS
    int t=0;
    char s[5];
    for (int drive=0;drive<26;drive++)
        {
        sprintf(s,"%c:\\",(drive+'A'));
		t=GetDriveType(s);
        if ((t!=DRIVE_UNKNOWN)&&(t!=DRIVE_NO_ROOT_DIR))
            {
            int pos=l.size();
            l.add(new i4_text_item_class(s,
            style,
            style->color_hint,
            style->font_hint->small_font,
            new i4_event_reaction_class(this,pos|DIRFLAG)));
            }
        }
#endif
    }
public:
  char *name() { return "load dialog"; }
};

//#ifdef _WINDOWS

/*
class open_string : public i4_str
{
public:
  open_string(char *fname)
    : i4_str(fname)
  {
    //len=strlen(fname);
    //memcpy(ptr, fname, len);
  }
};
*/
#if 0
#ifndef _CONSOLE
long FAR PASCAL WindowProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );



UINT APIENTRY win32_dialog_hook( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
  switch( message )
  {
    case WM_LBUTTONUP :

    case WM_RBUTTONUP :

    case WM_MBUTTONUP :
    case WM_ACTIVATEAPP :


    case WM_SYSKEYUP :
    case WM_KEYUP :
      WindowProc(hWnd, message, wParam, lParam);
      break;
  }

  return FALSE;
}

#endif

void i4_create_file_open_dialog(i4_graphical_style_class *style,
                                const i4_const_str &title_name,
                                const i4_const_str &start_dir,
                                const i4_const_str &file_mask,
                                const i4_const_str &mask_name,
                                i4_event_handler_class *tell_who,
                                w32 ok_id, w32 cancel_id)
{
  OPENFILENAME ofn;

  char mname[256], m[256], tname[256], idir[256], fname[256], curdir[256];

  ShowCursor(TRUE);

  _getcwd(curdir,256);

  i4_os_string(mask_name, mname, sizeof(mname));
  i4_os_string(file_mask, m, sizeof(m));
  i4_os_string(title_name, tname, sizeof(tname));
  i4_os_string(start_dir, idir, sizeof(idir));


  char *af="All files(*.*)\0*.*\0\0";

  int i=strlen(mname);
  int l=strlen(m) + 1;
  mname[i++]='(';
  memcpy(mname + i, m, l);
  i+=l;
  mname[i-1]=')';
  mname[i++]=0;
  memcpy(mname+i,m,l);
  i+=l;
  l=20;
  memcpy(mname + i, af, l);
  

  fname[0]=0;

  ofn.lStructSize = sizeof(OPENFILENAME);
#ifdef _CONSOLE
  ofn.hwndOwner=0;
#else
  ofn.hwndOwner = i4_win32_window_handle;
#endif
  ofn.hInstance = i4_win32_instance;
  ofn.lpstrCustomFilter = 0;
  ofn.nMaxCustFilter = 0;
  ofn.nFilterIndex = 0;
  ofn.lpstrFile = fname;
  ofn.nMaxFile = 256;

  ofn.nMaxFileTitle = 0;
  ofn.lpstrFileTitle = NULL;//tname;

  ofn.lpstrInitialDir = idir;
  ofn.nFileOffset = 0;
  ofn.nFileExtension = 0;
  ofn.lCustData = 0L;
  ofn.lpfnHook = NULL;
  ofn.lpTemplateName = NULL;
#ifndef _CONSOLE
  ofn.lpfnHook = win32_dialog_hook;
#endif

  ofn.lpstrFilter = mname;
  ofn.lpstrDefExt = "level";
  ofn.lpstrTitle = tname;
  ofn.Flags = OFN_HIDEREADONLY | OFN_ENABLEHOOK | OFN_EXPLORER |OFN_FILEMUSTEXIST ;
#ifndef _CONSOLE
  WindowProc(i4_win32_window_handle, WM_LBUTTONUP, 0,0);
  WindowProc(i4_win32_window_handle, WM_RBUTTONUP, 0,0);
  WindowProc(i4_win32_window_handle, WM_MBUTTONUP, 0,0);


  li_call("show_gdi_surface");//Be shure gdi-surface is on top
#endif
  if (GetOpenFileName(&ofn)) 
  {	
    i4_file_open_message_class o(ok_id, new i4_str(ofn.lpstrFile));
    i4_kernel.send_event(tell_who, &o);
  }
  else
  {
    i4_user_message_event_class o(cancel_id);
    i4_kernel.send_event(tell_who, &o);
  }
#ifndef _CONSOLE
  li_call("hide_gdi_surface");
#endif
  _chdir(curdir);

  ShowCursor(FALSE);
}
#else

void i4_create_file_open_dialog(i4_graphical_style_class *style,
                                const i4_const_str &title_name,
                                const i4_const_str &start_dir,
                                const i4_const_str &file_mask,
                                const i4_const_str &mask_name,
                                i4_event_handler_class *tell_who,
                                w32 ok_id, w32 cancel_id)
    {
    i4_get_save_load_dialog *dlg=new i4_get_save_load_dialog(
        title_name,start_dir,file_mask,mask_name,style,
        ok_id, cancel_id, tell_who,0 );
    
    i4_event_reaction_class *re;
    re=new i4_event_reaction_class(dlg, 
        new i4_user_message_event_class(i4_get_save_load_dialog::CLOSE));
    
    
    i4_parent_window_class *p;
    
    p=style->create_mp_window(-1, -1, dlg->width(), dlg->height(),
        title_name,
        re);
    
    p->add_child(0,0, dlg);
    dlg->mp_window=p;
    }

#endif

#if 0
void i4_create_file_save_dialog(i4_graphical_style_class *style,
                                const i4_const_str &default_name,
                                const i4_const_str &title_name,
                                const i4_const_str &start_dir,
                                const i4_const_str &file_mask,
                                const i4_const_str &mask_name,
                                i4_event_handler_class *tell_who,
                                w32 ok_id, w32 cancel_id)
{
  OPENFILENAME ofn;

  char mname[256], m[256], tname[256], idir[256], fname[256], curdir[256];

  _getcwd(curdir,256);

  i4_os_string(mask_name, mname, sizeof(mname));
  i4_os_string(file_mask, m, sizeof(m));
  i4_os_string(title_name, tname, sizeof(tname));
  i4_os_string(start_dir, idir, sizeof(idir));


  char *af="All files(*.*)\0*.*\0\0";

  int i=strlen(mname);
  int l=strlen(m) + 1;
  mname[i++]='(';
  memcpy(mname + i, m, l);
  i+=l;
  mname[i-1]=')';
  mname[i++]=0;
  memcpy(mname+i,m,l);
  i+=l;
  l=20;
  memcpy(mname + i, af, l);

  fname[0]=0;

  ofn.lStructSize = sizeof(OPENFILENAME);
#ifndef _CONSOLE
  ofn.hwndOwner = i4_win32_window_handle;
#else
  ofn.hwndOwner=0;
#endif
  ofn.hInstance = i4_win32_instance; 

  ofn.lpstrCustomFilter = 0;
  ofn.nMaxCustFilter = 0;
  ofn.nFilterIndex = 0;
  ofn.lpstrFile = fname;
  ofn.nMaxFile = 256;

  ofn.nMaxFileTitle = 0;
  ofn.lpstrFileTitle =NULL;

  ofn.lpstrInitialDir = idir;
  ofn.nFileOffset = 0;
  ofn.nFileExtension = 0;
  ofn.lCustData = 0L;
  ofn.lpfnHook = NULL;
  ofn.lpTemplateName = NULL;
#ifndef _CONSOLE
  ofn.lpfnHook = win32_dialog_hook;
#endif
  ofn.lpstrFilter = mname;
  ofn.lpstrDefExt = m;
  while (*ofn.lpstrDefExt && *ofn.lpstrDefExt!='.')
    ofn.lpstrDefExt++;

  ofn.lpstrTitle = tname;
  ofn.Flags = OFN_HIDEREADONLY | OFN_ENABLEHOOK | OFN_EXPLORER |OFN_OVERWRITEPROMPT;
#ifndef _CONSOLE
  WindowProc(i4_win32_window_handle, WM_LBUTTONUP, 0,0);
  WindowProc(i4_win32_window_handle, WM_RBUTTONUP, 0,0);
  WindowProc(i4_win32_window_handle, WM_MBUTTONUP, 0,0);

  li_call("show_gdi_surface");//Be shure gdi-surface is on top if page-flipped
#endif
  if (GetSaveFileName(&ofn)) 
  {	
    i4_file_open_message_class o(ok_id, new i4_str(ofn.lpstrFile));
    i4_kernel.send_event(tell_who, &o);
  }
  else
  {
    i4_user_message_event_class o(cancel_id);
    i4_kernel.send_event(tell_who, &o);
  }
#ifndef _CONSOLE
  li_call("hide_gdi_surface");
#endif
  _chdir(curdir);
}

#else

void i4_create_file_save_dialog(i4_graphical_style_class *style,
                                const i4_const_str &default_name,
                                const i4_const_str &title_name,
                                const i4_const_str &start_dir,
                                const i4_const_str &file_mask,
                                const i4_const_str &mask_name,
                                i4_event_handler_class *tell_who,
                                w32 ok_id, w32 cancel_id)
    {
    i4_get_save_load_dialog *dlg=new i4_get_save_load_dialog(
        title_name,start_dir,file_mask,mask_name,style,
        ok_id, cancel_id, tell_who, 
        i4_get_save_load_dialog::SAVE
        |i4_get_save_load_dialog::CONFIRMOVERWRITE);
    
    i4_event_reaction_class *re;
    re=new i4_event_reaction_class(dlg, 
        new i4_user_message_event_class(i4_get_save_load_dialog::CLOSE));
    
    
    i4_parent_window_class *p;
    
    p=style->create_mp_window(-1, -1, dlg->width(), dlg->height(),
        title_name,
        re);
    
    p->add_child(0,0, dlg);
    dlg->mp_window=p;
    }



#endif

#if 0

//Unix file open dialog - No more needed,
//since there's only one version now



/*
class open_string : public i4_str
{
public:
  open_string(char *fname)
    : i4_str(fname)
  {
    //len=strlen(fname);
    //memcpy(ptr, fname, len);

  }
};
*/


void i4_create_file_open_dialog(i4_graphical_style_class *style,
                                const i4_const_str &title_name,
                                const i4_const_str &start_dir,
                                const i4_const_str &file_mask,
                                const i4_const_str &mask_name,
                                i4_event_handler_class *tell_who,
                                w32 ok_id, w32 cancel_id)
{
  char mname[256], m[256], tname[256], idir[256], pcmd[1000];

  i4_os_string(mask_name, mname, sizeof(mname));
  i4_os_string(file_mask, m, sizeof(m));
  i4_os_string(title_name, tname, sizeof(tname));
  i4_os_string(start_dir, idir, sizeof(idir));

  char *display=getenv("DISPLAY");
  if (!display || display[0]==0)
    display=":0.0";

  char *tmp_name="/tmp/open_filename.tmp";
  sprintf(pcmd, "xgetfile -title \"%s\" -pattern \"%s\" -path \"%s\" -popup -display %s > %s",
          tname, m, idir, display, tmp_name);

  unlink(tmp_name);
  system(pcmd);
  FILE *fp=fopen(tmp_name,"rb");
  if (!fp || !fgets(m, 256, fp))
    m[0]=0;
  else
    while (m[strlen(m)-1]=='\n')
      m[strlen(m)-1]=0;
  
  if (fp) fclose(fp);
  unlink(tmp_name);


  char *s=m;

  if (*s==0 || *s=='\n')
  {
    i4_user_message_event_class o(cancel_id);
    i4_kernel.send_event(tell_who, &o);
  }
  else
  {
    i4_file_open_message_class o(ok_id, new i4_str(s));
    i4_kernel.send_event(tell_who, &o);
  }


};


class i4_get_save_dialog : public i4_color_window_class
{
  i4_text_input_class *ti;
  i4_graphical_style_class *style;
  w32 ok_id, cancel_id;
  i4_event_handler_class *tell_who;
  i4_str ext_default_name;
public:
  i4_parent_window_class *mp_window;

  enum { OK, CANCEL, CLOSE };

//don't use references here, as local stays in memory while caller will not.
  i4_get_save_dialog(i4_const_str default_name, i4_const_str start_dir, 
                     i4_graphical_style_class *style,
                     w32 ok_id, w32 cancel_id,
                     i4_event_handler_class *tell_who)
    : i4_color_window_class(0,0, style->color_hint->neutral(), style),
      style(style),
      ok_id(ok_id),
      cancel_id(cancel_id),
      tell_who(tell_who),
	ext_default_name(start_dir)
  {    
    mp_window=0;
    ext_default_name.insert(ext_default_name.end(),default_name);
    i4_create_dialog(i4gets("get_savename_dialog"), 
                     this,
                     style,
                     &ti,
                     &ext_default_name,
                     this, OK,
                     this, CANCEL);
                     
    resize_to_fit_children();
  }

  void receive_event(i4_event *ev)
  {
    if (ev->type()==i4_event::USER_MESSAGE)
    {
      CAST_PTR(uev, i4_user_message_event_class, ev);
      if (uev->sub_type==OK)          // tell who-ever what the user typed
      {
        i4_file_open_message_class o(ok_id,new i4_str(*ti->get_edit_string()));
        i4_kernel.send_event(tell_who, &o);
      }
      else
      {
        i4_user_message_event_class c(cancel_id); // tell whoever the user canceled
        i4_kernel.send_event(tell_who, &c);
      }

      style->close_mp_window(mp_window);   // close ourself
    } 
    else i4_parent_window_class::receive_event(ev);
  }
  
  char *name() { return "save dialog"; }
};



void i4_create_file_save_dialog(i4_graphical_style_class *style,
                                const i4_const_str &default_name,
                                const i4_const_str &title_name,
                                const i4_const_str &start_dir,
                                const i4_const_str &file_mask,
                                const i4_const_str &mask_name,
                                i4_event_handler_class *tell_who,
                                w32 ok_id, w32 cancel_id)
{
  i4_get_save_dialog *dlg=new i4_get_save_dialog(default_name, start_dir, style, 
                                                 ok_id, cancel_id, tell_who);

  i4_event_reaction_class *re;
  re=new i4_event_reaction_class(dlg, 
                                 new i4_user_message_event_class(i4_get_save_dialog::CLOSE));


  i4_parent_window_class *p;

  p=style->create_mp_window(-1, -1, dlg->width(), dlg->height(),
                            title_name,
                            re);

  p->add_child(0,0, dlg);
  dlg->mp_window=p;
}

#endif
