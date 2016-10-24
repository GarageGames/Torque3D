//-----------------------------------------------------------------------------
// Copyright (c) 2012 GarageGames, LLC
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------

#ifndef _MBOXBASE_H_
#define _MBOXBASE_H_

#ifndef _MPOINT3_H_
#include "math/mPoint3.h"
#endif


/// Base class for box geometries.
class BoxBase
{
   public:

      /// Indices of the corner points.
      ///
      /// @note The order defined here is expected by several places
      ///   in the code!
      enum Points
      {
         NearBottomRight,
         NearTopRight,
         NearTopLeft,
         NearBottomLeft,

         FarBottomRight,
         FarTopRight,
         FarTopLeft,
         FarBottomLeft,

         NUM_POINTS,
		 InvalidPoint = NUM_POINTS
      };

      /// Return the point index for the opposite corner of @a p.
      static Points getOppositePoint( Points p )
      {
         switch( p )
         {
            case NearBottomRight:   return FarTopLeft;
            case NearTopRight:      return FarBottomLeft;
            case NearTopLeft:       return FarBottomRight;
            case NearBottomLeft:    return FarTopRight;

            case FarBottomRight:    return NearTopLeft;
            case FarTopRight:       return NearBottomLeft;
            case FarTopLeft:        return NearBottomLeft;
            default:
            case FarBottomLeft:     return NearTopRight;
         }
      }
       
      /// Return the point index for the corner point that corresponds
      /// to the octant that @a p points to.
      static Points getPointIndexFromOctant( const Point3F& p )
      {
         if( p.x > 0.f ) // Right
         {
            if( p.y > 0.f ) // Far
            {
               if( p.z > 0.f ) // Top
                  return FarTopRight;
               else // Bottom
                  return FarBottomRight;
            }
            else // Near
            {
               if( p.z > 0.f ) // Top
                  return NearTopRight;
               else // Bottom
                  return NearBottomRight;
            }
         }
         else // Left
         {
            if( p.y > 0.f ) // Far
            {
               if( p.z > 0.f ) // Top
                  return FarTopLeft;
               else // Bottom
                  return FarBottomLeft;
            }
            else // Near
            {
               if( p.z > 0.f ) // Top
                  return NearTopLeft;
               else // Bottom
                  return NearBottomLeft;
            }
         }
      }

      /// Indices for the side planes of the box.  Each pair of planes
      /// has successive indices.  Also, the planes are ordered by X (left&right),
      /// Y (near&far), and Z (top&bottom).
      enum Planes
      {
         LeftPlane,
         RightPlane,
         NearPlane,
         FarPlane,
         TopPlane,
         BottomPlane,

         NUM_PLANES
      };

      enum PlaneMasks : U32
      {
         PlaneMaskLeft     = ( 1 << LeftPlane ),
         PlaneMaskRight    = ( 1 << RightPlane ),
         PlaneMaskTop      = ( 1 << TopPlane ),
         PlaneMaskBottom   = ( 1 << BottomPlane ),
         PlaneMaskNear     = ( 1 << NearPlane ),
         PlaneMaskFar      = ( 1 << FarPlane ),

         PlaneMaskAll      = 0xFFFFFFFF,
      };

      ///
      static Points getPlanePointIndex( Planes plane, U32 i )
      {
         switch( plane )
         {
            case LeftPlane:
               switch( i )
               {
                  case 0:  return NearBottomLeft;
                  case 1:  return NearTopLeft;
                  case 2:  return FarTopLeft;
                  case 3:  return FarBottomLeft;
                  default: AssertFatal( false, "BoxBase::getPlanePointIndex - Invalid index" );
               }
               break;
            case RightPlane:
               switch( i )
               {
                  case 0:  return NearBottomRight;
                  case 1:  return FarBottomRight;
                  case 2:  return FarTopRight;
                  case 3:  return NearTopRight;
                  default: AssertFatal( false, "BoxBase::getPlanePointIndex - Invalid index" );
               }
               break;
            case NearPlane:
               switch( i )
               {
                  case 0:  return NearBottomLeft;
                  case 1:  return NearBottomRight;
                  case 2:  return NearTopRight;
                  case 3:  return NearTopLeft;
                  default: AssertFatal( false, "BoxBase::getPlanePointIndex - Invalid index" );
               }
               break;
            case FarPlane:
               switch( i )
               {
                  case 0:  return FarBottomLeft;
                  case 1:  return FarTopLeft;
                  case 2:  return FarTopRight;
                  case 3:  return FarBottomRight;
                  default: AssertFatal( false, "BoxBase::getPlanePointIndex - Invalid index" );
               }
               break;
            case TopPlane:
               switch( i )
               {
                  case 0:  return NearTopLeft;
                  case 1:  return NearTopRight;
                  case 2:  return FarTopRight;
                  case 3:  return FarTopLeft;
                  default: AssertFatal( false, "BoxBase::getPlanePointIndex - Invalid index" );
               }
               break;
            case BottomPlane:
               switch( i )
               {
                  case 0:  return NearBottomLeft;
                  case 1:  return FarBottomLeft;
                  case 2:  return FarBottomRight;
                  case 3:  return NearBottomRight;
                  default: AssertFatal( false, "BoxBase::getPlanePointIndex - Invalid index" );
               }
               break;
            default:
               AssertFatal( false, "BoxBase::getPlanePointIndex - Invalid plane" );
         }
        return InvalidPoint;
      }

      /// Indices for the edges of the box.
      enum Edges
      {
         NearLeftEdge,
         NearBottomEdge,
         NearRightEdge,
         NearTopEdge,

         FarLeftEdge,
         FarTopEdge,
         FarRightEdge,
         FarBottomEdge,

         LeftTopEdge,
         LeftBottomEdge,

         RightTopEdge,
         RightBottomEdge,

         NUM_EDGES,
         InvalidEdge
      };

      /// Get the start and end point of the given edge.
      static void getEdgePointIndices( Edges edge, Points& outP1, Points& outP2 )
      {
         switch( edge )
         {
            case NearLeftEdge:      outP1 = NearTopLeft; outP2 = NearBottomLeft; return;
            case NearBottomEdge:    outP1 = NearBottomLeft; outP2 = NearBottomRight; return;
            case NearRightEdge:     outP1 = NearBottomRight; outP2 = NearTopRight; return;
            case NearTopEdge:       outP1 = NearTopRight; outP2 = NearTopLeft; return;

            case FarLeftEdge:       outP1 = FarBottomLeft; outP2 = FarTopLeft; return;
            case FarTopEdge:        outP1 = FarTopLeft; outP2 = FarTopRight; return;
            case FarRightEdge:      outP1 = FarTopRight; outP2 = FarBottomRight; return;
            case FarBottomEdge:     outP1 = FarBottomRight; outP2 = FarBottomLeft; return;

            case LeftTopEdge:       outP1 = NearTopLeft; outP2 = FarTopLeft; return;
            case LeftBottomEdge:    outP1 = FarBottomLeft; outP2 = NearBottomLeft; return;

            default:
            case RightTopEdge:      outP1 = FarTopRight; outP2 = NearTopRight; return;
            case RightBottomEdge:   outP1 = NearBottomRight; outP2 = FarBottomRight; return;
         }
      }
};

#endif // !_MBOXBASE_H_
