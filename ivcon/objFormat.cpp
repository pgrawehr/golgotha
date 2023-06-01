#include <cstdio>
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ivcon.h"
#include "objFormat.h"

/******************************************************************************/

int obj_read(FILE* filein)

/******************************************************************************/

/*
   Purpose:

	OBJ_READ reads a Wavefront OBJ file.

   Example:

 #  magnolia.obj

	mtllib ./vp.mtl

	g
	v -3.269770 -39.572201 0.876128
	v -3.263720 -39.507999 2.160890
	...
	v 0.000000 -9.988540 0.000000
	g stem
	s 1
	usemtl brownskn
	f 8 9 11 10
	f 12 13 15 14
	...
	f 788 806 774

   Modified:

	20 October 1998

   Author:

	John Burkardt

   See http://www.martinreddy.net/gfx/3d/OBJ.spec for a detailed description of the format
 */
{
	int count;
	int i;
	int ivert;
	char* next;
	char* next2;
	char* next3;
	int node;
	int vertex_normal_num;
	int vertex_uv_num;
	float r1;
	float r2;
	float r3;
	char token[LINE_MAX_LEN];
	char token2[LINE_MAX_LEN];
	int width;

	/*
	   Initialize.
	 */
	vertex_normal_num = 0;
	vertex_uv_num = 0;
	/*
	   Read the next line of the file into INPUT.
	 */
	char input[LINE_MAX_LEN];

	while (fgets(input, LINE_MAX_LEN, filein) != NULL)
	{

		text_num = text_num + 1;
		/*
		   Advance to the first nonspace character in INPUT.
		 */
		for (next = input; *next != '\0' && isspace(*next); next++)
		{
		}
		/*
		   Skip blank lines and comments.
		 */

		if (*next == '\0')
		{
			continue;
		}

		if (*next == '#' || *next == '$')
		{
			comment_num = comment_num + 1;
			continue;
		}
		/*
		   Extract the first word in this line.
		 */
		sscanf(next, "%s%n", token, &width);
		/*
		   Set NEXT to point to just after this token.
		 */

		next = next + width;
		/*
		   BEVEL
		   Bevel interpolation.
		 */
		if (leqi(token, "BEVEL") == TRUE)
		{
			continue;
		}
		/*
		   BMAT
		   Basis matrix.
		 */
		else if (leqi(token, "BMAT") == TRUE)
		{
			continue;
		}
		/*
		   C_INTERP
		   Color interpolation.
		 */
		else if (leqi(token, "C_INTERP") == TRUE)
		{
			continue;
		}
		/*
		   CON
		   Connectivity between free form surfaces.
		 */
		else if (leqi(token, "CON") == TRUE)
		{
			continue;
		}
		/*
		   CSTYPE
		   Curve or surface type.
		 */
		else if (leqi(token, "CSTYPE") == TRUE)
		{
			continue;
		}
		/*
		   CTECH
		   Curve approximation technique.
		 */
		else if (leqi(token, "CTECH") == TRUE)
		{
			continue;
		}
		/*
		   CURV
		   Curve.
		 */
		else if (leqi(token, "CURV") == TRUE)
		{
			continue;
		}
		/*
		   CURV2
		   2D curve.
		 */
		else if (leqi(token, "CURV2") == TRUE)
		{
			continue;
		}
		/*
		   D_INTERP
		   Dissolve interpolation.
		 */
		else if (leqi(token, "D_INTERP") == TRUE)
		{
			continue;
		}
		/*
		   DEG
		   Degree.
		 */
		else if (leqi(token, "DEG") == TRUE)
		{
			continue;
		}
		/*
		   END
		   End statement.
		 */
		else if (leqi(token, "END") == TRUE)
		{
			continue;
		}
		/*
		   F V1 V2 V3
			or
		   F V1/VT1/VN1 V2/VT2/VN2 ...
			or
		   F V1//VN1 V2//VN2 ...

		   Face.
		   A face is defined by the vertices.
		   Optionally, slashes may be used to include the texture vertex
		   and vertex normal indices.

		   OBJ line node indices are 1 based rather than 0 based.
		   So we have to decrement them before loading them into FACE.
		 */
		else if (leqi(token, "F") == TRUE)
		{

			ivert = 0;
			face_order[face_num] = 0;
			/*
			   Read each item in the F definition as a token, and then
			   take it apart.
			 */
			for (;; )
			{

				count = sscanf(next, "%s%n", token2, &width);
				next = next + width;

				if (count != 1)
				{
					break;
				}

				count = sscanf(token2, "%d%n", &node, &width);
				next2 = token2 + width;

				if (count != 1)
				{
					break;
				}

				if (ivert < ORDER_MAX && face_num < FACE_MAX)
				{
					face[ivert][face_num] = node - 1;
					vertex_material[ivert][face_num] = 0;
					face_order[face_num] = face_order[face_num] + 1;
					face_material[face_num] = material_num - 1 > 0 ? material_num - 1 : 0;
					face_flags[face_num] = FACE_FLAGS_MATERIAL_ASSIGNED | FACE_FLAGS_BOTHSIDED;
				}
				/*
				   If there's a slash, skip to the next slash, and extract the
				   index of the normal vector.
				 */
				if (*next2 == '/')
				{
					bool vtFound = false;
					for (next3 = next2 + 1; next3 < token2 + LINE_MAX_LEN; next3++)
					{
						if (*next3 == '/')
						{
							next3 = next3 + 1;
							count = sscanf(next3, "%d%n", &node, &width);

							node = node - 1;
							if (0 <= node && node < vertex_normal_num)
							{
								for (i = 0; i < 3; i++)
								{
									vertex_normal[i][ivert][face_num] = normal_temp[i][node];
								}
							}
							break;
						}
						else if (!vtFound) // Only if we haven't processed the vt entry yet, otherwise it would re-read it for every digit chopped off
						{
							vtFound = true;
							char* next4 = next3; // there's a vt reference here
							sscanf(next4, "%d%n", &node, &width);
							node = node - 1;
							if (0 <= node && node < vertex_uv_num)
							{
								if (ivert == 1 || ivert == 2 || ivert == 0 || ivert == 3)
								{
									float vt = normalize_texture_coordinates(vertex_uv_temp[0][node]);
									vertex_tex_uv[0][ivert][face_num] = vt;
									vt = normalize_texture_coordinates(vertex_uv_temp[1][node]);
									vertex_tex_uv[1][ivert][face_num] = vt;
								}
								else
								{
									// For vertexes 3 and 4, we swap the two
									float vt = normalize_texture_coordinates(vertex_uv_temp[1][node]);
									vertex_tex_uv[0][ivert][face_num] = vt;
									vt = normalize_texture_coordinates(vertex_uv_temp[0][node]);
									vertex_tex_uv[1][ivert][face_num] = vt;
								}
							}
						}
					}
				}
				ivert = ivert + 1;
			}
			face_num = face_num + 1;
		}
		/*
		   G
		   Group name.
		 */
		else if (leqi(token, "G") == TRUE)
		{
			continue;
		}
		/*
		   HOLE
		   Inner trimming hole.
		 */
		else if (leqi(token, "HOLE") == TRUE)
		{
			continue;
		}
		/*
		   L
		   I believe OBJ line node indices are 1 based rather than 0 based.
		   So we have to decrement them before loading them into LINE_DEX.
		 */
		else if (leqi(token, "L") == TRUE)
		{

			for (;; )
			{

				count = sscanf(next, "%d%n", &node, &width);
				next = next + width;

				if (count != 1)
				{
					break;
				}

				if (line_num < LINE_MAX)
				{
					line_dex[line_num] = node - 1;
					line_material[line_num] = 0;
				}
				line_num = line_num + 1;

			}

			if (line_num < LINE_MAX)
			{
				line_dex[line_num] = -1;
				line_material[line_num] = -1;
			}
			line_num = line_num + 1;

		}
		/*
		   LOD
		   Level of detail.
		 */
		else if (leqi(token, "LOD") == TRUE)
		{
			continue;
		}
		/*
		   MG
		   Merging group.
		 */
		else if (leqi(token, "MG") == TRUE)
		{
			continue;
		}
		/*
		   MTLLIB
		   Material library.
		 */
		else if (leqi(token, "MTLLIB") == TRUE)
		{
			continue;
		}
		/*
		   O
		   Object name.
		 */
		else if (leqi(token, "O") == TRUE)
		{
			continue;
		}
		/*
		   P
		   Point.
		 */
		else if (leqi(token, "P") == TRUE)
		{
			continue;
		}
		/*
		   PARM
		   Parameter values.
		 */
		else if (leqi(token, "PARM") == TRUE)
		{
			continue;
		}
		/*
		   S
		   Smoothing group
		 */
		else if (leqi(token, "S") == TRUE)
		{
			continue;
		}
		/*
		   SCRV
		   Special curve.
		 */
		else if (leqi(token, "SCRV") == TRUE)
		{
			continue;
		}
		/*
		   SHADOW_OBJ
		   Shadow casting.
		 */
		else if (leqi(token, "SHADOW_OBJ") == TRUE)
		{
			continue;
		}
		/*
		   SP
		   Special point.
		 */
		else if (leqi(token, "SP") == TRUE)
		{
			continue;
		}
		/*
		   STECH
		   Surface approximation technique.
		 */
		else if (leqi(token, "STECH") == TRUE)
		{
			continue;
		}
		/*
		   STEP
		   Stepsize.
		 */
		else if (leqi(token, "CURV") == TRUE)
		{
			continue;
		}
		/*
		   SURF
		   Surface.
		 */
		else if (leqi(token, "SURF") == TRUE)
		{
			continue;
		}
		/*
		   TRACE_OBJ
		   Ray tracing.
		 */
		else if (leqi(token, "TRACE_OBJ") == TRUE)
		{
			continue;
		}
		/*
		   TRIM
		   Outer trimming loop.
		 */
		else if (leqi(token, "TRIM") == TRUE)
		{
			continue;
		}
		/*
		   USEMTL
		   Material name.
		 */
		else if (leqi(token, "USEMTL") == TRUE)
		{
			char current_material_name[FILENAME_MAX];
			char* ptrToNewLine = strchr(next + 1, '\n');
			strncpy_s(current_material_name, FILENAME_MAX - 1, next + 1, ptrToNewLine - next - 1);
			strcpy(material_name[material_num], current_material_name);
			material_num++;
			continue;
		}
		/*
		   V X Y Z W
		   Geometric vertex.
		   W is optional, a weight for rational curves and surfaces.
		   The default for W is 1.
		 */
		else if (leqi(token, "V") == TRUE)
		{

			sscanf(next, "%e %e %e", &r1, &r2, &r3);

			if (cor3_num < COR3_MAX)
			{
				cor3[0][cor3_num] = r1;
				cor3[1][cor3_num] = r2;
				cor3[2][cor3_num] = r3;
			}

			cor3_num = cor3_num + 1;

		}
		/*
		   VN
		   Vertex normals.
		 */
		else if (leqi(token, "VN") == TRUE)
		{

			sscanf(next, "%e %e %e", &r1, &r2, &r3);

			if (vertex_normal_num < ORDER_MAX * FACE_MAX)
			{
				normal_temp[0][vertex_normal_num] = r1;
				normal_temp[1][vertex_normal_num] = r2;
				normal_temp[2][vertex_normal_num] = r3;
			}

			vertex_normal_num = vertex_normal_num + 1;

		}
		/*
		   VT
		   Vertex texture (texture coordinates per vertex)
		 */
		else if (leqi(token, "VT") == TRUE)
		{
			sscanf(next, "%e %e", &r1, &r2);

			if (vertex_uv_num < ORDER_MAX * FACE_MAX)
			{
				vertex_uv_temp[0][vertex_uv_num] = r1;
				vertex_uv_temp[1][vertex_uv_num] = r2;
			}

			vertex_uv_num = vertex_uv_num + 1;
		}
		/*
		   VP
		   Parameter space vertices.
		 */
		else if (leqi(token, "VP") == TRUE)
		{
			continue;
		}
		/*
		   Unrecognized
		 */
		else
		{
			bad_num = bad_num + 1;
		}

	}
	ConvertMaterialToTexture();
	return SUCCESS;
}

// Applies the mtl file to a loaded material list (which typically only contains names, but not colors or texture file names)
int mtl_read(FILE* filein)
{
	char input[LINE_MAX_LEN];
	char* next;
	char token[LINE_MAX_LEN];
	int width = 0;
	char currentMtl[LINE_MAX_LEN];
	currentMtl[0] = '\0';
	int matIndex = -1;
	while (fgets(input, LINE_MAX_LEN, filein) != NULL)
	{
		/*
		   Advance to the first nonspace character in INPUT.
		 */
		for (next = input; *next != '\0' && isspace(*next); next++)
		{
		}
		/*
		   Skip blank lines and comments.
		 */

		if (*next == '\0')
		{
			continue;
		}

		if (*next == '#' || *next == '$')
		{
			comment_num = comment_num + 1;
			continue;
		}
		/*
		   Extract the first word in this line.
		 */
		sscanf(next, "%s%n", token, &width);
		/*
		   Set NEXT to point to just after this token.
		 */

		next = next + width;

		if (leqi(token, "newmtl"))
		{
			sscanf(next, "%s%n", token, &width);
			strcpy(currentMtl, token);

			for (int i = 0; i < material_num; i++)
			{
				if (leqi(material_name[i], currentMtl))
				{
					matIndex = i;
					break;
				}
			}
			continue;
		}

		if (leqi(token, "Ka")) // Ambient color (is this the right one?)
		{
			if (matIndex >= 0)
			{
				float r, g, b;
				sscanf(next, "%e %e %e", &r, &g, &b);
				material_rgba[matIndex].R = r;
				material_rgba[matIndex].G = g;
				material_rgba[matIndex].B = b;
				material_rgba[matIndex].A = 1.0f;
			}
		}

		if (leqi(token, "Kd")) // Diffuse color (is this the right one?)
		{
			if (matIndex >= 0)
			{
				float r, g, b;
				sscanf(next, "%e %e %e", &r, &g, &b);
				material_rgba[matIndex].R = r;
				material_rgba[matIndex].G = g;
				material_rgba[matIndex].B = b;
				material_rgba[matIndex].A = 1.0f;
			}
		}

		if (leqi(token, "d")) // Transparency
		{
			if (matIndex >= 0)
			{
				float a;
				sscanf(next, "%e", &a);
				material_rgba[matIndex].A = a;
			}
		}
	}

	return SUCCESS;
}


int obj_write(FILE* fileout)

/******************************************************************************/

/*
   Purpose:

	OBJ_WRITE writes a Wavefront OBJ file.

   Example:

 #  magnolia.obj

	mtllib ./vp.mtl

	g
	v -3.269770 -39.572201 0.876128
	v -3.263720 -39.507999 2.160890
	...
	v 0.000000 -9.988540 0.000000
	g stem
	s 1
	usemtl brownskn
	f 8 9 11 10
	f 12 13 15 14
	...
	f 788 806 774

   Modified:

	01 September 1998

   Author:

	John Burkardt
 */
{
	int i;
	int iface;
	int indexvn;
	int ivert;
	int k;
	int newval;
	int text_num;
	float w;

	/*
	   Initialize.
	 */
	text_num = 0;
	w = 1.0;

	fprintf(fileout, "# %s created by IVCON.\n", fileout_name);
	fprintf(fileout, "# Original data in %s.\n", filein_name);
	fprintf(fileout, "\n");
	fprintf(fileout, "g %s\n", object_name);
	fprintf(fileout, "\n");

	text_num = text_num + 5;
	/*
	   V: vertex coordinates.
	 */
	for (i = 0; i < cor3_num; i++)
	{
		fprintf(fileout, "v %f %f %f %f\n",
			cor3[0][i], cor3[1][i], cor3[2][i], w);
		text_num = text_num + 1;
	}

	/*
	   VN: Vertex face normal vectors.
	 */
	if (face_num > 0)
	{
		fprintf(fileout, "\n");
		text_num = text_num + 1;
	}

	for (iface = 0; iface < face_num; iface++)
	{

		for (ivert = 0; ivert < face_order[iface]; ivert++)
		{

			fprintf(fileout, "vn %f %f %f\n", vertex_normal[0][ivert][iface],
				vertex_normal[1][ivert][iface], vertex_normal[2][ivert][iface]);
			text_num = text_num + 1;
		}
	}
	/*
	   F: faces.
	 */
	if (face_num > 0)
	{
		fprintf(fileout, "\n");
		text_num = text_num + 1;
	}

	indexvn = 0;

	for (iface = 0; iface < face_num; iface++)
	{

		fprintf(fileout, "f");
		for (ivert = 0; ivert < face_order[iface]; ivert++)
		{
			indexvn = indexvn + 1;
			fprintf(fileout, " %d//%d", face[ivert][iface] + 1, indexvn);
		}
		fprintf(fileout, "\n");
		text_num = text_num + 1;
	}
	/*
	   L: lines.
	 */
	if (line_num > 0)
	{
		fprintf(fileout, "\n");
		text_num = text_num + 1;
	}

	newval = TRUE;

	for (i = 0; i < line_num; i++)
	{

		k = line_dex[i];

		if (k == -1)
		{
			fprintf(fileout, "\n");
			text_num = text_num + 1;
			newval = TRUE;
		}
		else
		{
			if (newval == TRUE)
			{
				fprintf(fileout, "l");
				newval = FALSE;
			}
			fprintf(fileout, " %d", k + 1);
		}

	}

	fprintf(fileout, "\n");
	text_num = text_num + 1;
	/*
	   Report.
	 */
	printf("\n");
	printf("OBJ_WRITE - Wrote %d text lines.\n", text_num);

	return SUCCESS;
}