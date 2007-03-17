/*
 **************************************************************************
   Author: Terry Caton
   Date: January 24th, 2001
   Contact: tcaton@umr.edu
   Open Source. Use as you wish, but don't bitch if it breaks something. If you
   use this in any project worth mentioning, let me know, I'd love to see what
   you've done.

   Here's the framework for my 3D Studio File Loader. Currently, it supports
   loading the vertex/index list (of course), along with materials and hierarchy
   information. This is only a framework, however; you need to supply the code
   to input the data into your own buffers, as each API varies widely, as does
   each persons preference of how to organize data. At each point where data is
   read from the file, take that data and feed it into a buffer, or just discard
   it if you wish.

   The way I did it, and the way the framework here encourages, is to save all
   the data into a 3dsObject class (one for each object in the file), and then at
   the end when everything is done, assemble each into it's compiled vertex array/
   vertex buffer/whatever. Granted, this is my first attempt at a 3D engine, so
   there may be a few fundamental flaws here, but hey, go easy on me.

   If you so desire, contact me at tcaton@umr.edu. Questions, comments, money,
   whatever. Source to my engine is also available upon request, with a working
   example of this file loader in action. Thanks, and enjoy!!


 **************************************************************************
 */



#ifndef _3DS_H
#define _3DS_H

#include <stdio.h>


//>------ Primary Chunk, at the beginning of each file
#define PRIMARY       0x4D4D

//>------ Main Chunks
#define EDIT3DS       0x3D3D
#define VERSION       0x0002
#define MESHVERSION   0x3D3E
#define EDITKEYFRAME  0xB000

//>------ sub defines of EDIT3DS
#define EDITMATERIAL  0xAFFF
#define EDITOBJECT    0x4000

//>------ sub defines of EDITMATERIAL
#define MATNAME       0xA000
#define MATLUMINANCE  0xA010
#define MATDIFFUSE    0xA020
#define MATSPECULAR   0xA030
#define MATSHININESS  0xA040
#define MATMAP        0xA200
#define MATMAPFILE    0xA300

#define OBJTRIMESH    0x4100

//>------ sub defines of OBJTRIMESH
#define TRIVERT       0x4110
#define TRIFACE       0x4120
#define TRIFACEMAT    0x4130
#define TRIUV         0x4140
#define TRISMOOTH     0x4150
#define TRILOCAL      0x4160

//>------ sub defines of EIDTKEYFRAME
#define KFMESH        0xB002
#define KFHEIRARCHY   0xB030
#define KFNAME        0xB010


//>>------  these define the different color chunk types
#define RGBF   0x0010
#define RGB24  0x0011


class Chunk;

class Load3ds
{
public:
	Load3ds();
	~Load3ds();
	void *Create(char *);

private:
	int GetString(char *);
	int ReadChunk(Chunk *);
	int ReadColorChunk(Chunk *, float *);
	int ReadPercentChunk(Chunk *, float *);

	int ProcessNextChunk(Chunk *);
	int ProcessNextObjectChunk(Chunk *);
	int ProcessNextMaterialChunk(Chunk *);
	int ProcessNextKeyFrameChunk(Chunk *);
	int FillVertexBuffer(Chunk *);
	int FillIndexBuffer(Chunk *);
	int FillTexCoordBuffer(Chunk *);
	int SortIndicesByMaterial(Chunk *);

	int ComputeNormals();
	int CompileObjects();
	int CleanUp();

	FILE *mFile;
	void *mBuffer;

	Chunk *mCurrentChunk;
	Chunk *mTempChunk;

};


class Chunk
{
public:
	Chunk();
	~Chunk();

	unsigned short int mID;
	unsigned int mLength;
	unsigned int mBytesRead;
};


#endif
