/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/
#include "pch.h"
#include "maxtool/max_object.h"
#include "saver_id.h"
#include "loaders/dir_save.h"
#include "checksum/checksum.h"
#include "string/str_checksum.h"
#include "octree.h"
//#include "gui/smp_dial.h"

// Save section

/*Kleine Hilfe für diejenigen, die nicht besonders gut englisch sprechen
Little Help for translation from english to german

  German expressions	English expressions
  Ecke, Knoten			Vertex, Point
  Kante                 Edge
  Gewicht				Weight
  Fläche, Vierreck		Quad[rilateral] (May also have 3 corners here)

*/



void m1_poly_object_class::save_quads(i4_saver_class *fp)
//{{{
{
  int i;
  w32 j;

  fp->mark_section(G1_SECTION_MODEL_QUADS);

  int tq=num_quad;

  fp->write_16(tq);
  for (i=0; i<tq; i++)
  {
    for (j=0; j<quad[i].max_verts(); ++j)
    {
      fp->write_16(quad[i].vertex_ref[j]);
      fp->write_float(quad[i].u[j]);
      fp->write_float(quad[i].v[j]);
    }
    fp->write_float(quad[i].texture_scale);
    fp->write_16((w16)quad[i].flags);

    fp->write_float(quad[i].normal.x);
    fp->write_float(quad[i].normal.y);
    fp->write_float(quad[i].normal.z);
  }
}
//}}}

void m1_poly_object_class::save_texture_names(i4_saver_class *fp)
//{{{
{
  fp->mark_section(G1_SECTION_MODEL_TEXTURE_NAMES);
  int tq=num_quad;
  fp->write_16(tq);
  for (int i=0; i<tq; i++)
    fp->write_counted_str(*texture_names[i]);
}
//}}}

void m1_poly_object_class::save_vert_animations(i4_saver_class *fp)
//{{{
{
  int i,j,k;

  fp->mark_section(G1_SECTION_MODEL_VERT_ANIMATION);

  fp->write_16(num_vertex);

  int ta=num_animations;

  fp->write_16(ta);
  for (i=0; i<ta; i++)
  {
    fp->write_counted_str(*animation_names[i]);
    int nf=animation[i].num_frames;
    fp->write_16(nf);

    for (j=0; j<nf; j++)
    {
      g1_vert_class *v=get_verts(i,j);

      for (k=0; k<num_vertex; k++)        
      {
        fp->write_float(v[k].v.x);
        fp->write_float(v[k].v.y);
        fp->write_float(v[k].v.z);

        fp->write_float(v[k].normal.x);
        fp->write_float(v[k].normal.y);
        fp->write_float(v[k].normal.z);
      }
    }
  }
}
//}}}

void m1_poly_object_class::save_mount_points(i4_saver_class *fp)
//{{{
{
  int i;

  if (num_mounts==0)
    return;

  fp->mark_section("GMOD Mounts");

  fp->write_16(num_mounts);
  for (i=0; i<num_mounts; i++)
  {
    fp->write_counted_str(*mount_names[i]);
    
    fp->write_float(mount[i].x);
    fp->write_float(mount[i].y);
    fp->write_float(mount[i].z);
  }
}
//}}}

void m1_poly_object_class::save_specials(i4_saver_class *fp)
//{{{
{
  int i;

  if (num_special==0)
    return;

  fp->mark_section("GMOD Texture Animations");

  fp->write_16(num_special);
  for (i=0; i<num_special; i++)
  {
    fp->write_format("211fff",
                     &special[i].quad_number,
                     &special[i].max_frames,
                     &special[i].frames_x,
                     &special[i].speed,
                     &special[i].du,
                     &special[i].dv);
  }
}
//}}}

void m1_poly_object_class::calc_quad_normal(g1_vert_class *v,
                                            g1_quad_class &q)
//{{{
{
  // find the surface normal used in lighting
  i4_3d_point_class p[4];

  for (w32 i=0; i<q.num_verts();  i++)
    p[i]=v[q.vertex_ref[i]].v;

  p[1] -= p[0];
  p[2] -= p[0];

  q.set_flags(g1_quad_class::INVALID_QUAD,0);
  if (q.num_verts()==3)
    q.normal.cross(p[1], p[2]);
  else
  {
    i4_3d_vector normal1, normal2;

    p[3] -= p[0];
    normal1.cross(p[1], p[2]);
    normal2.cross(p[2], p[3]);

    i4_float len1 = normal1.length();
    i4_float len2 = normal2.length();

    if (len1>len2)
      q.normal = normal1;
    else
      q.normal = normal2;

    if (len1==0.0 || len2==0.0)
      // bad normal lengths
      q.set_flags(g1_quad_class::INVALID_QUAD);
    else
    {
      normal1 /= len1;
      normal2 /= len2;

      if (normal1.dot(normal2)<0.5)
        q.set_flags(g1_quad_class::INVALID_QUAD);
    }
  }

  if (q.normal.x==0 && q.normal.y==0 && q.normal.z==0)
  {
    // 0 check - invalid polygon!
    q.normal=i4_3d_vector(0,0,1);
    q.set_flags(g1_quad_class::INVALID_QUAD);
    // i4_warning("very invalid polygon detected!");
  }
  else
    q.normal.normalize();
}
//}}}

void m1_poly_object_class::calc_vert_normals()
//{{{
{
  for (int a=0; a<num_animations; a++)
  {
    // calculate normal for each face
    for (w32 i=0; i<num_quad; i++)
      calc_quad_normal(get_verts(0,0), quad[i]);

    for (w32 v=0; v<num_vertex; v++)
    {
      w32 t=0;
      i4_3d_vector sum=i4_3d_vector(0,0,0);

      for (w32 j=0; j<num_quad; j++)
      {             
        if (quad[j].vertex_ref[0]==v ||
            quad[j].vertex_ref[1]==v ||
            quad[j].vertex_ref[2]==v ||
            quad[j].vertex_ref[3]==v)
        {
          t++;
          sum+=quad[j].normal;
        }      
      }

      if (sum.x==0 && sum.y==0 && sum.z==0)
        sum.set(0,0,1);
      else
        sum.normalize();
      (get_verts(a,0)+v)->normal=sum;
    }
  }
}
//}}}

//BSPCODE
//To save your bsp-tree, do something like
/*
void m1_poly_object_class::save_bsp_tree(i4_saver_class *fp)
{
  if (!bsp)
    return;
  fp->mark_section("Embedded GMOD BSP Tree");
  fp->start_version(BSP_TREE_VERSION);
  //... here comes your code to save the tree.
  fp->end_version();
}
*/

void m1_poly_object_class::save_octree(i4_saver_class *fp)
	{
	if (!octree)
		return;
	fp->mark_section("Embedded GMOD OCTANT Tree");
	fp->start_version(1);
	octree->save(fp);
	fp->end_version();
	}

void m1_poly_object_class::save(i4_saver_class *fp)
//{{{
{
  //int i,j;
  calc_vert_normals();

  save_texture_names(fp);
  save_quads(fp);
  save_vert_animations(fp);
  save_mount_points(fp);
  save_specials(fp);
  //BSPCODE
  //here is the right place to call your new function to save a bsp tree
  //save_bsp_tree(fp);
  save_octree(fp);
}
//}}}

w32 m1_poly_object_class::add_anim(i4_str &name)
	{
	I4_ASSERT(num_animations>0,"Must always have at least one animation");
	
	animation_class ani;
	g1_vert_class *v;
	backup_references();
	v=vert_store.add_many(num_vertex);
	restore_references();
	num_animations++;
	ani.num_frames=1;
	ani.vertex=v;
	anim_store.add(ani);
	animation= &anim_store[0];
	//animation[num_animations-1].vertex=ani.vertex;
	//animation[num_animations-1].num_frames=1;
	for (int i=0;i<num_vertex;i++)
		{
		anim_store[anim_store.size()-1].vertex[i]=anim_store[0].vertex[i];//copy first frame of first anim
		}
	i4_str *n=new i4_str(name);//this should get deleted on destroy
	animation_names.add(n);
	i4_warning("Added a new animation");
	return num_animations-1;
	}

void m1_poly_object_class::remove_anim(w32 num)
	{
	if (num_animations==1||num==0) return;
	int offset=animation[num].vertex-&vert_store[0];
	int j=offset;
	int len=(num_vertex*animation[num].num_frames);
	for (;j<offset+len;j++)
		{
		vert_store.remove(j);
		}
	for (j=num+1;j<num_animations;j++)
		{
		anim_store[num].vertex-=len;
		}
	anim_store.remove(num);
	delete animation_names[num];
	animation_names.remove(num);
	num_animations--;
	i4_warning("Removed animation %i",num);
	}

void m1_poly_object_class::change_animation_name(w32 animation_number, const i4_const_str &st)
	{
	delete animation_names[animation_number];
	i4_str *n=new i4_str(st);
	animation_names[animation_number]=n;
	}

void m1_poly_object_class::change_mount_name(w32 mount_number, const i4_const_str &st)
	{
	delete mount_names[mount_number];
	i4_str *n=new i4_str(st);
	mount_names[mount_number]=n;
	mount_id[mount_number]=i4_str_checksum(st);
	}

void m1_poly_object_class::change_mount(w32 mount_number, i4_3d_vector v)
	{
	I4_ASSERT(mount_number<(w32)mount_store.size(),"INTERNAL: Invalid mount number.");
	mount_store[mount_number]=v;
	}

void m1_poly_object_class::backup_references()
	{
	if (vert_store.size()==0) 
		{
		vert_backup=0;
		return;
		}
	vert_backup= &vert_store[0];
	vert_offsets.clear();
	for (int i=0;i<num_animations;i++)
		{
		w32 of=animation[i].vertex-vert_backup;
		vert_offsets.add(of);
		}
	}

void m1_poly_object_class::restore_references()
	{
	g1_vert_class *v= &vert_store[0];
	if (vert_backup&&v!=vert_backup)//has something changed?
		{
		for (int i=0;i<num_animations;i++)
			{
			w32 of=vert_offsets[i];
			animation[i].vertex=v+of;
			}
		}
	}

void m1_poly_object_class::add_frame(w32 anim, w32 frame)
//{{{
//insert a frame after location "frame" into animation "anim" of current object
	{
	//i4_error("write me!");
	animation[anim].num_frames++;//increase frame count
	backup_references();
	vert_store.add_many(num_vertex);//add place for one frame of vertices
	restore_references();
	for(w32 aa=num_animations-1;aa>anim;aa--)
		{
		animation[aa].vertex+=num_vertex;//adjust vertex pointers
		//not including the own.
		}
		
		//for(int kk=animation[aa].num_frames-1;kk>frame;kk--)
		//	{
			
			//copy over the previous frame to the current frame
			w32 copy_from_vertex;
			//w32 copy_to_frame;
			w32 end=animation[anim].vertex- &vert_store[0];
			end+=num_vertex*(frame+1);//last vertex written
			for(w32 i=vert_store.size()-1;i>=end;i--)
				{
				copy_from_vertex=i-num_vertex;
				vert_store[i].v.x=
					vert_store[copy_from_vertex].v.x;
				vert_store[i].v.y=
					vert_store[copy_from_vertex].v.y;
				vert_store[i].v.z=
					vert_store[copy_from_vertex].v.z;
				vert_store[i].normal.x=
					vert_store[copy_from_vertex].normal.x;
				vert_store[i].normal.y=
					vert_store[copy_from_vertex].normal.y;
				vert_store[i].normal.z=
					vert_store[copy_from_vertex].normal.z;
				}
			//}
		//}
	i4_warning("Added a new Frame to animation %i.",anim);
	}
//}}}

void m1_poly_object_class::remove_frame(w32 anim, w32 frame)
//{{{
{//Hint: Need to check wheter vertex numbers are relative to the current frame or relative to animation
  //i4_error("write me!");
  if (animation[anim].num_frames==1)
	  {
	  i4_error("USER: remove_frame: Cannot remove last frame of animation. Delete animation instead");
	  return;
	  }
  
  w32 offset=frame*num_vertex;
  for (int i=0;i<num_vertex;i++)
	  {
	  vert_store.remove(offset);//automatically copies contents
	  }
  animation[anim].num_frames--;
  i4_warning("Removed frame %i from animation %i.",frame,anim);
}
//}}}

w32 m1_poly_object_class::add_vertex()
//{{{
{
  //i4_error("write me!");
  backup_references();
  g1_vert_class *v=vert_store.add();//add one extra element for safety
  restore_references();
  //add_at will otherwise fail on adding something at the end
  num_vertex++;
  for (int aa=0;aa<num_animations;aa++)
	  {
	  for(int ff=0;ff<animation[aa].num_frames;ff++)
		  {
		  int offset=animation[aa].vertex- &vert_store[0];
		  backup_references();
		  v=vert_store.add_at(offset+(((ff+1)*num_vertex)-1));
		  restore_references();
		  //add as last vertex of frame. (otherwise we'll need to change the quads)
		  v->v.x=0;v->v.y=0;v->v.z=0;
		  v->normal.x=0;v->normal.y=0;v->normal.z=1;//|normal|=0 ???

		  }
	  for(int bb=aa+1;bb<num_animations;bb++)//update references of other anims
		  {
		  animation[bb].vertex+=animation[aa].num_frames;
		  //added one vertex per frame
		  }
	  }
  vert_store.remove(vert_store.size()-1);//remove last element
  return num_vertex-1;//the index of the new vertex
}
//}}}
int remover_inverse_sorter(const int *a,const int *b)
	{
	if (*a==*b) return 0;
	if (*a>*b) return -1;//if a > b we want that a comes before b
	else return 1;
	}


void m1_poly_object_class::remove_vertex(w32 num)
//{{{
{
  //i4_error("write me!");
  g1_quad_class *q=0;
  i4_array<int> quad_remover(10,10);
  int i,k;
  w32 j;
  for (i=0;i<num_quad;i++)
	  {
      q= &quad_store[i];
	  for (j=0;j<q->num_verts();j++)
		  {
		  if (num==q->vertex_ref[j])
			  {
			  quad_remover.add(i);
			  break;
			  }
		  else if (q->vertex_ref[j]>num)
			  {
			  q->vertex_ref[j]--;//the vertices will be renumbered
			  }

		  }
	  }
  quad_remover.sort(&remover_inverse_sorter);
  for (k=0;k<quad_remover.size();k++)
	  {
	  remove_quad(quad_remover[k]);
	  }
  i=0;
  //g1_vert_class *v=vert_store.add();//add one extra element for safety
  num_vertex--;
  for (;i<num_animations;i++)
	  {
	  for(int ff=0;ff<animation[i].num_frames;ff++)
		  {
		  int offset=animation[i].vertex- &vert_store[0];
		  vert_store.remove(offset+num);//remove num'th vertex of frame
		  
		  }
	  for(int bb=i+1;bb<num_animations;bb++)
		  {
		  animation[bb].vertex-=animation[i].num_frames;
		  //removed one vertex per frame
		  }
	  }
  //vert_store.remove(vert_store.size()-1);//remove last element
  
}
//}}}

w32 m1_poly_object_class::add_quad()
//{{{
{
  g1_quad_class tmp;
  w32 ret=quad_store.add(tmp);
  texture_names.add(new i4_str(""));

  quad = &quad_store[0];
  num_quad = quad_store.size();

  return ret;
}
//}}}

void m1_poly_object_class::remove_quad(w32 num)
//{{{
{
  int i=0;

  for (i=num_special-1; i>=0; i--)
  {
    if (special[i].quad_number == num)
      remove_special(i);
    else if (special[i].quad_number > num)
      special[i].quad_number--;
  }
  
  quad_store.remove(num);
  texture_names.remove(num);
  num_quad = quad_store.size();
}
//}}}

w32 m1_poly_object_class::add_mount(i4_const_str &name)
{
  w32 ret=mount_store.add(i4_3d_vector(0,0,0));
  i4_str *n=new i4_str(name);
  mount_id_store.add(i4_check_sum32(n, name.length()));
  mount_names.add(n);

  mount = &mount_store[0];
  mount_id = &mount_id_store[0];
  num_mounts = mount_store.size();

  return ret;
}
w32 m1_poly_object_class::add_mount()
//{{{
{
  w32 ret=mount_store.add(i4_3d_vector(0,0,0));
  mount_id_store.add(0);
  mount_names.add(new i4_str(""));

  mount = &mount_store[0];
  mount_id = &mount_id_store[0];
  num_mounts = mount_store.size();

  return ret;
}
//}}}

i4_bool m1_poly_object_class::enum_animation_names(int index, i4_const_str &name)
	{
	if (index<animation_names.size())
		{
		name=*animation_names[index];
		}
	return (index<animation_names.size()-1);
	}

i4_bool m1_poly_object_class::enum_mount_names(int index, i4_const_str &name)
	{
	if (index<mount_names.size())
		{
		name=*mount_names[index];
		}
	return (index<mount_names.size()-1);
	}

void m1_poly_object_class::remove_mount(w32 num)
//{{{
{
  int i=0;

  mount_store.remove(num);
  mount_id_store.remove(num);
  delete mount_names[num];
  mount_names.remove(num);
  num_mounts = mount_store.size();
}
//}}}

w32 m1_poly_object_class::add_special(w32 quad_number)
//{{{
{
  for (int i=0; i<num_special; i++)
    if (special[i].quad_number==quad_number)
      return i;

  g1_texture_animation tmp;
  tmp.du=0;
  tmp.dv=0;
  tmp.frames_x=1;
  tmp.max_frames=1;
  tmp.quad_number=0;
  tmp.speed=0;
  w32 ret=special_store.add(tmp);

  special = &special_store[0];
  num_special = special_store.size();

  return ret;
}
//}}}

void m1_poly_object_class::remove_special(w32 num)
//{{{
{
  int i=0;

  special_store.remove(num);
  num_special = quad_store.size();
}
//}}}

void m1_poly_object_class::calc_texture_scales()
{
  g1_vert_class *src_vert = get_verts(0,0);

  for (int i=0; i<num_quad; i++)
  {
    i4_3d_point_class *p1=&src_vert[quad[i].vertex_ref[0]].v;
    i4_3d_point_class *p2=&src_vert[quad[i].vertex_ref[1]].v;


    double edge_len=sqrt((p1->z-p2->z)*(p1->z-p2->z)+
                        (p1->y-p2->y)*(p1->y-p2->y)+
                        (p1->x-p2->x)*(p1->x-p2->x));

    double uv_length = sqrt ((quad[i].u[1]-quad[i].u[0]) * (quad[i].u[1]-quad[i].u[0]) +
                            (quad[i].v[1]-quad[i].v[0]) * (quad[i].v[1]-quad[i].v[0]));
    

    quad[i].texture_scale=(float) (edge_len / uv_length);
      
  }

}
//BSPCODE
//This is a possible location to add the code to actually gernerate the bsp
//tree. You could also choose to use maxtool__m1_commands.cpp instead.

//{{{ Emacs Locals
// Local Variables:
// folded-file: t
// End:
//}}}
