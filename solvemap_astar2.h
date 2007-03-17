//.h code fragments

//=================

#ifndef SOLVEMAP_ASTAR2_INCLUDED

#define SOLVEMAP_ASTAR2_INCLUDED

struct NODE {

	float f,h;

	float g,tmpg;

	int x,y;

	//int NodeNum;

	//CPoint NodeElement;

	struct NODE *Parent;

	struct NODE *Child[8];       /* a node may have upto 8+(NULL) children. */

	struct NODE *NextNode;       /* for filing purposes */

};



struct FASTMAP {

	struct NODE *on_open;

	struct NODE *on_closed;

};



struct STACK {

	struct NODE *NodePtr;

	struct STACK *NextStackPtr;

};



// Algorithm stuff



/*    struct NODE *OPEN;

   	struct NODE *CLOSED;

   	struct STACK *Stack;



   // Algorithm stuff

   	BOOL FreeTile(int x, int y);

   	struct NODE* FindPath(int sx, int sy, int dx, int dy);

   	struct NODE* ReturnBestNode(void);

   	void GenerateSuccessors(struct NODE *BestNode, int dx, int dy);

   	void GenerateSucc(struct NODE *BestNode, int x, int y, int dx, int dy);

   	struct NODE *CheckOPEN(int x, int y);

   	struct NODE *CheckCLOSED(int x, int y);

   	void Insert(struct NODE *Successor);

   	void PropagateDown(struct NODE *Old);

   	void Push(struct NODE *Node);

   	struct NODE *Pop(void);

 */

class Path :
	public g1_map_solver_class

{

protected:

	g1_block_map_class *block;

	g1_map_class *m_pMap;

	long mapxsize,mapysize;

	long lPropagateDown,lPropLoop,lGenerateSuccessors,lGenerateSucc,lCheckOpen,

		 lOpenLoop,lCheckClosed,lClosedLoop,lInsert,lInsertLoop,

		 lPush,lPop,lReturnBestNode,lStackSize,lMaxStackSize; //these are for performance analysis only



	struct NODE *OPEN,*CLOSED;

	struct STACK *Stack;

	FASTMAP *fastmap;

public:

	void cleanup();

	Path();

	~Path();

	void set_block_map(g1_block_map_class *_block)

	{

		block=_block;

		m_pMap=g1_get_map();

	};

	i4_bool path_solve(i4_float startx, i4_float starty,

					   i4_float destx, i4_float desty,

					   w8 sizex, w8 sizey, w8 grade, i4_float *point, w16 &points);

	struct NODE *ReturnBestNode();

	i4_bool FreeTile(int x, int y, w8 dir);

	void GenerateSuccessors(struct NODE *Bestnode, int dx, int dy);

	void GenerateSucc(struct NODE *BestNode, int x, int y, int dx, int dy);

	struct NODE *CheckOPEN(int x, int y);

	struct NODE *CheckCLOSED(int x, int y);

	void Insert(struct NODE *Successor);

	void PropagateDown(struct NODE *Old);

	void Push(struct NODE *Node);

	struct NODE *Pop(void);



	w32 MovementPenalty(int x,int y);

};



#endif
