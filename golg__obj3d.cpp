/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/
#include "pch.h"
#include <string.h>
#include "obj3d.h"
#include "octree.h"
#include "loaders/dir_load.h"
#include "checksum/checksum.h"
#include "string/str_checksum.h"
#include "saver_id.h"
#include "time/profile.h"

//#ifdef i4_NEW_CHECK
//#undef new
//#endif

g1_quad_object_class::~g1_quad_object_class()
	{
	delete octree;
	octree=0;
	}

void g1_quad_object_class::scale(i4_float value)
{
  for (int j=0; j<num_animations; j++)
  {
    g1_vert_class *p=animation[j].vertex;
    for (int i=0; i<animation[j].num_frames; i++)
    {
      g1_vert_class *p=get_verts(j,i);
      for (int k=0; k<num_vertex; k++)
        p[k].v*=value;
    }
  }
    
  for (w32 a=0; a<num_quad; a++)
    quad[a].texture_scale *= value;

  for (w32 m=0; m<num_mounts; m++)
    mount[m] *= value;

  if (octree)
	  {
	  octree->scale(value);
	  }

  //extent *= value;
  calc_extents();
  //if (extent<0.8) extend*=2;//cheat a bit.
}

void g1_quad_object_class::translate(i4_float xadd, i4_float yadd, i4_float zadd)
{
  for (int j=0; j<num_animations; j++)
  {
    g1_vert_class *p=animation[j].vertex;

    for (int i=0; i<animation[j].num_frames; i++)
    {
      g1_vert_class *p=get_verts(j,i);
      for (int k=0; k<num_vertex; k++)
        p[k].v += i4_3d_point_class(xadd, yadd, zadd);
    }      
  }
}

void g1_quad_object_class::calc_extents()
{
  int i,j,k;

  i4_float minx, maxx, miny, maxy, minz, maxz;

  extent=0.0;

  minx=miny=minz=9999;
  maxx=maxy=maxz=-9999;
  
  for(i=0;i<num_animations;i++)
  {
    for(j=0;j<animation[i].num_frames;j++)
    {
      for(k=j*num_vertex;k<(j*num_vertex)+num_vertex;k++)
      {
        if(animation[i].vertex[k].v.length()>extent) extent=animation[i].vertex[k].v.length();
        if(animation[i].vertex[k].v.x < minx) minx=animation[i].vertex[k].v.x;
        if(animation[i].vertex[k].v.y < miny) miny=animation[i].vertex[k].v.y;
        if(animation[i].vertex[k].v.z < minz) minz=animation[i].vertex[k].v.z;
        if(animation[i].vertex[k].v.x > maxx) maxx=animation[i].vertex[k].v.x;
        if(animation[i].vertex[k].v.y > maxy) maxy=animation[i].vertex[k].v.y;
        if(animation[i].vertex[k].v.z > maxz) maxz=animation[i].vertex[k].v.z;
      }
    }
  }
}

void g1_quad_object_class::update(i4_float time)
{
  for (int i=0; i<num_special; i++)
  {
    g1_texture_animation *t = &special[i];
    g1_quad_class *q = &quad[t->quad_number];
    int max_frames = t->max_frames;

    if (max_frames==0)
    {
      // panning

      i4_float fr, du,dv;
      
      fr = time*t->speed;
      fr -= i4_float(int(fr));
      du = t->du*fr;
      dv = t->dv*fr;

      for (int n=0; n<(int)q->num_verts(); n++)
      {
        q->u[n] = t->u[n] + du;
        q->v[n] = t->v[n] + dv;
      }
    }
    else
    {
      // animation
      int frame = int(time*t->speed)%max_frames;

      int x = frame%t->frames_x;
      int y = frame/t->frames_x;

      for (int n=0; n<(int)q->num_verts(); n++)
      {
        q->u[n] = t->u[n] + t->du*x;
        q->v[n] = t->v[n] + t->dv*y;
      }
    }
  }
}

i4_bool g1_quad_object_class::intersect(const i4_3d_vector &point,
                                        const i4_3d_vector &ray,
                                        int anim, int frame,
                                        i4_float *intersect_t,
                                        int *hit_poly,
                                        i4_3d_vector *normal)
    {
    g1_vert_class *verts;
    verts = get_verts(anim, frame);
    
    g1_quad_class *q = quad;
    i4_bool hit = i4_F;
    i4_float new_t = 1.0;
    i4_float d,num,den,t;
    i4_float 
            dx1, dy1, dx2,dy2;
    int 
            x_sign1, y_sign1, x_sign2, y_sign2;
    int inside;
    i4_3d_vector new_ray,new_point,abs_normal;
    int j, u,v,i;
    g1_octree *curnode;
    g1_octree *nodes[3];
    if (octree)
        {
        new_point=point;
        new_point+=ray;
        //although not perfectly true, we assume that
        //the ray doesn't pass any other nodes than the source
        //and the target.
        nodes[2]=0;
        nodes[0]=octree->GetLeafAt(point);
        nodes[1]=octree->GetLeafAt(new_point);
        if (nodes[0]==nodes[1])
            nodes[1]=NULL;
        int the_cur_node=0;
        curnode=nodes[the_cur_node];
        i=0;
        while (curnode)
            {
            q=curnode->GetQuad(i);
            den = q->normal.dot(ray);
            if (den>=0)
                // wrong direction, next quad
                goto next_loop;
        
            // find intersection point in polygon's plane
            d = q->normal.dot(verts[q->vertex_ref[0]].v);
            num = d - q->normal.dot(point);
        
            //originally, the line t=num/den was before the following
            //line and we tested for t<0, but since we now here that
            //den<0, t can only become negative if num is positive. 
            if (num>0) 
                // behind the observer, next
                goto next_loop;
        
            t = num/den;
            if (t>=new_t)
                // check if new intersection is closer than previous ones
                goto next_loop;
        
            // project point into plane
        
            new_ray=ray;
            new_ray *= t;
            new_point=point;
            new_point += new_ray;
        
            // determine orthogonal plane for 2D point in polygon test
        
            abs_normal.set((float)fabs(q->normal.x), (float)fabs(q->normal.y), (float)fabs(q->normal.z));
        
            if (abs_normal.x>abs_normal.y && abs_normal.x>abs_normal.z)
                {
                // test in yz plane
                u = 1; v = 2;
                }
            else if (abs_normal.y>abs_normal.x && abs_normal.y>abs_normal.z)
                {
                // test in zx plane
                u = 2; v = 0;
                }
            else
                {
                // test in xy plane
                u = 0; v = 1;
                }
        
            // count first axis crossings of polygon to determine whether point in polygon
        
            dx2 = verts[q->vertex_ref[0]].v[u] - new_point[u],
            dy2 = verts[q->vertex_ref[0]].v[v] - new_point[v];
        
        
            x_sign2 = dx2>0,
            y_sign2 = dy2>0;
        
            inside=0;
        
            for (j=q->num_verts()-1; j>=0; j--)
                {
                dx1 = verts[q->vertex_ref[j]].v[u] - new_point[u];
                dy1 = verts[q->vertex_ref[j]].v[v] - new_point[v];
                x_sign1 = dx1>0;
                y_sign1 = dy1>0;
                if (y_sign1 != y_sign2)
                    {
                    // crosses x-axis
                    if (x_sign1 != x_sign2)
                        {
                        // check if crossing point is positive ((slope2>slope1) ^ (y2>y1))
                        if ((dx2*dy1 > dx1*dy2) ^ (dy2>dy1))
                            inside = !inside;
                        }
                    else if (x_sign1)
                        // crosses +x-axis
                        inside = !inside;
                    y_sign2 = y_sign1;
                    }
                x_sign2 = x_sign1;
                dx2 = dx1;
                dy2 = dy1;
                }
        
            if (inside)
                {
                // update current intersection
                new_t = t;
                if (hit_poly) 
                    *hit_poly=i;
                if (normal) 
                    *normal=q->normal;
                hit = i4_T;
                }
next_loop:
            ++i;
            if (i>=curnode->GetNumQuads())
                {
                i=0;
                //restart with the quads from the other node.
                ++the_cur_node;
                curnode=nodes[the_cur_node];
                }
            }
        if (intersect_t) 
            *intersect_t=new_t;
        return hit;
        }

    else //not octree bellow
        {
        for (i=0; i<num_quad; i++, q++)
            {
            den = q->normal.dot(ray);
            if (den>=0)
                // wrong direction, next quad
                continue;
        
            // find intersection point in polygon's plane
            d = q->normal.dot(verts[q->vertex_ref[0]].v);
            num = d - q->normal.dot(point);
        
            //originally, the line t=num/den was before the following
            //line and we tested for t<0, but since we now here that
            //den>=0, t can only become negative if num is positive. 
            //PG: Yuk, that gave me quite a shock... Thought I had
            //introduced a very hard to find collision bug when I
            //noted that I had just used the wrong comparison here.
            //Change this to (num<0) and see how far tanks can fly ;) 
            if (num>0) 
                // behind the observer, next
                continue;
        
            t = num/den;
            if (t>=new_t)
                // check if new intersection is closer than previous ones
                continue;
        
            // project point into plane
        
            new_ray=ray;
            new_ray *= t;
            new_point=point;
            new_point += new_ray;
        
            // determine orthogonal plane for 2D point in polygon test
        
            abs_normal.set((float)fabs(q->normal.x), (float)fabs(q->normal.y), (float)fabs(q->normal.z));
        
            if (abs_normal.x>abs_normal.y && abs_normal.x>abs_normal.z)
                {
                // test in yz plane
                u = 1; v = 2;
                }
            else if (abs_normal.y>abs_normal.x && abs_normal.y>abs_normal.z)
                {
                // test in zx plane
                u = 2; v = 0;
                }
            else
                {
                // test in xy plane
                u = 0; v = 1;
                }
        
            // count first axis crossings of polygon to determine whether point in polygon
        
            dx2 = verts[q->vertex_ref[0]].v[u] - new_point[u],
            dy2 = verts[q->vertex_ref[0]].v[v] - new_point[v];
        
        
            x_sign2 = dx2>0,
            y_sign2 = dy2>0;
        
            inside=0;
        
            for (j=q->num_verts()-1; j>=0; j--)
                {
                dx1 = verts[q->vertex_ref[j]].v[u] - new_point[u];
                dy1 = verts[q->vertex_ref[j]].v[v] - new_point[v];
                x_sign1 = dx1>0;
                y_sign1 = dy1>0;
                if (y_sign1 != y_sign2)
                    {
                    // crosses x-axis
                    if (x_sign1 != x_sign2)
                        {
                        // check if crossing point is positive ((slope2>slope1) ^ (y2>y1))
                        if ((dx2*dy1 > dx1*dy2) ^ (dy2>dy1))
                            inside = !inside;
                        }
                    else if (x_sign1)
                        // crosses +x-axis
                        inside = !inside;
                    y_sign2 = y_sign1;
                    }
                x_sign2 = x_sign1;
                dx2 = dx1;
                dy2 = dy1;
                }
        
            if (inside)
                {
                // update current intersection
                new_t = t;
                if (hit_poly) 
                    *hit_poly=i;
                if (normal) 
                    *normal=q->normal;
                hit = i4_T;
                }
            }
        if (intersect_t) 
            *intersect_t=new_t;
        return hit;

        } //end of else not octree

}

i4_bool g1_quad_object_class::get_mount_point(char *name, i4_3d_vector& vect) const
{
  w32 id = i4_check_sum32(name, strlen(name));

  for (int i=0; i<num_mounts; i++)
    if (mount_id[i] == id)
    {
      vect = mount[i];
      return i4_T;
    }
  return i4_F;
}


static i4_profile_class pf_load_tnames("models:load_tname");
static i4_profile_class pf_load_quads("models:load_quads");
static i4_profile_class pf_load_verts("models:load_verts");
static i4_profile_class pf_load_mounts("models:load_mounts");
static i4_profile_class pf_load_tanim("models:load_tanim");



g1_quad_object_class *g1_base_object_loader_class::load(i4_loader_class *fp)
{
  int i,j,k;
  
  if (!fp) 
    return 0;

  if (!fp->goto_section(G1_SECTION_MODEL_TEXTURE_NAMES))
    return 0;

  obj=allocate_object();

  pf_load_tnames.start();
  w16 quads=fp->read_16();
  set_num_quads(quads);

  char name[256];
  for (i=0; i<quads; i++)
  {    
    w16 len = fp->read_16();
    fp->read(name, len);
    name[len]=0;
    store_texture_name(i, name);
  }  
  pf_load_tnames.stop();

  if (!fp->goto_section(G1_SECTION_MODEL_QUADS))
    return 0;

  pf_load_quads.start();
  fp->read_16();                // repeat of num quads (why?)
  w16 highest_ref=0;
  for (i=0; i<quads; i++)
  {
    w16 ref[4];
    i4_float u[4],v[4], scale;
    i4_3d_vector normal;
    w32 flags;

    for (j=0; j<4; ++j)
    {
      ref[j]=fp->read_16();
	  if (ref[j]!=0xffff)
		  {
		  // Check validity of references (the actual number of vertices in the file is stored
		  //further down. 
		  highest_ref=ref[j];
		  }
	  if ((ref[j]==0xffff) && j!=3)
		  {
		  i4_error("ERROR: g1_base_object_loader_class::load: One face of this object has less than 3 vertices assigned.");
		  delete obj;//will this work??
		  return 0;
		  }
      u[j]=fp->read_float();
      v[j]=fp->read_float();
    }

    scale=fp->read_float();
    flags=fp->read_16();

    normal.x=fp->read_float();
    normal.y=fp->read_float();
    normal.z=fp->read_float();     

    create_quad(i, 4, ref, flags);
    store_texture_params(i, scale, u, v);
    store_quad_normal(i, normal);
  }
  pf_load_quads.stop();

  if (!fp->goto_section(G1_SECTION_MODEL_VERT_ANIMATION))
    return 0;
  
  pf_load_verts.start();
  w16 nv=fp->read_16();  // number of vertexes in object
  if (nv<highest_ref)
  {
	  if (i4_error("WARNING: g1_base_object_loader_class::load: Reference to high vertex number found. GMOD file damaged?")==2)//IDCANCEL
		  return 0;
  }
  w16 na=fp->read_16();  // number of animations

  set_num_vertex(nv);
  set_num_animations(na);

  for (i=0; i<na; i++)
  {
    int len=fp->read_16();
    fp->read(name, len);
    name[len]=0;

    w16 frames = fp->read_16();

    create_animation(i, name, frames);

    for (j=0; j<frames; j++)
    {
      for (k=0; k<nv; k++)
      {
        i4_3d_vector v,n;

        v.x=fp->read_float();
        v.y=fp->read_float();
        v.z=fp->read_float();

        n.x=fp->read_float();
        n.y=fp->read_float();
        n.z=fp->read_float();

        create_vertex(i,j,k,v);
        store_vertex_normal(i,j,k,n);
      }
    }
  }  
  pf_load_verts.stop();

  // optional mount points
  if (fp->goto_section("GMOD Mounts"))
  {
    pf_load_mounts.start();
  
    w16 mounts = fp->read_16();
    set_num_mount_points(mounts);
    
    for (i=0; i<mounts; i++)
    {
      i4_3d_vector v;
      
      int len=fp->read_16();
      fp->read(name, len);
      name[len]=0;

      v.x = fp->read_float();
      v.y = fp->read_float();
      v.z = fp->read_float();
      
      create_mount_point(i,name,v);
    }
    pf_load_mounts.stop();
  
  }

  // optional mount points
  if (fp->goto_section("GMOD Texture Animations"))
  {
    pf_load_tanim.start();
    w16 anims = fp->read_16();
    set_num_texture_animations(anims);
    
    for (i=0; i<anims; i++)
    {
      w16 q;
      w8 maxf, frx;
      i4_float sp,du,dv;
      
      fp->read_format("211fff",&q,&maxf,&frx,&sp,&du,&dv);
      
      if (maxf>0)
        create_texture_animation(i,q,maxf,frx,du,dv,sp);
      else
        create_texture_pan(i,q,du,dv,sp);
    }
    pf_load_tanim.stop();
  }
  
  //BSPCODE
  /*
  //Here comes the code that loads the bsp tree from the file.
  //do something like: 

  //The following string can be anything you like but be shure that:
  //1. It must be unique
  //2. It must be EXACTLY the same as used when saving the file
  //3. It MUST NOT be changed ever after
  if (fp->goto_section("Embedded GMOD BSP Tree")
	{//if the above check fails, no bsp tree is stored in this file
	//load up your data
	if (!fp->check_version(BSP_TREE_VERSION))
		return;//wrong version found -> ignore
	//... here comes the actual loading code
	fp->end_version(I4_LF);
	}
  */
  if (fp->goto_section("Embedded GMOD OCTANT Tree"))
	  {
	  if (fp->check_version(1))
		  {
		  obj->octree=new g1_octree(obj,fp);
		  fp->end_version(I4_LF);
		  }
	  }

  finish_object();

  return obj;
}


