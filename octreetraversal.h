#ifndef __OCTREETRAVERSAL_H_
#define __OCTREETRAVERSAL_H_

//-------------------------------------------------------------------------------
//  Copyright 2002 Mitsubishi Electric Research Laboratories.
//  All Rights Reserved.
//
//  Permission to use, copy, modify and distribute this software and its
//  documentation for educational, research and non-profit purposes, without fee,
//  and without a written agreement is hereby granted, provided that the above
//  copyright notice and the following three paragraphs appear in all copies.
//
//  To request permission to incorporate this software into commercial products
//  contact MERL - Mitsubishi Electric Research Laboratories, 201 Broadway,
//  Cambridge, MA 02139.
//
//  IN NO EVENT SHALL MERL BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL,
//  INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING OUT OF
//  THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF MERL HAS BEEN ADVISED
//  OF THE POSSIBILITY OF SUCH DAMAGES.
//
//  MERL SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//  THE SOFTWARE PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND MERL HAS NO
//  OBLIGATIONS TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR
//  MODIFICATIONS.
//-------------------------------------------------------------------------------



// This is a modified version of the quadtree methods to support octree calculations.
//
// author Sascha Scandella

//-------------------------------------------------------------------------------
//  Macro to traverse an octree from a specified cell (typically the root cell)
//  to a leaf cell by following the x, y and z locational codes, xLocCode, yLocCode
//  and zLocCode. Upon entering, cell is the specified cell and nextLevel is one less
//  than the level of the specified cell. Upon termination, cell is the leaf cell
//  and nextLevel is one less than the level of the leaf cell.
//-------------------------------------------------------------------------------
#define OT_TRAVERSE(cell,nextLevel,xLocCode,yLocCode,zLocCode)                    \
{                                                                                 \
    while ((cell)->IsSubdivided()) {                                                    \
        unsigned int childBranchBit = 1 << (nextLevel);                           \
        unsigned int childIndex = ((((xLocCode) & childBranchBit) >> (nextLevel)) \
		+ (((yLocCode) & childBranchBit) >> ((nextLevel - 1)))                    \
        + (((zLocCode) & childBranchBit) >> ((nextLevel - 2))));				  \
		nextLevel--;															  \
		(cell) = (((cell)->m_pOctreeNodes)[childIndex]);								  \
    }                                                                             \
}


//-------------------------------------------------------------------------------
//  Macro to traverse an octree from a specified cell to an offspring cell by
//  following the x, y and z locational codes, xLocCode, yLocCode and zLocCode.
//  The offpring cell is either at a specified level or is a leaf cell if a leaf
//  cell is reached before the specified level. Upon entering, cell is the specified
//  cell and nextLevel is one less than the level of the specified cell. Upon
//  termination, cell is the offspring cell and nextLevel is one less than the
//  level of the offspring cell.
//-------------------------------------------------------------------------------
#define OT_TRAVERSE_TO_LEVEL(cell,nextLevel,xLocCode,yLocCode, zLocCode,level)    \
{                                                                                 \
    unsigned int n = (nextLevel) - (level) + 1;                                   \
    while (n--) {                                                                 \
        unsigned int childBranchBit = 1 << (nextLevel);                           \
        unsigned int childIndex = ((((xLocCode) & childBranchBit) >> (nextLevel)) \
		+ (((yLocCode) & childBranchBit) >> ((nextLevel - 1)))                    \
        + (((zLocCode) & childBranchBit) >> ((nextLevel - 2))));				  \
		nextLevel--;															  \
		(cell) = (((cell)->m_pOctreeNodes)[childIndex]);								  \
		if ((cell)->isLeaf()) break;                                             \
    }                                                                             \
}


//-------------------------------------------------------------------------------
//  Macro for traversing an octree to a common ancestor of a specified cell
//  and its neighbor, whose x, y and z locational code differs from the cell's
//  corresponding x, y and z locational code by binaryDiff (determined by XOR'ing the
//  appropriate pair of x, y and z locational codes). Upon entering, cell is the
//  specified cell and cellLevel is the cell's level. Upon termination, cell is
//  the common ancestor and cellLevel is the common ancestor's level.
//-------------------------------------------------------------------------------
#define OT_GET_COMMON_ANCESTOR(cell,cellLevel,binaryDiff)                         \
{                                                                                 \
    while ((binaryDiff) & (1 << (cellLevel))) {                                   \
        (cell) = (cell)->m_pParent;                                                  \
        (cellLevel)++;                                                            \
    }                                                                             \
}

#endif	// __OCTREETRAVERSAL_H_