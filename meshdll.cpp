#include <windows.h>
#include <math.h>
#define DllExport __declspec( dllexport )



class Face
	{
	public:
	DWORD flags;
	Face(){flags=0;};
	~Face(){};
	DllExport void setEdgeVisFlags(int i, int j, int k){};
	};

class Mesh {
	friend class Face;
	public:
	DllExport Mesh(){};
	DllExport ~Mesh(){}
	DllExport BOOL 	setNumVerts(int ct, BOOL keep=FALSE){ return true;};

	DllExport BOOL	setNumFaces(int ct, BOOL keep=FALSE){return true;};

	DllExport void	InvalidateEdgeList(){}; // RB

	DllExport void	buildBoundingBox(void){};

	DllExport void  buildNormals(void){};
	};
