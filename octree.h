#ifndef _OCTREE_H
#define _OCTREE_H

//#include "Main.h"
//#include "3ds.h"
#include "math/transform.h"
#include "math/vector.h"
#include "maxtool/render2.h"
#include "draw_context.h"
#include "obj3d.h"

class g1_octree_globals:public i4_init_class
	{
	public:
// The maximum amount of triangles per node
	static int g_MaxTriangles;

// The maximum amount of subdivisions allow (Levels of subdivision)
	static int g_MaxSubdivisions;

// The amount of end nodes created in the octree (That hold vertices)
	static int g_EndNodeCount;

// This stores the amount of nodes that are in the frustum
    static int g_TotalNodesDrawn;

// Turn lighting on initially
    static bool g_bLighting;

// This stores the render mode (LINES = false or TRIANGLES = true)
    static bool g_RenderMode;

	void init()
		{
		g_MaxTriangles=200;
		g_EndNodeCount=0;
		g_bLighting=true;
		g_RenderMode=true;
		g_MaxSubdivisions=6;
		}
	};


// These values (0 - 7) store the index ID's for the octree node array (m_pOctreeNodes)
enum eOctreeNodes
{
	TOP_LEFT_FRONT,			// 0
	TOP_LEFT_BACK,			// 1
	TOP_RIGHT_BACK,			// etc...
	TOP_RIGHT_FRONT,
	BOTTOM_LEFT_FRONT,
	BOTTOM_LEFT_BACK,
	BOTTOM_RIGHT_BACK,
	BOTTOM_RIGHT_FRONT
};


/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *

// This is used for our pLists in CreateNode() to help partition the world into
// different nodes.

struct tFaceList
{
	// This is a vector of booleans to store if the face index is in the nodes 3D Space
	i4_array<i4_bool> pFaceList;	

	// This stores the total face count that is in the nodes 3D space (how many "true"'s)
	int totalFaceCount;
	tFaceList():pFaceList(0,100),totalFaceCount(0){};
};

/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *


// This is our debug lines class to view the octree visually
class g1_octree_debug : public i4_init_class
{

public:

	g1_octree_debug();

	// This adds a line to our list of debug lines
	void AddDebugLine(i4_3d_vector vPoint1, i4_3d_vector vPoint2);

	// This adds a rectangle with a given center, width, height and depth to our list
	void AddDebugRectangle(i4_3d_vector vCenter, float width, float height, float depth);

	// This renders all of the lines
	void RenderDebugLines(i4_transform_class *transform);		

	// This clears all of the debug lines
	void Clear();				
	~g1_octree_debug();
	void uninit();

private:
	//r1_render_api_class *api;
	// This is the vector list of all of our lines
	i4_array<i4_3d_vector> m_vLines;		
};

typedef i4_array<g1_quad_class *> g1_quadlist;
extern g1_octree_debug g_Debug;

// This is our octree class
class g1_octree
{

	protected:

	// The constructor and deconstructor
	g1_octree();
	
	public:
	static g1_octree *Build(g1_quad_object_class *pWorld);

	
	
	~g1_octree();

	// This returns the center of this node
	i4_3d_vector GetCenter() {	 return m_vCenter;	}

	// This returns the triangle count stored in this node
	int GetTriangleCount()  {   return m_TriangleCount;	}

	// This returns the widht of this node (since it's a cube the height and depth are the same)
	//float GetWidth() {	 return m_Width;	}

		// This returns if this node is subdivided or not
	bool IsSubDivided()  {   return m_bSubDivided;	}


/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *

	// This sets the initial width, height and depth for the whole scene
	void GetSceneDimensions(g1_quad_object_class *pWorld);

	// This returns the number of polygons in our entire scene
	int GetSceneTriangleCount(g1_quad_object_class *pWorld);

	// This returns this nodes display list ID
	//int GetDisplayListID()		{   return m_DisplayListID;		}

	// This sets the nodes display list ID
	//void SetDisplayListID(int displayListID)	{	m_DisplayListID = displayListID;  }

	// This adds the object index to our end node's list of object indices
	//void AddObjectIndexToList(int index);

/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *


	// This takes in the previous nodes center, width and which node ID that will be subdivided
	i4_3d_vector GetNewNodeCenter(i4_3d_vector vCenter, 
		float xwidth, float ywidth, float zwidth, int nodeID);


/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *

	// This subdivides a node depending on the triangle count and node width
	i4_bool CreateNode(g1_quad_object_class *pWorld, tFaceList *pList, int numberOfTriangles, i4_3d_vector vCenter, 
		float xwidth,float ywidth, float zwidth);

	// This cleans up the new subdivided node creation process, so our code isn't HUGE!
	i4_bool CreateNewNode(g1_quad_object_class *pWorld,	tFaceList &pList, int triangleCount,
				  	   i4_3d_vector vCenter, 
					   float xwidth, float ywidth, float zwidth, 
					   int nodeID);

	// Once we are finished subdividing we need to assign the face indices to the end node.
	void AssignTrianglesToNode(g1_quad_object_class *pWorld, int numberOfTriangles);

	// This goes through each of the nodes and then draws the end nodes vertices.
	// This function should be called by starting with the root node.  Now it uses display lists.
	void DrawOctree(i4_transform_class *transform, g1_quadlist &quads);

	// This recursively creates a display list ID for every end node in the octree
	//PG: We can't use display lists with the golg engine, so this
	//most probably will be dropped
	//void CreateDisplayList(g1_octree *pNode, g1_quad_object_class *pRootWorld, int displayListOffset);

/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *


private:

	// This tells us if we have divided this node into more sub nodes
	bool m_bSubDivided;

	//we need this to be static, because the subdivision goes trough
	//multiple instances.
	//we can safely assume that we're not generating two octrees
	//simultaneously.
	static int m_CurrentSubdivision;

	// These are the size of the cube for this current node
public:
	float m_xWidth;
	float m_yWidth;
	float m_zWidth;

	// This holds the amount of triangles stored in this node
	int m_TriangleCount;

	// This is the center (X, Y, Z) point in this node
	i4_3d_vector m_vCenter;


/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *
private:
	// This holds all the scene information (verts, normals, texture info, etc..) for this node
	g1_quad_object_class *m_pWorld;

	//This stores a list of all the quads in this node.
	i4_array<int>	m_pQuadList;

	// This holds the display list ID for the current node, which increases the rendering speed
	//int m_DisplayListID;

/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *


	// These are the eight nodes branching down from this current node
	g1_octree *m_pOctreeNodes[8];	
};


// This returns the cross product between 2 vectors
//CVector3 Cross(CVector3 vVector1, CVector3 vVector2);

// This returns the magnitude of a vector
//float Magnitude(CVector3 vNormal);

// This returns a normalized vector
//CVector3 Normalize(CVector3 vVector);
class i4_grow_heap_class;
extern i4_grow_heap_class *g1_object_heap;

extern g1_octree_debug g_Debug;

#endif


/////////////////////////////////////////////////////////////////////////////////
//
// * QUICK NOTES * 
//
// This file added the new tFaceList structure, along with a couple functions
// and member variables to the COctree class.  Most of the functions were changed
// to handle a t3DModel instead of just pure vertices.  Since we are using display
// lists, a bunch of display list data and functions were added.  
//
//
// Ben Humphrey (DigiBen)
// Game Programmer
// DigiBen@GameTutorials.com
// www.GameTutorials.com
//
//
