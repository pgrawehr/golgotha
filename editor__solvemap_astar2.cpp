
/*--------------------------------------------------------------------------------


Hello Steve:


   Here are C++ code fragments for A* using Visual C++ 2.2.  I apologize for 
any spacing problems - my text editor seemed to chew up the source.

   The code works, but it is slow on large maps with varying terrain.  As far
as I know, the implementation of the algorithm is fine, so there must be 
something else wrong (or else it's just slow in Windows).  I have since 
optimized this code a lot, but I'd like to see if anyone else can find ways
to optimize it.  I'd also like to know how to make the paths straight.  I have
some ideas, but I haven't had time to try them.  Once I've compiled
feedback from others and finished my own optimizing, I will post a much
nicer set of code.  I don't have time to explain the algorithm or its variables,
f, g, and h, but perhaps I or someone else will do that later.

   Note:  I ported the algorithm from someone's C source off of the net.  The 
file was called spath.zip - thanks to the author.
*/



//.cpp code fragments
//===================
#include "pch.h"
#include "map.h"
#include "map_man.h"
#include "solvemap.h"
#include "solvemap_astar2.h"
#include "map_cell.h"
#include "tile.h"
#include <math.h>

#undef MAX /*just in case... */
#define MAX(x,y) ((x)>=(y)?(x):(y))
#define sqrt2 (1.41421f)

void Path::cleanup()
{ 
    struct NODE *Node,*prev;
	struct STACK *prstack;
    Node=OPEN->NextNode;
    while (Node != NULL)
    {
		prev=Node;
        Node=Node->NextNode;
		free(prev);
    }
	free(OPEN);
	OPEN=0;
    Node=CLOSED->NextNode;
    while (Node != NULL)
    {
        prev=Node;
        Node=Node->NextNode;
		free(prev);
    }
	free(CLOSED);
	CLOSED=0;
	while(Stack->NextStackPtr)
		{
		prstack=Stack;
		Stack=Stack->NextStackPtr;
		free(prstack);
		}
	free(Stack);
	Stack=0;
	free(fastmap);
};
/*
void Path::DefinePath(int x1, int y1, int x2, int y2)
{    
    struct NODE *path, *Node;
    //CApp* pApp = (CApp*) AfxGetApp();
    //MainMap* pMainMap = (MainMap*) pApp->GetMainMap();    
        
    path=(struct NODE *)FindPath(x1, y1, x2, y2);
    
    // Draw the path on the main map
    while (path->Parent != NULL)
    {
        path=path->Parent;
        CPoint point(path->x, path->y);
        Location* pLoc = pMainMap->GetLocation(point);
        ((MainMapLocation*)(pLoc))->m_bPath = TRUE;
    }

    // Free Nodes up
    Node=OPEN->NextNode;
    while (Node != NULL)
    {
        free(Node);
        Node=Node->NextNode;
    }
    Node=CLOSED->NextNode;
    while (Node != NULL)
    {
        free(Node);
        Node=Node->NextNode;
    }
}
*/
/////////////////////////////////////////////////////////////////////////
// Algorithm to find shortest path (A*)
/////////////////////////////////////////////////////////////////////////

i4_bool Path::FreeTile(int x, int y, w8 dir)   // returns 1 if tile is free(nothing on it).
{
     //CPoint point(x, y);    
    
    // Check for points off of the map.
    if (x < 0 || x >= mapxsize || y < 0 || y >= mapysize)
        return (0);

    //Location* pLoc = m_pMap->GetLocation(point);
    if (block->is_blocked(x,y,dir))
        return 0;
        
    return (1);
    // Check if this location's terrain is impassable to this army.
    // ISPassable(pLoc->GetTerrain());
};

Path::Path()
	{
	OPEN=0;
	CLOSED=0;
	flags=IS_MAP_SOLVER;
	m_pMap=0;	
	fastmap=0;
	};

Path::~Path()
	{
	}
// A* Algorithm
//struct NODE* Path::FindPath(int sx, int sy, int dx, int dy)
i4_bool Path::path_solve(i4_float startx, i4_float starty, 
			       i4_float destx, i4_float desty, 
                   w8 sizex, w8 sizey, w8 grade, i4_float *point, w16 &points)
{
    lPropagateDown=lPropLoop=lGenerateSuccessors=lGenerateSucc=lCheckOpen=lOpenLoop = 0;
    lCheckClosed=lClosedLoop=lInsert=lInsertLoop=lPush=lPop=lReturnBestNode = 0;
    lStackSize=lMaxStackSize=0;
	int sx=(int)startx;
	int sy=(int)starty;
	int dx=(int)destx;
	int dy=(int)desty;
    struct NODE *Node, *BestNode;
    //CApp* pApp = (CApp*) AfxGetApp();
    w32 deltaX, deltaY;
    m_pMap=g1_get_map();
    block = m_pMap->get_block_map(grade);
	mapxsize=m_pMap->width();
	mapysize=m_pMap->height();
    
    OPEN=(struct NODE *)calloc(1,sizeof( struct NODE ));
    CLOSED=(struct NODE *)calloc(1,sizeof( struct NODE ));
    // Setup the Stack.
    Stack=( struct STACK *)calloc(1,sizeof(struct STACK));
	//OPEN=new NODE;
	//CLOSED=new NODE;
	//Stack=new STACK;

	//Node=new NODE;
	fastmap=(struct FASTMAP *)calloc(mapxsize*mapysize,sizeof(struct FASTMAP));

    Node=(struct NODE *)calloc(1,sizeof( struct NODE ));
    Node->g = 0;    
    deltaX = abs(sx-dx);
    deltaY = abs(sy-dy);
    //Node->h = MAX(deltaX, deltaY);
	Node->h=sqrt((float)(deltaX*deltaX+deltaY*deltaY));
    Node->f = Node->g + Node->h;  
    Node->x = sx;
    Node->y = sy;

    OPEN->NextNode = Node;        // make Open List point to first node 
    for (;;)
    {
        BestNode = (struct NODE *)ReturnBestNode();	    
        // if we've found the end, break and finish 	    
        if (!BestNode || BestNode->x == dx && BestNode->y == dy)
            break;
        GenerateSuccessors(BestNode, dx, dy);
    }
	/* Here comes the find-link code*/
	points = 1;
	if (!BestNode)
		{
		points=0;
		cleanup();
		return i4_F;
		}
	struct NODE *current=BestNode->Parent;
	if (!current)
		{
		points=0;
		cleanup();
		return i4_F;
		}
	point[0]=(float) dx;
	point[1]=(float) dy;
	int dix,diy,dilx,dily;
	dix=dx-current->x;
	diy=dy-current->y;
	while(current->Parent)//we do not _want_ the first node to appear in the list
		{
		dilx=dix;
		dily=diy;
		dix=current->x-current->Parent->x;
		diy=current->y-current->Parent->y;
		if (dilx!=dix||dily!=diy)
			{
			point[2*points]=(float)current->x;
			point[2*points+1]=(float)current->y;
			points++;
			}
		current=current->Parent;
		
		}

    //return (BestNode);
	cleanup();
    return i4_T;
}

struct NODE* Path::ReturnBestNode(void)
{
    lReturnBestNode += 1;
    struct NODE *tmp;

    if (OPEN->NextNode == NULL)
    {
        //i4_error("ERROR: Cannot resolve path, no more nodes on OPEN");
        //RestoreKeyboard();    // restore BIOS keyboard handling
        //closegraph();	    
        //exit(0);
        //ASSERT(FALSE);
        return NULL;
    }

// Pick Node with lowest f, in this case it's the first node in list
//  because we sort the OPEN list wrt lowest f. Call it BESTNODE. 

    tmp = OPEN->NextNode;   // point to first node on OPEN
    OPEN->NextNode = tmp->NextNode;    // Make OPEN point to nextnode or NULL.
	long index=tmp->y*mapxsize+tmp->x;
	fastmap[index].on_open=0;
	fastmap[index].on_closed=tmp;
// Next take BESTNODE (or temp in this case) and put it on CLOSED

    tmp->NextNode = CLOSED->NextNode;
    CLOSED->NextNode = tmp;

    return(tmp);
}

void Path::GenerateSuccessors(struct NODE *BestNode, int dx, int dy)
{
    lGenerateSuccessors += 1;
    int x, y;
//East and west seem to be flipped. is this correct?
		    // Upper-Left
    if (FreeTile(x = BestNode->x - 1, y = BestNode->y - 1,G1_NORTH|G1_EAST))
        GenerateSucc(BestNode, x, y, dx, dy);
		    // Upper
    if (FreeTile(x = BestNode->x, y = BestNode->y - 1,G1_NORTH))
        GenerateSucc(BestNode, x, y, dx, dy);
		    // Upper-Right
    if (FreeTile(x = BestNode->x + 1, y = BestNode->y - 1,G1_NORTH|G1_WEST))
        GenerateSucc(BestNode, x, y, dx, dy);
		    // Right
    if (FreeTile(x = BestNode->x + 1, y = BestNode->y,G1_WEST))
        GenerateSucc(BestNode, x, y, dx, dy);
		    // Lower-Right
    if (FreeTile(x = BestNode->x + 1, y = BestNode->y + 1,G1_SOUTH|G1_WEST))
        GenerateSucc(BestNode, x, y, dx, dy);
		    // Lower
    if (FreeTile(x = BestNode->x , y = BestNode->y + 1,G1_SOUTH))
        GenerateSucc(BestNode, x, y, dx, dy);
		    // Lower-Left
    if (FreeTile(x = BestNode->x - 1, y = BestNode->y + 1,G1_SOUTH|G1_EAST))
        GenerateSucc(BestNode, x, y, dx, dy);
		    // Left
    if (FreeTile(x = BestNode->x - 1, y = BestNode->y,G1_EAST))
        GenerateSucc(BestNode, x, y, dx, dy);
}

w32 Path::MovementPenalty(int x, int y)
	{
	g1_map_cell_class *cell_on=m_pMap->cell((w16)x,(w16)y);
	g1_tile_class *t=g1_tile_man.get(cell_on->type);
	return (w32) t->friction_fraction*5;
	}

void Path::GenerateSucc(struct NODE *BestNode, int x, int y, int dx, int dy)
{
    lGenerateSucc += 1;
    float g;  // total path cost - as stored in the linked lists.
    int c = 0;
    w32 nTerrainCost;   // Movement cost of this terrain.
    w32 deltaX, deltaY;
    struct NODE *Old, *Successor;

    // g(Successor)=g(BestNode)+cost of getting from BestNode to Successor 
    //nTerrainCost = m_nMovementPenalty[m_pMap->GetLocation(x, y)->GetTerrainType()];
//	if (cur_grade==G1_GRADE_LEVELS-1)
//		{

	//get movement penalty. Most important for stank, which can go over water
		//g1_map_cell_class *cell_on=m_pMap->cell((w16)x,(w16)y);
  
		//w16 handle=cell_on->type;
	//  i4_float newheight;
  
		//g1_tile_class *t = g1_tile_man.get(handle);
		//nTerrainCost = (UINT) t->friction_fraction*10;
	nTerrainCost=MovementPenalty(x,y);
//		}
//	else 
//		nTerrainCost =0;

    g = BestNode->g + nTerrainCost;    

    if ((Old = CheckOPEN(x, y)) != NULL) // if equal to NULL then not in OPEN list, else it returns the Node in Old 
    {
    	for(c = 0; c < 8; c++)
    	    if(BestNode->Child[c] == NULL) // Add Old to the list of BestNode's Children (or Successors). 
    	        break;
    	BestNode->Child[c] = Old;

    	if (g < Old->g)  // if our new g value is < Old's then reset Old's parent to point to BestNode
    	{
    	    Old->Parent = BestNode;
    	    Old->g = g;
    	    Old->f = g + Old->h;
    	}
    }
    else if ((Old = CheckCLOSED(x, y)) != NULL) // if equal to NULL then not in OPEN list, else it returns the Node in Old
    {
        for(c = 0; c < 8; c++)
	        if (BestNode->Child[c] == NULL) // Add Old to the list of BestNode's Children (or Successors).
	            break;
	    BestNode->Child[c] = Old;

    	if (g < Old->g)  // if our new g value is < Old's then reset Old's parent to point to BestNode
    	{
    	    Old->Parent = BestNode;
    	    Old->g = g;
    	    Old->f = g + Old->h;
    	    PropagateDown(Old);       // Since we changed the g value of Old, we need
    					 // to propagate this new value downwards, i.e.
    					 // do a Depth-First traversal of the tree! 
    	}
    }
    else
    {
        Successor = (struct NODE *) calloc(1,sizeof( struct NODE ));
        Successor->Parent = BestNode;
        Successor->g = g;    	
        deltaX = abs(x-dx);
        deltaY = abs(y-dy);
		
		//Important: The distance estimates. Wich one is best?
        //Successor->h = MAX(deltaX, deltaY);//maximum distance extimate
		//Successor->h=sqrt(deltaX*deltaX+deltaY*deltaY);//euclidean (would require floats)
		//Successor->h=(deltaX==deltaY)?sqrt2:1;//faster euclidean(dito)
		Successor->h=deltaX+deltaY;//Manhattan distance extimate

        Successor->f = g + Successor->h;
        Successor->x = x;
        Successor->y = y;
		fastmap[y*mapxsize+x].on_open=Successor;
        Insert(Successor);     // Insert Successor on OPEN list wrt f
        for(c = 0; c < 8; c++)
            if (BestNode->Child[c] == NULL) // Add Old to the list of BestNode's Children (or Successors).
    	  break;
        BestNode->Child[c] = Successor;
    }
}

struct NODE* Path::CheckOPEN(int x, int y)
{
    lCheckOpen += 1;
    //struct NODE *tmp;

    //tmp = OPEN->NextNode;
	return fastmap[y*mapxsize+x].on_open;
    //while (tmp != NULL)
    //{
    //	lOpenLoop += 1;
    //	if (tmp->x == x && tmp->y == y)
    //	    return (tmp);
    //	else
    //	    tmp = tmp->NextNode;
    //}
    //return (NULL);
}

struct NODE* Path::CheckCLOSED(int x, int y)
{
    lCheckClosed += 1;
    //struct NODE *tmp;

    //tmp = CLOSED->NextNode;
	
	return fastmap[y*mapxsize+x].on_closed;
    //while (tmp != NULL)
    //{
    //	lClosedLoop += 1;
    //	if (tmp->x == x && tmp->y == y)
    //	    return (tmp);
    //	else
    //	    tmp = tmp->NextNode;
    //}
    //return (NULL);
}

void Path::Insert(struct NODE *Successor)
{
    lInsert += 1;
    struct NODE *tmp1, *tmp2;
    float f;
	
    if (OPEN->NextNode == NULL)
    {
	    OPEN->NextNode = Successor;
	    return;
    }

    // insert into OPEN successor wrt f
    f = Successor->f;
    tmp1 = OPEN;
    tmp2 = OPEN->NextNode;

    while ((tmp2 != NULL) && (tmp2->f < f))
    {
        lInsertLoop += 1;
        tmp1 = tmp2;
        tmp2 = tmp2->NextNode;
    }
	Successor->NextNode = tmp2;
	tmp1->NextNode = Successor;
}

void Path::PropagateDown(struct NODE *Old)
{
    lPropagateDown += 1;
    int     c;
    float    g;
    float    nTerrainCost;
    struct NODE *Child, *Father;    

    g = Old->g;            // alias.
    for(c = 0; c < 8; c++)
    {
        Child = Old->Child[c];  // Create alias for faster access.
        if (Child == NULL)
            break;
        nTerrainCost = MovementPenalty(Child->x,Child->y);        
        if (g + nTerrainCost < Child->g)
        {
            Child->g = g + nTerrainCost;
            Child->f = Child->g + Child->h;
            Child->Parent = Old;          // reset parent to new path.
            Push(Child);                  // Now the Child's branch need to be
        }     // checked out. Remember the new cost must be propagated down.
    }

    while (Stack->NextStackPtr != NULL)
    {
    	lPropLoop += 1;
    	Father = Pop();
    	for(c = 0; c < 8; c++)
    	{
    	    Child = Father->Child[c];
            if (Child == NULL)       // we may stop the propagation 2 ways: either
    	        break;
            nTerrainCost = MovementPenalty(Child->x, Child->y);    	    
    	    if (Father->g + nTerrainCost < Child->g) // there are no children, or that the g value of
    	    {                           // the child is equal or better than the cost we're propagating
                        Child->g = Father->g + nTerrainCost;
                        Child->f = Child->g + Child->h;
    	        Child->Parent = Father;
    	        Push(Child);
    	    }
    	}
    }
}

///////////////////////////////////////////////////////////////////////////
// STACK Functions
///////////////////////////////////////////////////////////////////////////

void Path::Push(struct NODE *Node)
{
    lPush += 1;
	lStackSize++;
	if (lMaxStackSize<lStackSize) 
		lMaxStackSize=lStackSize;
    struct STACK *tmp;

    //tmp = (struct STACK *) calloc(1,sizeof(struct STACK));
    tmp = (struct STACK *) malloc(sizeof(struct STACK));
    if (!tmp)
        I4_ASSERT(i4_F,"INTERNAL: Search stack underflow");

    tmp->NodePtr = Node;
    tmp->NextStackPtr = Stack->NextStackPtr;
    Stack->NextStackPtr = tmp;
}

struct NODE* Path::Pop()
{
    lPop += 1;
	lStackSize--;
    struct NODE *tmp;
    struct STACK *tmpSTK;

    tmpSTK = Stack->NextStackPtr;
    tmp = tmpSTK->NodePtr;

    Stack->NextStackPtr = tmpSTK->NextStackPtr;
    free(tmpSTK);
    return (tmp);
}
