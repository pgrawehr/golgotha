//  A file of the revival project. 
/*! \file octree.h
The definition file for the octree classes.
This file contains the declarations for creating octrees. 

\par Disclaimer 
This code has been adapted from an octree tutorial by \n
Ben Humphrey (DigiBen) \n
Game Programmer \n
mailto:DigiBen@GameTutorials.com \n
http://www.GameTutorials.com \n

*/

#ifndef _OCTREE_H
#define _OCTREE_H

//#include "Main.h"
//#include "3ds.h"
#include "math/transform.h"
#include "math/vector.h"
#include "maxtool/render2.h"
#include "draw_context.h"
#include "obj3d.h"
//! Globals used by the octree creation routine. 
//! The default values for the fields of this class are experimental
//! and possibly need some further adjustments. 
class g1_octree_globals:public i4_init_class
	{
	public:
//! The maximum amount of triangles per node.
	static int g_MaxTriangles;

//! The maximum amount of subdivisions allow (Levels of subdivision).
	static int g_MaxSubdivisions;

//! The amount of end nodes created in the octree (That hold vertices).
	static int g_EndNodeCount;

//! This stores the amount of nodes that are in the frustum.
    static int g_TotalNodesDrawn;

//! Turn lighting on initially.
    static bool g_bLighting;

//! This stores the render mode (LINES = false or TRIANGLES = true).
    static bool g_RenderMode;

//! The init function inherited from i4_init_class. 
	void init()
		{
		g_MaxTriangles=200;
		g_EndNodeCount=0;
		g_bLighting=true;
		g_RenderMode=true;
		g_MaxSubdivisions=16;
		}
	};


//! These values (0 - 7) store the index ID's for the octree node array (m_pOctreeNodes).
//! The numbering is a bit strange because we merged two projects which had different definitions
//! for these. 
enum eOctreeNodes
{
	TOP_LEFT_FRONT      = 4,			
	TOP_LEFT_BACK       = 6,			
	TOP_RIGHT_BACK      = 7,			
	TOP_RIGHT_FRONT     = 5,
	BOTTOM_LEFT_FRONT   = 0,
	BOTTOM_LEFT_BACK    = 2,
	BOTTOM_RIGHT_BACK   = 3,
	BOTTOM_RIGHT_FRONT  = 1
};

enum eCubeSides
{
	CS_LEFT,
	CS_RIGHT,
	CS_TOP,
	CS_BOTTOM,
	CS_FRONT,
	CS_BACK
};


/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *

//! This is used for our pLists in CreateNode() to help partition the world into
//! different nodes.

struct tFaceList
{
	/// This is a vector of booleans to store if the face index is in the nodes 3D Space
	i4_array<i4_bool> pFaceList;	

	/// This stores the total face count that is in the nodes 3D space (how many "true"'s)
	int totalFaceCount;

	/// The constructor is necessary because i4_array doesn't have a default ctor.
	tFaceList():pFaceList(0,100),totalFaceCount(0){};
};

/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *


/// This is our debug lines class to view the octree visually.
class g1_octree_debug : public i4_init_class
{

public:

	/// The default constructor. 
    /// Initializes the member fields. 
	g1_octree_debug();

	/// This adds a line to our list of debug lines.
    /// Add a line to the line list. 
	/// \param vPoint1 Starting point
	/// \param vPoint2 End point
	void AddDebugLine(i4_3d_vector vPoint1, i4_3d_vector vPoint2);

	/// This adds a rectangle with a given center, width, height and depth to our list.
    /// Add a rectangle of lines. 
	/// \param vCenter The center of the rectangle.
	/// \param width   Size of the rectangle in X-Dimension
	/// \param height  Size of the rectangle in Y-Dimension
	/// \param depth   Size of the rectangle in Z-Dimension
	void AddDebugRectangle(i4_3d_vector vCenter, float width, float height, float depth);

	/// This renders all of the lines.
    /// This is for debugging purposes only!
	/// \param transform The world-to-camera transform
	void RenderDebugLines(i4_transform_class *transform);		

	/// This clears all of the debug lines
	void Clear();				

	/// The destructor; should actually do nothing.
	~g1_octree_debug();
	void uninit();

private:
	/// This is the vector list of all of our lines
	i4_array<i4_3d_vector> m_vLines;		
};

typedef i4_array<g1_quad_class *> g1_quadlist;
extern g1_octree_debug g_Debug;

//These values are used to generate the location codes. I suppose they have
//to be corresponding to the order the sub-nodes are organized.
const int octreeChildCenterOffsetInt[][3] = {
	{-1, -1, -1},
	{ 1, -1, -1},
	{-1,  1, -1},
	{ 1,  1, -1},
	{-1, -1,  1},
	{ 1, -1,  1},
	{-1,  1,  1},
	{ 1,  1,  1}
};


//! This is our octree class.
//! It contains an octree and is the same as a node of the tree.
class g1_octree
{

	protected:

	/// Protected default constructor.
	/// This constructor is only used internally when building the tree.
	g1_octree();
	
	public:
	/// This static function builds an octree from the object
	/// pWorld points to. 
	/// \param pWorld Object a quadtree needs to be built on.
	/// This object won't be modified by the call.
	/// \return An octree of pWorld or NULL if it is useless to build
	/// an octree of this object, i.e. because it contains only very
	/// few vertices. 
	static g1_octree *Build(g1_quad_object_class *pWorld);

	/// The deserialization constructor. 
	/// This constructor loads the octree from the stream fp.
	/// The stream position must already be at set properly.
	/// \param pWorld The (already loaded) object we are loading the
	/// octree for.
	/// \param fp The file stream to be loaded from. 
	g1_octree(g1_quad_object_class *pWorld, i4_loader_class *fp);
	/// The destructor. 
	~g1_octree();

	/// Saves an octree.
	/// Writes a serialized version of the octree to the disk.
	/// \param fp The file pointer to be used.
	void save(i4_saver_class *fp);

	/// This returns the center of this node.
	i4_3d_vector GetCenter() const {	 return m_vCenter;	}

	/// This returns the triangle count stored in this node.
	int GetTriangleCount() const  {   return m_TriangleCount;	}

	// This returns the widht of this node (since it's a cube the height and depth are the same)
	//float GetWidth() {	 return m_Width;	}

	/// This returns if this node is subdivided or not.
	i4_bool IsSubDivided() const {   return m_bSubDivided;	}


/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *

	/// This sets the initial width, height and depth for the whole scene.
	/// It modifies the current instance.
	/// \param pWorld The object we're checking for. 
	void GetSceneDimensions(g1_quad_object_class *pWorld);

	/// This returns the number of polygons in our entire scene.
	/// \param pWorld The master object we're counting triangles on.
	/// \return A triangle count. The returned value is actually the same
	/// as pWorld->num_verts.
	int GetSceneTriangleCount(g1_quad_object_class *pWorld);

	// This returns this nodes display list ID
	//int GetDisplayListID()		{   return m_DisplayListID;		}

	// This sets the nodes display list ID
	//void SetDisplayListID(int displayListID)	{	m_DisplayListID = displayListID;  }

	// This adds the object index to our end node's list of object indices
	//void AddObjectIndexToList(int index);

/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *


	/// This takes in the previous nodes center, width and which node ID that will be subdivided
	/// \param vCenter Center of the new node.
	/// \param xwidth Width of the new Node.
	/// \param ywidth Height of the new Node. 
	/// \param zwidth Depth of the new Node.
	/// \param nodeID ID indicated which center is requested.
	/// \return A vector that point to the center of the new node. 
	i4_3d_vector GetNewNodeCenter(i4_3d_vector vCenter, 
		float xwidth, float ywidth, float zwidth, int nodeID);


/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *

	/// This subdivides a node depending on the triangle count and node width.
	i4_bool CreateNode(g1_quad_object_class *pWorld, tFaceList *pList, int numberOfTriangles, i4_3d_vector vCenter, 
		float xwidth,float ywidth, float zwidth);

	/// This cleans up the new subdivided node creation process, so our code isn't HUGE!
	i4_bool CreateNewNode(g1_quad_object_class *pWorld,	tFaceList &pList, int triangleCount,
				  	   i4_3d_vector vCenter, 
					   float xwidth, float ywidth, float zwidth, 
					   int nodeID);

	/// Once we are finished subdividing we need to assign the face indices to the end node.
	void AssignTrianglesToNode(g1_quad_object_class *pWorld, int numberOfTriangles);

	/// Prepare for the drawing of the octree. 
	/// This goes through each of the nodes and then draws the end nodes vertices.
	/// This function should be called by starting with the root node.  
	/// \param transform The world-to-object transform
	/// \param quads A reference to an array that will contain the list of
	/// quads to be drawn. The list will be returned unsorted and can contain
	/// a large number of duplicate entries.
	/// \param depth The current depth of the tree iteration. Used for internal
	/// bookkeeping. You should usualy pass 0 as this argument. If you
	/// pass anything else, the quad list is always constructed and the
	/// method returns never false. 
	/// \return i4_T if it makes sense to draw using the octree, false
	/// if not. 
	i4_bool DrawOctree(i4_transform_class *transform, g1_quadlist &quads, int depth);

	/// Scale the octree cubes.
	/// If the models are scaled to fit the size of the world (the common
	/// scaling factor for golgotha is 0.1), we also need to scale the
	/// size of the nodes. Otherwise, the clipping yields bogus results.
	/// \param value A factor to scale the model with (relative to the
	/// current size. )
    void scale(i4_float value);

	// This recursively creates a display list ID for every end node in the octree
	//PG: We can't use display lists with the golg engine, so this
	//most probably will be dropped
	//void CreateDisplayList(g1_octree *pNode, g1_quad_object_class *pRootWorld, int displayListOffset);

    int GetNumQuads() const
        {
        return m_TriangleCount;
        };

    float xWidth() const 
        {
        return m_xWidth;
        }
    float yWidth() const
        {
        return m_yWidth;
        }

    float zWidth() const 
        {
        return m_zWidth;
        }

	void GetNodeSize(float &x, float &y, float &z) const
	{
		x=m_xWidth;
		y=m_yWidth;
		z=m_zWidth;
	}

    i4_bool isLeaf() const
        {
        return !IsSubDivided();
        }

    /// Returns the i'th quad of this object.
    /// Make sure the index is in range.
    g1_quad_class *GetQuad(int i) const
        {
        //unfortunatelly, we need a double indirection here.
        return &m_pWorld->quad[m_pQuadList[i]];
        }

	//! Returns the vertices of the quad specified by the given side of this node.
	//! The normal vector is always facing outwards. It can be used to 
	//! check for collisions with this node. 
	void GetBorderSide(int side, 
		i4_3d_vector &v1, 
		i4_3d_vector &v2,
		i4_3d_vector &v3,
		i4_3d_vector &v4,
		i4_3d_vector &normal);


    /// Returns the Leaf node the point is in.
    /// The vector must already have been transformed to local coordinates.
    /// May return 0 if point is outside object. 
    g1_octree *GetLeafAt(i4_3d_vector where) const;


	/**
	* This method returns a pointer of the left neighbour. If no neighbour is found,
	* a NULL pointer is returned.
	* @param root
	*				specifies the <code>OcTree</code> the node belongs to.
	* @return returns a pointer to the left neighbour or NULL.
	*/
	g1_octree *GetNeighbourLeft(g1_octree *root);

	/**
	* This method returns a pointer to the left neighbour. If no neighbour is found,
	* a NULL pointer is returned.
	*
	* <b>Note:</b> The neighbour has to be on the same level as the current node.
	*              Otherwise a NULL pointer is returned.
	* @param root
	*				specifies the <code>OcTree</code> the node belongs to.
	* @return returns a pointer to the left neighbour with same level or NULL.
	*/
	g1_octree *GetNeighbourLeftSameLevel(g1_octree *root);

	/**
	* This method returns a pointer of the right neighbour. If no neighbour is found,
	* a NULL pointer is returned.
	* @param root
	*				specifies the <code>OcTree</code> the node belongs to.
	* @return returns a pointer to the right neighbour or NULL.
	*/
	g1_octree *GetNeighbourRight(g1_octree *root);

	/**
	* This method returns a pointer to the right neighbour. If no neighbour is found,
	* a NULL pointer is returned.
	*
	* <b>Note:</b> The neighbour has to be on the same level as the current node.
	*              Otherwise a NULL pointer is returned.
	* @param root
	*				specifies the <code>OcTree</code> the node belongs to.
	* @return returns a pointer to the right neighbour with same level or NULL.
	*/
	g1_octree *GetNeighbourRightSameLevel(g1_octree *root);

	/**
	* This method returns a pointer of the front neighbour. If no neighbour is found,
	* a NULL pointer is returned.
	* @param root
	*				specifies the <code>OcTree</code> the node belongs to.
	* @return returns a pointer to the front neighbour or NULL.
	*/
	g1_octree *GetNeighbourFront(g1_octree *root);

	/**
	* This method returns a pointer to the front neighbour. If no neighbour is found,
	* a NULL pointer is returned.
	*
	* <b>Note:</b> The neighbour has to be on the same level as the current node.
	*              Otherwise a NULL pointer is returned.
	* @param root
	*				specifies the <code>OcTree</code> the node belongs to.
	* @return returns a pointer to the front neighbour with same level or NULL.
	*/
	g1_octree *GetNeighbourFrontSameLevel(g1_octree *root);

	/**
	* This method returns a pointer of the rear neighbour. If no neighbour is found,
	* a NULL pointer is returned.
	* @param root
	*				specifies the <code>OcTree</code> the node belongs to.
	* @return returns a pointer to the rear neighbour or NULL.
	*/	
	g1_octree *GetNeighbourRear(g1_octree *root);

	/**
	* This method returns a pointer to the rear neighbour. If no neighbour is found,
	* a NULL pointer is returned.
	*
	* <b>Note:</b> The neighbour has to be on the same level as the current node.
	*              Otherwise a NULL pointer is returned.
	* @param root
	*				specifies the <code>OcTree</code> the node belongs to.
	* @return returns a pointer to the rear neighbour with same level or NULL.
	*/
	g1_octree *GetNeighbourRearSameLevel(g1_octree *root);

	/**
	* This method returns a pointer of the bottom neighbour. If no neighbour is found,
	* a NULL pointer is returned.
	* @param root
	*				specifies the <code>OcTree</code> the node belongs to.
	* @return returns a pointer to the bottom neighbour or NULL.
	*/	
	g1_octree *GetNeighbourBottom(g1_octree *root);

	/**
	* This method returns a pointer to the bottom neighbour. If no neighbour is found,
	* a NULL pointer is returned.
	*
	* <b>Note:</b> The neighbour has to be on the same level as the current node.
	*              Otherwise a NULL pointer is returned.
	* @param root
	*				specifies the <code>OcTree</code> the node belongs to.
	* @return returns a pointer to the bottom neighbour with same level or NULL.
	*/
	g1_octree *GetNeighbourBottomSameLevel(g1_octree *root);

	/**
	* This method returns a pointer of the top neighbour. If no neighbour is found,
	* a NULL pointer is returned.
	* @param root
	*				specifies the <code>OcTree</code> the node belongs to.
	* @return returns a pointer to the top neighbour or NULL.
	*/	
	g1_octree *GetNeighbourTop(g1_octree *root);

	/**
	* This method returns a pointer to the top neighbour. If no neighbour is found,
	* a NULL pointer is returned.
	*
	* <b>Note:</b> The neighbour has to be on the same level as the current node.
	*              Otherwise a NULL pointer is returned.
	* @param root
	*				specifies the <code>OcTree</code> the node belongs to.
	* @return returns a pointer to the top neighbour with same level or NULL.
	*/
	g1_octree *GetNeighbourTopSameLevel(g1_octree *root);

	/**
	* This method returns all 26 neighbours of the current OcTree node to.
	* Nodes are sorted lexicographically from left to right, front to rear
	* and bottom to top.
	*
	* @param root
	*				specifies the <code>OcTree</code> the node belongs to.
	* @return returns a pointer to an array of neighbours or NULL.
	*/
	i4_array<g1_octree*> getNeighbourCells(g1_octree *root);

	/**
	* This method returns all 26 neighbours of same level the current OcTree node to.
	* Nodes are sorted lexicographically from left to right, front to rear
	* and bottom to top.
	*
	* <b>Note:</b> If a neighbour does not have the same level as the current
	*				node, a NULL pointer is stored instead.
	*
	* @param root
	*				specifies the <code>OcTree</code> the node belongs to.
	* @return returns a pointer to an array of neighbours or NULL.
	*/
	i4_array<g1_octree*> getNeighbourCellsSameLevel(g1_octree *root);

	/**
	* Testing methods for the neighbour cells methods.
	* 
	* @param root
	*				specifies the <code>OcTree</code> the node belongs to.
	* @return returns true if test successful.
	*/
	bool GetNeighbourCellsTest(g1_octree *root);

	/**
	* Testing methods for the neighbour cells methods with same level.
	* 
	* @param root
	*				specifies the <code>OcTree</code> the node belongs to.
	* @return returns true if test successful.
	*/
	bool GetNeighbourCellsSameLevelTest(g1_octree *root);

	class iterator
	{
	private:
		g1_octree *node;
		friend class g1_octree;
	public:
		iterator(g1_octree* startnode)
			:node(startnode)
		{
		}
		iterator():node(NULL)
		{
		}
		iterator(const iterator &it)
			:node(it.node)
		{
		}
		/*iterator& operator=(const iterator &it)
		{
			this->node=it.node;
			return *this;
		}*/
		bool operator==(iterator i)
		{
			return node==i.node;
		}
		bool operator!=(iterator i)
		{
			return node!=i.node;
		}
		g1_octree* operator->() const
		{
			return node;
		}
		g1_octree& operator*() const
		{
			return *node;
		}
		g1_octree* get()
		{
			return node;
		}
		
		iterator& operator++()
		{
			Next();
			return *this;
		}
		iterator operator++(int)
		{
			iterator it=*this;
			Next();
			return it;
		}
	protected:
		//Traverse the octree pre-order-wise. (Root first, then children)
		void Next()
		{
			g1_octree* oldnode=node;
			for(int i=0;i<8;i++)
			{
				if (node->m_pOctreeNodes[i]!=NULL)
				{
					node=node->m_pOctreeNodes[i];
					return;
				}
			}
			g1_octree *p=node->m_pParent;
			if (p==NULL)
			{
				//No children and no parent -> End
				node=NULL;
				return; 
			}
			bool bfoundchild=false;
			while (p!=NULL)
			{
				int j;
				for (j=0;j<8;j++)
				{
					if (p->m_pOctreeNodes[j]==oldnode)
					{
						bfoundchild=true;
						// we were the last child of our parent, continue seaching up the tree
						if (j==7)
						{
							break; //out of for
						}
						continue;
					}
					if (bfoundchild&&p->m_pOctreeNodes[j]!=NULL)
					{
						node=p->m_pOctreeNodes[j];
						return;
					}

				}
				oldnode=p;
				p=p->m_pParent;
				bfoundchild=false;
			}
			node=NULL;
		}

	};

	iterator begin()
	{
		return iterator(this);
	}
	iterator end() const
	{
		return NULL;
	}


protected:
    i4_bool PointInCube(i4_3d_vector p) const;
	i4_bool RayShorterThanCubeSize(i4_3d_vector ray) const;
private:

	/// This tells us if we have divided this node into more sub nodes
	i4_bool m_bSubDivided;

	/// The current subdivision we are at. 
	/// We need this to be static, because the subdivision goes trough
	/// multiple instances.
	/// we can safely assume that we're not generating two octrees
	/// simultaneously.
	static int m_CurrentSubdivision;
protected:
	// These are the size of the cube for this current node
	float m_xWidth; ///< Width of this node
	float m_yWidth; ///< Height of this node
	float m_zWidth; ///< Depth of this node

	/// This holds the amount of triangles stored in this node
	int m_TriangleCount;
	// The following values are used for neighbour finding (see code from pointshop3d, 
	// diffuser_plugin, which is an implementation of samet, 1990a)
	w32 m_level;    ///< Depth of this node
	w32 m_xLocCode; ///< location codes for nearest neighbour finding. 
	w32 m_yLocCode;
	w32 m_zLocCode;
public:
	enum 
	{
		NO_FLAGS=0,
		IS_ROOT=1,
		BOUNDARY=2,
		ENUM_FORCE_DWORD=0xFFFFFFFF
	};
protected:
	w32 m_flags;
	void calculate_location_codes();
	void internal_calc_loc_codes(int cur_level);
public:
	w32 get_flag(w32 x) const 
	{ 
		return m_flags & x; 
	}
	void set_flag(w32 x, w32 value=ENUM_FORCE_DWORD) 
	{ 
		if (value) 
			m_flags|=x; 
		else 
			m_flags&=(~x); 
	} 
protected:
	/// This is the center (X, Y, Z) point in this node
	i4_3d_vector m_vCenter;

	/// This holds all the scene information (verts, normals, texture info, etc..) for this node.
	/// It is actually a reference to the object this octree belongs to (the reference is needed
	/// since we need to have access to the vertices and quads defined there. 
	g1_quad_object_class *m_pWorld;

	/// This stores a list of all the quads in this node.
	i4_array<int>	m_pQuadList;

	/// These are the eight nodes branching down from this current node
	g1_octree *m_pOctreeNodes[8];	
	/// The parent element
	g1_octree *m_pParent;
};


class i4_grow_heap_class;
extern i4_grow_heap_class *g1_object_heap;

extern g1_octree_debug g_Debug;

#endif


