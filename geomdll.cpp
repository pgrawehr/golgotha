//This file contains dummy code to create a core.lib file 

#include <math.h>
#include <windows.h>
#define DllExport __declspec( dllexport )
#include <point3.h>
#include <matrix3.h>



Point3 DllExport operator*(const Matrix3& A, const Point3& V)
	{return 0;}
 // Transform Point with matrix

Point3 DllExport operator*(const Point3& V,const Matrix3& A)
	{return 0;}

Matrix3 DllExport TransMatrix(const Point3& V){return 0;}

void DllExport Matrix3::IdentityMatrix(void){};


