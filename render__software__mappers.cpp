/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/

#include "pch.h"
#include "render/software/r1_software.h"
#include "render/software/r1_software_globals.h"
#include "render/software/mappers.h"
#include "device/processor.h"
#include "render/software/inline_fpu.h"


//global instance of the various function tables

texture_scanline_func_ptr texture_scanline_functions[SPAN_TRI_LAST];
poly_setup_func_ptr poly_setup_functions[SPAN_TRI_LAST];
tri_draw_func_ptr tri_draw_functions[SPAN_TRI_LAST];
span_draw_func_ptr span_draw_functions[SPAN_TRI_LAST];

//this is super important. this points to the current scanline texturemapping function
//and is used by all of the rasterizers / span drawers (so before you call the rasterizer /
//setup function, make sure you set this pointer correctly)
texture_scanline_func_ptr cur_scanline_texture_func = 0;


//the current span list builder (either the one optimized for intel or amd3d)
build_triangle_span_lists_func_ptr cur_build_span_lists_function = 0;
i4_bool disable_realalpha=i4_F; //set this to true before initializing
//to use fake alpha.

void r1_software_class::initialize_function_pointers(i4_bool amd3d)
{
	//zero them out to start with
	memset(texture_scanline_functions, 0, sizeof(texture_scanline_func_ptr) * SPAN_TRI_LAST);
	memset(poly_setup_functions,       0, sizeof(poly_setup_func_ptr)       * SPAN_TRI_LAST);
	memset(tri_draw_functions,         0, sizeof(tri_draw_func_ptr)         * SPAN_TRI_LAST);
	memset(span_draw_functions,        0, sizeof(span_draw_func_ptr)        * SPAN_TRI_LAST);

	cur_build_span_lists_function = intel_build_triangle_span_lists;

	//scanline texturing / drawing functions
	texture_scanline_functions[SPAN_TRI_AFFINE_UNLIT]              = texture_scanline_affine_unlit;
	texture_scanline_functions[SPAN_TRI_AFFINE_UNLIT_HOLY]         = texture_scanline_affine_unlit_holy;
	texture_scanline_functions[SPAN_TRI_AFFINE_UNLIT_HOLY_BLEND]   = texture_scanline_affine_unlit_holy_blend;
	if (disable_realalpha)
	{
		texture_scanline_functions[SPAN_TRI_AFFINE_UNLIT_ALPHA]    = texture_scanline_affine_unlit_alpha;
	}
	else
	{
		texture_scanline_functions[SPAN_TRI_AFFINE_UNLIT_ALPHA]        = texture_scanline_affine_unlit_true_alpha;
	}


	//default is no asm
#ifdef USE_ASM
	//this is a special case mapper optimized in asm only
	//(ignores the t gradient, basically, since sprites have no delta t per screen x)
	texture_scanline_functions[SPAN_TRI_AFFINE_UNLIT_ALPHA_SPRITE] = texture_scanline_affine_unlit_alpha_sprite;
#else
	if (disable_realalpha)
	{
		texture_scanline_functions[SPAN_TRI_AFFINE_UNLIT_ALPHA_SPRITE]=texture_scanline_affine_unlit_alpha;
	}
	else
	{
		texture_scanline_functions[SPAN_TRI_AFFINE_UNLIT_ALPHA_SPRITE] = texture_scanline_affine_unlit_true_alpha;
	}
#endif

	texture_scanline_functions[SPAN_TRI_AFFINE_LIT]              = texture_scanline_affine_lit;
	texture_scanline_functions[SPAN_TRI_PERSPECTIVE_LIT]         = texture_scanline_perspective_lit;
	if (disable_realalpha)
	{
		texture_scanline_functions[SPAN_TRI_PERSPECTIVE_UNLIT_ALPHA]=texture_scanline_perspective_unlit_alpha;
	}
	else
	{
		texture_scanline_functions[SPAN_TRI_PERSPECTIVE_UNLIT_ALPHA] = texture_scanline_perspective_unlit_true_alpha;
	}
	texture_scanline_functions[SPAN_TRI_PERSPECTIVE_UNLIT]       = texture_scanline_perspective_unlit;
	texture_scanline_functions[SPAN_TRI_PERSPECTIVE_UNLIT_HOLY]  = texture_scanline_perspective_unlit_holy;

	texture_scanline_functions[SPAN_TRI_SOLID_FILL]              = texture_scanline_solid_fill;
	texture_scanline_functions[SPAN_TRI_SOLID_BLEND]             = texture_scanline_solid_blend_half;

	//poly setup functions (calculates deltas, gradients, etc)
	poly_setup_functions[SPAN_TRI_AFFINE_UNLIT]              = poly_setup_affine_lit;
	poly_setup_functions[SPAN_TRI_AFFINE_UNLIT_HOLY]         = poly_setup_affine_lit;
	poly_setup_functions[SPAN_TRI_AFFINE_UNLIT_HOLY_BLEND]   = poly_setup_affine_lit;
	poly_setup_functions[SPAN_TRI_AFFINE_UNLIT_ALPHA]        = poly_setup_affine_lit;
	poly_setup_functions[SPAN_TRI_AFFINE_UNLIT_ALPHA_SPRITE] = poly_setup_affine_lit;
	poly_setup_functions[SPAN_TRI_AFFINE_LIT]                = poly_setup_affine_lit;

	poly_setup_functions[SPAN_TRI_PERSPECTIVE_UNLIT]       = poly_setup_perspective_lit;
	poly_setup_functions[SPAN_TRI_PERSPECTIVE_UNLIT_HOLY]  = poly_setup_perspective_lit;
	poly_setup_functions[SPAN_TRI_PERSPECTIVE_UNLIT_ALPHA] = poly_setup_perspective_lit;
	poly_setup_functions[SPAN_TRI_PERSPECTIVE_LIT]         = poly_setup_perspective_lit;

	poly_setup_functions[SPAN_TRI_SOLID_FILL]              = poly_setup_solid_color;
	poly_setup_functions[SPAN_TRI_SOLID_BLEND]             = poly_setup_solid_color;

	//span drawing functions (sets up some variables, steps through spans, calculates starting s,t,etc, for each span)
	span_draw_functions[SPAN_TRI_AFFINE_UNLIT]              = span_draw_affine_unlit;
	span_draw_functions[SPAN_TRI_AFFINE_UNLIT_HOLY]         = span_draw_affine_unlit;
	span_draw_functions[SPAN_TRI_AFFINE_UNLIT_HOLY_BLEND]   = span_draw_affine_unlit;
	span_draw_functions[SPAN_TRI_AFFINE_UNLIT_ALPHA]        = span_draw_affine_unlit;
	span_draw_functions[SPAN_TRI_AFFINE_UNLIT_ALPHA_SPRITE] = span_draw_affine_unlit;
	span_draw_functions[SPAN_TRI_AFFINE_LIT]                = span_draw_affine_lit;
	span_draw_functions[SPAN_TRI_PERSPECTIVE_UNLIT]         = span_draw_perspective_unlit;
	span_draw_functions[SPAN_TRI_PERSPECTIVE_UNLIT_HOLY]    = span_draw_perspective_unlit;
	span_draw_functions[SPAN_TRI_PERSPECTIVE_UNLIT_ALPHA]   = span_draw_perspective_unlit;
	span_draw_functions[SPAN_TRI_PERSPECTIVE_LIT]           = span_draw_perspective_lit;

	span_draw_functions[SPAN_TRI_SOLID_FILL]              = span_draw_solid_color;
	span_draw_functions[SPAN_TRI_SOLID_BLEND]             = span_draw_solid_color;

	//triangle drawing functions (rasterizes triangles, steps the necessary values (s,t,l,etc) )
	tri_draw_functions[SPAN_TRI_AFFINE_UNLIT]              = tri_draw_affine_unlit;
	tri_draw_functions[SPAN_TRI_AFFINE_UNLIT_HOLY]         = tri_draw_affine_unlit;
	tri_draw_functions[SPAN_TRI_AFFINE_UNLIT_HOLY_BLEND]   = tri_draw_affine_unlit;
	tri_draw_functions[SPAN_TRI_AFFINE_UNLIT_ALPHA]        = tri_draw_affine_unlit;
	tri_draw_functions[SPAN_TRI_AFFINE_UNLIT_ALPHA_SPRITE] = tri_draw_affine_unlit;
	tri_draw_functions[SPAN_TRI_AFFINE_LIT]                = tri_draw_affine_lit;

	tri_draw_functions[SPAN_TRI_PERSPECTIVE_UNLIT]       = tri_draw_perspective_unlit;
	tri_draw_functions[SPAN_TRI_PERSPECTIVE_UNLIT_HOLY]  = tri_draw_perspective_unlit;
	tri_draw_functions[SPAN_TRI_PERSPECTIVE_UNLIT_ALPHA] = tri_draw_perspective_unlit;
	tri_draw_functions[SPAN_TRI_PERSPECTIVE_LIT]         = tri_draw_perspective_lit;

	tri_draw_functions[SPAN_TRI_SOLID_FILL]              = 0; //tri_draw_solid_color;
	tri_draw_functions[SPAN_TRI_SOLID_BLEND]             = 0; //tri_draw_solid_color;

	do_amd3d = i4_F;
	//default is use generic code
#ifdef USE_AMD3D
	if (amd3d)
	{
		i4_cpu_info_struct s;
		i4_get_cpu_info(&s);

		if (s.cpu_flags & i4_cpu_info_struct::AMD3D)
		{
			do_amd3d = i4_T;

			//we're compiling amd3d stuff, its requesting the amd3d functions,
			//the processor supports them, do it
			cur_build_span_lists_function = amd3d_build_triangle_span_lists;

			span_draw_functions[SPAN_TRI_AFFINE_UNLIT]              = span_draw_affine_unlit_amd3d;
			span_draw_functions[SPAN_TRI_AFFINE_UNLIT_HOLY]         = span_draw_affine_unlit_amd3d;
			span_draw_functions[SPAN_TRI_AFFINE_UNLIT_HOLY_BLEND]   = span_draw_affine_unlit_amd3d;
			span_draw_functions[SPAN_TRI_AFFINE_UNLIT_ALPHA]        = span_draw_affine_unlit_amd3d;
			span_draw_functions[SPAN_TRI_AFFINE_UNLIT_ALPHA_SPRITE] = span_draw_affine_unlit_amd3d;
			span_draw_functions[SPAN_TRI_AFFINE_LIT]                = span_draw_affine_lit_amd3d;
			span_draw_functions[SPAN_TRI_PERSPECTIVE_UNLIT]         = span_draw_perspective_unlit_amd3d;
			span_draw_functions[SPAN_TRI_PERSPECTIVE_UNLIT_HOLY]    = span_draw_perspective_unlit_amd3d;
			span_draw_functions[SPAN_TRI_PERSPECTIVE_UNLIT_ALPHA]   = span_draw_perspective_unlit_amd3d;
			span_draw_functions[SPAN_TRI_PERSPECTIVE_LIT]           = span_draw_perspective_lit_amd3d;

			texture_scanline_functions[SPAN_TRI_AFFINE_LIT]              = texture_scanline_affine_lit_amd3d;
			texture_scanline_functions[SPAN_TRI_PERSPECTIVE_LIT]         = texture_scanline_perspective_lit_amd3d;
			texture_scanline_functions[SPAN_TRI_PERSPECTIVE_UNLIT_ALPHA] = texture_scanline_perspective_unlit_alpha_amd3d;
			texture_scanline_functions[SPAN_TRI_PERSPECTIVE_UNLIT]       = texture_scanline_perspective_unlit_amd3d;
			texture_scanline_functions[SPAN_TRI_PERSPECTIVE_UNLIT_HOLY]  = texture_scanline_perspective_unlit_holy_amd3d;
		}
	}
#endif
}

//PG: To avoid this big mess, we really copied the code here
/*
 #ifdef USE_ASM

   //laziness. include all the scanline texturemappers / fillers
 #include "render\software/affine_map_lit_asm.cpp"
 #include "render\software/affine_map_unlit_asm.cpp"
 #include "render\software/affine_map_unlit_holy_asm.cpp"
 #include "render\software/affine_map_unlit_holy_blend_asm.cpp"
 #include "render\software/affine_map_unlit_alpha_asm.cpp"
 #include "render\software/affine_map_unlit_alpha_sprite_asm.cpp"
 #include "render\software/affine_map_unlit_true_alpha_c.cpp"
 #include "render\software/perspective_map_lit_asm.cpp"
 #include "render\software/perspective_map_unlit_asm.cpp"
 #include "render\software/perspective_map_unlit_holy_asm.cpp"
 #include "render\software/perspective_map_unlit_alpha_asm.cpp"
 #include "render\software/perspective_map_unlit_true_alpha_c.cpp"
 #include "render\software/solid_blend_half_asm.cpp"
 #include "render\software/solid_fill_asm.cpp"

 #ifdef USE_AMD3D
   //include all the amd3d texturemappers too
 #include "render\software/amd3d/affine_map_lit_asm_amd3d.cpp"
 #include "render\software/amd3d/perspective_map_lit_asm_amd3d.cpp"
 #include "render\software/amd3d/perspective_map_unlit_alpha_asm_amd3d.cpp"
 #include "render\software/amd3d/perspective_map_unlit_asm_amd3d.cpp"
 #include "render\software/amd3d/perspective_map_unlit_holy_asm_amd3d.cpp"
 #endif

 #else

 #include "software/affine_map_lit_c.cpp"
 #include "software/affine_map_unlit_c.cpp"
 #include "software/affine_map_unlit_holy_c.cpp"
 #include "software/affine_map_unlit_holy_blend_c.cpp"
 #include "software/affine_map_unlit_alpha_c.cpp"
 #include "software/affine_map_unlit_true_alpha_c.cpp"
 #include "software/perspective_map_lit_c.cpp"
 #include "software/perspective_map_unlit_c.cpp"
 #include "software/perspective_map_unlit_holy_c.cpp"
 #include "software/perspective_map_unlit_alpha_c.cpp"
 #include "software/perspective_map_unlit_true_alpha_c.cpp"
 #include "software/solid_blend_half_c.cpp"
 #include "software/solid_fill_c.cpp"

 #endif
 */

#ifdef USE_ASM

//here comes everything what was #included above (just insert the code here)

#ifdef USE_AMD3D

#endif //USE_AMD3D

#else //USE_ASM

//affine_map_lit_c.cpp
/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/

//Plain C, so no assembly allowed and no self-modification allowed
//It's not portable at all, or try to find the 0xdeadbeef in sparc
//risc assembly code... It just won't be there the way one would think
//it is.

/*
   w32 *texture_affine_lit_starter()
   {
   w32 returnval;
   _asm
   {
   	mov eax,OFFSET dumb_addr
   dumb_addr:
   	mov returnval,eax
   }
   return (w32 *)returnval;
   }
 */

void texture_scanline_affine_lit(w16 *start_pixel,
								 sw32 start_x,
								 void *_left, //perspective_span *left,
								 sw32 width)
{
	//temporary dwords for lighting calculations
	w16 texel;
	w32 t1,t2;
	w32 l_lookup;

	start_pixel = (w16 *)((w8 *)start_pixel + start_x);

	affine_span *left = (affine_span *)_left;

	left_s = left->s + cur_grads.s_adjust;
	left_t = left->t + cur_grads.t_adjust;
	left_l = left->l;

	temp_dsdx = qftoi(cur_grads.dsdx);
	temp_dtdx = qftoi(cur_grads.dtdx);

	width_global = width;

	while (width_global)
	{
		texel = *(r1_software_texture_ptr + (left_s>>16) + ((left_t>>16)<<r1_software_twidth_log2));

		l_lookup = left_l & (NUM_LIGHT_SHADES<<8);

		//lookup low bits
		//t1 = ((w32 *)(0xDEADBEEF))[l_lookup + (texel & 0xFF)];
		t1=(DEADBEEFPTR)[l_lookup + (texel & 0xFF)];

		//lookup high bits
		//t2 = ((w32 *)(0xDEADBEEF)+ctable_size)[l_lookup + (texel>>8)];
		t2=(DEADBEEFPTR+ctable_size)[l_lookup + (texel>>8)];

		*start_pixel = (w16)(t1+t2);

		start_pixel++;

		left_s += temp_dsdx;
		left_t += temp_dtdx;
		left_l += dldx_fixed;

		width_global--;
	}
}

/*
   w32 *texture_affine_lit_sentinel()
   {
   w32 returnval;
   _asm
   {
   	mov eax,OFFSET dumb_addr
   dumb_addr:
   	mov returnval,eax
   }
   return (w32 *)returnval;
   }
 */

void insert_color_modify_address_low(w32 *address);
void insert_color_modify_address_high(w32 *address);
extern w32 color_modify_list[];
extern sw32 num_color_modifies;

void setup_color_modify_affine_lit()
{
	return;
	/*
	   w32 *stop = texture_affine_lit_sentinel();

	   w32 *search = texture_affine_lit_starter();
	   //start searching for 0xDEADBEEF
	   //This searches in the code of the above method
	   while (search < stop)
	   {
	   //casting craziness
	   search = (w32 *)((w8 *)search + 1);
	   if (*search==0xDEADBEEF)
	   {
	   	insert_color_modify_address_low(search);
	   }
	   else
	   if (*search==(0xDEADBEEF + ctable_size_bytes))
	   {
	   	insert_color_modify_address_high(search);
	   }
	   }
	 */
}

//affine_map_unlit_alpha_c.cpp
/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/

void texture_scanline_affine_unlit_alpha(w16 *start_pixel,
										 sw32 start_x,
										 void *_left, //perspective_span *left,
										 sw32 width)
{
	w32 accumulated_alpha = (1<<12);

	start_pixel = (w16 *)((w8 *)start_pixel + start_x);

	affine_span *left = (affine_span *)_left;

	left_s = left->s;
	left_t = left->t;

	width_global = width;

	while (width_global)
	{
		w16 texel = *(r1_software_texture_ptr + (left_s>>16) + ((left_t>>16)<<r1_software_twidth_log2));

		accumulated_alpha += ((w32)(texel) & (w32)(15<<12));

		if (accumulated_alpha & 65536)
		{
			*start_pixel = alpha_table[texel & 4095];
			accumulated_alpha &= (~65536);
			accumulated_alpha += (1<<12);
		}

		start_pixel++;

		left_s += temp_dsdx;
		left_t += temp_dtdx;

		width_global--;
	}
}

//affine_map_unlit_c.cpp
/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/


void texture_scanline_affine_unlit(w16 *start_pixel,
								   sw32 start_x,
								   void *_left, //perspective_span *left,
								   sw32 width)
{
	start_pixel = (w16 *)((w8 *)start_pixel + start_x);

	affine_span *left = (affine_span *)_left;

	left_s = left->s + cur_grads.s_adjust;
	left_t = left->t + cur_grads.t_adjust;

	temp_dsdx = qftoi(cur_grads.dsdx);
	temp_dtdx = qftoi(cur_grads.dtdx);

	width_global = width;

	while (width_global)
	{
		*start_pixel = *(r1_software_texture_ptr + (left_s>>16) + ((left_t>>16)<<r1_software_twidth_log2));

		start_pixel++;

		left_s += temp_dsdx;
		left_t += temp_dtdx;

		width_global--;
	}
}

//affine_map_unlit_holy_c.cpp
/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/


void texture_scanline_affine_unlit_holy(w16 *start_pixel,
										sw32 start_x,
										void *_left, //perspective_span *left,
										sw32 width)
{
	start_pixel = (w16 *)((w8 *)start_pixel + start_x);

	affine_span *left = (affine_span *)_left;

	left_s = left->s + cur_grads.s_adjust;
	left_t = left->t + cur_grads.t_adjust;

	temp_dsdx = qftoi(cur_grads.dsdx);
	temp_dtdx = qftoi(cur_grads.dtdx);

	width_global = width;

	while (width_global)
	{
		w16 texel = *(r1_software_texture_ptr + (left_s>>16) + ((left_t>>16)<<r1_software_twidth_log2));

		if (texel!=G1_16BIT_CHROMA_565)
		{
			// What should it be?
			//someone just wrote "if (texel)" which of course
			//is wrong, because chroma color is not black
			*start_pixel = texel;
		}

		start_pixel++;

		left_s += temp_dsdx;
		left_t += temp_dtdx;

		width_global--;
	}
}

//affine_map_unlit_true_alpha_c.cpp
/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/


void texture_scanline_affine_unlit_true_alpha(w16 *start_pixel,
											  sw32 start_x,
											  void *_left, //perspective_span *left,
											  sw32 width)
{
	w32 accumulated_alpha = (1<<12);

	start_pixel = (w16 *)((w8 *)start_pixel + start_x);

	affine_span *left = (affine_span *)_left;

	left_s = left->s;
	left_t = left->t;

	width_global = width;

	while (width_global)
	{
		register w32 alpha_texel = *(r1_software_texture_ptr + (left_s>>16) + ((left_t>>16)<<r1_software_twidth_log2));

		register w32 l_lookup = (alpha_texel & (w16)(15<<12)) >> (8-NUM_LIGHT_SHADES_LOG2);

		if (l_lookup)
		{
			register w32 texel = (w32)alpha_table[alpha_texel & 4095];

			//lookup low bits
			register w32 output_texel = ((w32 *)(software_color_tables))[l_lookup + (texel & 0xFF)];

			//lookup high bits
			output_texel += ((w32 *)(software_color_tables)+ctable_size)[l_lookup + (texel>>8)];

			l_lookup ^= (NUM_LIGHT_SHADES<<8);

			if (l_lookup)
			{
				texel       = (w32)*start_pixel;

				//lookup low bits
				output_texel += ((w32 *)(software_color_tables))[l_lookup + (texel & 0xFF)];

				//lookup high bits
				output_texel += ((w32 *)(software_color_tables)+ctable_size)[l_lookup + (texel>>8)];
			}

			*start_pixel = (w16)(output_texel);
		}

		start_pixel++;

		left_s += temp_dsdx;
		left_t += temp_dtdx;

		width_global--;
	}
}
//affine_map_unlit_holy_blend_c.cpp
/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/


void texture_scanline_affine_unlit_holy_blend(w16 *start_pixel,
											  sw32 start_x,
											  void *_left, //perspective_span *left,
											  sw32 width)
{
	start_pixel = (w16 *)((w8 *)start_pixel + start_x);

	affine_span *left = (affine_span *)_left;

	left_s = left->s + cur_grads.s_adjust;
	left_t = left->t + cur_grads.t_adjust;

	temp_dsdx = qftoi(cur_grads.dsdx);
	temp_dtdx = qftoi(cur_grads.dtdx);

	width_global = width;

	while (width_global)
	{
		w16 texel = *(r1_software_texture_ptr + (left_s>>16) + ((left_t>>16)<<r1_software_twidth_log2));

		if (texel!=G1_16BIT_CHROMA_565)
		{
			*start_pixel = ((*start_pixel & pre_blend_and)>>1) + ((texel & pre_blend_and)>>1);
		}

		start_pixel++;

		left_s += temp_dsdx;
		left_t += temp_dtdx;

		width_global--;
	}
}

//perspective_map_lit_c.cpp
/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/

/*
   w32 *texture_perspective_lit_starter()
   {
   w32 returnval;
   _asm
   {
   	mov eax,OFFSET dumb_addr
   dumb_addr:
   	mov returnval,eax
   }
   return (w32 *)returnval;
   }
 */

void texture_scanline_perspective_lit(w16 *start_pixel,
									  sw32 start_x,
									  void *_left, //perspective_span *left,
									  sw32 width)
{
	//temporary stuff for lighting calculations
	w16 texel;
	w32 t1,t2;
	w32 l_lookup;

	start_pixel = (w16 *)((w8 *)start_pixel + start_x);

	perspective_span *left = (perspective_span *)_left;

	left_z = 1.f / left->ooz;
	left_s = qftoi(left->soz * left_z) + cur_grads.s_adjust;
	left_t = qftoi(left->toz * left_z) + cur_grads.t_adjust;

	//dont forget the lighting
	left_l = left->l;

	sw32 had_subdivisions = width & (~15);

	num_subdivisions = width >> 4;
	num_leftover     = width & 15;

	if (num_subdivisions)
	{
		ooz_right = left->ooz + (cur_grads.doozdxspan);
		soz_right = left->soz + (cur_grads.dsozdxspan);
		toz_right = left->toz + (cur_grads.dtozdxspan);

		right_z = 1.f / ooz_right;

		while (num_subdivisions)
		{

			right_s = qftoi(soz_right * right_z) + cur_grads.s_adjust;
			if (right_s < 0)
			{
				right_s = 0;
			}
			else
			if (right_s > s_mask)
			{
				right_s = s_mask;
			}

			right_t = qftoi(toz_right * right_z) + cur_grads.t_adjust;
			if (right_t < 0)
			{
				right_t = 0;
			}
			else
			if (right_t > t_mask)
			{
				right_t = t_mask;
			}

			temp_dsdx = (right_s - left_s) >> 4;
			temp_dtdx = (right_t - left_t) >> 4;

			if (num_subdivisions!=1)
			{
				ooz_right += (cur_grads.doozdxspan);
				soz_right += (cur_grads.dsozdxspan);
				toz_right += (cur_grads.dtozdxspan);

				right_z = 1.f / ooz_right;
			}

			width_global = 16;

			while (width_global)
			{
				texel = *( r1_software_texture_ptr + (left_s>>16) + ((left_t>>16) << r1_software_twidth_log2) );

				l_lookup = left_l & (NUM_LIGHT_SHADES<<8);

				//lookup low bits
				//t1 = ((w32 *)(0xDEADBEEF))[l_lookup + (texel & 0xFF)];
				t1=(DEADBEEFPTR)[l_lookup + (texel & 0xFF)];

				//lookup high bits
				//t2 = ((w32 *)(0xDEADBEEF)+ctable_size)[l_lookup + (texel>>8)];
				t2=(DEADBEEFPTR+ctable_size)[l_lookup+ ( texel>>8)];

				*start_pixel = (w16)(t1+t2);

				start_pixel++;

				left_s += temp_dsdx;
				left_t += temp_dtdx;
				left_l += dldx_fixed;

				width_global--;
			}

			left_s = right_s;
			left_t = right_t;

			num_subdivisions--;
		}
	}

	if (num_leftover)
	{
		if (num_leftover > 1)
		{
			if (had_subdivisions!=0)
			{
				ooz_right += (cur_grads.doozdx * num_leftover);
				soz_right += (cur_grads.dsozdx * num_leftover);
				toz_right += (cur_grads.dtozdx * num_leftover);

				right_z = 1.f / ooz_right;
			}
			else
			{
				ooz_right = left->ooz + (cur_grads.doozdx * num_leftover);
				soz_right = left->soz + (cur_grads.dsozdx * num_leftover);
				toz_right = left->toz + (cur_grads.dtozdx * num_leftover);

				right_z = 1.f / ooz_right;
			}

			right_s = qftoi(soz_right * right_z) + cur_grads.s_adjust;
			if (right_s < 0)
			{
				right_s = 0;
			}
			else
			if (right_s > s_mask)
			{
				right_s = s_mask;
			}

			right_t = qftoi(toz_right * right_z) + cur_grads.t_adjust;
			if (right_t < 0)
			{
				right_t = 0;
			}
			else
			if (right_t > t_mask)
			{
				right_t = t_mask;
			}

			temp_dsdx = qftoi((float)(right_s - left_s) * inverse_leftover_lookup[num_leftover]);
			temp_dtdx = qftoi((float)(right_t - left_t) * inverse_leftover_lookup[num_leftover]);

			while (num_leftover)
			{
				texel = *(r1_software_texture_ptr + (left_s>>16) + ((left_t>>16)<<r1_software_twidth_log2));

				l_lookup = left_l & (NUM_LIGHT_SHADES<<8);

				//lookup low bits
				//t1 = ((w32 *)(0xDEADBEEF))[l_lookup + (texel & 0xFF)];
				t1=(DEADBEEFPTR)[l_lookup + (texel & 0xFF)];

				//lookup high bits
				//t2 = ((w32 *)(0xDEADBEEF)+ctable_size)[l_lookup + (texel>>8)];
				t2=(DEADBEEFPTR+ctable_size)[l_lookup + (texel>>8)];

				*start_pixel = (w16)(t1+t2);

				start_pixel++;

				left_s += temp_dsdx;
				left_t += temp_dtdx;
				left_l += dldx_fixed;

				num_leftover--;
			}
		}
		else
		{
			texel = *(r1_software_texture_ptr + (left_s>>16) + ((left_t>>16)<<r1_software_twidth_log2));

			l_lookup = left_l & (NUM_LIGHT_SHADES<<8);

			//lookup low bits
			t1 = (DEADBEEFPTR)[l_lookup + (texel & 0xFF)];

			//lookup high bits
			t2 = (DEADBEEFPTR+ctable_size)[l_lookup + (texel>>8)];

			*start_pixel = (w16)(t1+t2);
		}
	}
}

/*
   w32 *texture_perspective_lit_sentinel()
   {
   w32 returnval;
   _asm
   {
   	mov eax,OFFSET dumb_addr
   dumb_addr:
   	mov returnval,eax
   }
   return (w32 *)returnval;
   }
 */

void insert_color_modify_address_low(w32 *address);
void insert_color_modify_address_high(w32 *address);
extern w32 color_modify_list[];
extern sw32 num_color_modifies;

void setup_color_modify_perspective_lit()
{
	return;
	/*
	   w32 *stop = texture_perspective_lit_sentinel();

	   w32 *search = texture_perspective_lit_starter();
	   //start searching for 0xDEADBEEF
	   while (search < stop)
	   {
	   //casting craziness
	   search = (w32 *)((w8 *)search + 1);
	   if (*search==0xDEADBEEF)
	   {
	   	insert_color_modify_address_low(search);
	   }
	   else
	   if (*search==(0xDEADBEEF + ctable_size_bytes))
	   {
	   	insert_color_modify_address_high(search);
	   }
	   }
	 */
}
//perspective_map_unlit_alpha_c.cpp
/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/


void texture_scanline_perspective_unlit_alpha(w16 *start_pixel,
											  sw32 start_x,
											  void *_left, //perspective_span *left,
											  sw32 width)
{
	//temporary stuff for lighting calculations
	w32 accumulated_alpha = (1<<12);

	start_pixel = (w16 *)((w8 *)start_pixel + start_x);

	perspective_span *left = (perspective_span *)_left;

	left_z = 1.f / left->ooz;
	left_s = qftoi(left->soz * left_z) + cur_grads.s_adjust;
	left_t = qftoi(left->toz * left_z) + cur_grads.t_adjust;

	sw32 had_subdivisions = width & (~15);

	num_subdivisions = width >> 4;
	num_leftover     = width & 15;

	if (num_subdivisions)
	{
		ooz_right = left->ooz + (cur_grads.doozdxspan);
		soz_right = left->soz + (cur_grads.dsozdxspan);
		toz_right = left->toz + (cur_grads.dtozdxspan);

		right_z = 1.f / ooz_right;

		while (num_subdivisions)
		{

			right_s = qftoi(soz_right * right_z) + cur_grads.s_adjust;
			if (right_s < 0)
			{
				right_s = 0;
			}
			else
			if (right_s > s_mask)
			{
				right_s = s_mask;
			}

			right_t = qftoi(toz_right * right_z) + cur_grads.t_adjust;
			if (right_t < 0)
			{
				right_t = 0;
			}
			else
			if (right_t > t_mask)
			{
				right_t = t_mask;
			}

			temp_dsdx = (right_s - left_s) >> 4;
			temp_dtdx = (right_t - left_t) >> 4;

			if (num_subdivisions!=1)
			{
				ooz_right += (cur_grads.doozdxspan);
				soz_right += (cur_grads.dsozdxspan);
				toz_right += (cur_grads.dtozdxspan);

				right_z = 1.f / ooz_right;
			}

			width_global = 16;

			while (width_global)
			{
				w16 texel = *( r1_software_texture_ptr + (left_s>>16) + ((left_t>>16) << r1_software_twidth_log2) );

				accumulated_alpha += ((w32)(texel) & (w32)(15<<12));

				if (accumulated_alpha & 65536)
				{
					*start_pixel = alpha_table[texel & 4095];
					accumulated_alpha &= (~65536);
					accumulated_alpha += (1<<12);
				}

				start_pixel++;

				left_s += temp_dsdx;
				left_t += temp_dtdx;

				width_global--;
			}

			left_s = right_s;
			left_t = right_t;

			num_subdivisions--;
		}
	}

	if (num_leftover)
	{
		if (num_leftover > 1)
		{
			if (had_subdivisions!=0)
			{
				ooz_right += (cur_grads.doozdx * num_leftover);
				soz_right += (cur_grads.dsozdx * num_leftover);
				toz_right += (cur_grads.dtozdx * num_leftover);

				right_z = 1.f / ooz_right;
			}
			else
			{
				ooz_right = left->ooz + (cur_grads.doozdx * num_leftover);
				soz_right = left->soz + (cur_grads.dsozdx * num_leftover);
				toz_right = left->toz + (cur_grads.dtozdx * num_leftover);

				right_z = 1.f / ooz_right;
			}

			right_s = qftoi(soz_right * right_z) + cur_grads.s_adjust;
			if (right_s < 0)
			{
				right_s = 0;
			}
			else
			if (right_s > s_mask)
			{
				right_s = s_mask;
			}

			right_t = qftoi(toz_right * right_z) + cur_grads.t_adjust;
			if (right_t < 0)
			{
				right_t = 0;
			}
			else
			if (right_t > t_mask)
			{
				right_t = t_mask;
			}

			temp_dsdx = qftoi((float)(right_s - left_s) * inverse_leftover_lookup[num_leftover]);
			temp_dtdx = qftoi((float)(right_t - left_t) * inverse_leftover_lookup[num_leftover]);

			while (num_leftover)
			{
				w16 texel = *(r1_software_texture_ptr + (left_s>>16) + ((left_t>>16)<<r1_software_twidth_log2));

				accumulated_alpha += ((w32)(texel) & (w32)(15<<12));

				if (accumulated_alpha & 65536)
				{
					*start_pixel = alpha_table[texel & 4095];
					accumulated_alpha &= (~65536);
					accumulated_alpha += (1<<12);
				}

				start_pixel++;

				left_s += temp_dsdx;
				left_t += temp_dtdx;

				num_leftover--;
			}
		}
		else
		{
			*start_pixel = *(r1_software_texture_ptr + (left_s>>16) + ((left_t>>16)<<r1_software_twidth_log2));
		}
	}
}

//perspective_map_unlit_c.cpp
/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/


void texture_scanline_perspective_unlit(w16 *start_pixel,
										sw32 start_x,
										void *_left, //perspective_span *left,
										sw32 width)
{
	//temporary stuff for lighting calculations
	//w16 texel;
	//w32 t1,t2;
	//w32 l_lookup;

	start_pixel = (w16 *)((w8 *)start_pixel + start_x);

	perspective_span *left = (perspective_span *)_left;

	left_z = 1.f / left->ooz;
	left_s = qftoi(left->soz * left_z) + cur_grads.s_adjust;
	left_t = qftoi(left->toz * left_z) + cur_grads.t_adjust;

	sw32 had_subdivisions = width & (~15);

	num_subdivisions = width >> 4;
	num_leftover     = width & 15;

	if (num_subdivisions)
	{
		ooz_right = left->ooz + (cur_grads.doozdxspan);
		soz_right = left->soz + (cur_grads.dsozdxspan);
		toz_right = left->toz + (cur_grads.dtozdxspan);

		right_z = 1.f / ooz_right;

		while (num_subdivisions)
		{

			right_s = qftoi(soz_right * right_z) + cur_grads.s_adjust;
			if (right_s < 0)
			{
				right_s = 0;
			}
			else
			if (right_s > s_mask)
			{
				right_s = s_mask;
			}

			right_t = qftoi(toz_right * right_z) + cur_grads.t_adjust;
			if (right_t < 0)
			{
				right_t = 0;
			}
			else
			if (right_t > t_mask)
			{
				right_t = t_mask;
			}

			temp_dsdx = (right_s - left_s) >> 4;
			temp_dtdx = (right_t - left_t) >> 4;

			if (num_subdivisions!=1)
			{
				ooz_right += (cur_grads.doozdxspan);
				soz_right += (cur_grads.dsozdxspan);
				toz_right += (cur_grads.dtozdxspan);

				right_z = 1.f / ooz_right;
			}

			width_global = 16;

			while (width_global)
			{
				*start_pixel = *( r1_software_texture_ptr + (left_s>>16) + ((left_t>>16) << r1_software_twidth_log2) );

				start_pixel++;

				left_s += temp_dsdx;
				left_t += temp_dtdx;

				width_global--;
			}

			left_s = right_s;
			left_t = right_t;

			num_subdivisions--;
		}
	}

	if (num_leftover)
	{
		if (num_leftover > 1)
		{
			if (had_subdivisions!=0)
			{
				ooz_right += (cur_grads.doozdx * num_leftover);
				soz_right += (cur_grads.dsozdx * num_leftover);
				toz_right += (cur_grads.dtozdx * num_leftover);

				right_z = 1.f / ooz_right;
			}
			else
			{
				ooz_right = left->ooz + (cur_grads.doozdx * num_leftover);
				soz_right = left->soz + (cur_grads.dsozdx * num_leftover);
				toz_right = left->toz + (cur_grads.dtozdx * num_leftover);

				right_z = 1.f / ooz_right;
			}

			right_s = qftoi(soz_right * right_z) + cur_grads.s_adjust;
			if (right_s < 0)
			{
				right_s = 0;
			}
			else
			if (right_s > s_mask)
			{
				right_s = s_mask;
			}

			right_t = qftoi(toz_right * right_z) + cur_grads.t_adjust;
			if (right_t < 0)
			{
				right_t = 0;
			}
			else
			if (right_t > t_mask)
			{
				right_t = t_mask;
			}

			temp_dsdx = qftoi((float)(right_s - left_s) * inverse_leftover_lookup[num_leftover]);
			temp_dtdx = qftoi((float)(right_t - left_t) * inverse_leftover_lookup[num_leftover]);

			while (num_leftover)
			{
				*start_pixel = *(r1_software_texture_ptr + (left_s>>16) + ((left_t>>16)<<r1_software_twidth_log2));

				start_pixel++;

				left_s += temp_dsdx;
				left_t += temp_dtdx;

				num_leftover--;
			}
		}
		else
		{
			*start_pixel = *(r1_software_texture_ptr + (left_s>>16) + ((left_t>>16)<<r1_software_twidth_log2));
		}
	}
}
//perspective_map_unlit_holy_c.cpp
/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/

void texture_scanline_perspective_unlit_holy(w16 *start_pixel,
											 sw32 start_x,
											 void *_left, //perspective_span *left,
											 sw32 width)
{
	//temporary stuff for lighting calculations
	w16 texel;
	//w32 t1,t2;
	//w32 l_lookup;

	start_pixel = (w16 *)((w8 *)start_pixel + start_x);

	perspective_span *left = (perspective_span *)_left;

	left_z = 1.f / left->ooz;
	left_s = qftoi(left->soz * left_z) + cur_grads.s_adjust;
	left_t = qftoi(left->toz * left_z) + cur_grads.t_adjust;

	sw32 had_subdivisions = width & (~15);

	num_subdivisions = width >> 4;
	num_leftover     = width & 15;

	if (num_subdivisions)
	{
		ooz_right = left->ooz + (cur_grads.doozdxspan);
		soz_right = left->soz + (cur_grads.dsozdxspan);
		toz_right = left->toz + (cur_grads.dtozdxspan);

		right_z = 1.f / ooz_right;

		while (num_subdivisions)
		{

			right_s = qftoi(soz_right * right_z) + cur_grads.s_adjust;
			if (right_s < 0)
			{
				right_s = 0;
			}
			else
			if (right_s > s_mask)
			{
				right_s = s_mask;
			}

			right_t = qftoi(toz_right * right_z) + cur_grads.t_adjust;
			if (right_t < 0)
			{
				right_t = 0;
			}
			else
			if (right_t > t_mask)
			{
				right_t = t_mask;
			}

			temp_dsdx = (right_s - left_s) >> 4;
			temp_dtdx = (right_t - left_t) >> 4;

			if (num_subdivisions!=1)
			{
				ooz_right += (cur_grads.doozdxspan);
				soz_right += (cur_grads.dsozdxspan);
				toz_right += (cur_grads.dtozdxspan);

				right_z = 1.f / ooz_right;
			}

			width_global = 16;

			while (width_global)
			{
				texel = *( r1_software_texture_ptr + (left_s>>16) + ((left_t>>16) << r1_software_twidth_log2) );

				if (texel!=G1_16BIT_CHROMA_565)
				{
					*start_pixel = texel;
				}

				start_pixel++;

				left_s += temp_dsdx;
				left_t += temp_dtdx;

				width_global--;
			}

			left_s = right_s;
			left_t = right_t;

			num_subdivisions--;
		}
	}

	if (num_leftover)
	{
		if (num_leftover > 1)
		{
			if (had_subdivisions!=0)
			{
				ooz_right += (cur_grads.doozdx * num_leftover);
				soz_right += (cur_grads.dsozdx * num_leftover);
				toz_right += (cur_grads.dtozdx * num_leftover);

				right_z = 1.f / ooz_right;
			}
			else
			{
				ooz_right = left->ooz + (cur_grads.doozdx * num_leftover);
				soz_right = left->soz + (cur_grads.dsozdx * num_leftover);
				toz_right = left->toz + (cur_grads.dtozdx * num_leftover);

				right_z = 1.f / ooz_right;
			}

			right_s = qftoi(soz_right * right_z) + cur_grads.s_adjust;
			if (right_s < 0)
			{
				right_s = 0;
			}
			else
			if (right_s > s_mask)
			{
				right_s = s_mask;
			}

			right_t = qftoi(toz_right * right_z) + cur_grads.t_adjust;
			if (right_t < 0)
			{
				right_t = 0;
			}
			else
			if (right_t > t_mask)
			{
				right_t = t_mask;
			}

			temp_dsdx = qftoi((float)(right_s - left_s) * inverse_leftover_lookup[num_leftover]);
			temp_dtdx = qftoi((float)(right_t - left_t) * inverse_leftover_lookup[num_leftover]);

			while (num_leftover)
			{
				texel = *(r1_software_texture_ptr + (left_s>>16) + ((left_t>>16)<<r1_software_twidth_log2));

				if (texel!=G1_16BIT_CHROMA_565)
				{
					*start_pixel = texel;
				}

				start_pixel++;

				left_s += temp_dsdx;
				left_t += temp_dtdx;

				num_leftover--;
			}
		}
		else
		{
			*start_pixel = *(r1_software_texture_ptr + (left_s>>16) + ((left_t>>16)<<r1_software_twidth_log2));
		}
	}
}
//perspective_map_unlit_true_alpha_c.cpp
/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/

void texture_scanline_perspective_unlit_true_alpha(w16 *start_pixel,
												   sw32 start_x,
												   void *_left, //perspective_span *left,
												   sw32 width)
{
	//temporary stuff for lighting calculations
	w32 accumulated_alpha = (1<<12);

	start_pixel = (w16 *)((w8 *)start_pixel + start_x);

	perspective_span *left = (perspective_span *)_left;

	left_z = 1.f / left->ooz;
	left_s = qftoi(left->soz * left_z) + cur_grads.s_adjust;
	left_t = qftoi(left->toz * left_z) + cur_grads.t_adjust;

	sw32 had_subdivisions = width & (~15);

	num_subdivisions = width >> 4;
	num_leftover     = width & 15;

	if (num_subdivisions)
	{
		ooz_right = left->ooz + (cur_grads.doozdxspan);
		soz_right = left->soz + (cur_grads.dsozdxspan);
		toz_right = left->toz + (cur_grads.dtozdxspan);

		right_z = 1.f / ooz_right;

		while (num_subdivisions)
		{

			right_s = qftoi(soz_right * right_z) + cur_grads.s_adjust;
			if (right_s < 0)
			{
				right_s = 0;
			}
			else
			if (right_s > s_mask)
			{
				right_s = s_mask;
			}

			right_t = qftoi(toz_right * right_z) + cur_grads.t_adjust;
			if (right_t < 0)
			{
				right_t = 0;
			}
			else
			if (right_t > t_mask)
			{
				right_t = t_mask;
			}

			temp_dsdx = (right_s - left_s) >> 4;
			temp_dtdx = (right_t - left_t) >> 4;

			if (num_subdivisions!=1)
			{
				ooz_right += (cur_grads.doozdxspan);
				soz_right += (cur_grads.dsozdxspan);
				toz_right += (cur_grads.dtozdxspan);

				right_z = 1.f / ooz_right;
			}

			width_global = 16;

			while (width_global)
			{
				w16 alpha_texel = *(r1_software_texture_ptr + (left_s>>16) + ((left_t>>16)<<r1_software_twidth_log2));

				w32 l_lookup_1 = (alpha_texel & (w16)(15<<12)) >> (8-NUM_LIGHT_SHADES_LOG2);
				w32 l_lookup_2 = l_lookup_1 ^ (NUM_LIGHT_SHADES<<8);

				w16 texel = alpha_table[alpha_texel & 4095];

				//lookup low bits
				w16 t1 = (w16)((w32 *)(software_color_tables))[l_lookup_1 + (texel & 0xFF)];

				//lookup high bits
				w16 t2 = (w16)((w32 *)(software_color_tables)+ctable_size)[l_lookup_1 + (texel>>8)];

				texel = *start_pixel;

				//lookup low bits
				w16 t3 = (w16)((w32 *)(software_color_tables))[l_lookup_2 + (texel & 0xFF)];

				//lookup high bits
				w16 t4 = (w16)((w32 *)(software_color_tables)+ctable_size)[l_lookup_2 + (texel>>8)];

				*start_pixel = (w16)(t1+t2+t3+t4);

				start_pixel++;

				left_s += temp_dsdx;
				left_t += temp_dtdx;

				width_global--;
			}

			left_s = right_s;
			left_t = right_t;

			num_subdivisions--;
		}
	}

	if (num_leftover)
	{
		if (num_leftover > 1)
		{
			if (had_subdivisions!=0)
			{
				ooz_right += (cur_grads.doozdx * num_leftover);
				soz_right += (cur_grads.dsozdx * num_leftover);
				toz_right += (cur_grads.dtozdx * num_leftover);

				right_z = 1.f / ooz_right;
			}
			else
			{
				ooz_right = left->ooz + (cur_grads.doozdx * num_leftover);
				soz_right = left->soz + (cur_grads.dsozdx * num_leftover);
				toz_right = left->toz + (cur_grads.dtozdx * num_leftover);

				right_z = 1.f / ooz_right;
			}

			right_s = qftoi(soz_right * right_z) + cur_grads.s_adjust;
			if (right_s < 0)
			{
				right_s = 0;
			}
			else
			if (right_s > s_mask)
			{
				right_s = s_mask;
			}

			right_t = qftoi(toz_right * right_z) + cur_grads.t_adjust;
			if (right_t < 0)
			{
				right_t = 0;
			}
			else
			if (right_t > t_mask)
			{
				right_t = t_mask;
			}

			temp_dsdx = qftoi((float)(right_s - left_s) * inverse_leftover_lookup[num_leftover]);
			temp_dtdx = qftoi((float)(right_t - left_t) * inverse_leftover_lookup[num_leftover]);

			while (num_leftover)
			{
				register w32 alpha_texel = *(r1_software_texture_ptr + (left_s>>16) + ((left_t>>16)<<r1_software_twidth_log2));

				register w32 l_lookup = (alpha_texel & (w16)(15<<12)) >> (8-NUM_LIGHT_SHADES_LOG2);

				if (l_lookup)
				{
					register w32 texel = (w32)alpha_table[alpha_texel & 4095];

					//lookup low bits
					register w32 output_texel = ((w32 *)(software_color_tables))[l_lookup + (texel & 0xFF)];

					//lookup high bits
					output_texel += ((w32 *)(software_color_tables)+ctable_size)[l_lookup + (texel>>8)];

					l_lookup ^= (NUM_LIGHT_SHADES<<8);

					if (l_lookup)
					{
						texel       = (w32)*start_pixel;

						//lookup low bits
						output_texel += ((w32 *)(software_color_tables))[l_lookup + (texel & 0xFF)];

						//lookup high bits
						output_texel += ((w32 *)(software_color_tables)+ctable_size)[l_lookup + (texel>>8)];
					}

					*start_pixel = (w16)(output_texel);
				}

				start_pixel++;

				left_s += temp_dsdx;
				left_t += temp_dtdx;

				num_leftover--;
			}
		}
		else
		{
			register w32 alpha_texel = *(r1_software_texture_ptr + (left_s>>16) + ((left_t>>16)<<r1_software_twidth_log2));

			register w32 l_lookup = (alpha_texel & (w16)(15<<12)) >> (8-NUM_LIGHT_SHADES_LOG2);

			if (l_lookup)
			{
				register w32 texel = (w32)alpha_table[alpha_texel & 4095];

				//lookup low bits
				register w32 output_texel = ((w32 *)(software_color_tables))[l_lookup + (texel & 0xFF)];

				//lookup high bits
				output_texel += ((w32 *)(software_color_tables)+ctable_size)[l_lookup + (texel>>8)];

				l_lookup ^= (NUM_LIGHT_SHADES<<8);

				if (l_lookup)
				{
					texel       = (w32)*start_pixel;

					//lookup low bits
					output_texel += ((w32 *)(software_color_tables))[l_lookup + (texel & 0xFF)];

					//lookup high bits
					output_texel += ((w32 *)(software_color_tables)+ctable_size)[l_lookup + (texel>>8)];
				}

				*start_pixel = (w16)(output_texel);
			}
		}
	}
}
//solid_blend_half_c.cpp
/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/


void texture_scanline_solid_blend_half(w16 *start_pixel,
									   sw32 start_x,
									   void *left, //solid_blend_span *left,
									   sw32 width)
{
	w16 color = (((solid_blend_span *)left)->color & pre_blend_and)>>1;

	start_pixel = (w16 *)((w8 *)start_pixel + start_x);

	while (width)
	{
		*start_pixel = color + ((*start_pixel & pre_blend_and)>>1);
		start_pixel++;

		width--;
	}
}

void texture_scanline_solid_true_alpha(w16 *start_pixel,
									   sw32 start_x,
									   void *left, //solid_blend_span *left,
									   sw32 width)
{
	register w32 pixel;
	register w32 color = ((solid_blend_span *)left)->color;
	register w32 alpha = ((solid_blend_span *)left)->alpha & NUM_LIGHT_SHADES;

	if (!alpha)
	{
		return;
	}

	//lookup low bits
	register w32 output_color = ((w32 *)(software_color_tables))[alpha + (color & 0xFF)];

	//lookup high bits
	output_color += ((w32 *)(software_color_tables)+ctable_size)[alpha + (color>>8)];

	color = output_color;

	alpha ^= (NUM_LIGHT_SHADES<<8);

	if (alpha)
	{
		while (width)
		{
			output_color = color;

			pixel = (w32)(*start_pixel);

			//lookup low bits
			output_color += ((w32 *)(software_color_tables))[alpha + (pixel & 0xFF)];

			//lookup high bits
			output_color += ((w32 *)(software_color_tables)+ctable_size)[alpha + (pixel>>8)];

			*start_pixel = (w16)output_color;

			start_pixel++;
			width--;
		}
	}
	else
	{
		while (width)
		{
			*start_pixel = (w16)color;
			start_pixel++;

			width--;
		}
	}
}
//solid_fill_c.cpp
/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/


void texture_scanline_solid_fill(w16 *start_pixel,
								 sw32 start_x,
								 void *left, //solid_blend_span *left,
								 sw32 width)
{
	w16 color = ((solid_blend_span *)left)->color;

	start_pixel = (w16 *)((w8 *)start_pixel + start_x);

	while (width)
	{
		*start_pixel = color;
		start_pixel++;

		width--;
	}
}



#endif //USE_ASM
