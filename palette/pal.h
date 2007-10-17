/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/

#ifndef __PALETTE_HPP_
#define __PALETTE_HPP_

#include "arch.h"
#include "isllist.h"
#include "init/init.h"
typedef unsigned char w24[3];
enum i4_pixel_depth
{
	I4_32BIT=32,    // 16.7 mil & alpha, or 16.7 mil & padding, used on most modern graphic cards
	I4_24BIT=24,    // 16.7 mil, no alpha, used by older cards to support true color but save memory
	I4_16BIT=16,    // 64K color
	I4_8BIT=8,     // 256 color
	I4_4BIT=4,     // 16 color
	I4_2BIT=2      // 2 color
} ;

class i4_file_class;
struct i4_pixel_format
{
	// these not valid for 2,4, and 8 bit pixel depths
	w32 red_mask, red_shift, red_bits;
	w32 green_mask, green_shift, green_bits;
	w32 blue_mask, blue_shift, blue_bits;
	w32 alpha_mask, alpha_shift, alpha_bits;

	// lookup is not valid for 16 & 32 bit pixel depths
	w32 * lookup;

	i4_pixel_depth pixel_depth;

	void calc_shift();

	void default_format();
	w32 write(i4_file_class * f);
	void read(i4_file_class * f);
};

class i4_pal
{
public:
	i4_pal * next;

	i4_pixel_format source;

	virtual i4_bool can_convert(const i4_pixel_format * source_fmt,
								const i4_pixel_format * dest_fmt) const = 0;

	virtual i4_color convert(const i4_color source_pixel,
							 const i4_pixel_format * dest_fmt) const = 0;

	virtual ~i4_pal()
	{
		;
	}
};

class i4_lookup_pal :
	public i4_pal
{
public:
	virtual i4_bool can_convert(const i4_pixel_format * source,
								const i4_pixel_format * dest) const;

	virtual i4_color convert(const i4_color source,
							 const i4_pixel_format * dest) const;

	i4_lookup_pal(i4_pixel_format * source);
	virtual ~i4_lookup_pal();

};

class i4_rgb_pal :
	public i4_pal
{
public:
	i4_rgb_pal(i4_pixel_format * source);

	virtual i4_bool can_convert(const i4_pixel_format * source,
								const i4_pixel_format * dest) const;

	virtual i4_color convert(const i4_color source,
							 const i4_pixel_format * dest) const;


};


class i4_pal_manager_class :
	public i4_init_class
{
	i4_isl_list<i4_pal> pal_list;

	i4_pal * def32, * def_na_32, * def8, * def_na_24;

public:
	void init();
	void uninit();

	i4_pal *register_pal(i4_pixel_format * format);

	// c will be converted from 32bit color to dest_format
	i4_color convert_32_to(i4_color c, const i4_pixel_format * dest_format) const;
	i4_color convert_24_to(i4_color c, const i4_pixel_format * dest_format) const;

	// c will be converted from source_format to 32bit color
	i4_color convert_to_32(i4_color c, const i4_pal * source_format) const;
	// c will be converted to 24bit
	i4_color convert_to_24(i4_color c, const i4_pal * source_format) const;

	const i4_pal *default_no_alpha_32()
	{
		return def_na_32;
	}
	const i4_pal *default_no_alpha_24()
	{
		return def_na_24;
	}
	const i4_pal *default_32()
	{
		return def32;
	}
	const i4_pal *default_8()
	{
		return def8;
	}
} ;

extern i4_pal_manager_class i4_pal_man;

w32 g1_light_color(w32 color_32bit, float intensity);

#endif
