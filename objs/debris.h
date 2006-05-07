/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef DEBRIS_HH
#define DEBRIS_HH

#include "g1_object.h"
#include "player.h"
#include "path.h"
#include "objs/model_draw.h"

class g1_solid_class;
class g1_map_piece_class;

class g1_debris_class : public g1_object_class
{
protected:
  enum { MAX_DEBRIS = 200 };
  i4_3d_vector debris[MAX_DEBRIS],ldebris[MAX_DEBRIS];
  void make_chunk(int i);
  int type;
  i4_float rad, strength;
public:
  enum
  {
    TORNADO = 0,
    RING_CHUNKS,
  };
  virtual i4_float occupancy_radius() const { return 0.7f; }

  g1_debris_class(g1_object_type id, g1_loader_class *fp);
  virtual void save(g1_saver_class *fp);
  void load(g1_loader_class *fp);
  void skipload(g1_loader_class *fp);

  virtual void setup(i4_float sx, i4_float sy, i4_float sz, int type);

  virtual void draw(g1_draw_context_class *context, i4_3d_vector& viewer_position);
  virtual void think();
};

#endif
