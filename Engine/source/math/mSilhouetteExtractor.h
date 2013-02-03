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

#ifndef _MSILHOUETTEEXTRACTOR_H_
#define _MSILHOUETTEEXTRACTOR_H_

#ifndef _FRAMEALLOCATOR_H_
#include "core/frameAllocator.h"
#endif

#ifndef _TVECTOR_H_
#include "core/util/tVector.h"
#endif


/// @file
/// Routines for extracting silhouette polygons from polyhedrons.



template< typename Polyhedron >
struct SilhouetteExtractorBase
{
      typedef Polyhedron PolyhedronType;

   protected:

      /// The polyhedron from which we are extracting silhouettes.
      const PolyhedronType* mPolyhedron;

      SilhouetteExtractorBase( const PolyhedronType& polyhedron )
         : mPolyhedron( &polyhedron ) {}
};



/// Silhouette extraction routines for perspective projections.
template< typename Polyhedron >
struct SilhouetteExtractorBasePerspective : public SilhouetteExtractorBase< Polyhedron >
{
   private:

      enum Orientation
      {
         FrontFacing,
         BackFacing
      };

      /// @name Per-Extraction Data
      /// @{

      /// The facing direction of each of the polygons.
      mutable Orientation* mPolygonOrientations;

      /// Frame allocator water mark to release temporary memory after silhouette extraction.
      mutable U32 mWaterMark;

      /// @}

   public:

      SilhouetteExtractorBasePerspective( const Polyhedron& polyhedron )
         : SilhouetteExtractorBase< Polyhedron >( polyhedron ),
           mWaterMark( 0 ),
           mPolygonOrientations( NULL ) {}

      /// Initialize extraction.
      ///
      /// @param objectView View->object matrix.
      bool begin( const MatrixF& camView ) const
      {
         mWaterMark = FrameAllocator::getWaterMark();

         // Determine orientation of each of the polygons.

         const U32 numPolygons = this->mPolyhedron->getNumPlanes();
         mPolygonOrientations = ( Orientation* ) FrameAllocator::alloc( sizeof( Orientation ) * numPolygons );

         Point3F camPos = camView.getPosition();

         for( U32 i = 0; i < numPolygons; ++ i )
         {
            if (this->mPolyhedron->getPlanes()[i].whichSide( camPos ) == PlaneF::Front)
               mPolygonOrientations[i] = FrontFacing;
            else
               mPolygonOrientations[i] = BackFacing;
         }

         return true;
      }

      /// End extraction.
      void end() const
      {
         FrameAllocator::setWaterMark( mWaterMark );

         mWaterMark = 0;
         mPolygonOrientations = NULL;
      }

      /// Return true if the given edge is a silhouette edge with respect to the
      /// current perspective transform.
      ///
      /// @param edgeIndex Index of edge to test.
      /// @return True if the given edge is a silhouette when looked at from the given view position.
      ///
      /// @note This method depends on inward-facing normals!
      bool isSilhouetteEdge( U32 edgeIndex ) const
      {
         AssertFatal( edgeIndex < this->mPolyhedron->getNumEdges(), "SilhouetteExtractorBasePerspective::isSilhouetteEdge - Index out of range!" );

         const typename Polyhedron::EdgeType& edge = this->mPolyhedron->getEdges()[ edgeIndex ];

         const U32 face0 = edge.face[ 0 ];
         const U32 face1 = edge.face[ 1 ];

         return ( mPolygonOrientations[ face0 ] != mPolygonOrientations[ face1 ] );
      }
};


/// Silhouette extraction routines for orthographic projections.
template< typename Polyhedron >
struct SilhouetteExtractorBaseOrtho : public SilhouetteExtractorBase< Polyhedron >
{
   private:

      /// @name Per-Extraction Data
      /// @{

      /// Precomputed dot products between view direction and plane normals
      /// in the polyhedron.
      mutable F32* mFaceDotProducts;

      /// Frame allocator water mark.
      mutable U32 mWaterMark;

      /// @}

   public:

      SilhouetteExtractorBaseOrtho( const Polyhedron& polyhedron )
         : SilhouetteExtractorBase< Polyhedron >( polyhedron ),
           mFaceDotProducts( NULL ),
           mWaterMark( 0 )
      {
      }

      /// Initialize the extractor.
      void begin( const Point3F& viewDirOS ) const
      {
         const typename Polyhedron::PlaneType* planes = this->mPolyhedron->getPlanes();
         const U32 numPlanes = this->mPolyhedron->getNumPlanes();

         mWaterMark = FrameAllocator::getWaterMark();
         mFaceDotProducts = ( F32* ) FrameAllocator::alloc( sizeof( F32 ) * numPlanes );

         for( U32 i = 0; i < numPlanes; ++ i )
            mFaceDotProducts[ i ] = mDot( planes[ i ], viewDirOS );
      }
      
      /// Finish extraction.
      void end() const
      {
         FrameAllocator::setWaterMark( mWaterMark );

         mFaceDotProducts = NULL;
         mWaterMark = 0;
      }

      /// Return true if the given edge is a silhouette edge with respect to the
      /// view direction.
      ///
      /// @param edgeIndex Index of edge to test.
      /// @return True if the given edge is a silhouette in the projection along the view direction.
      ///
      /// @note This method depends on inward-facing normals!
      bool isSilhouetteEdge( U32 edgeIndex ) const
      {
         AssertFatal( edgeIndex < this->mPolyhedron->getNumEdges(), "SilhouetteExtractorBaseOrtho::isSilhouetteEdge - Index out of range!" );

         const typename Polyhedron::EdgeType& edge = this->mPolyhedron->getEdges()[ edgeIndex ];

         const U32 face0 = edge.face[ 0 ];
         const U32 face1 = edge.face[ 1 ];

         // Not a silhouette if both planes are facing the same way.

         if( mSign( mFaceDotProducts[ face0 ] ) == mSign( mFaceDotProducts[ face1 ] ) )
            return false;

         // Find out which face is the front facing one.  Since we expect normals
         // to be pointing inwards, this means a reversal of the normal back facing
         // test and we're looking for a normal facing the *same* way as our projection.

         const U32 frontFace = mFaceDotProducts[ face0 ] > 0.f ? face0 : face1;
         if( mFaceDotProducts[ frontFace ] <= 0.f )
            return false; // This face or other face is perpendicular to us.

         return true;
      }
};


/// Common implementation parts for silhouette extraction.
template< typename Base >
struct SilhouetteExtractorImpl : public Base
{
      typedef typename Base::PolyhedronType PolyhedronType;

      SilhouetteExtractorImpl( const PolyhedronType& polyhedron )
         : Base( polyhedron ) {}

      U32 extractSilhouette( U32* outIndices, U32 maxOutIndices ) const
      {
         // First, find the silhouette edges.  We do this with a brute-force
         // approach here.  This can be optimized (see "Silhouette Algorithms" by Bruce Gooch, Mark
         // Hartner, and Nathan Beddes).

         U32 numSilhouetteEdges = 0;
         const U32 numTotalEdges = this->mPolyhedron->getNumEdges();
         const typename PolyhedronType::EdgeType* edges = this->mPolyhedron->getEdges();
         FrameTemp< const typename PolyhedronType::EdgeType* > silhouetteEdges( numTotalEdges );

         for( U32 i = 0; i < numTotalEdges; ++ i )
            if( this->isSilhouetteEdge( i ) )
               silhouetteEdges[ numSilhouetteEdges ++ ] = &edges[ i ];

         // Allow this to happen rather than asserting as projection-based silhouettes
         // may fail.
         if( numSilhouetteEdges < 3 )
            return 0;

         // Now walk the edge list and find the edges that are connected
         // with each other.  From this information, emit the silhouette
         // polygon.

         U32 idx = 0;

         if( idx >= maxOutIndices )
            return 0;
         outIndices[ idx ++ ] = silhouetteEdges[ 0 ]->vertex[ 1 ];

         U32 currentIndex = silhouetteEdges[ 0 ]->vertex[ 1 ];
         U32 currentEdge = 0;

         for( U32 i = 1; i < numSilhouetteEdges; ++ i )
         {
            // Find edge that continues on from the current vertex.
            for( U32 n = 0; n < numSilhouetteEdges; ++ n )
            {
               // Skip current edge.
               if( n == currentEdge )
                  continue;

               if( silhouetteEdges[ n ]->vertex[ 0 ] == currentIndex )
                  currentIndex = silhouetteEdges[ n ]->vertex[ 1 ];
               else if( silhouetteEdges[ n ]->vertex[ 1 ] == currentIndex )
                  currentIndex = silhouetteEdges[ n ]->vertex[ 0 ];
               else
                  continue;

               if( idx >= maxOutIndices )
                  return 0;

               currentEdge = n;
               outIndices[ idx ++ ] = currentIndex;
               break;
            }
         }

         return idx;
      }
};


/// Silhouette edge extraction for orthographic projections.
template< typename Polyhedron >
struct SilhouetteExtractorOrtho
{
   protected:

      typedef SilhouetteExtractorImpl< SilhouetteExtractorBaseOrtho< Polyhedron > > ExtractorType;

      /// The actual extractor implementation.
      ExtractorType mExtractor;

   public:

      SilhouetteExtractorOrtho( const Polyhedron& polyhedron )
         : mExtractor( polyhedron ) {}

      /// Generate a silhouette polygon for the polyhedron based on the given view direction.
      ///
      /// @param viewDirOS Object-space normalized view vector.
      /// @param outIndices Array where the resulting vertex indices will be stored.  Must have
      ///   enough room.  If you don't know the exact size that you need, just allocate one index
      ///   for any point in the mesh.
      /// @param maxOutIndices The number of indices that can be stored in @a outIndices.  If insufficient,
      ///   the return value will be 0.
      ///
      /// @return Number of indices written to @a outIndices or 0 if the silhouette extraction failed.
      ///
      /// @note Be aware that silhouette polygons are in most cases non-planar!
      /// @note The direction of the ordering of the resulting indices is undefined meaning that
      ///   different silhouettes extracted from the same polyhedron may have different CCW/CW ordering.
      ///   The only guarantee is that the resulting indices are consecutive.
      U32 extractSilhouette( const Point3F& viewDirOS, U32* outIndices, U32 maxOutIndices ) const
      {
         U32 result = 0;

         mExtractor.begin( viewDirOS );
         result = mExtractor.extractSilhouette( outIndices, maxOutIndices );
         mExtractor.end();

         return result;
      }
};


/// Silhouette edge extraction for perspective projections.
template< typename Polyhedron >
struct SilhouetteExtractorPerspective
{
   protected:

      typedef SilhouetteExtractorImpl< SilhouetteExtractorBasePerspective< Polyhedron > > ExtractorType;

      /// The actual extractor implementation.
      ExtractorType mExtractor;

   public:

      SilhouetteExtractorPerspective( const Polyhedron& polyhedron )
         : mExtractor( polyhedron ) {}

      /// Generate a silhouette polygon for this polyhedron based on the transforms.
      ///
      /// @param camView View->object matrix.
      /// @param outIndices Array where the resulting vertex indices will be stored.  Must have
      ///   enough room.  If you don't know the exact size that you need, just allocate one index
      ///   for any point in the mesh.
      /// @param maxOutIndices The number of indices that can be stored in @a outIndices.  If insufficient,
      ///   the return value will be 0.
      ///
      /// @return Number of indices written to @a outIndices.
      ///
      /// @note Be aware that silhouette polygons are in most cases non-planar!
      /// @note The direction of the ordering of the resulting indices is undefined meaning that
      ///   different silhouettes extracted from the same polyhedron may have different CCW/CW ordering.
      ///   The only guarantee is that the resulting indices are consecutive.
      U32 extractSilhouette( const MatrixF& camView, U32* outIndices, U32 maxOutIndices ) const
      {
         U32 result = 0;

         if( mExtractor.begin( camView ) )
            result = mExtractor.extractSilhouette( outIndices, maxOutIndices );

         mExtractor.end();

         return result;         
      }
};

#endif // !_MSILHOUETTEEXTRACTOR_H_
