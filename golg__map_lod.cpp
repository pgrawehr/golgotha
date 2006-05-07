/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/
#include "pch.h"
#include "map.h"
#include "map_man.h"
#include "map_vert.h"
#include "map_cell.h"
#include "main/win_main.h"
#include "math/d_transform.h"

//#include "div_table.cc"
/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

//static const i4_float div_table[] = 
//{
//  1.0f       ,1.0f/  1.0f,1.0f/  2.0f,1.0f/  3.0f,1.0f/  4.0f,1.0f/  5.0f,1.0f/  6.0f,1.0f/  7.0f,1.0f/  8.0f,1.0f/  9.0f,
//  1.0f/ 10.0f,1.0f/ 11.0f,1.0f/ 12.0f,1.0f/ 13.0f,1.0f/ 14.0f,1.0f/ 15.0f,1.0f/ 16.0f,1.0f/ 17.0f,1.0f/ 18.0f,1.0f/ 19.0f,
//  1.0f/ 20.0f,1.0f/ 21.0f,1.0f/ 22.0f,1.0f/ 23.0f,1.0f/ 24.0f,1.0f/ 25.0f,1.0f/ 26.0f,1.0f/ 27.0f,1.0f/ 28.0f,1.0f/ 29.0f,
//  1.0f/ 30.0f,1.0f/ 31.0f,1.0f/ 32.0f,1.0f/ 33.0f,1.0f/ 34.0f,1.0f/ 35.0f,1.0f/ 36.0f,1.0f/ 37.0f,1.0f/ 38.0f,1.0f/ 39.0f,
//  1.0f/ 40.0f,1.0f/ 41.0f,1.0f/ 42.0f,1.0f/ 43.0f,1.0f/ 44.0f,1.0f/ 45.0f,1.0f/ 46.0f,1.0f/ 47.0f,1.0f/ 48.0f,1.0f/ 49.0f,
//  1.0f/ 50.0f,1.0f/ 51.0f,1.0f/ 52.0f,1.0f/ 53.0f,1.0f/ 54.0f,1.0f/ 55.0f,1.0f/ 56.0f,1.0f/ 57.0f,1.0f/ 58.0f,1.0f/ 59.0f,
//  1.0f/ 60.0f,1.0f/ 61.0f,1.0f/ 62.0f,1.0f/ 63.0f,1.0f/ 64.0f,1.0f/ 65.0f,1.0f/ 66.0f,1.0f/ 67.0f,1.0f/ 68.0f,1.0f/ 69.0f,
//  1.0f/ 70.0f,1.0f/ 71.0f,1.0f/ 72.0f,1.0f/ 73.0f,1.0f/ 74.0f,1.0f/ 75.0f,1.0f/ 76.0f,1.0f/ 77.0f,1.0f/ 78.0f,1.0f/ 79.0f,
//  1.0f/ 80.0f,1.0f/ 81.0f,1.0f/ 82.0f,1.0f/ 83.0f,1.0f/ 84.0f,1.0f/ 85.0f,1.0f/ 86.0f,1.0f/ 87.0f,1.0f/ 88.0f,1.0f/ 89.0f,
//  1.0f/ 90.0f,1.0f/ 91.0f,1.0f/ 92.0f,1.0f/ 93.0f,1.0f/ 94.0f,1.0f/ 95.0f,1.0f/ 96.0f,1.0f/ 97.0f,1.0f/ 98.0f,1.0f/ 99.0f,
//  1.0f/100.0f,1.0f/101.0f,1.0f/102.0f,1.0f/103.0f,1.0f/104.0f,1.0f/105.0f,1.0f/106.0f,1.0f/107.0f,1.0f/108.0f,1.0f/109.0f,
//  1.0f/110.0f,1.0f/111.0f,1.0f/112.0f,1.0f/113.0f,1.0f/114.0f,1.0f/115.0f,1.0f/116.0f,1.0f/117.0f,1.0f/118.0f,1.0f/119.0f,
//  1.0f/120.0f,1.0f/121.0f,1.0f/122.0f,1.0f/123.0f,1.0f/124.0f,1.0f/125.0f,1.0f/126.0f,1.0f/127.0f,1.0f/128.0f,1.0f/129.0f,
//  1.0f/130.0f,1.0f/131.0f,1.0f/132.0f,1.0f/133.0f,1.0f/134.0f,1.0f/135.0f,1.0f/136.0f,1.0f/137.0f,1.0f/138.0f,1.0f/139.0f,
//  1.0f/140.0f,1.0f/141.0f,1.0f/142.0f,1.0f/143.0f,1.0f/144.0f,1.0f/145.0f,1.0f/146.0f,1.0f/147.0f,1.0f/148.0f,1.0f/149.0f,
//  1.0f/150.0f,1.0f/151.0f,1.0f/152.0f,1.0f/153.0f,1.0f/154.0f,1.0f/155.0f,1.0f/156.0f,1.0f/157.0f,1.0f/158.0f,1.0f/159.0f
//};
//map_fast.cc

#include "camera.h"
#include "render/r1_api.h"
#include "render/tmanage.h"
#include "render/r1_clip.h"
#include "g1_render.h"
#include "tile.h"
#include "controller.h"
#include "statistics.h"
#include "lisp/li_class.h"
#include "lisp/li_init.h"
#include "resources.h"
#include "time/profile.h"
#include "light.h"

static i4_profile_class pf_map_fast("map_fast"), 
  pf_calc_map_lod("calc_map_lod"),
  pf_gather_objects("gather_objects");
  //pf_draw_tri("draw_tri");


i4_image_class *render_map_section(int x1, int y1, int x2, int y2, int im_w, int im_h);

inline float fmin(float a, float b) { return (a<b)?a:b; }
inline float fmax(float a, float b) { return (a>b)?a:b; }

class texture_context
{
public:
  i4_float x1,y1;                                 // offset in the map
  i4_float sx, sy;                                // scale factors to get [0,1] texture coords.
  r1_texture_handle texture;                      // texture to use
};

class lod_node
{
public:
  w16 x1,y1,x2,y2;                                // bounding box for this LOD node
  i4_float z1,z2;
  i4_float metric;                                // metric for determining LOD "importance"
  lod_node *child[2];                             // orthogonal BSP children
  w8 texture_context;                             // which texture context to use or 0 for 1x1

  w8 flags;
  enum
  {
    CLIPPED    = 1<<0,                            // this node is completely out of the view
    IN_VIEW    = 1<<1,                            //                         in the view
    CLIP_FLAGS = CLIPPED | IN_VIEW,
    V_SPLIT    = 1<<2,                            // node split vertically
  };

  lod_node(w16 x1,w16 y1,w16 x2,w16 y2, i4_float metric=0)
    : x1(x1), y1(y1), x2(x2), y2(y2), metric(metric)
  {
    child[0] = 0;
    child[1] = 0;
  }
};

class g1_lod_context_class
{
public:
  // texture contexts for the large subdivided texture used to cover the whole map
  enum {MAXCONTEXTS=6};
  texture_context map_texture[MAXCONTEXTS];
  int num_context, last_context;

  // LOD tree
  lod_node *root;

  // breadth first ordering of the lod_nodes
  enum {MAX_QUEUE_LENGTH=(150*100*2)};
  lod_node *queue[MAX_QUEUE_LENGTH*2];
  int num_queued, front;
  
  // lod_nodes submitted for drawing
  lod_node *quad[MAX_QUEUE_LENGTH];
  int num_quads;

  // current controller
  g1_object_controller_class *cont;
  i4_3d_vector pos;

  g1_lod_context_class() : root(0),max_fly_height(3.0f),show_all(false) {}
  ~g1_lod_context_class(void)
	  {
	  if (root)
	  delete_lod_tree(root);
	  }

  void init_lod()
  // make LOD tree
  {
    if (root)
      delete_lod_tree(root);

    num_context=0;
    make_lod_node(&root,0,0,0,g1_get_map()->width(),g1_get_map()->height());

	//we render the map here. This is used for the lowest lod in the main 3d view.
	//everything beyond max_view_distance is rendered with this texture.
    for (int i=1; i<=num_context; i++)
    {
      int x1,y1,x2,y2;

      x1 = i4_f_to_i(map_texture[i].x1);
      y1 = i4_f_to_i(map_texture[i].y1);
      x2 = x1 + i4_f_to_i(1.0f/map_texture[i].sx + 0.5f);
      y2 = y1 + i4_f_to_i(1.0f/map_texture[i].sy + 0.5f);
      i4_image_class *im = render_map_section(x1,y1,x2,y2, 256,256);
      map_texture[i].texture = g1_render.r_api->get_tmanager()->register_image(im);
      delete im;
    }
  }

  void update_lod_texture()
	  {
	  for (int i=1; i<=num_context; i++)
		{
        int x1,y1,x2,y2;

      x1 = i4_f_to_i(map_texture[i].x1);
      y1 = i4_f_to_i(map_texture[i].y1);
      x2 = x1 + i4_f_to_i(1.0f/map_texture[i].sx + 0.5f);
      y2 = y1 + i4_f_to_i(1.0f/map_texture[i].sy + 0.5f);
      i4_image_class *im = render_map_section(x1,y1,x2,y2, 256,256);
      //map_texture[i].texture = g1_render.r_api->get_tmanager()->register_image(im);
	  g1_render.r_api->get_tmanager()->set_texture_image(map_texture[i].texture,im);
      delete im;
		}
	  }

  void uninit_lod()
  {
    delete_lod_tree(root);
  }

  void use_controller(g1_object_controller_class *_cont) { cont = _cont; }

  i4_float max_fly_height;
  i4_bool show_all;
  int test_clip(lod_node *p)
  // test bounding region of lod_node against the view frustrum
  {
    //const i4_float MAX_FLY_HEIGHT=3.0;
    int clip = cont->test_clip(i4_3d_vector(p->x1,p->y1,p->z1), 
                               i4_3d_vector(p->x2,p->y2,p->z2+max_fly_height));

    w8 flags = 
      (clip<0)? lod_node::CLIPPED : 
      (clip>0)? lod_node::IN_VIEW :
      0;

    p->flags = (p->flags & ~lod_node::CLIP_FLAGS) | flags;

    return clip>=0;
  }

  void make_lod_node(lod_node **pp, lod_node *parent, int x1, int y1, int x2, int y2, int level=0)
  {
    int dx = x2-x1, dy = y2-y1;
    int mx = (x1+x2)/2, my = (y1+y2)/2;

    if (dx==0 || dy==0)
      return;

    lod_node *p;
    *pp = p = new lod_node(x1,y1,x2,y2);
    
    // hack to get texture contexts
    if (level<2)
      p->texture_context = 0;
    else if (level==2)
    {
      p->texture_context = ++num_context;
	  if (num_context>MAXCONTEXTS)
		  {
		  i4_warning("WARNING: Map is too large for LOD handling.");
		  }
      map_texture[num_context].x1 = (float)x1;
      map_texture[num_context].y1 = (float)y1;
      map_texture[num_context].sx = 1.0f/(x2-x1); //earlier div_table was used for this, but
      map_texture[num_context].sy = 1.0f/(y2-y1); //I actually doubt this is much faster
	  //and it fails if the map is larger than the table (no error, only values are wrong)
    }
    else
      p->texture_context = parent->texture_context;

    if (dx>1 || dy>1)
    {
      if (dx>=dy)
      {
        p->flags |= lod_node::V_SPLIT;
        make_lod_node(&p->child[0], p, x1,y1,mx,y2, level+1);
        make_lod_node(&p->child[1], p, mx,y1,x2,y2, level+1);
      }
      else
      {
        p->flags &= ~lod_node::V_SPLIT;
        make_lod_node(&p->child[0], p, x1,y1,x2,my, level+1);
        make_lod_node(&p->child[1], p, x1,my,x2,y2, level+1);
      }
      p->z1 = fmin(p->child[0]->z1, p->child[1]->z1);
      p->z2 = fmax(p->child[0]->z2, p->child[1]->z2);

      // area metric
      p->metric = p->child[0]->metric + p->child[1]->metric;
    }
    else
    {
      i4_float
        z11 = g1_get_map()->vertex(x1,y1)->get_height(),
        z21 = g1_get_map()->vertex(x2,y1)->get_height(),
        z12 = g1_get_map()->vertex(x1,y2)->get_height(),
        z22 = g1_get_map()->vertex(x2,y2)->get_height();
      
      p->z1 = p->z2 = z11;
      p->z1 = fmin(p->z1, z21);
      p->z2 = fmax(p->z2, z21);
      p->z1 = fmin(p->z1, z12);
      p->z2 = fmax(p->z2, z12);
      p->z1 = fmin(p->z1, z22);
      p->z2 = fmax(p->z2, z22);

      // area metric
      i4_float dz1,dz2;//Updated by Forum

      /*dz1 = z22 - z21;
      dz2 = z11 - z21;
      p->metric = (float)sqrt(dz1*dz1 + dz2*dz2 + 1);
      dz1 = z22 - z12;
      dz2 = z11 - z12;
      p->metric += dz1*dz1 + dz2*dz2 + 1;*/
	  dz1 = z22 - z21; 
	  dz2 = z11 - z21; 
	  p->metric = (float) sqrt(dz1*dz1 + dz2*dz2 + 1); 
      dz1 = z22 - z12; 
	  dz2 = z11 - z12; 
	  p->metric += (float) sqrt(dz1*dz1 + dz2*dz2 + 1); 
    }
  }

  void delete_lod_tree(lod_node *p)
  {
    if (!p)
      return;
    delete_lod_tree(p->child[0]);
    delete_lod_tree(p->child[1]);
    delete p;
  }

  void calculate_lod(lod_node *p)
  {
//     cont->view.get_camera_pos(pos);
    cont->get_pos(pos);

    num_quads=0;
    front = num_queued = 0;
    queue[num_queued++] = p;
    root->flags = 0;
	max_fly_height=3.0f;
	if (g1_get_map()->inside_map(pos.x,pos.y))
	{
		i4_float terrain_height=g1_get_map()->vertex(pos.x,pos.y)->get_height();
		i4_float diff=i4_fabs(pos.z-terrain_height);
		if (diff>max_fly_height)
			max_fly_height=diff;
		show_all=false;
		if (pos.z<terrain_height-3.0f)
			show_all=true;
	}
    while (front<num_queued)
    {
      p = queue[front++];
      int x1=p->x1,y1=p->y1,x2=p->x2,y2=p->y2;
      w8 sub_flags=p->flags & lod_node::IN_VIEW;

      // do some kind of clip test
      if (!sub_flags)
		  {
          if (test_clip(p))
            sub_flags=p->flags & lod_node::IN_VIEW;
          else
            continue;
		  }
//       int dx = x2-x1, dy = y2-y1;

		//Updated by Forum
//      int mx = (x1+x2)/2, my = (y1+y2)/2;

//      i4_float fx = pos.x - mx, fy = pos.y - my, fz = pos.z - g1_get_vertex(mx,my)->get_height();
//      i4_float d1 = fx*fx+fy*fy+fz*fz;
//      i4_float d2 = p->metric*20;

		int mx = (x1+x2)/2, my = (y1+y2)/2; 

		i4_float fx = pos.x - mx, fy = pos.y - my, fz = pos.z - g1_get_vertex(mx,my)->get_height(); 
		i4_float d1 = fx*fx+fy*fy+fz*fz; 
		i4_float d2 = p->metric*i4_win32_startup_options.max_view_distance;  // *200; 
		//i4_float d2=i4_win32_startup_options.max_view_distance;

      if (p->child[0]&&d1<d2)
      {
        p->child[0]->flags = (p->child[0]->flags & ~lod_node::CLIP_FLAGS) | sub_flags;
        p->child[1]->flags = (p->child[1]->flags & ~lod_node::CLIP_FLAGS) | sub_flags;
        queue[num_queued++] = p->child[0];
        queue[num_queued++] = p->child[1];

        // count & mark t splits
        lod_node *q = p->child[0];
        if (p->flags & lod_node::V_SPLIT)
        {
          g1_get_vertex(q->x2,q->y1)->flags ^= g1_map_vertex_class::T_INTERSECTION;
          g1_get_vertex(q->x2,q->y2)->flags ^= g1_map_vertex_class::T_INTERSECTION;
        }
        else
        {
          g1_get_vertex(q->x1,q->y2)->flags ^= g1_map_vertex_class::T_INTERSECTION;
          g1_get_vertex(q->x2,q->y2)->flags ^= g1_map_vertex_class::T_INTERSECTION;
        }
      }
      else
      {
        g1_map_vertex_class *v11, *v12, *v21, *v22;
      
        v11 =  g1_get_vertex(x1,y1);
        v21 =  g1_get_vertex(x2,y1);
        v12 =  g1_get_vertex(x1,y2);
        v22 =  g1_get_vertex(x2,y2);

        v11->t_height = v11->get_height();
        v21->t_height = v21->get_height();
        v12->t_height = v12->get_height();
        v22->t_height = v22->get_height();
        quad[num_quads++] = p;
      }
    }

    // T intersection fixing

    // Map of directions
    //          dzdx2
    //    z22*  ---->  *
    //       ^         ^
    // dzdy1 |         | dzdy2
    //       |         |
    //    z12*  ---->  *
    //      z11 dzdx1 z21
    for (int test=0; test<2; test++)
      for (front = 0; front<num_quads; front++)
      {
        int 
          x1=quad[front]->x1,y1=quad[front]->y1,
          x2=quad[front]->x2,y2=quad[front]->y2;
        int i,j;
        i4_float z11,z12,z21,z22;
        i4_float dzdx1,dzdx2,dzdy1,dzdy2;

        z11 = g1_get_vertex(x1,y1)->t_height;
        z21 = g1_get_vertex(x2,y1)->t_height;
        z12 = g1_get_vertex(x1,y2)->t_height;
        z22 = g1_get_vertex(x2,y2)->t_height;

        dzdx1 = (z21 - z11)*(1.0f/(x2 - x1));
        dzdx2 = (z22 - z12)*(1.0f/(x2 - x1));
        dzdy1 = (z12 - z11)*(1.0f/(y2 - y1));
        dzdy2 = (z22 - z21)*(1.0f/(y2 - y1));

        z22 = z12;
        z12 = z11;
      
        i4_float adj1,adj2;

#define DEPIXEL_ADJ 0.1f

        adj1 = (pos.x<x1)?DEPIXEL_ADJ:-DEPIXEL_ADJ;
        adj2 = (pos.x>x2)?DEPIXEL_ADJ:-DEPIXEL_ADJ;
        for (j=y1+1; j<y2; j++)
        {
          z11 += dzdy1;
          z21 += dzdy2;

//           if (g1_get_vertex(x1,j)->flags & g1_map_vertex_class::T_INTERSECTION)
            g1_get_vertex(x1,j)->t_height = z11 + adj1;
//           if (g1_get_vertex(x2,j)->flags & g1_map_vertex_class::T_INTERSECTION)
            g1_get_vertex(x2,j)->t_height = z21 + adj2;
        }
        
        adj1 = (pos.y<y1)?DEPIXEL_ADJ:-DEPIXEL_ADJ;
        adj2 = (pos.y>y2)?DEPIXEL_ADJ:-DEPIXEL_ADJ;
        for (i=x1+1; i<x2; i++)
        {
          z12 += dzdx1;
          z22 += dzdx2;
//           if (g1_get_vertex(i,y1)->flags & g1_map_vertex_class::T_INTERSECTION)
            g1_get_vertex(i,y1)->t_height = z12 + adj1;
//           if (g1_get_vertex(i,y2)->flags & g1_map_vertex_class::T_INTERSECTION)
            g1_get_vertex(i,y2)->t_height = z22 + adj2;
        }
      }
  }

} g1_lod;

LI_HEADER(update_lod_texture)
	{
	if (g1_map_is_loaded())
		{
		g1_lod.update_lod_texture();
		
		}
	return li_nil;
	}

li_automatic_add_function(li_update_lod_texture,"update_lod_texture");

void g1_map_class::init_lod()
{
  g1_lod.init_lod();
}

void g1_map_class::calc_map_lod(g1_object_controller_class *cont)
{
  pf_calc_map_lod.start();

  g1_lod.use_controller(cont);
  g1_lod.calculate_lod(g1_lod.root);

  pf_calc_map_lod.stop();
}

static r1_texture_ref g1_default_texture("tron_grid");
const w32          g1_max_objs_in_view = 2048;
//w32                g1_num_objs_in_view = 0;
//w32 g1_objs_in_view[g1_max_objs_in_view];
r1_vert temp_buf_1[9];
r1_vert temp_buf_2[9];

i4_array<g1_object_class *> g1_objs_in_view_dyn(0, g1_max_objs_in_view);
i4_array<i4_transform_class> g1_obj_transforms_in_view(0, g1_max_objs_in_view);
class transform_killer_class : public i4_init_class
{
  //fix this trey!  can't have global i4_array's without something to clean it up.
  void uninit() 
	  { 
	  g1_obj_transforms_in_view.uninit(); 
	  //g1_num_objs_in_view=0;
	  g1_objs_in_view_dyn.uninit();
	  }
} transform_killer;

static r1_texture_handle last_texture=0;
static sw32 last_texture_size=0;
static w32 num_terrain_polys=0;
static i4_bool g1_draw_tri(r1_vert *points, 
                           w16 a, w16 b, w16 c,
                           r1_texture_handle texture, w16 clip=1)
//{{{
{
  //pf_draw_tri.start();

  //g1_stat_counter.increment(g1_statistics_counter_class::TERRAIN_POLYS);            
  //g1_stat_counter.increment(g1_statistics_counter_class::TOTAL_POLYS);
  num_terrain_polys++;

  i4_bool ret = i4_F;
  r1_render_api_class *api = g1_render.r_api;
  i4_float size=0;
  sw32 num_poly_verts = 3;
  r1_vert *clipped_poly, *v;
  w16 *clipped_refs;
  static w16 clipping_refs[16] = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15 };
  w16 refs[3];
  i4_float near_w=0.0001f;      
  int j; //i

  refs[0] = a;
  refs[1] = b;
  refs[2] = c;
  clipped_poly = points;
  clipped_refs = &refs[0];

  // hacked backface culling (should do this in worldspace before projection...)
  i4_3d_vector vt1(clipped_poly[a].v),vt2(clipped_poly[c].v),normal;

  vt1 -= clipped_poly[b].v;
  vt2 -= clipped_poly[b].v;
  normal.cross(vt2,vt1);
  if (normal.dot(clipped_poly[b].v)>0)
  {
    //pf_draw_tri.stop();
    return i4_F;
  }

  if ((clipped_poly[a].outcode | clipped_poly[b].outcode | clipped_poly[c].outcode)==0)
  {
    for (j=0; j<num_poly_verts; j++)
    {
      v = &clipped_poly[clipped_refs[j]];
      if (v->w > near_w)
        near_w=v->w;
    }
  }
  else
  {
    clipped_poly = api->clip_poly(&num_poly_verts,
                                  clipped_poly,
                                  clipped_refs,
                                  temp_buf_1,
                                  temp_buf_2,
                                  R1_CLIP_NO_CALC_OUTCODE);

    if (!clipped_poly || num_poly_verts<3)      
      num_poly_verts=0;

    if (num_poly_verts)      
    {
      clipped_refs = clipping_refs;

      for (j=0; j<num_poly_verts; j++)
      {
        v = &clipped_poly[j];

        float ooz = r1_ooz(v->v.z);            
          
        v->px = v->v.x*ooz*g1_render.center_x + g1_render.center_x;
        v->py = v->v.y*ooz*g1_render.center_y + g1_render.center_y;
        v->w  = ooz;                  
        if (v->w > near_w)
          near_w=v->w;
      }
    }
  }

  if (num_poly_verts)
  {
    if (texture)
    {
      sw32 texture_size=i4_f_to_i(g1_render.center_x * near_w * 0.5f * 0.5f);
      if ((texture!=last_texture)||(texture_size>last_texture_size))
        api->use_texture(texture, texture_size, 0);
      last_texture=texture;
      last_texture_size=texture_size;
    }

    if (g1_render.draw_mode==g1_render_class::WIREFRAME)
    {     
      api->set_constant_color(0x7f7f7f);
      api->disable_texture();
      api->set_shading_mode(R1_SHADE_DISABLED);

      r1_vert v[4];
      v[0]=clipped_poly[0];
      v[1]=clipped_poly[1];
      v[2]=clipped_poly[2];
      v[3]=clipped_poly[0];

      r1_clip_render_lines(3, v, g1_render.center_x, g1_render.center_y, api);
      api->set_shading_mode(R1_WHITE_SHADING);
    }
    else
      api->render_poly(num_poly_verts, clipped_poly, clipped_refs);

    ret = i4_T;
  }

  //pf_draw_tri.stop();
  return ret;
}

static void gather_objects(int x1, int y1, int x2, int y2)
{
  pf_gather_objects.start();
  int y,x;
  g1_map_cell_class *map_cell;
  g1_object_class *obj;
  g1_object_chain_class *o;
  for (y=y1; y<y2; y++)
  {
    map_cell=g1_get_cell(x1,y);

    for (x=x1; x<x2; x++, map_cell++)
    {
      // collect objects
      for (o=map_cell->get_obj_list(); o; o=o->next)
      {
        obj=o->object;
        
        if (!obj->get_flag(g1_object_class::SCRATCH_BIT))
        {
          //if (g1_num_objs_in_view<g1_max_objs_in_view) 
          //{              
          
            //g1_objs_in_view[g1_num_objs_in_view] = obj->global_id;
			if (obj->global_id)
                //otherwise they are "Managed"
				//meaning that somebody else (a "parent") is supposed
				//to draw them
				{
				g1_objs_in_view_dyn.add(obj);
				obj->world_transform = g1_obj_transforms_in_view.add();
          
				//g1_num_objs_in_view++;
          
				//have the object update his transform
				obj->calc_world_transform(g1_render.frame_ratio);          
				obj->set_flag(g1_object_class::SCRATCH_BIT, 1);
				}
			else
				{
				obj->world_transform=0;
				//someone who wants to draw this needs to do the following
				//obj->world_transform= new i4_transform_class();
				//obj->calc_world_transform(g1_render.frame_ratio)
				//obj->draw(context); //g1_draw_context_class *
				//delete obj->world_transform;
				//obj->world_transform=0;
				}
          //}   
        }
      }
    }
  }

  pf_gather_objects.stop();
}


static i4_transform_class comp_t;
int object_compare(const w32 *a, const w32 *b)
{
  //g1_object_class 
  //  *oa = g1_global_id.get(*((w32*)a)),
  //  *ob = g1_global_id.get(*((w32*)b));
  g1_object_class *oa,*ob;
  oa=g1_global_id.get(*a);
  ob=g1_global_id.get(*b);

  // sort objects with alpha last
  if (!oa)
	  return 1;
  else 
	  if (!ob)
		  return -1;
  if (oa->get_type()->get_flag(g1_object_definition_class::HAS_ALPHA))
  {
    if (!ob->get_type()->get_flag(g1_object_definition_class::HAS_ALPHA))
      return -1;
  }
  else if (ob->get_type()->get_flag(g1_object_definition_class::HAS_ALPHA))
    return 1;
    

  i4_float
    za = comp_t.x.z*oa->x + comp_t.y.z*oa->y + comp_t.z.z*oa->h + comp_t.t.z,
    zb = comp_t.x.z*ob->x + comp_t.y.z*ob->y + comp_t.z.z*ob->h + comp_t.t.z;

  if (za>zb)
    return 1;
  else
    return -1;
}

void g1_map_class::fast_draw_cells(g1_draw_context_class  *context)
{
  pf_map_fast.start();
  //This code is HIGHLY time critical
  //avoid unessesary stack operations be pre-declaring all variables
  //optimize for SPEEEEED.
  r1_render_api_class *api = g1_render.r_api;
  i4_transform_class t(*context->transform);
  //we corrently don't need this one. 
  //context->dtransform=new i4_dtransform_class(*context->transform);
  i4_3d_vector pos;


//   g1_lod.cont->view.get_camera_pos(pos);
  pos = g1_lod.pos;

  g1_objs_in_view_dyn.clear();
  g1_obj_transforms_in_view.clear();

  if (g1_render.draw_mode==g1_render_class::SOLID)
    api->use_texture(g1_default_texture.get(), 1, 0);
  
  int i=0,j,k,l;
  g1_map_vertex_class *vt[4];
  g1_map_vertex_class *v=verts;
  lod_node *p;
  int x1,y1,x2,y2,clip;
  g1_map_cell_class *map_cell;
  const float u_s[4]={0,1,1,0},v_s[4]={1,1,0,0};
  r1_texture_handle han=0;
  last_texture=0;
  last_texture_size=0;
  num_terrain_polys=0;
  sw32 mw_p1;
  i4_3d_vector mid;
  r1_vert poly[4];
  int uv_on,uv_dir;
  texture_context *tc=0;
  
  g1_object_class *o=0;
  li_class *old_this=li_this;
  comp_t = *context->transform;
  g1_lod.last_context=0;
  float scale_x=g1_render.scale_x;
  float scale_y=g1_render.scale_y;
  float center_x=g1_render.center_x;
  float center_y=g1_render.center_y;
  for (j=0; j<g1_lod.num_quads; j++)
  {
    p= g1_lod.quad[j];
    x1 = p->x1; y1 = p->y1;
    x2 = p->x2; y2 = p->y2;
    clip = !(p->flags & lod_node::IN_VIEW);

    mw_p1=width()+1;

    vt[0]=v + x1 + y1 * mw_p1;          
    vt[1]=v + x2 + y1 * mw_p1;          
    vt[2]=v + x2 + y2 * mw_p1;          
    vt[3]=v + x1 + y2 * mw_p1;          

    vt[0]->transform(t, x1, y1, scale_x,scale_y);
    vt[1]->transform(t, x2, y1, scale_x,scale_y);
    vt[2]->transform(t, x2, y2, scale_x,scale_y);
    vt[3]->transform(t, x1, y2, scale_x,scale_y);

    if (vt[0]->calc_clip_code()==0)
      vt[0]->project(center_x, center_y);
    if (vt[1]->calc_clip_code()==0)
      vt[1]->project(center_x, center_y);
    if (vt[2]->calc_clip_code()==0)
      vt[2]->project(center_x, center_y);
    if (vt[3]->calc_clip_code()==0)      
      vt[3]->project(center_x, center_y);

    // collect objects
    mid.set(i4_float(x2+x1)/2.0f,i4_float(y2+y1)/2.0f,i4_float(p->z2+p->z1)/2.0f);
    mid -= pos;


    if (mid.dot(mid) < g1_resources.lod_disappear_dist)
      gather_objects(x1,y1,x2,y2);
    //else
    //  i++;

    if (x1+1==x2 && y1+1==y2)
    {
      // single cell
      map_cell=cells + x1 + y1*width();

      g1_lod.last_context=0;
      han = g1_tile_man.get_texture(map_cell->type);//Where is this type field set on load?
      if (han && han!=g1_tile_man.get_pink())
      {
        
        uv_on=map_cell->get_rotation();
        uv_dir=map_cell->mirrored() ? 3 : 1;
        
        
        for (k=0; k<4; k++)
        {
          vt[k]->set_r1_vert(&poly[k]);
          poly[k].s=u_s[uv_on];
          poly[k].t=v_s[uv_on];
          poly[k].a=1.0;
          uv_on=(uv_on+uv_dir)&3;
        }
        vt[0]->get_rgb(poly[0].r, poly[0].g, poly[0].b, x1, y1);
        vt[1]->get_rgb(poly[1].r, poly[1].g, poly[1].b, x2, y1);
        vt[2]->get_rgb(poly[2].r, poly[2].g, poly[2].b, x2, y2);
        vt[3]->get_rgb(poly[3].r, poly[3].g, poly[3].b, x1, y2);
        
        if (post_cell_draw)
            {
            //this is a function pointer!
            post_cell_draw(x1,y1, post_cell_draw_context);
            //force the following g1_draw_tri to really set the texture
            //even if it was the same as last time (post_cell_draw might
            //change the current texture)
            last_texture_size=0;
            last_texture=0;
            }

        if (g1_draw_tri(poly, 0,1,2, han))
          han = 0;
        g1_draw_tri(poly, 0,2,3, han);
      }
    }
    else
    {
      // larger LOD cell

      tc = &g1_lod.map_texture[p->texture_context];
      

      if (g1_lod.last_context==p->texture_context)
        han = 0;
      else
        han = tc->texture;

      for (l=0; l<4; l++)
      {
        vt[l]->set_r1_vert(&poly[l]);
        poly[l].a=1.0;
      }
      vt[0]->get_rgb(poly[0].r, poly[0].g, poly[0].b, x1, y1);
      vt[1]->get_rgb(poly[1].r, poly[1].g, poly[1].b, x2, y1);
      vt[2]->get_rgb(poly[2].r, poly[2].g, poly[2].b, x2, y2);
      vt[3]->get_rgb(poly[3].r, poly[3].g, poly[3].b, x1, y2);

      poly[0].s=(x1 - tc->x1)*tc->sx;
      poly[0].t=(y1 - tc->y1)*tc->sy;
      poly[1].s=(x2 - tc->x1)*tc->sx;
      poly[1].t=(y1 - tc->y1)*tc->sy;
      poly[2].s=(x2 - tc->x1)*tc->sx;
      poly[2].t=(y2 - tc->y1)*tc->sy;
      poly[3].s=(x1 - tc->x1)*tc->sx;
      poly[3].t=(y2 - tc->y1)*tc->sy;

      if (g1_draw_tri(poly, 0,1,2, han))
        han = 0;
      if (g1_draw_tri(poly, 0,2,3, han))
        han = 0;
      if (han==0)
        g1_lod.last_context = p->texture_context;
    }
  }

  //for (i=0; i<(int)g1_num_objs_in_view; i++)
  //{
  //  o =g1_global_id.checked_get(g1_objs_in_view[i]);
  //  if (o)
  //    o->set_flag(g1_object_class::SCRATCH_BIT, 0);
  //}

  //draw the objects BACK TO FRONT
  
  //qsort(g1_objs_in_view, g1_num_objs_in_view, sizeof(g1_objs_in_view[0]), object_compare);

  //TEST: Disabled for performance tests.
  //We have a z-buffer, so all we really need to do is ensure
  //objects with alpha get drawn last.
  //this can be accomplished much faster than with qsort,
  //because we only need to partition the set which is O(n), not O(n*log(n))
  //This actually corresponds to doing one iteration of qsort.
  //g1_objs_in_view_dyn.sort(object_compare);
  int last_non_alpha=g1_objs_in_view_dyn.size()-1;
  while (last_non_alpha>=0&&
      g1_objs_in_view_dyn[last_non_alpha]->get_type()->get_flag(g1_object_definition_class::HAS_ALPHA))
      {
      last_non_alpha--;
      }
  //this will iterate zero times in the rare cases ALL visible objects
  //have alpha.
  //this solution is not entirelly correct if multiple alpha objects overlap
  g1_object_class *temp;
  for (i=0;i<last_non_alpha;i++)
      {
      if (g1_objs_in_view_dyn[i]->get_type()->get_flag(g1_object_definition_class::HAS_ALPHA))
          {
          temp=g1_objs_in_view_dyn[i];
          g1_objs_in_view_dyn[i]=g1_objs_in_view_dyn[last_non_alpha];
          g1_objs_in_view_dyn[last_non_alpha]=temp;
          last_non_alpha--;
          }
      }
  
	  //Finally, draw all objects in view. 
  for (i=0;i<g1_objs_in_view_dyn.size();i++)
  {
    o=g1_objs_in_view_dyn[i];
    if (o)
    {
      li_this=o->vars;
	  //reset the scratch bit (was set to indicate the object is already in the list)
      o->set_flag(g1_object_class::SCRATCH_BIT, 0);
      if (o->world_transform!=0)
		  //pass the draw context (window settings) and the current position to the
		  //draw function, for clipping. 
        o->draw(context,pos); 
    }
  }

  if (context->draw_editor_stuff)
  {
    for (i=g1_objs_in_view_dyn.size()-1;i>=0;i--)
    {
      o=g1_objs_in_view_dyn[i];
      if (o)
      {
        li_this=o->vars;
        o->editor_draw(context);
      }
    }
  }  
  
  li_this=old_this;
    
  for (j=0; j<g1_lod.num_quads; j++)
  {
    p = g1_lod.quad[j];
    x1 = p->x1;
	y1 = p->y1;
    x2 = p->x2;
	y2 = p->y2;

    mw_p1=width()+1;

    vt[0]=v + x1 + y1 * mw_p1;          
    vt[1]=v + x2 + y1 * mw_p1;          
    vt[2]=v + x2 + y2 * mw_p1;          
    vt[3]=v + x1 + y2 * mw_p1;          

    vt[0]->clear_calculations();
    vt[1]->clear_calculations();
    vt[2]->clear_calculations();
    vt[3]->clear_calculations();
  }

  g1_stat_counter.increment(g1_statistics_counter_class::FRAMES);
  g1_stat_counter.add(g1_statistics_counter_class::TERRAIN_POLYS,
      num_terrain_polys);
  g1_stat_counter.add(g1_statistics_counter_class::TOTAL_POLYS,
      num_terrain_polys);
  //delete context->dtransform;
  //context->dtransform=0;

  pf_map_fast.stop();
}


class num_in_view_reset_class : public g1_global_id_reset_notifier
{
public:
  virtual void reset() 
  {
    //g1_num_objs_in_view=0;   
	g1_objs_in_view_dyn.clear();
    g1_obj_transforms_in_view.clear();
  }
} num_in_view_reset_inst;
