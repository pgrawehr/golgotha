/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef R1_RENDER_API_HH
#define R1_RENDER_API_HH

#include "render/tex_id.h"
#include "video/display.h"
#include "render/r1_vert.h"
#include "render/tnode.h"
#include "memory/array.h"
#include "render/tmanage.h"

#include "render/mip.h"

//! Defines the tint value for a quad.
//! Tints are used to modify the objects colors depending
//! on their owner.
typedef struct
{
  i4_float r,g,b;
} r1_color_tint;

//! A handle to a tint value.
typedef w32 r1_color_tint_handle;

//! Used to set the state of the alpha transparency texturing stage.
enum r1_alpha_type 
{ 
  R1_ALPHA_DISABLED,          //< Disable all alpha
  R1_ALPHA_CONSTANT,          //< Use the constant color's alpha 
  R1_ALPHA_LINEAR             //< Use the alpha specified in the verts
};

//! Constants used to declare how vertices are lighted. 
//! Remember that we are using gouraud shading most of the time, so
//! vertex colors define the tint and the lighting of a polygon. 
enum r1_shading_type
{
  R1_SHADE_DISABLED,          //< routines will not light
  R1_CONSTANT_SHADING,        //< routines will use the constant color
  R1_WHITE_SHADING,           //< routines will use only the red component (as white)
  R1_COLORED_SHADING          //< routines will use r,g, and b
};

//! Constants for the write mask.
//! This mask is used to define what clear_area does. 
//! The default value for this mask is 7.
enum
{
  R1_WRITE_COLOR     = 1,    // write pixels to frame buffer
  R1_WRITE_W         = 2,    // write w values to depth buffer
  R1_COMPARE_W       = 4     // if compare is off w then the depth buffer is not consulted
};
typedef w32 r1_write_mask_type;

//! Constants for the filter values.
//! The meaning is platform-dependent.
enum r1_filter_type
{
  R1_NO_FILTERING,
  R1_BILINEAR_FILTERING
};

//! Flags used mainly for the software renderer.
//! These flags can be used to reduce the resolution on very slow 
//! machines. 
enum r1_expand_type 
{ 
  R1_COPY_1x1,               //< 1 - 1 pixel copy  (render size = w,h)
  R1_COPY_2x2,               //< 2 - 1 pixel blow up (render size = w/2, h/2)
  R1_COPY_1x1_SCANLINE_SKIP  //< skips a scan line on each row (render size = w, h/2)
}; 

//! This flag should be used by software renderers in their type description field.
enum r1_render_flags
{
  R1_SOFTWARE=1
};

enum r1_feature_flags
{
  R1_SPANS=1,
  R1_PERSPECTIVE_CORRECT=2,
  R1_LOCK_CHEAT=4,
  R1_Z_BIAS=8, //everything drawn after this is turned on is more likely to be drawn 
               //in front of things drawn before it was turned on
  R1_ALL_FEATURES = R1_SPANS | R1_PERSPECTIVE_CORRECT | R1_LOCK_CHEAT | R1_Z_BIAS
};

//Don't use empty class declarations - Will only give trouble
//class r1_last_node;
//class r1_texture_manager_class;
//class r1_miplevel_t;

//class r1_texture_manager;
//class g1_texture_node_struct;
//one exception here: otherwise, we have a circular reference problem
class r1_render_window_class;      // defined in r1_win.hh

/// The abstract hardware-independent rendering interface.
/// This abstract class defines the methods a rendering device must
/// support to be usable under golgotha. The most important function
/// is the rendering of textured and shaded polygons. 
class r1_render_api_class
{
protected:
  friend r1_render_api_class *r1_get_api(i4_display_class *for_display);
  friend r1_render_api_class *r1_create_api(i4_display_class *for_display, char *name);
  //Friendship of related base classes. The subclasses often also
  //declare this friendship.
  friend class r1_render_window_class;

  friend void r1_destroy_api(r1_render_api_class *r);  

  r1_texture_manager_class *tmanager;//the first one
  i4_array<r1_texture_manager_class*> tmanagers;//if there's more than one

  // make these global so they can be accessed without pointer indirection (faster)
  static r1_miplevel_t          *last_node;
  static r1_shading_type         shade_mode;
  static r1_alpha_type           alpha_mode;
  static r1_write_mask_type      write_mask;
  static w32                     const_color;
  static r1_filter_type          filter_mode;
  static r1_render_api_class    *first;  
  
  static i4_float               r_tint_mul;
  static i4_float               g_tint_mul;
  static i4_float               b_tint_mul;
  static i4_bool                color_tint_on;

  enum { MAX_COLOR_TINTS = 32 };

  static r1_color_tint          color_tint_list[MAX_COLOR_TINTS];
  static sw8                    num_color_tints;

  r1_render_api_class           *next;
  
  //currently the only flag is R1_SOFTWARE
  w32                           render_device_flags;

  /// Attempt to initialize the rendering device. 
  /// Returns false if display is not compatible with render_api, i.e. if you pass
  /// the directx display to the glide render api.
  /// Init will create the texture manager, which can be used after this call.
  /// \param display The reference to the display driver in use.
  /// \return True if successfull, false if not. Reasons of failure are:
  /// Wrong display driver for this renderer, insufficient memory, 
  /// missing hardware support. 
  virtual i4_bool init(i4_display_class *display);

  /// Deinitialize the renderer. 
  /// this will delete the texture manager (and free textures associated with) created by init
  virtual void uninit();

  /// Copy an image to the screen. 
  virtual void copy_part(i4_image_class *im,                                          
                         int x, int y,             // position on screen
                         int x1, int y1,           // area of image to copy 
                         int x2, int y2) = 0;

public:
  w32                     get_render_device_flags()           { return render_device_flags; }
  r1_shading_type         get_shade_mode()                    { return shade_mode; }
  r1_alpha_type           get_alpha_mode()                    { return alpha_mode; }
  r1_write_mask_type      get_write_mask()                    { return write_mask; }
  w32                     get_constant_color()                { return const_color; }
  r1_filter_type          get_filter_mode()                   { return filter_mode; }
  i4_bool states_have_changed;
  r1_render_api_class() :tmanagers(0,64)//i4_mem_manager has not been initialized->don't allocate mem now
  {
    tmanager=0;
	render_device_flags=0;
    next=first;
    first=this;
	states_have_changed=i4_F;
  }
//Call this method after the render window gets a DISPLAY_CHANGE notification
  virtual i4_bool resize(w16 newx, w16 newy);
  virtual i4_bool redepth(w16 new_bitdepth);

  static i4_draw_context_class  *context;

  virtual i4_bool pixel_double() { return i4_F; }

  r1_texture_manager_class *get_tmanager() { return tmanager; }  // created by init()

  r1_texture_manager_class *get_tmanager(w32 index) {
	  if (index==-1) return NULL;
	  if (index>=(w32)tmanagers.size()) return NULL;
	  return tmanagers[index];
	  }

  void delete_tmanager(w32 index);

  //r1_texture_manager_class *set_tmanager(w32 index); //set the default tmanager to the index - not used, too dangerous

  /// Create a new texture manager.
  /// Creates a new texture manager (the kind depends on the subclass) and 
  /// returns the new and its index.  
  virtual r1_texture_manager_class *install_new_tmanager(w32 &index)=0;

  /// Use a texture for rendering.
  /// The texture handle is obtained from the texture manager, 
  /// this implicitly enables texture mapping. 
  /// This function uses the default texture manager. 
  /// \param material_ref The texture handle to the texture to use.
  /// \param desired_width How large should the texture be? This is used
  /// to guess the required miplevel.
  /// \param frame The current frame number, this is used to handle
  /// the in-memory texture cache MRU buffer. 
  virtual void use_texture(r1_texture_handle material_ref, 
                           sw32 desired_width,
                           w32 frame)                                = 0;
  virtual void use_texture(w32 tman_index, //which tman?
	  r1_texture_handle material_ref,//texture handle
	  sw32 desired_width,
	  w32 frame)=0;

  void use_default_texture(w32 tman_index=-1)
      {
      r1_texture_handle mat=0;
      if (tman_index==-1)
          {
          mat=get_tmanager()->null_texture_handle;
          use_texture(mat,8,0);
          }
      else
          {
          mat=get_tmanager(tman_index)->null_texture_handle;
          use_texture(tman_index,mat,8,0);
          }
      }
  /// Disable texturing. 
  /// Drawing will use the constant color to render with if textures are disabled
  virtual void disable_texture()                                                      = 0;
  
  /// Registers a color tint (usually a player color)
  /// Tints are all r g b values (multiplies)
  virtual r1_color_tint_handle register_color_tint(i4_float r, i4_float g, i4_float b);
  
  /// Use a color tint.
  /// Calling this with a handle of 0 disables tinting
  virtual void set_color_tint(r1_color_tint_handle color_tint_handle);

  /// Set the constant color for untextured rendering. 
  /// Constant color is used by line drawing, and poly draws if 
  /// the textures are disabled.  
  virtual void set_constant_color(w32 color)                          { const_color=color; }
  /// Sets the shading mode.
  /// See R1_SHADE* constants.
  virtual void set_shading_mode(r1_shading_type type)                 { shade_mode=type;   }
  /// Sets the alpha mode.
  /// See R1_ALPHA* constants
  virtual void set_alpha_mode(r1_alpha_type type)                     { alpha_mode=type;   }
  /// Sets the write mask.
  /// This is helpful if you want to prevent writting the z-buffer
  /// for some reason during a call.
  virtual void set_write_mode(r1_write_mask_type mask)                { write_mask=mask;   }
  /// Enables/Disables bilinear filtering.
  /// Might or might not be supported by the current rendering device.
  virtual void set_filter_mode(r1_filter_type type)                   { filter_mode=type;  }
  /// Enable/Disable fogging.
  /// This is not yet supported. 
  virtual void set_fogging_mode(w32 fogcolor, i4_float startvalue, i4_float endvalue)
	  {//currently only supported for d3d rendering
	  };
  
  protected:
  /** Set the Range of the Z-Buffer.
  * The api will scale all z's (or w's) to reflect this range with best percision
  * near_z must be greater than 0.
  * Hint: For golg, a great z value (default 1000.0) is far away, 
  * for directx, this is mapped to 0.0, the near value, since the
  * z-buffer is used the wrong way round. (With comparison for greater
  * instead of less)
  * Any implementation of this function must set r1_near_clip_z and
  * r1_far_clip_z.
  */
  virtual void set_z_range(float near_z, float far_z)                                 = 0;
  public:
  /** Render polygons with the current render settings. 
  * This function is used to render triangle primitives. It draws
  * \a t_verts - 2 triangles using the current color or texture to the
  * display. The \a verts parameter must point to the array of vertices
  * to be used. The vertices must already be transformed and lit.
  * The triangles are to be rendered as TRIANGLE_FAN.
  * \param t_verts The number of vertices to use. Since we use 
  * TRIANGLE_FAN's this number must be 3 or larger.
  * \param verts The pointer to the transformed and lit vertices.
  */
  virtual void render_poly(int t_verts, r1_vert *verts)                               = 0;
  /// Render polygons , indexed.
  /// This does the same as the 2-parameter version, except that the
  /// indices must not be in order.
  virtual void render_poly(int t_verts, r1_vert *verts, int *vertex_index);
  virtual void render_poly(int t_verts, r1_vert *verts, w16 *vertex_index);
  virtual void flush_vert_buffer()//Only used for dx right now
	  {
	  };
  //this had better be a rectangle
  virtual void render_sprite(r1_vert *verts);

  /// Renders single pixels.
  /// This method uses the corresponding method of the underlying
  /// HAL, so it might not work properly (ie dx5 just eats these calls).
  /// Prefer using a higher-level call of the g1_render interface.
  /// \param t_points The number of poinst to draw.
  /// \param pixel The array of vertices. Must be at least as long as t_points.
  /// Use screen-coordinates.
  virtual void render_pixel(int t_points, r1_vert *pixel)                             = 0;
  /// Renders lines as line-strips.
  /// Not properly supported by all HAL layers, but might be emulated 
  /// using triangle strips (the only primitive known to be fully supported
  /// by all HAL drivers and gfx cards)
  /// \param t_lines The number of lines to draw. This method draws line
  /// strips (polylines). Minimum value=1, Maximum value=255
  /// \param verts The vertices of the corners of the polylines. 
  /// The array must be one larger than t_lines. 
  virtual void render_lines(int t_lines, r1_vert *verts )                             = 0;

  /// Writtes a rectangle of the given color to the screen. 
  /// Color is standard argb, (z should be within range specifed by set_z_range)
  /// The function takes care of the render settings, so if z-buffer
  /// write is disabled, it will NOT be cleared. 
  /// The function might decide to just render polygons. 
  virtual void clear_area(int x1, int y1, int x2, int y2, w32 color, float z);
  
  /// Get an image compatible to the screen. 
  /// Creates an image of the same bit depth and palette of screen (for use with put_image)
  virtual i4_image_class *create_compatible_image(w16 w, w16 h)                       = 0;

  // these should be called for an image before drawing to them
  virtual void lock_image(i4_image_class *im) { ; } 
  virtual void unlock_image(i4_image_class *im) { ; }

  /// Put an image to the screen. 
  /// this function does clipping (based on context) and calls copy_part
  virtual void put_image(i4_image_class *im,                                          
                         int x, int y,             // position on screen
                         int x1, int y1,           // area of image to copy 
                         int x2, int y2);

  /// Clip polygons to make them suitable for rendering. 
  /// Takes a polygon and clips it. 
  /// \param num_clip_verts How many verts in the clipped polygon. 
  /// \param t_vertices Pointer to the vertices array
  /// \param indices Pointer to the indices array (which point themselves
  /// to the vertices)
  /// \param clip_buf_1 Pointer to clip buffer 1 (used internally)
  /// \param clip_buf_2 Pointer to clip buffer 2
  /// \param flags flags to be considered when clipping. 
  /// R1_CLIP_NO_CALC_OUTCODE is currently the only supported flag. It
  /// is used when the outcode has already been calculated on the vertices. 
  /// \return Either clip_buf_1 or clip_buf_2, depending on the result. 
  virtual r1_vert *clip_poly(sw32 *num_clip_verts, 
                             r1_vert *t_vertices,  
                             w16 *indices,         
                             r1_vert *clip_buf_1, 
                             r1_vert *clip_buf_2,  
                             w8 flags);            
  
  /// Clip polygons to make them suitable for rendering. 
  /// Takes a polygon and clips it. 
  /// \param num_clip_verts How many verts in the clipped polygon. 
  /// \param t_vertices Pointer to the vertices array
  /// \param indices Pointer to the indices array (which point themselves
  /// to the vertices)
  /// \param clip_buf_1 Pointer to clip buffer 1 (used internally)
  /// \param clip_buf_2 Pointer to clip buffer 2
  /// \param flags flags to be considered when clipping. 
  /// R1_CLIP_NO_CALC_OUTCODE is currently the only supported flag. It
  /// is used when the outcode has already been calculated on the vertices. 
  /// \return Either clip_buf_1 or clip_buf_2, depending on the result. 
  virtual r1_vert *clip_poly(sw32 *num_clip_verts, //how many verts in this polygon
                             r1_vert *t_vertices,  //pointer to the vertices
                             w32 *indices,         //pointer to the indices
                             r1_vert *clip_buf_1,  //pointer to clip buffer 1
                             r1_vert *clip_buf_2,  //pointer to clip buffer 2
                             w8 flags);            //flags to be considered when clipping

  /// Create a render window. 
  /// Creates a window in which rendering can occur. 
  /// visable_w and & h is the area the window takes up on the actual screen
  /// expand type will determine the size you can actually render to.  
  virtual r1_render_window_class *create_render_window(int visable_w, int visable_h,
                                                       r1_expand_type type=R1_COPY_1x1) = 0;


  virtual i4_bool expand_type_supported(r1_expand_type type) 
  { return (i4_bool)(type==R1_COPY_1x1); }

  virtual void modify_features(w32 feature_bits, int on) { ; }
  virtual i4_bool is_feature_on(w32 feature_bits) { return i4_F; }

  virtual void default_state()
  {
    disable_texture();
    set_constant_color(0xffffff);
    //set_z_range(0.001f,10000.0f);    
    set_shading_mode(R1_COLORED_SHADING);
    set_write_mode(R1_WRITE_COLOR | R1_WRITE_W | R1_COMPARE_W);
    set_alpha_mode(R1_ALPHA_DISABLED);    
    set_filter_mode(R1_BILINEAR_FILTERING);
    set_color_tint(0);

    modify_features(R1_ALL_FEATURES, 0);         // turn off these other features
    modify_features(R1_SPANS, 1);
    modify_features(R1_PERSPECTIVE_CORRECT, 1);
  }

  virtual char *name() = 0;
};

extern r1_render_api_class *r1_render_api_class_instance;

/// Creates the rendering api by iterating over all registered apis.
r1_render_api_class *r1_create_api(i4_display_class *for_display, char *api_name=0);

/// Destroys the api.
void r1_destroy_api(r1_render_api_class *render_api);

/// Inverts the z value. 
/// Used for projection. 
inline i4_float r1_ooz(i4_float z) { return 0.9999f/z; }

#endif
