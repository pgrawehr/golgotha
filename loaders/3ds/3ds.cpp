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


Special thanks to John Ratcliff and all the guys I worked with, for getting
me interested in all this. 


**************************************************************************
*/



#include "3ds.h"
#include <stdlib.h>
#include <string.h>


// Global instance of loader
Load3ds gLoad3ds;

Load3ds::Load3ds()
{
	mBuffer = 0;
	mCurrentChunk = 0;
	mTempChunk = new Chunk;

	// initialize any other members you choose to add
}


Load3ds::~Load3ds()
{
	delete mTempChunk;

}


void * Load3ds::Create(char * aFilename)
{
	int lBytesRead = 0;
	mCurrentChunk = new Chunk;

	mFile = fopen(aFilename, "rb");
	ReadChunk(mCurrentChunk);
	if (mCurrentChunk->mID != PRIMARY)
		exit(1107);
	// Largest possible buffer we'd need is MAX_INDICES * sizeof(triangle)
	mBuffer = new unsigned char[65536 * 3 * sizeof(float)];

	// Begin loading objects, by calling recursive function
	ProcessNextChunk(mCurrentChunk);

	// All data read, compile them into a friendly format
	CompileObjects();

	// Clean up after everything, to prepare for next time
	CleanUp();

	return 0;
}


int Load3ds::CleanUp()
{

	fclose(mFile);
	delete [] mBuffer;
	delete mCurrentChunk;
	
/*
Add any cleanup routines you may need between calls to Load3ds::Create(),
such as deleting temporary vertex/index lists, materials, or whatever
else you choose to add
*/

	return 1;
}


int Load3ds::ProcessNextChunk(Chunk * aPreviousChunk)
{
	mCurrentChunk = new Chunk;

	while (aPreviousChunk->mBytesRead < aPreviousChunk->mLength)
	{
		// Read next chunk
		ReadChunk(mCurrentChunk);

		switch (mCurrentChunk->mID)
		{
		case VERSION:
			// Check version (must be 3 or less)
			mCurrentChunk->mBytesRead += fread(mBuffer, 1, mCurrentChunk->mLength - mCurrentChunk->mBytesRead, mFile);
			if (*(unsigned short *) mBuffer > 0x03)
				exit(1107);
			break;

		case EDITMATERIAL:
			// Proceed to material loading function
			ProcessNextMaterialChunk(mCurrentChunk);
			break;

		case EDIT3DS:
			// Check mesh verion, then proceed to mesh loading function			
			ReadChunk(mTempChunk);
			mTempChunk->mBytesRead += fread(mBuffer, 1, mTempChunk->mLength - mTempChunk->mBytesRead, mFile);
			mCurrentChunk->mBytesRead += mTempChunk->mBytesRead;
			if (mTempChunk->mID != MESHVERSION || *(unsigned short *)mBuffer > 0x03)
				exit(1107);
			ProcessNextChunk(mCurrentChunk);
			break;

		case EDITOBJECT:
			mCurrentChunk->mBytesRead += GetString((char *)mBuffer);
			// mBuffer now contains name of the object to be edited
			ProcessNextObjectChunk(mCurrentChunk);
			break;

		case EDITKEYFRAME:
			ProcessNextKeyFrameChunk(mCurrentChunk);
			break;

		default:  // unrecognized/unsupported chunk
			mCurrentChunk->mBytesRead += fread(mBuffer, 1, mCurrentChunk->mLength - mCurrentChunk->mBytesRead, mFile);
			break;
		}

	aPreviousChunk->mBytesRead += mCurrentChunk->mBytesRead;
	}

	delete mCurrentChunk;
	mCurrentChunk = aPreviousChunk;

	return 1;
}


int Load3ds::ProcessNextObjectChunk(Chunk * aPreviousChunk)
{
	mCurrentChunk = new Chunk;

	while (aPreviousChunk->mBytesRead < aPreviousChunk->mLength)
	{
		ReadChunk(mCurrentChunk);

		switch (mCurrentChunk->mID)
		{
		case OBJTRIMESH:
			// at this point, mBuffer will contain the name of the object being described
			ProcessNextObjectChunk(mCurrentChunk);
			ComputeNormals();

			break;

		case TRIVERT:
			FillVertexBuffer(mCurrentChunk);
			break;

		case TRIFACE:
			FillIndexBuffer(mCurrentChunk);
			break;

		case TRIFACEMAT:
			// your getting a list of triangles that belong to a certain material
			SortIndicesByMaterial(mCurrentChunk);
			break;

		case TRIUV:
			FillTexCoordBuffer(mCurrentChunk);
			break;

		default:  // unrecognized/unsupported chunk
			mCurrentChunk->mBytesRead += fread(mBuffer, 1, mCurrentChunk->mLength - mCurrentChunk->mBytesRead, mFile);
			break;
		}

	aPreviousChunk->mBytesRead += mCurrentChunk->mBytesRead;
	}

	delete mCurrentChunk;
	mCurrentChunk = aPreviousChunk;

	return 1;
}


int Load3ds::ProcessNextMaterialChunk(Chunk * aPreviousChunk)
{
	mCurrentChunk = new Chunk;

	while (aPreviousChunk->mBytesRead < aPreviousChunk->mLength)
	{
		ReadChunk(mCurrentChunk);

		switch (mCurrentChunk->mID)
		{
		case MATNAME:
			mCurrentChunk->mBytesRead += fread(mBuffer, 1, mCurrentChunk->mLength - mCurrentChunk->mBytesRead, mFile);
			break;

		case MATLUMINANCE:
			ReadColorChunk(mCurrentChunk, (float *) mBuffer);
			break;

		case MATDIFFUSE:
			ReadColorChunk(mCurrentChunk, (float *) mBuffer);
			break;

		case MATSPECULAR:
			ReadColorChunk(mCurrentChunk, (float *) mBuffer);
			break;
		
		case MATSHININESS:
			ReadColorChunk(mCurrentChunk, (float *) mBuffer);
			break;
		
		case MATMAP:
			// texture map chunk, proceed
			ProcessNextMaterialChunk(mCurrentChunk);
			break;

		case MATMAPFILE:
			mCurrentChunk->mBytesRead += fread((char *)mBuffer, 1, mCurrentChunk->mLength - mCurrentChunk->mBytesRead, mFile);
			// mBuffer now contains the filename of the next texture; load it if you wish
			break;
		
		default:  // unrecognized/unsupported chunk
			mCurrentChunk->mBytesRead += fread(mBuffer, 1, mCurrentChunk->mLength - mCurrentChunk->mBytesRead, mFile);
			break;
		}

	aPreviousChunk->mBytesRead += mCurrentChunk->mBytesRead;
	}

	delete mCurrentChunk;
	mCurrentChunk = aPreviousChunk;

	return 1;
}


int Load3ds::ProcessNextKeyFrameChunk(Chunk * aPreviousChunk)
{
	mCurrentChunk = new Chunk;

	short int lCurrentID, lCurrentParentID;
	
	while (aPreviousChunk->mBytesRead < aPreviousChunk->mLength)
	{
		ReadChunk(mCurrentChunk);

		switch (mCurrentChunk->mID)
		{
		case KFMESH:
			ProcessNextKeyFrameChunk(mCurrentChunk);
			break;

		case KFHEIRARCHY:
			mCurrentChunk->mBytesRead += fread(&lCurrentID, 1, 2, mFile);
			// lCurrentID now contains the ID of the current object being described
			//  Save this if you want to support an object hierarchy
			break;

		case KFNAME:
			mCurrentChunk->mBytesRead += GetString((char *)mBuffer);
			// mBuffer now contains the name of the object whose KF info will
			//   be described

			mCurrentChunk->mBytesRead += fread(mBuffer, 1, 4, mFile);  // useless, ignore
			mCurrentChunk->mBytesRead += fread(&lCurrentParentID, 1, 2, mFile);
			// lCurrentParentID now contains the ID of the parent of the current object 
			// being described
			break;

		default:  // unrecognized/unsupported chunk
			mCurrentChunk->mBytesRead += fread(mBuffer, 1, mCurrentChunk->mLength - mCurrentChunk->mBytesRead, mFile);
			break;
		}

	aPreviousChunk->mBytesRead += mCurrentChunk->mBytesRead;
	}

	delete mCurrentChunk;
	mCurrentChunk = aPreviousChunk;

	return 1;
}


int Load3ds::GetString(char * aBuffer)
{
	unsigned int lBytesRead = 0;
	int index = 0;

	fread(aBuffer, 1, 1, mFile);

	while (*(aBuffer + index) != 0)
		fread(aBuffer + ++index, 1, 1, mFile);

	return strlen(aBuffer) + 1;
}


int Load3ds::ReadChunk(Chunk * aChunk)
{
	aChunk->mBytesRead = fread(&aChunk->mID, 1, 2, mFile);
	aChunk->mBytesRead += fread(&aChunk->mLength, 1, 4, mFile);

	return 1;
}


int Load3ds::ReadColorChunk(Chunk * aChunk, float * aVector)
{
	ReadChunk(mTempChunk);
	mTempChunk->mBytesRead += fread(mBuffer, 1, mTempChunk->mLength - mTempChunk->mBytesRead, mFile);

	aChunk->mBytesRead += mTempChunk->mBytesRead;
	return 1;
}


int Load3ds::ReadPercentChunk(Chunk * aChunk, float * aPercent)
{
	ReadChunk(mTempChunk);
	mTempChunk->mBytesRead += fread(mBuffer, 1, mTempChunk->mLength - mTempChunk->mBytesRead, mFile);
	*aPercent = (float) *((short int *) mBuffer) / 100.0f;

	aChunk->mBytesRead += mTempChunk->mBytesRead;
	return 1;
}


int Load3ds::FillIndexBuffer(Chunk * aPreviousChunk)
{
	short int lNumFaces;
	aPreviousChunk->mBytesRead += fread(&lNumFaces, 1, 2, mFile);
	aPreviousChunk->mBytesRead += fread(mBuffer, 1, aPreviousChunk->mLength - aPreviousChunk->mBytesRead, mFile);
	// mBuffer now contains an array of indices (unsigned short ints)
	//   Careful, the list consists of 3 vertex indices and then an edge
	//   flag (safe to ignore, probably
	//   * bit 0: CA visible, bit 1: BC visible, bit 2: AB visible

	return 1;
}


int Load3ds::SortIndicesByMaterial(Chunk * aPreviousChunk)
{
	unsigned short int lNumFaces;
	aPreviousChunk->mBytesRead += GetString((char *) mBuffer);
	// mBuffer contains the name of the material that is associated
	//  with the following triangles (set of 3 indices which index into the vertex list
	//  of the current object chunk)

	aPreviousChunk->mBytesRead += fread(&lNumFaces, 1, 2, mFile);

	aPreviousChunk->mBytesRead += fread(mBuffer, 1, aPreviousChunk->mLength - aPreviousChunk->mBytesRead, mFile);
	// mBuffer now contains a list of triangles that use the material specified above

	return 1;
}


int Load3ds::FillTexCoordBuffer(Chunk * aPreviousChunk)
{
	int lNumTexCoords;
	aPreviousChunk->mBytesRead += fread(&lNumTexCoords, 1, 2, mFile);
	aPreviousChunk->mBytesRead += fread(mBuffer, 1, aPreviousChunk->mLength - aPreviousChunk->mBytesRead, mFile);
	// mBuffer now contains a list of UV coordinates (2 floats)

	return 1;
}


int Load3ds::FillVertexBuffer(Chunk * aPreviousChunk)
{
	int lNumVertices;
	aPreviousChunk->mBytesRead += fread(&lNumVertices, 1, 2, mFile);
	aPreviousChunk->mBytesRead += fread(mBuffer, 1, aPreviousChunk->mLength - aPreviousChunk->mBytesRead, mFile);
	// mBuffer now contains a list of vertex coordinates (3 floats)

	return 1;
}


int Load3ds::ComputeNormals()
{
/*
Compute your normals here. Quick way of doing it (no smoothing groups, though):

  for (each vertex in vertex list)
	{
	for (each triangle in triangle list)
	  {
	  if (vertex is in triangle)
	    {
		compute normal of triangle
		add to total_normal
		}
	  }
	normalize(total_normal)
	save_to_normal_list(total_normal)
	total_normal = 0
	}
	
This seemed to work well for me, though it's my first time having to compute
normals myself. One might think that normalizing the normal right after it's 
computed and added to total_normal would be best, but I've found this not to be
the case. If this is done, then small triangles have just as much influence on the 
final normal as larger triangles. My way, the model comes out looking much more 
smooth, especially if there's small flaws in the model. 
*/

	return 1;
}


int Load3ds::CompileObjects()
{

/*
By now, you should have a collection of objects (or a single object), with 
proper hierarchy information, vertices, indices, and materials. Now's your
time to assemble these into however you want them to send them down the 
OGL/D3D pipeline.

*/
	return 1;
}


///////////////////////////////////////////////////////////////////



Chunk::Chunk()
{
	mID = 0;
	mLength = 0;
	mBytesRead = 0;
}


Chunk::~Chunk()
{
}
