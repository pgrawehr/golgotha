//***********************************************************************//
//																		 //
//		- "Talk to me like I'm a 3 year old!" Programming Lessons -		 //
//                                                                       //
//		$Author:		DigiBen		digiben@gametutorials.com			 //
//																		 //
//		$Program:		Octree3	 										 //
//																		 //
//		$Description:	A working octree with a .3ds file format scene   //
//																		 //
//		$Date:			2/16/02											 //
//																		 //
//***********************************************************************//

// Include our associated .h file
#include "pch.h"
#include "obj3d.h"
#include "octree.h"
#include "g1_render.h"
#include "load3d.h"

// This is the final version of our octree code.  It will allow you to take  
// a 3D model, regardless of the file format (as long as it conforms to the t3DModel struct).
// The difference from the last 2 tutorials is that we store face indices, rather
// than vertices.  Check out the top of Main.cpp for a explanation of why we chose this way.
// 
// Here is a list of the important new functions added to our g1_octree class:
//
//	//This returns the number of polygons in our entire scene
//	int GetSceneTriangleCount(t3DModel *pWorld);
//
//  // This adds the current object index to our object list
//	void AddObjectIndexToList(int index);
//
//	// This recursively creates a display list ID for every end node in the octree
//	void CreateDisplayList(g1_octree *pNode, t3DModel *pRootWorld, int displayListOffset);
//
// Since we need to know the total triangle count for each node that we will
// be potentially splitting, GetSceneTriangleCount() was created to go through
// all of the objects in the model and add up their triangles to a total number.
// This number is then returned and used for the root node to pass in.
//
// Instead of going through every object in the model's object list when drawing
// each end node, we store a list of indices into the object list that are in the end node.  
// This way we don't do unnecessary looping if the triangles in that end node aren't in 
// those objects.  The AddObjectIndexToList() function adds the passed in index to our 
// index list, if it's not already there.
//
// To get a greater efficiency out of the drawing of our octree, display lists are used.
// CreateDisplayList() recursively goes through each node and creates a display list ID for each.
// This is only done once, then we can use the display ID to render that end node liquid fast.
//
// Keep in mind, that we do NOT split the triangles over the node's planes.  Instead, we
// are just storing the face indices for each object. You might wonder how this works, since 
// our t3DModel structure has multiple objects, with multiple face index arrays.  Simple,
// we just create a pointer to a t3DModel structure for every end node (m_pWorld).  Then,
// we only store the objects that are in that end node.  Of course, there is no need to
// store anything but the face indices, number of faces and objects.  We will then use
// the face indices stored in our m_pWorld pointer to pass into the original world model's
// structure to draw it.  Remember, we don't store all the objects from the original
// model's structure, just the ones that are in our node's dimensions.
//
// There is so much that is going on in this tutorial, that I suggest having a second
// window open to look at the Octree2 project, so you can contrast between what is changed
// and what isn't.  This was necessary even for me when creating this tutorial.  Also,
// I think this helps you to understand the new code if you understand what the simplified
// octree code was doing.  Follow along each function with the old code and the new code.
// The same concepts are being coded, but with full world data, not just vertices.
//
//


// This holds the current amount of subdivisions we are currently at.
// This is used to make sure we don't go over the max amount
//PG: This will be a class variable of g1_octree.
//int g_CurrentSubdivision = 0;

g1_octree_debug g_Debug;

//the following class contains only statics; the instance is needed
//only because we want the init()-method to be called on it.
g1_octree_globals g1_octree_globals_instance;

//definitions of statics:
bool g1_octree_globals::g_bLighting=true;
int g1_octree_globals::g_EndNodeCount=0;
int g1_octree_globals::g_MaxSubdivisions=6;
int g1_octree_globals::g_MaxTriangles=200;
bool g1_octree_globals::g_RenderMode=true;
int g1_octree_globals::g_TotalNodesDrawn=0;

int g1_octree::m_CurrentSubdivision=0;

g1_octree_debug::g1_octree_debug():m_vLines(0,100)
	{
	}

g1_octree_debug::~g1_octree_debug()
	{
	m_vLines.uninit();
	}
///////////////////////////////// RENDER DEBUG LINES \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This goes through all of the lines that we stored in our list and draws them
/////
///////////////////////////////// RENDER DEBUG LINES \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

// This renders all of the lines
void g1_octree_debug::RenderDebugLines(i4_transform_class *transform)				
{
    
	//glDisable(GL_LIGHTING);					// Turn OFF lighting so the debug lines are bright yellow

	//glBegin(GL_LINES);						// Start rendering lines

	//	glColor3ub(255, 255, 0.0f);			// Turn the lines yellow
	r1_render_api_class *api=g1_render.r_api;
	r1_shading_type shade=api->get_shade_mode();
	api->set_shading_mode(R1_SHADE_DISABLED);
	api->set_constant_color(0xFFFF00);
		// Go through the whole list of lines stored in the vector m_vLines.
		for(int i = 0; i < m_vLines.size(); i+=2)
		{
			// Pass in the current point to be rendered as part of a line
			i4_3d_point_class v1(m_vLines[i]);
			i4_3d_point_class v2(m_vLines[i+1]);
			
			g1_render.render_3d_line(v1,v2,0xffff00,0xffff00,
				transform,i4_T);
			//glVertex3f(m_vLines[i].x, m_vLines[i].y, m_vLines[i].z);
		}	

	//glEnd();								// Stop rendering lines

	// If we have lighting turned on, turn the lights back on
	//if(g_bLighting) 
	//	glEnable(GL_LIGHTING);
	api->set_shading_mode(shade);
}

///////////////////////////////// ADD DEBUG LINE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This adds a debug LINE to the stack of lines
/////
///////////////////////////////// ADD DEBUG LINE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

void g1_octree_debug::AddDebugLine(i4_3d_vector vPoint1, i4_3d_vector vPoint2)
{
	// Add the 2 points that make up the line into our line list.
	m_vLines.push_back(vPoint1);
	m_vLines.push_back(vPoint2);
}


///////////////////////////////// ADD DEBUG RECTANGLE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This adds a debug RECTANGLE to the stack of lines
/////
///////////////////////////////// ADD DEBUG RECTANGLE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

void g1_octree_debug::AddDebugRectangle(i4_3d_vector vCenter, float width, float height, float depth)
{
	// So we can work with the code better, we divide the dimensions in half.
	// That way we can create the cube from the center outwards.
	width /= 2.0f;	height /= 2.0f;	depth /= 2.0f;

	// Below we create all the 8 points so it will be easier to input the lines
	// of the cube.  With the dimensions we calculate the points.
	i4_3d_vector vTopLeftFront( vCenter.x - width, vCenter.y + height, vCenter.z + depth);
	i4_3d_vector vTopLeftBack(  vCenter.x - width, vCenter.y + height, vCenter.z - depth);
	i4_3d_vector vTopRightBack( vCenter.x + width, vCenter.y + height, vCenter.z - depth);
	i4_3d_vector vTopRightFront(vCenter.x + width, vCenter.y + height, vCenter.z + depth);

	i4_3d_vector vBottom_LeftFront( vCenter.x - width, vCenter.y - height, vCenter.z + depth);
	i4_3d_vector vBottom_LeftBack(  vCenter.x - width, vCenter.y - height, vCenter.z - depth);
	i4_3d_vector vBottomRightBack( vCenter.x + width, vCenter.y - height, vCenter.z - depth);
	i4_3d_vector vBottomRightFront(vCenter.x + width, vCenter.y - height, vCenter.z + depth);

	////////// TOP LINES ////////// 

	m_vLines.push_back(vTopLeftFront);		m_vLines.push_back(vTopRightFront);
	m_vLines.push_back(vTopLeftBack);  		m_vLines.push_back(vTopRightBack);
	m_vLines.push_back(vTopLeftFront);		m_vLines.push_back(vTopLeftBack);
	m_vLines.push_back(vTopRightFront);		m_vLines.push_back(vTopRightBack);

	////////// BOTTOM LINES ////////// 

	m_vLines.push_back(vBottom_LeftFront);	m_vLines.push_back(vBottomRightFront);
	m_vLines.push_back(vBottom_LeftBack);	m_vLines.push_back(vBottomRightBack);
	m_vLines.push_back(vBottom_LeftFront);	m_vLines.push_back(vBottom_LeftBack);
	m_vLines.push_back(vBottomRightFront);	m_vLines.push_back(vBottomRightBack);

	////////// SIDE LINES ////////// 

	m_vLines.push_back(vTopLeftFront);		m_vLines.push_back(vBottom_LeftFront);
	m_vLines.push_back(vTopLeftBack);		m_vLines.push_back(vBottom_LeftBack);
	m_vLines.push_back(vTopRightBack);		m_vLines.push_back(vBottomRightBack);
	m_vLines.push_back(vTopRightFront);		m_vLines.push_back(vBottomRightFront);
}


///////////////////////////////// CLEAR \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This clears all of the debug lines
/////
///////////////////////////////// CLEAR \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

void g1_octree_debug::Clear()						
{
	// Destroy the list using the standard vector clear() function
	m_vLines.clear();
}

void g1_octree_debug::uninit()
	{
	m_vLines.uninit();
	}


//-------------------------------------------------------------------------\\ 



///////////////////////////////// OCTREE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	The g1_octree contstructor which calls our init function
/////
///////////////////////////////// OCTREE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

g1_octree::g1_octree():m_pQuadList(0,200)
{
	// Set the subdivided flag to false
	m_bSubDivided = false;

	// Set the dimensions of the box to false
	m_xWidth = 0; 
	m_yWidth = 0;
	m_zWidth = 0;

	// Initialize the triangle count
	m_TriangleCount = 0;

	// Initialize the center of the box to the 0
	m_vCenter = i4_3d_vector(0, 0, 0);

/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *
	
	// Notice that we got rid of our InitOctree() function and just stuck the
	// initialization code in our constructor.  This is because we no longer need
	// to create the octree in real-time.

	// Initialize our world data to NULL.  This stores all the object's
	// face indices that need to be drawn for this node.  
	m_pWorld = NULL;

/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *

	// Set the sub nodes to NULL
	memset(m_pOctreeNodes, 0, sizeof(m_pOctreeNodes));		
}


///////////////////////////////// ~OCTREE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	The g1_octree destructor which calls our destroy function
/////
///////////////////////////////// ~OCTREE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

g1_octree::~g1_octree()
{

/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *

	// Notice that we got rid of DestroyOctree() from that last tutorials.  Like
	// the InitOctree(), we didn't need them any more because we will not be able
	// to change the octree restrictions in real-time.

	// If our chopped up world data exists, then we need to free it
   
	/*
	//That pointer points to the PARENT of ourselves, so don't delete
	//anything there.
	if( m_pWorld )
	{
		// Go through all the objects that were in our end node
		for(int i = 0; i < m_pWorld->numOfObjects; i++)
		{
			// Free the triangle data if it's not NULL
			if( m_pWorld->pObject[i].pFaces )
			{
				delete [] m_pWorld->pObject[i].pFaces;	
				m_pWorld->pObject[i].pFaces = NULL;
			}		

			// Free the index data if it's not NULL
			if( m_pWorld->pObject[i].pIndices )
			{
				delete [] m_pWorld->pObject[i].pIndices;	
				m_pWorld->pObject[i].pIndices = NULL;
			}
		}
	
		// Delete our end node face index data and set it to NULL
		delete m_pWorld;
		m_pWorld = NULL;
	}

  */
/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *

	// Go through all of the nodes and free them if they were allocated
	for(int i = 0; i < 8; i++)
	{
		// Make sure this node is valid
		if( m_pOctreeNodes[i] )
		{
			// Free this array index.  This will call the deconstructor which will
			// free the octree data correctly.  This allows us to forget about it's clean up
			delete m_pOctreeNodes[i];
			m_pOctreeNodes[i] = NULL;
		}
	}
	
}

g1_octree *g1_octree::Build(g1_quad_object_class *pWorld)
	{
	g_Debug.Clear();
	g1_octree *newtree=new g1_octree;
	newtree->m_pWorld=pWorld;
	newtree->GetSceneDimensions(pWorld);
	int totaltrianglecount=newtree->GetSceneTriangleCount(pWorld);
	if (!newtree->CreateNode(pWorld,NULL,totaltrianglecount,newtree->GetCenter(),
		newtree->m_xWidth,newtree->m_yWidth, newtree->m_zWidth))
		{
		//The tree consists of only one node -> drop it, it's of no use.
		delete newtree;
		return 0;
		}
	
	return newtree;
	}


/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *

//////////////////////////// GET SCENE TRIANGLE COUNT \\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This returns the total number of polygons in our scene
/////
//////////////////////////// GET SCENE TRIANGLE COUNT \\\\\\\\\\\\\\\\\\\\\\\\\\*

int g1_octree::GetSceneTriangleCount(g1_quad_object_class *pWorld)
{
	// This function is only called once, right before we create our first root node.
	// Basically, we just go through all of the objects in our scene and add up their triangles.

	// Initialize a variable to hold the total amount of polygons in the scene
	return pWorld->num_quad;
}


///////////////////////////////// OCTREE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This sets our initial width of the scene, as well as our center point
/////
///////////////////////////////// OCTREE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

void g1_octree::GetSceneDimensions(g1_quad_object_class *pWorld)
{

	// Return from this function if we passed in bad data.  This used to be a check
	// to see if the vertices passed in were allocated, now it's a check for world data.
	if(!pWorld) return;

/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *


	// Initialize a variable to hold the total amount of vertices in the scene
	int numberOfVerts = 0;


/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *

	// This code is still doing the same things as in the previous tutorials,
	// except that we have to go through every object in the scene to find the
	// center point.

	// Go through all of the object's vertices and add them up to eventually find the center
	/*
	for(int i = 0; i < pWorld->numOfObjects; i++)
	{
		// Increase the total vertice count
		numberOfVerts += pWorld->pObject[i].numOfVerts;

		// Add the current object's vertices up
		for(int n = 0; n < pWorld->pObject[i].numOfVerts; n++)
		{
			// Add the current vertex to the center variable (Using operator overloading)
			m_vCenter = m_vCenter + pWorld->pObject[i].pVerts[n];
		}
	}
	*/
	numberOfVerts=pWorld->num_vertex;
	//we assume that objects wich have a quadtree don't have animations
	g1_vert_class *verts=pWorld->get_verts(0,0);
	for (int i=0; i<pWorld->num_vertex; i++)
		{
		m_vCenter=m_vCenter+verts[i].v;
		}

/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *


	// Divide the total by the number of vertices to get the center point.
	// We could have overloaded the / symbol but I chose not to because we rarely use it.
	//m_vCenter.x /= numberOfVerts;
	//m_vCenter.y /= numberOfVerts;	
	//m_vCenter.z /= numberOfVerts;
	m_vCenter/=(float)numberOfVerts;

	// Now that we have the center point, we want to find the farthest distance from
	// our center point.  That will tell us how big the width of the first node is.
	// Once we get the farthest height, width and depth, we then check them against each
	// other.  Which ever one is higher, we then use that value for the cube width.

	float currentWidth = 0, currentHeight = 0, currentDepth = 0;

	// Initialize some temporary variables to hold the max dimensions found
	float maxWidth = 0, maxHeight = 0, maxDepth = 0;
	

/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *

	// This code still does the same thing as in the previous octree tutorials,
	// except we need to go through each object in the scene to find the max dimensions.

	
	// Go through all of the current objects vertices
	for(int j = 0; j < pWorld->num_vertex; j++)
	{
		// Get the distance in width, height and depth this vertex is from the center.
		currentWidth  = i4_fabs(verts[j].v.x - m_vCenter.x);	
		currentHeight = i4_fabs(verts[j].v.y - m_vCenter.y);		
		currentDepth  = i4_fabs(verts[j].v.z - m_vCenter.z);

		// Check if the current width value is greater than the max width stored.
		if(currentWidth  > maxWidth)	maxWidth  = currentWidth;

		// Check if the current height value is greater than the max height stored.
		if(currentHeight > maxHeight)	maxHeight = currentHeight;

		// Check if the current depth value is greater than the max depth stored.
		if(currentDepth > maxDepth)		maxDepth  = currentDepth;
	}

/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *

	// Set the member variable dimensions to the max ones found.
	// We multiply the max dimensions by 2 because this will give us the
	// full width, height and depth.  Otherwise, we just have half the size
	// because we are calculating from the center of the scene.

	maxWidth *= 2;		maxHeight *= 2;		maxDepth *= 2;

	// Check if the width is the highest value and assign that for the cube dimension
	//if(maxWidth > maxHeight && maxWidth > maxDepth)
	//	m_Width = maxWidth;

	// Check if the height is the heighest value and assign that for the cube dimension
	//else if(maxHeight > maxWidth && maxHeight > maxDepth)
	//	m_Width = maxHeight;

	// Else it must be the depth or it's the same value as some of the other ones
	//else
	//	m_Width = maxDepth;
	m_xWidth=maxWidth;
	m_yWidth=maxHeight;
	m_zWidth=maxDepth;
}


///////////////////////////////// GET NEW NODE CENTER \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This returns the center point of the new subdivided node, depending on the ID
/////
///////////////////////////////// GET NEW NODE CENTER \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

i4_3d_vector g1_octree::GetNewNodeCenter(i4_3d_vector vCenter, 
										 float xwidth, 
										 float ywidth,
										 float zwidth, 
										 int nodeID)
{
	// I created this function which takes an enum ID to see which node's center
	// we need to calculate.  Once we find that we need to subdivide a node we find
	// the centers of each of the 8 new nodes.  This is what that function does.
	// We just tell it which node we want.

	// Initialize the new node center
	i4_3d_vector vNodeCenter(0, 0, 0);

	// Create a dummy variable to cut down the code size
	i4_3d_vector vCtr = vCenter;

	// Switch on the ID to see which subdivided node we are finding the center
	switch(nodeID)							
	{
		case TOP_LEFT_FRONT:
			// Calculate the center of this new node
			vNodeCenter = i4_3d_vector(vCtr.x - xwidth/4, vCtr.y + ywidth/4, vCtr.z + zwidth/4);
			break;

		case TOP_LEFT_BACK:
			// Calculate the center of this new node
			vNodeCenter = i4_3d_vector(vCtr.x - xwidth/4, vCtr.y + ywidth/4, vCtr.z - zwidth/4);
			break;

		case TOP_RIGHT_BACK:
			// Calculate the center of this new node
			vNodeCenter = i4_3d_vector(vCtr.x + xwidth/4, vCtr.y + ywidth/4, vCtr.z - zwidth/4);
			break;

		case TOP_RIGHT_FRONT:
			// Calculate the center of this new node
			vNodeCenter = i4_3d_vector(vCtr.x + xwidth/4, vCtr.y + ywidth/4, vCtr.z + zwidth/4);
			break;

		case BOTTOM_LEFT_FRONT:
			// Calculate the center of this new node
			vNodeCenter = i4_3d_vector(vCtr.x - xwidth/4, vCtr.y - ywidth/4, vCtr.z + zwidth/4);
			break;

		case BOTTOM_LEFT_BACK:
			// Calculate the center of this new node
			vNodeCenter = i4_3d_vector(vCtr.x - xwidth/4, vCtr.y - ywidth/4, vCtr.z - zwidth/4);
			break;

		case BOTTOM_RIGHT_BACK:
			// Calculate the center of this new node
			vNodeCenter = i4_3d_vector(vCtr.x + xwidth/4, vCtr.y - ywidth/4, vCtr.z - zwidth/4);
			break;

		case BOTTOM_RIGHT_FRONT:
			// Calculate the center of this new node
			vNodeCenter = i4_3d_vector(vCtr.x + xwidth/4, vCtr.y - ywidth/4, vCtr.z + zwidth/4);
			break;
	}

	// Return the new node center
	return vNodeCenter;
}


/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *

///////////////////////////////// CREATE NEW NODE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This figures out the new node information and then passes it into CreateNode()
/////
///////////////////////////////// CREATE NEW NODE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*


i4_bool g1_octree::CreateNewNode(g1_quad_object_class *pWorld, tFaceList &pList, int triangleCount,
					  	    i4_3d_vector vCenter, 
							float xwidth, 
							float ywidth,
							float zwidth, int nodeID)
{
	// This function is used as our helper function to partition the world data
	// to pass into the subdivided nodes.  The same things go on as in the previous
	// tutorials, but it's dealing with more than just vertices.  We are given
	// the world data that needs to be partitioned, the list of faces that are in
	// the new node about to be created, the triangle count, the parent node's center
	// and width, along with the enum ID that tells us which new node is being created.
	//
	// The tFaceList structure stores a vector of booleans, which tell us if that face
	// index is in our end node (true) or not (false).  It also contains a integer
	// to tell us how many of those faces (triangles) are "true", or in other words, 
	// are in our node that is being created.  

	// Check if the first node found some triangles in it, if not we don't continue
	if(!triangleCount) return i4_F;
	
	// Here we create the temporary partitioned data model, which will contain
	// all the objects and triangles in this end node.
	//originally, this was a t3dmodel, but for golg, there's no difference
	//between the world and an object (in this context) as 
	//the world is something completelly different.
	//g1_quad_object_loader_class helper(g1_object_heap);
	//g1_quad_object_class *pTempWorld = helper.allocate_object();
	//helper.obj=pTempWorld; //set the internal reference

	// Intialize the temp model data and assign the object count to it
	// We shan't do a memset on classes with virtual functions...
	//memset(pTempWorld, 0, sizeof(t3DModel));
	//pTempWorld->numOfObjects = pWorld->numOfObjects;
	
	
	// Get a pointer to the current object to avoid ugly code
	//t3DObject *pObject = &(pWorld->pObject[i]);

	// Create a new object, initialize it, then add it to our temp partition
	//t3DObject newObject;
	//memset(&newObject, 0, sizeof(t3DObject));
	//pTempWorld->pObject.push_back(newObject);

	// Assign the new node's face count, material ID, texture boolean and 
	// vertices to the new object.  Notice that it's not that pObject's face
	// count, but the pList's.  Also, we are just assigning the pointer to the
	// vertices, not copying them.
	//pTempWorld->num_vertex = pList[i].totalFaceCount;
	//pTempWorld->pObject[i].materialID  = pObject->materialID;
	//pTempWorld->pObject[i].bHasTexture = pObject->bHasTexture;
	//pTempWorld->pObject[i].pVerts      = pObject->pVerts;

	// Allocate memory for the new face list
	//pTempWorld->pObject[i].pFaces = new tFace [pTempWorld->pObject[i].numOfFaces];
	//helper.set_num_animations(1);

	// Create a counter to count the current index of the new node vertices
	//int index = 0;

	// Go through all of the current object's faces and only take the ones in this new node
	
	// PG: This allocates the array for the faces
	// We iterate over all the faces from the parent object (pWorld)
	// and copy all that are marked in the list (pList) to the new object. (pTempWorld)
	
	//helper.set_num_quads(pList.totalFaceCount); 
	//for(int j = 0; j < pWorld->num_quad; j++)
	//{
		// If this current triangle is in the node, assign it's index to our new face list
	//	if(pList.pFaceList[j])	
	//	{
			//pTempWorld->pObject[i].pFaces[index] = pObject->pFaces[j];
	//	    g1_quad_class *q=&pWorld->quad[j];
	//		pTempWorld->quad[index].set(q->vertex_ref[0],q->vertex_ref[1],
	//			q->vertex_ref[2],q->vertex_ref[3]);
	//		index++;
	//	}
	//}


/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *

	// Now comes the initialization of the node.  First we allocate memory for
	// our node and then get it's center point.  Depending on the nodeID, 
	// GetNewNodeCenter() knows which center point to pass back (TOP_LEFT_FRONT, etc..)

	// Allocate a new node for this octree
	g1_octree *nextree=new g1_octree;
	nextree->m_pWorld=pWorld;
	m_pOctreeNodes[nodeID] = nextree;

	// Get the new node's center point depending on the nodexIndex (which of the 8 subdivided cubes).
	i4_3d_vector vNodeCenter = GetNewNodeCenter(vCenter, xwidth, 
		ywidth,zwidth,nodeID);
		
	// Below, before and after we recurse further down into the tree, we keep track
	// of the level of subdivision that we are in.  This way we can restrict it.

	// Increase the current level of subdivision
	m_CurrentSubdivision++;


/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *

	// This chance is just that we pass in the temp partitioned world for this node,
	// instead of passing in just straight vertices.

	// Recurse through this node and subdivide it if necessary
	nextree->CreateNode(pWorld, &pList, triangleCount, vNodeCenter, 
		xwidth / 2,ywidth/2, zwidth/2);

/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *


	// Decrease the current level of subdivision
	m_CurrentSubdivision--;


/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *

	// To free the temporary partition, we just go through all of it's objects and
	// free the faces.  The rest of the dynamic data was just being pointed too and
	// does not to be deleted.  Finally, we delete the allocated pTempWorld.

	//PG: We have fully working destructors (and use only copy semantics)
	// Go through all of the objects in our temporary partition
	//for(i = 0; i < pWorld->numOfObjects; i++)
	//{
		// If there are faces allocated for this object, delete them
	//	if(pTempWorld->pObject[i].pFaces)
	//		delete [] pTempWorld->pObject[i].pFaces;
	//}

	// Delete the allocated partition
	//delete pTempWorld;

/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *
	return i4_T;
}


///////////////////////////////// CREATE NODE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This is our recursive function that goes through and subdivides our nodes
/////
///////////////////////////////// CREATE NODE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

i4_bool g1_octree::CreateNode(g1_quad_object_class *pWorld, tFaceList *pList, 
							  int numberOfTriangles, i4_3d_vector vCenter, 
							  float xwidth,
							  float ywidth,
							  float zwidth)
{
	// Initialize this node's center point.  Now we know the center of this node.
	m_vCenter = vCenter;

	// Initialize this nodes cube width.  Now we know the width of this current node.
	m_xWidth = xwidth;
	m_yWidth = ywidth;
	m_zWidth = zwidth;

	// Add the current node to our debug rectangle list so we can visualize it.
	g_Debug.AddDebugRectangle(vCenter, xwidth, ywidth, zwidth);

	// Check if we have too many triangles in this node and we haven't subdivided
	// above our max subdivisions.  If so, then we need to break this node into
	// 8 more nodes (hence the word OCTree).  Both must be true to divide this node.
	if( (numberOfTriangles > g1_octree_globals::g_MaxTriangles) && 
		(m_CurrentSubdivision < g1_octree_globals::g_MaxSubdivisions) )
		{
		// Since we need to subdivide more we set the divided flag to true.
		m_bSubDivided = true;


/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *
	
		// This function pretty much stays the same, except a small twist because
		// we are dealing with multiple objects for the scene, not just an array of vertices.
		// In the previous tutorials, we used a vector<> of booleans, but now we use our
		// tFaceList to store a vector of booleans for each object.

		// Create the list of tFaceLists for each child node
		tFaceList pList1;		// TOP_LEFT_FRONT node list
		tFaceList pList2;		// TOP_LEFT_BACK node list
		tFaceList pList3;		// TOP_RIGHT_BACK node list
		tFaceList pList4;		// TOP_RIGHT_FRONT node list
		tFaceList pList5;		// BOTTOM_LEFT_FRONT node list
		tFaceList pList6;		// BOTTOM_LEFT_BACK node list
		tFaceList pList7;		// BOTTOM_RIGHT_BACK node list
		tFaceList pList8;		// BOTTOM_RIGHT_FRONT node list
	
		// Create this variable to cut down the thickness of the code below (easier to read)
		i4_3d_vector vCtr = vCenter;

		// Go through every object in the current partition of the world
		
		// Store a point to the current object
		g1_quad_object_class *pObject = pWorld;

		// Now, we have a face list for each object, for every child node.
		// We need to then check every triangle in this current object
		// to see if it's in any of the child nodes dimensions.  We store a "true" in
		// the face list index to tell us if that's the case.  This is then used
		// in CreateNewNode() to create a new partition of the world for that child node.

		// Resize the current face list to be the size of this object's face count
		pList1.pFaceList.resize(pObject->num_quad);
		pList2.pFaceList.resize(pObject->num_quad);
		pList3.pFaceList.resize(pObject->num_quad);
		pList4.pFaceList.resize(pObject->num_quad);
		pList5.pFaceList.resize(pObject->num_quad);
		pList6.pFaceList.resize(pObject->num_quad);
		pList7.pFaceList.resize(pObject->num_quad);
		pList8.pFaceList.resize(pObject->num_quad);

		// Go through all the triangles for this object
		int j;
		for (j=0; j<pObject->num_quad;j++)
			{
			pList1.pFaceList[j]=false;
			pList2.pFaceList[j]=false;
			pList3.pFaceList[j]=false;
			pList4.pFaceList[j]=false;
			pList5.pFaceList[j]=false;
			pList6.pFaceList[j]=false;
			pList7.pFaceList[j]=false;
			pList8.pFaceList[j]=false;
			}
		int totalclassified=0;
		int doubleclassified=0;
		for(j = 0; j < pObject->num_quad; j++)
		{
			//don't classify faces completelly outside of this
			//node. pList==0 means use all, not use none.
			if (pList&&pList->pFaceList[j]==false)
				continue;
			totalclassified++;
			g1_quad_class *q=&pObject->quad[j];
			int nodeor=0;
			// Check every vertice in the current triangle to see if it's inside a child node
			for(int whichVertex = 0; whichVertex < 4; whichVertex++)
			{
				// Store the current vertex to be checked against all the child nodes
				//i4_3d_vector vPoint = pObject->pVerts[pObject->pFaces[j].vertIndex[whichVertex]];
			    i4_3d_vector vPoint;
				g1_vert_class *vert=pObject->get_verts(0,0);
				w16 vertex=q->vertex_ref[whichVertex];
				if (vertex==0xffff)
					break;
				vPoint=vert[vertex].v;
				int isinnodes=0;
				// Check if the point lies within the TOP LEFT FRONT node
				if( (vPoint.x <= vCtr.x) && (vPoint.y >= vCtr.y) && (vPoint.z >= vCtr.z) ) 
					{
					pList1.pFaceList[j] = true;
					isinnodes++;
					nodeor|=1;
					}

				// Check if the point lies within the TOP LEFT BACK node
				if( (vPoint.x <= vCtr.x) && (vPoint.y >= vCtr.y) && (vPoint.z <= vCtr.z) ) 
					{
					pList2.pFaceList[j] = true;
					isinnodes++;
					nodeor|=2;
					}

				// Check if the point lies within the TOP RIGHT BACK node
				if( (vPoint.x >= vCtr.x) && (vPoint.y >= vCtr.y) && (vPoint.z <= vCtr.z) ) 
					{
					pList3.pFaceList[j] = true;
					isinnodes++;
					nodeor|=4;
					}

				// Check if the point lies within the TOP RIGHT FRONT node
				if( (vPoint.x >= vCtr.x) && (vPoint.y >= vCtr.y) && (vPoint.z >= vCtr.z) ) 
					{
					pList4.pFaceList[j] = true;
					isinnodes++;
					nodeor|=8;
					}

				// Check if the point lies within the BOTTOM LEFT FRONT node
				if( (vPoint.x <= vCtr.x) && (vPoint.y <= vCtr.y) && (vPoint.z >= vCtr.z) ) 
					{
					pList5.pFaceList[j] = true;
					isinnodes++;
					nodeor|=16;
					}

				// Check if the point lies within the BOTTOM LEFT BACK node
				if( (vPoint.x <= vCtr.x) && (vPoint.y <= vCtr.y) && (vPoint.z <= vCtr.z) ) 
					{
					pList6.pFaceList[j] = true;
					isinnodes++;
					nodeor|=32;
					}

				// Check if the point lies within the BOTTOM RIGHT BACK node
				if( (vPoint.x >= vCtr.x) && (vPoint.y <= vCtr.y) && (vPoint.z <= vCtr.z) ) 
					{
					pList7.pFaceList[j] = true;
					isinnodes++;
					nodeor|=64;
					}

				// Check if the point lines within the BOTTOM RIGHT FRONT node
				if( (vPoint.x >= vCtr.x) && (vPoint.y <= vCtr.y) && (vPoint.z >= vCtr.z) ) 
					{
					pList8.pFaceList[j] = true;
					isinnodes++;
					nodeor|=128;
					}
				I4_ASSERT(isinnodes>0,"Found a point that can't be classified");
			}
			//for debugging purposes: If more than one bit is set
			//in nodeor, the poly goes to multiple cubes.
			int bitsfound=0;
			for (int bit=0;bit<8;bit++)
				{
				bitsfound+=nodeor&1;
				nodeor=nodeor>>1;
				}
			if (bitsfound>1)
				{
				//q->set_flags(g1_quad_class::SELECTED);
				doubleclassified++;
				}
		}
		
		i4_warning("HINT: Of %d polygons in this cube at level %d, we classified %d multiple times.",
			totalclassified,m_CurrentSubdivision,doubleclassified);

		// Here we initialize the face count for each list that holds how many triangles
		// were found for each of the 8 subdivided nodes.
		pList1.totalFaceCount = 0;		pList2.totalFaceCount = 0;
		pList3.totalFaceCount = 0;		pList4.totalFaceCount = 0;
		pList5.totalFaceCount = 0;		pList6.totalFaceCount = 0;
		pList7.totalFaceCount = 0;		pList8.totalFaceCount = 0;
		

		// Here we create a variable for each list that holds how many triangles
		// were found for each of the 8 subdivided nodes.
		int triCount1 = 0;	int triCount2 = 0;	int triCount3 = 0;	int triCount4 = 0;
		int triCount5 = 0;	int triCount6 = 0;	int triCount7 = 0;	int triCount8 = 0;
			
		// Go through all of the objects of this current partition
		//for(i = 0; i < pWorld->numOfObjects; i++)
		//{
			// Go through all of the current objects triangles
			for(j = 0; j < pWorld->num_quad; j++)
			{
				// Increase the triangle count for each node that has a "true" for the index i.
				// In other words, if the current triangle is in a child node, add 1 to the count.
				// We need to store the total triangle count for each object, but also
				// the total for the whole child node.  That is why we increase 2 variables.
				if(pList1.pFaceList[j])	{ pList1.totalFaceCount++; triCount1++; }
				if(pList2.pFaceList[j])	{ pList2.totalFaceCount++; triCount2++; }
				if(pList3.pFaceList[j])	{ pList3.totalFaceCount++; triCount3++; }
				if(pList4.pFaceList[j])	{ pList4.totalFaceCount++; triCount4++; }
				if(pList5.pFaceList[j])	{ pList5.totalFaceCount++; triCount5++; }
				if(pList6.pFaceList[j])	{ pList6.totalFaceCount++; triCount6++; }
				if(pList7.pFaceList[j])	{ pList7.totalFaceCount++; triCount7++; }
				if(pList8.pFaceList[j])	{ pList8.totalFaceCount++; triCount8++; }
			}
		//}

		// Next we do the dirty work.  We need to set up the new nodes with the triangles
		// that are assigned to each node, along with the new center point of the node.
		// Through recursion we subdivide this node into 8 more potential nodes.

		// Create the subdivided nodes if necessary and then recurse through them.
		// The information passed into CreateNewNode() are essential for creating the
		// new nodes.  We pass the 8 ID's in so it knows how to calculate it's new center.
		CreateNewNode(pWorld, pList1, triCount1, vCenter, xwidth, ywidth, zwidth, TOP_LEFT_FRONT);
		CreateNewNode(pWorld, pList2, triCount2, vCenter, xwidth, ywidth, zwidth, TOP_LEFT_BACK);
		CreateNewNode(pWorld, pList3, triCount3, vCenter, xwidth, ywidth, zwidth, TOP_RIGHT_BACK);
		CreateNewNode(pWorld, pList4, triCount4, vCenter, xwidth, ywidth, zwidth, TOP_RIGHT_FRONT);
		CreateNewNode(pWorld, pList5, triCount5, vCenter, xwidth, ywidth, zwidth, BOTTOM_LEFT_FRONT);
		CreateNewNode(pWorld, pList6, triCount6, vCenter, xwidth, ywidth, zwidth, BOTTOM_LEFT_BACK);
		CreateNewNode(pWorld, pList7, triCount7, vCenter, xwidth, ywidth, zwidth, BOTTOM_RIGHT_BACK);
		CreateNewNode(pWorld, pList8, triCount8, vCenter, xwidth, ywidth, zwidth, BOTTOM_RIGHT_FRONT);
		int subtcount=0;
		for (int subts=0;subts<8;subts++)
			{
			subtcount+=m_pOctreeNodes[subts]?m_pOctreeNodes[subts]->m_TriangleCount:0;
			}
		m_TriangleCount=subtcount;
		}
	else
		{
		// If we get here we must either be subdivided past our max level, or our triangle
		// count went below the minimum amount of triangles so we need to store them.
		
		// We pass in the current partition of world data to be assigned to this end node
		//AssignTrianglesToNode(pWorld, numberOfTriangles);

		//if we are still at the root node (pList was initially NULL)
		//we return false, because it seems useless to subdivide this
		//object. (under the given partitioning settings)
		if (!pList)
			return i4_F; 
		int tcount=0;
		for (int face=0;face<pList->pFaceList.size();face++)
			{
			if (pList->pFaceList[face])
				{
				tcount++;
				m_pQuadList.add(face);
				}
			}
		m_TriangleCount=tcount;
		}

/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *
		return i4_T;
}


/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *

//////////////////////////// ADD OBJECT INDEX TO LIST \\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This adds the index into the model's object list to our object index list
/////
//////////////////////////// ADD OBJECT INDEX TO LIST \\\\\\\\\\\\\\\\\\\\\\\\\\\*


//PG: Dropped because it's not needed
/*
void g1_octree::AddObjectIndexToList(int index)
{
	// To eliminate the need to loop through all of the objects in the original
	// model, when drawing the end nodes, we create an instance of our t3DModel
	// structure to hold only the objects that lie in the child node's 3D space.

	// Go through all of the objects in our face index list
	for(int i = 0; i < m_pObjectList.size(); i++)
	{
		// If we already have this index stored in our object index list, don't add it.
		if(m_pObjectList[i] == index)
			return;
	}

	// Add this index to our object index list, which indexes into the root world object list
	m_pObjectList.push_back(index);
}
*/


//////////////////////////// ASSIGN TRIANGLES TO NODE \\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This allocates memory for the face indices to assign to the current end node
/////
//////////////////////////// ASSIGN TRIANGLES TO NODE \\\\\\\\\\\\\\\\\\\\\\\\\\\*


//PG: It's probably easier to drop this and do it directly in the
//Node code, since we must not change the quad indices.
/*
void g1_octree::AssignTrianglesToNode(g1_quad_object_class *pWorld, int numberOfTriangles)
{
	// We take our pWorld partition and then copy it into our member variable
	// face list, m_pWorld.  This holds the face indices that need to be rendered.
	// Since we are using vertex arrays, we can't use the tFace structure for the
	// indices, so we need to create an array that has all the face indices in a row.
	// This will be stored in our pIndices array, which is of type unsigned int.
	// Remember, it must be unsigned int for vertex arrays to register it.

	// Since we did not subdivide this node we want to set our flag to false
	m_bSubDivided = false;

	// Initialize the triangle count of this end node 
	m_TriangleCount = numberOfTriangles;

	// Create and init an instance of our model structure to store the face index information
	//m_pWorld = new t3DModel;
	//memset(m_pWorld, 0, sizeof(t3DModel));

	// Assign the number of objects to our face index list
	//m_pWorld->numOfObjects = pWorld->numOfObjects;

	//PG: m_pWorld is just a reference to the root object.
	//  This does newer change, so we don't need to copy anything
	m_pWorld=pWorld;

	// Go through all of the objects in the partition that was passed in
	//for(int i = 0; i < pWorld->numOfObjects; i++)
	//{
		// Create a pointer to the current object
		g1_quad_object_class *pObject = pWorld;

		// Create and init a new object to hold the face index information
		//t3DObject newObject;
		//memset(&newObject, 0, sizeof(t3DObject));

		// If this object has face information, add it's index to our object index list
		//PG: We can drop this shortcut, as we will iterate over the
		//faces, not the objects
		//if(pObject->numOfFaces)
		//	AddObjectIndexToList(i);

		// Add our new object to our face index list
		//m_pWorld->pObject.push_back(newObject);

		// Store the number of faces in a local variable
		int numOfFaces = pObject->num_quad;

		// Assign the number of faces to this current face list
		//m_pWorld->pObject[i].numOfFaces = numOfFaces;

		// Allocate memory for the face indices.  Remember, we also have faces indices
		// in a row, pIndices, which can be used to pass in for vertex arrays.  
		//m_pWorld->pObject[i].pFaces = new tFace [numOfFaces];
		//m_pWorld->pObject[i].pIndices = new UINT [numOfFaces * 3];

		m_pQuadList.resize(numOfFaces);

		// Initialize the face indices for vertex arrays (are copied below
		//memset(m_pWorld->pObject[i].pIndices, 0, sizeof(UINT) * numOfFaces * 3);

		// Copy the faces from the partition passed in to our end nodes face index list
		//memcpy(m_pWorld->pObject[i].pFaces, pObject->pFaces, sizeof(tFace) * numOfFaces);



		// Since we are using vertex arrays, we want to create a array with all of the
		// faces in a row.  That way we can pass it into glDrawElements().  We do this below.

		// Go through all the faces and assign them in a row to our pIndices array
		for(int j = 0; j < numOfFaces * 3; j += 3)
		{
			m_pWorld->pObject[i].pIndices[j]     = m_pWorld->pObject[i].pFaces[j / 3].vertIndex[0];
			m_pWorld->pObject[i].pIndices[j + 1] = m_pWorld->pObject[i].pFaces[j / 3].vertIndex[1];
			m_pWorld->pObject[i].pIndices[j + 2] = m_pWorld->pObject[i].pFaces[j / 3].vertIndex[2];
		}

		// We can now free the pFaces list if we want since it isn't going to be used from here
		// on out.  If you do NOT want to use vertex arrays, don't free the pFaces, and get
		// rid of the loop up above to store the pIndices.

		delete [] m_pWorld->pObject[i].pFaces;
		m_pWorld->pObject[i].pFaces = NULL;
	//}

	// Assign the current display list ID to be the current end node count
	m_DisplayListID = g_EndNodeCount;

	// Increase the amount of end nodes created (Nodes with vertices stored)
	g_EndNodeCount++;
}
*/
/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *


//////////////////////////////// DRAW OCTREE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This function recurses through all the nodes and draws the end node's vertices
/////
//////////////////////////////// DRAW OCTREE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

void g1_octree::DrawOctree(i4_transform_class *transform, g1_quadlist &quads)
{

/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *

	// To draw our octree, all that needs to be done is call our display list ID.
	// First we want to check if the current node is even in our frustum.  If it is,
	// we make sure that the node isn't subdivided.  We only can draw the end nodes.

/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *

	// Make sure a valid node was passed in, otherwise go back to the last node
	if(!this) return;

	// Check if the current node is in our frustum
	//r1_vert v;
	
	if (!g1_render.cube_in_frustrum(m_vCenter,m_xWidth,m_yWidth, m_zWidth, transform))
		{
		return;
		};

	//if(!g_Frustum.CubeInFrustum(pNode->m_vCenter.x, pNode->m_vCenter.y, 
	//							pNode->m_vCenter.z, pNode->m_Width / 2) )
	//{
	//	return;
	//}

	// Check if this node is subdivided. If so, then we need to recurse and draw it's nodes
	if(IsSubDivided())
		{
		// Recurse to the bottom of these nodes and draw the end node's vertices
		// Like creating the octree, we need to recurse through each of the 8 nodes.
	//PG: Why don't we just use the this pointer for pNode?
	//And pRootWorld doesn't need to be passed either, so we
	//safe the time to copy unecessary parameters to the stack.
		//DrawOctree(pNode->m_pOctreeNodes[TOP_LEFT_FRONT],		pRootWorld);
		//DrawOctree(pNode->m_pOctreeNodes[TOP_LEFT_BACK],		pRootWorld);
		//DrawOctree(pNode->m_pOctreeNodes[TOP_RIGHT_BACK],		pRootWorld);
		//DrawOctree(pNode->m_pOctreeNodes[TOP_RIGHT_FRONT],		pRootWorld);
		//DrawOctree(pNode->m_pOctreeNodes[BOTTOM_LEFT_FRONT],	pRootWorld);
		//DrawOctree(pNode->m_pOctreeNodes[BOTTOM_LEFT_BACK],		pRootWorld);
		//DrawOctree(pNode->m_pOctreeNodes[BOTTOM_RIGHT_BACK],	pRootWorld);
		//DrawOctree(pNode->m_pOctreeNodes[BOTTOM_RIGHT_FRONT],	pRootWorld);
		m_pOctreeNodes[TOP_LEFT_FRONT]->DrawOctree(transform,quads);
		m_pOctreeNodes[TOP_LEFT_BACK]->DrawOctree(transform,quads);
		m_pOctreeNodes[TOP_RIGHT_BACK]->DrawOctree(transform,quads);
		m_pOctreeNodes[TOP_RIGHT_FRONT]->DrawOctree(transform,quads);
		m_pOctreeNodes[BOTTOM_LEFT_FRONT]->DrawOctree(transform,quads);
		m_pOctreeNodes[BOTTOM_LEFT_BACK]->DrawOctree(transform,quads);
		m_pOctreeNodes[BOTTOM_RIGHT_BACK]->DrawOctree(transform,quads);
		m_pOctreeNodes[BOTTOM_RIGHT_FRONT]->DrawOctree(transform,quads);
		}
	else
		{
		// Increase the amount of nodes in our viewing frustum (camera's view)
		g1_octree_globals::g_TotalNodesDrawn++;


/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *

		// Make sure we have valid data assigned to this node
		
			
		// Call the list with our end node's display list ID
		//glCallList(pNode->m_DisplayListID);
		//PG: here comes the code that actually draws this node.

		for (int poly=0;poly<m_pQuadList.size();poly++)
			{
			int rpoly=m_pQuadList[poly];
			g1_quad_class *pq=&m_pWorld->quad[rpoly];
			quads.add(pq);
			//if (pq->get_flags(g1_quad_class::DRAWN))
			//	continue;
			//g1_render.clip_render_quad(pq,m_pWorld->get_verts(0,0),
			//	transform,0);
			//pq->set_flags(g1_quad_class::DRAWN);
			}

/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *

		}
}


/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *

//////////////////////////////// CREATE DISPLAY LIST \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This function recurses through all the nodes and creates a display list for them
/////
//////////////////////////////// CREATE DISPLAY LIST \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

//PG: The golg engine doesn't know about something like display lists.
//We will just push the polygons in a brute-force manner to the rendering
//device.
/*
void g1_octree::CreateDisplayList(g1_octree *pNode, g1_quad_object_class *pRootWorld, int displayListOffset)
{
	// This function handles our rendering code in the beginning and assigns it all
	// to a display list.  This increases our rendering speed, as long as we don't flood
	// the pipeline with a TON of data.  Display lists can actually be to bloated or too small.
	// Like our DrawOctree() function, we need to find the end nodes by recursing down to them.
	// We only create a display list for the end nodes and ignore the rest.  The 
	// displayListOffset is used to add to the end nodes current display list ID, in case
	// we created some display lists before creating the octree.  Usually it is just 1 otherwise.

	// Make sure a valid node was passed in, otherwise go back to the last node
	if(!pNode) return;

	// Check if this node is subdivided. If so, then we need to recurse down to it's nodes
	if(pNode->IsSubDivided())
	{
		// Recurse down to each one of the children until we reach the end nodes
		CreateDisplayList(pNode->m_pOctreeNodes[TOP_LEFT_FRONT],	pRootWorld, displayListOffset);
		CreateDisplayList(pNode->m_pOctreeNodes[TOP_LEFT_BACK],		pRootWorld, displayListOffset);
		CreateDisplayList(pNode->m_pOctreeNodes[TOP_RIGHT_BACK],	pRootWorld, displayListOffset);
		CreateDisplayList(pNode->m_pOctreeNodes[TOP_RIGHT_FRONT],	pRootWorld, displayListOffset);
		CreateDisplayList(pNode->m_pOctreeNodes[BOTTOM_LEFT_FRONT],	pRootWorld, displayListOffset);
		CreateDisplayList(pNode->m_pOctreeNodes[BOTTOM_LEFT_BACK],	pRootWorld, displayListOffset);
		CreateDisplayList(pNode->m_pOctreeNodes[BOTTOM_RIGHT_BACK],	pRootWorld, displayListOffset);
		CreateDisplayList(pNode->m_pOctreeNodes[BOTTOM_RIGHT_FRONT],pRootWorld, displayListOffset);
	}
	else 
	{
		// Make sure we have valid data assigned to this node
		if(!pNode->m_pWorld) return;

		// Add our display list offset to our current display list ID
		pNode->m_DisplayListID += displayListOffset;

		// Start the display list and assign it to the end nodes ID
		glNewList(pNode->m_DisplayListID,GL_COMPILE);

		// Create a temp counter for our while loop below to store the objects drawn
		int counter = 0;
		
		// Store the object count and material count in some local variables for optimization
		int objectCount = pNode->m_pObjectList.size();
		int materialCount = pRootWorld->pMaterials.size();

		// Go through all of the objects that are in our end node
		while(counter < objectCount)
		{
			// Get the first object index into our root world
			int i = pNode->m_pObjectList[counter];

			// Store pointers to the current face list and the root object 
			// that holds all the data (verts, texture coordinates, normals, etc..)
			t3DObject *pObject     = &pNode->m_pWorld->pObject[i];
			t3DObject *pRootObject = &pRootWorld->pObject[i];

			// Check to see if this object has a texture map, if so, bind the texture to it.
			if(pRootObject->bHasTexture) 
			{
				// Turn on texture mapping and turn off color
				glEnable(GL_TEXTURE_2D);

				// Reset the color to normal again
				glColor3ub(255, 255, 255);

				// Bind the texture map to the object by it's materialID
				glBindTexture(GL_TEXTURE_2D, g_Texture[pRootObject->materialID]);
			} 
			else 
			{
				// Turn off texture mapping and turn on color
				glDisable(GL_TEXTURE_2D);

				// Reset the color to normal again
				glColor3ub(255, 255, 255);
			}

			// Check to see if there is a valid material assigned to this object
			if(materialCount && pRootObject->materialID >= 0) 
			{
				// Get and set the color that the object is, since it must not have a texture
				BYTE *pColor = pRootWorld->pMaterials[pRootObject->materialID].color;

				// Assign the current color to this model
				glColor3ub(pColor[0], pColor[1], pColor[2]);
			}

			// Now we get to the more unknown stuff, vertex arrays.  If you haven't
			// dealt with vertex arrays yet, let me give you a brief run down on them.
			// Instead of doing loops to go through and pass in each of the vertices
			// of a model, we can just pass in the array vertices, then an array of
			// indices that MUST be an unsigned int, which gives the indices into
			// the vertex array.  That means that we can send the vertices to the video
			// card with one call to glDrawElements().  There are a bunch of other
			// functions for vertex arrays that do different things, but I am just going
			// to mention this one.  Since texture coordinates, normals and colors are also
			// associated with vertices, we are able to point OpenGL to these arrays before
			// we draw the geometry.  It uses the same indices that we pass to glDrawElements()
			// for each of these arrays.  Below, we point OpenGL to our texture coordinates,
			// vertex and normal arrays.  This is done with calls to glTexCoordPointer(), 
			// glVertexPointer() and glNormalPointer().
			//
			// Before using any of these functions, we need to enable their states.  This is
			// done with glEnableClientState().  You just pass in the ID of the type of array 
			// you are wanting OpenGL to look for.  If you don't have data in those arrays,
			// the program will most likely crash.
			//
			// If you don't want to use vertex arrays, you can just render the world like normal.
			// That is why I saved the pFace information, as well as the pIndices info.  This
			// way you can use what ever method you are comfortable with.  I tried both, and
			// by FAR the vertex arrays are incredibly faster.  You decide :)

			// Make sure we have texture coordinates to render
			if(pRootObject->pTexVerts) 
			{
				// Turn on the texture coordinate state
				glEnableClientState(GL_TEXTURE_COORD_ARRAY);

				// Point OpenGL to our texture coordinate array.
				// We have them in a pair of 2, of type float and 0 bytes of stride between them.
				glTexCoordPointer(2, GL_FLOAT, 0, pRootObject->pTexVerts);
			}

			// Make sure we have vertices to render
			if(pRootObject->pVerts)
			{
				// Turn on the vertex array state
				glEnableClientState(GL_VERTEX_ARRAY);

				// Point OpenGL to our vertex array.  We have our vertices stored in
				// 3 floats, with 0 stride between them in bytes.
				glVertexPointer(3, GL_FLOAT, 0, pRootObject->pVerts);
			}

			// Make sure we have normals to render
			if(pRootObject->pNormals)
			{
				// Turn on the normals state
				glEnableClientState(GL_NORMAL_ARRAY);

				// Point OpenGL to our normals array.  We have our normals
				// stored as floats, with a stride of 0 between.
				glNormalPointer(GL_FLOAT, 0, pRootObject->pNormals);
			}

			// Here we pass in the indices that need to be rendered.  We want to
			// render them in triangles, with numOfFaces * 3 for indice count,
			// and the indices are of type UINT (important).
			glDrawElements(GL_TRIANGLES,    pObject->numOfFaces * 3, 
						   GL_UNSIGNED_INT, pObject->pIndices);

			// Increase the current object count rendered
			counter++;
		}

		// End the display list for this ID
		glEndList();
	}
}
*/
/////// * /////////// * /////////// * NEW * /////// * /////////// * /////////// *


/////////////////////////////////////////////////////////////////////////////////
//
// * QUICK NOTES * 
//
// Holy cow!  That was a ton of technical garbage to sift through.  This is probably
// one of the LEAST intuitive tutorials that I have created, but hey, there is a lot
// to creating octrees.  The basics are easy to understand, but implementing it with
// a model structure can be tricky to do.  If you find any blaring errors or obvious
// notable optimizations to this code, please let me know.  It's 4am as I am finishing
// this, so I could have left out something important :)  Let's go over a brief overview
// of what was changed.
//
// The most important thing that was change in this tutorial was that we stored face
// indices instead of vertices.  In order to do this, we had to create a helper structure
// called tFaceList to partition the nodes in CreateNode().  We also created a instance
// of the t3DModel structure in our octree class.  This holds the face indices for
// each object that is in the end node.  Only the end nodes allocate an instance of
// this structure, and they do not store any other information besides the face indices.
// The rest of the data is ignored because it will be referenced from the original object
// when being drawn in CreateDisplayList().  Since we are using vertex arrays, we use the
// pIndices array of face indices to draw the world, verses the pFaces array.  If you
// don't want to use vertex arrays, go to the end AssignTrianglesToNode() and don't free
// this array, and forget about allocating the pIndices array.
//
// To create the display lists, after the octree has been creating, we just make a call
// to CreateDisplayList(), which recursively goes through all the end nodes and assigns
// then to a display list ID.  This way we can just call a list ID to draw an end node,
// and not have any loops.  This increases our frame rate drastically, especially accompanied
// with vertex arrays.  
//
// I hope this tutorial doesn't take such a huge leap from the previous that it doesn't
// offer much help.  Let me know if you have any questions, I would love to make anything
// clearer.  Please call me on things.  When creating this much code, I am bound to say
// something wrong or not explain something enough.  
//
// Good luck!
// 
//
// Ben Humphrey
// Game Programmer
// DigiBen@GameTutorials.com
// www.GameTutorials.com
//
// © 2001 GameTutorials
