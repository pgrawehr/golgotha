/**********************************************************************
 \file
   Defines the structures used for miplevel handling.
   <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/

#ifndef _MIP_HH_
#define _MIP_HH_

#define R1_MAX_MIP_LEVELS  8

#include "arch.h"
#include "file/file.h"
#include <stdio.h>
#include "palette/pal.h"

typedef void *r1_vram_handle_type;

inline int r1_mip_header_disk_size()
{
	return 4 * R1_MAX_MIP_LEVELS + 4 + 256 +2+2+1+1;
}

struct mipheader_t
{
	sw32 offsets[R1_MAX_MIP_LEVELS];
	w32 average_color;
	char tname[256];
	sw16 base_width,base_height;
	w8 flags;
	sw8 num_mip_levels;

	void write(i4_file_class *fp)
	{
		for (int i=0; i<R1_MAX_MIP_LEVELS; i++)
		{
			fp->write_32(offsets[i]);
		}
		fp->write_32(average_color);
		fp->write(tname, 256);
		fp->write_16(base_width);
		fp->write_16(base_height);
		fp->write_8(flags);
		fp->write_8(num_mip_levels);
	}

	void read(i4_file_class *fp)
	{
		for (int i=0; i<R1_MAX_MIP_LEVELS; i++)
		{
			offsets[i]=fp->read_32();
		}
		average_color=fp->read_32();
		fp->read(tname, 256);
		base_width=fp->read_16();
		base_height=fp->read_16();
		flags=fp->read_8();
		num_mip_levels=fp->read_8();
	}
};

#define R1_MIP_IS_TRANSPARENT      1
#define R1_MIP_IS_ALPHATEXTURE     2
#define R1_MIP_IS_JPG_COMPRESSED  64

//use this only temporarily. isnt necessarily maintained
#define R1_MIP_EXTRA_FLAG                32

class r1_miplevel_t;
class r1_texture_entry_struct;

#define R1_MIP_LOAD_NO_ROOM 1
#define R1_MIP_LOAD_BUSY    2
#define R1_MIP_LOAD_MISSING 3

#define R1_MIPFLAGS_USE8 1
#define R1_MIPFLAGS_USE16 2
#define R1_MIPFLAGS_USE24 4
#define R1_MIPFLAGS_USE32 8
#define R1_MIPFLAGS_USEFLAGS 15
#define R1_MIPFLAGS_FORCE8 16
#define R1_MIPFLAGS_FORCE16 32
#define R1_MIPFLAGS_FORCE24 64
#define R1_MIPFLAGS_FORCE32 128
#define R1_MIPFLAGS_FORCEFLAGS (16+32+64+128)
#define R1_MIPFLAGS_SRC8 256
#define R1_MIPFLAGS_SRC16 512
#define R1_MIPFLAGS_SRC24 1024
#define R1_MIPFLAGS_SRC32 2048
#define R1_MIPFLAGS_SRCFLAGS (256+512+1024+2048)
#define R1_MIPFLAGS_HASCHROMA 4096
#define R1_MIPFLAGS_HASALPHA 8192



class r1_mip_load_info
{
public:
	r1_miplevel_t *dest_mip;
	i4_file_class *src_file;
	w32 flags;
	w8 error;
	char texture_name[200];
};

w32 r1_pal_to_flags(const i4_pal *pal,w32 def);

#define R1_MIPLEVEL_IS_LOADING 1
#define R1_MIPLEVEL_LOAD_JPG   2
#define R1_MIPLEVEL_JPG_ALREADY_LOADED 4
#define R1_MIPLEVEL_RAW_PRESENT 8
#define R1_MIPLEVEL_LOAD_32BIT 16
#define R1_MIPLEVEL_LOAD_16BIT 0 //the default
#define R1_MIPLEVEL_LOAD_24BIT 32
#define R1_MIPLEVEL_LOAD_CHROMA 64
#define R1_MIPLEVEL_LOAD_ALPHA 128

class r1_miplevel_t
{
public:
	r1_miplevel_t()
	{
		level=0;
		width=height=0;
		vram_handle=0;
		entry=0;
		last_frame_used=0;
		flags=0;
	}

	//every time get_texture() is called and this miplevel is returned,
	//the current frame # is copied into last_frame_used
	sw32 last_frame_used;

	r1_texture_entry_struct *entry;
	r1_vram_handle_type vram_handle; //this is just a void * (defined above)

	sw16 width,height; //not technically necessary, can be recalculated via entry and level
	w8 level;
	w8 flags;
};


struct i4_pixel_format;
void r1_setup_decompression(i4_pixel_format *reg_fmt,
							i4_pixel_format *chroma_fmt,
							i4_pixel_format *alpha_fmt,
							w32 chroma_color,
							i4_bool square_textures);

void r1_end_decompression();

i4_bool r1_decompress_to_local_mip(i4_file_class *src_file,
								   i4_file_class *dst_lowest_mip,
								   char *network_file,
								   char *local_file,
								   mipheader_t *old_mipheader,
								   sw32 max_mip_dimention);

void process_intern_jpg_decomp_dummy(i4_file_class *dst_file,
									 i4_file_class *dst_lowmip_file,
									 i4_file_class *src_file,
									 mipheader_t &new_mipheader,
									 mipheader_t *&old_mipheader,
									 sw32 &start_here);
void process_intern_raw_decomp(i4_file_class *dst_file,
							   i4_file_class *dst_lowmip_file,
							   i4_file_class *src_file,
							   mipheader_t &new_mipheader,
							   mipheader_t *&old_mipheader,
							   sw32 &start_here);
#define EXTENDED_MIP_BUFFER "tex_mip.dat"

//i4_file_class *build_temp_tex_file(w32 id);
class i4_image_class;
void get_header_info(mipheader_t &header, i4_image_class *src_texture,
					 char *texture_name, w32 chroma_color);
void get_mip_size(sw32 miplevel,sw32 width,sw32 height, sw32 &mipwidth, sw32 &mipheight);

#endif
