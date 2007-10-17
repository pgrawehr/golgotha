/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/

//! \file
//! This file contains declarations for the global collision detection functions.
//! These functions are globally used for collision detection. Any coldet calls inside the engine
//! should finally end in the implementations of these functions.

#ifndef G1_MODEL_COLLIDE_HH
#define G1_MODEL_COLLIDE_HH

#include "objs/model_draw.h"
#include "math/point.h"

//! Check for collisions.
//! This method checks wheter the object pointed to by \a source intersects with
//! the object \a _this if it would move by the distance \a ray from it's current location.
//! The function checks 8 points around \a source for collision, which may in some cases not be
//! very accurate (mainly in cases where the target object has a lot of small surfaces). Maybe
//! we once need to change this, but this would have a significant impact on the performance.
//! Internally, this function uses g1_model_collide_polygonal().
//! \param[in] _this The object agains which collision is checked (NOT the object which is moving)
//! \param[in] source The object which is attempting to move
//! \param[in,out] ray The distance the source is attempting to move. May be modified uppon return if
//! some movement is possible, but not the whole distance or not exactly in the direction desired.
//! \param[out] hit_normal The normal of the face we hit.
//! \param[out] minimum_t Nearest intersection point distance.
//! \return True if an object was hit (and ray was modified somehow)
i4_bool g1_model_collide_polygonal_ex(g1_object_class * _this,
									  g1_object_class * source,
									  i4_3d_vector &ray,
									  i4_3d_vector &hit_normal,
									  i4_float * minimum_t=0);

//! Check for ray-to-object intersections.
//! This function checks wheter the object \a _this described by
//! \a params is intersected by the \a ray starting at \a start.
//! \param[in] _this The object against which collision should be checked.
//! \param[in] params The model of _this (usually _this->draw_params).
//! \param[in] start The start position of the ray (usually the position of _this).
//! \param[in,out] ray The desired movement vector. Might have been changed to the possible
//! movement vector.
//! \param[out] hit_normal The normal of the face we hit.
//! \param[out] minimum_t Nearest intersection point distance.
//! \return True if an object was hit (and ray was modified somehow)
i4_bool g1_model_collide_polygonal(g1_object_class * _this,
								   g1_model_draw_parameters &params,
								   const i4_3d_vector &start,
								   i4_3d_vector &ray,
								   i4_3d_vector &hit_normal,
								   i4_float * minimum_t=0);

//! Check for ray-to-object intersection (the fast way).
//! This function checks wheter the \a ray starting at \a start will intersect with the bounding
//! sphere of the object described by \a params. The bounding sphere is defined by
//! g1_object_class::occupancy_radius() which gets it from the \a params.model object.
//! This function should only be used if the collision accuracy doesn't need to be very exact and
//! the target object is more-or-less round.
//! \param[in] _this The object against which collision should be checked.
//! \param[in] params The model of _this (usually _this->draw_params).
//! \param[in] start The origin of the ray (usually the position of _this).
//! \param[in,out] ray The desired movement vector. Might be changed by the function to something
//! that works without colliding.
//! \return True if an object was hit (and ray was modified somehow)
i4_bool g1_model_collide_radial(g1_object_class * _this,
								g1_model_draw_parameters &params,
								const i4_3d_vector &start,
								i4_3d_vector &ray);

#endif
