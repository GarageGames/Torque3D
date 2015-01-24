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
#ifndef _SCENEPOLYHEDRALOBJECT_IMPL_H_
#define _SCENEPOLYHEDRALOBJECT_IMPL_H_

#include "platform/platform.h"
#include "scene/mixin/scenePolyhedralObject.h"

#include "console/consoleTypes.h"
#include "gfx/gfxDrawUtil.h"
#include "gfx/gfxTransformSaver.h"
#include "core/stream/bitStream.h"
#include "math/mathIO.h"

#if 0 // Enable when enabling debug rendering below.
#include "scene/sceneRenderState.h"
#include "gfx/sim/debugDraw.h"
#endif


//-----------------------------------------------------------------------------

template< typename Base, typename P >
void ScenePolyhedralObject< Base, P >::initPersistFields()
{
   Parent::addGroup( "Internal" );

      Parent::addProtectedField( "plane", TypeRealString, NULL,
         &_setPlane, &defaultProtectedGetFn,
         "For internal use only.",
         AbstractClassRep::FIELD_HideInInspectors );
      Parent::addProtectedField( "point", TypeRealString, NULL,
         &_setPoint, &defaultProtectedGetFn,
         "For internal use only.",
         AbstractClassRep::FIELD_HideInInspectors );
      Parent::addProtectedField( "edge", TypeRealString, NULL,
         &_setEdge, &defaultProtectedGetFn,
         "For internal use only.",
         AbstractClassRep::FIELD_HideInInspectors );

   Parent::endGroup( "Internal" );

   Parent::initPersistFields();
}

//-----------------------------------------------------------------------------

template< typename Base, typename P >
bool ScenePolyhedralObject< Base, P >::onAdd()
{
   // If no polyhedron has been initialized for the zone, default
   // to object box.  Do this before calling the parent's onAdd()
   // so that we set the object box correctly.

   if( mPolyhedron.getNumPlanes() == 0 )
   {
      mPolyhedron.buildBox( MatrixF::Identity, this->getObjBox() );
      mIsBox = true;
   }
   else
   {
      mIsBox = false;

      // Compute object-space bounds from polyhedron.
      this->mObjBox = mPolyhedron.getBounds();
   }
   
   if( !Parent::onAdd() )
      return false;

   return true;
}

//-----------------------------------------------------------------------------

template< typename Base, typename P >
bool ScenePolyhedralObject< Base, P >::containsPoint( const Point3F& point )
{
   // If our shape is the OBB, use the default implementation
   // inherited from SceneObject.

   if( this->mIsBox )
      return Parent::containsPoint( point );

   // Take the point into our local object space.

   Point3F p = point;
   this->getWorldTransform().mulP( p );
   p.convolveInverse( this->getScale() );

   // See if the polyhedron contains the point.

   return mPolyhedron.isContained( p );
}

//-----------------------------------------------------------------------------

template< typename Base, typename P >
void ScenePolyhedralObject< Base, P >::_renderObject( ObjectRenderInst* ri, SceneRenderState* state, BaseMatInstance* overrideMat )
{
   if( overrideMat )
      return;

   if( this->mIsBox )
      Parent::_renderObject( ri, state, overrideMat );
   else if( !this->mEditorRenderMaterial )
   {
      GFXTransformSaver saver;

      MatrixF mat = this->getRenderTransform();
      mat.scale( this->getScale() );

      GFX->multWorld( mat );

      GFXStateBlockDesc desc;
      desc.setZReadWrite( true, false );
      desc.setBlend( true );
      desc.setCullMode( GFXCullNone );

      GFX->getDrawUtil()->drawPolyhedron( desc, mPolyhedron, this->_getDefaultEditorSolidColor() );

      // Render black wireframe.

      desc.setFillModeWireframe();
      GFX->getDrawUtil()->drawPolyhedron( desc, mPolyhedron, this->_getDefaultEditorWireframeColor() );
   }
   else
   {
      //TODO: render polyhedron with material
   }

   // Debug rendering.

   #if 0
   if( state->isDiffusePass() )
      DebugDrawer::get()->drawPolyhedronDebugInfo( mPolyhedron, this->getTransform(), this->getScale() );
   #endif
}

//-----------------------------------------------------------------------------

template< typename Base, typename P >
U32 ScenePolyhedralObject< Base, P >::packUpdate( NetConnection* connection, U32 mask, BitStream* stream )
{
   U32 retMask = Parent::packUpdate( connection, mask, stream );

   if( stream->writeFlag( !mIsBox && ( mask & PolyMask ) ) )
   {
      // Write planes.

      const U32 numPlanes = mPolyhedron.getNumPlanes();
      const typename PolyhedronType::PlaneType* planes = mPolyhedron.getPlanes();

      stream->writeInt( numPlanes, 8 );
      for( U32 i = 0; i < numPlanes; ++ i )
         mathWrite( *stream, planes[ i ] );

      // Write points.

      const U32 numPoints = mPolyhedron.getNumPoints();
      const typename PolyhedronType::PointType* points = mPolyhedron.getPoints();

      stream->writeInt( numPoints, 8 );
      for( U32 i = 0; i < numPoints; ++ i )
         mathWrite( *stream, points[ i ] );

      // Write edges.

      const U32 numEdges = mPolyhedron.getNumEdges();
      const typename PolyhedronType::EdgeType* edges = mPolyhedron.getEdges();

      stream->writeInt( numEdges, 8 );
      for( U32 i = 0; i < numEdges; ++ i )
      {
         const typename PolyhedronType::EdgeType& edge = edges[ i ];

         stream->writeInt( edge.face[ 0 ], 8 );
         stream->writeInt( edge.face[ 1 ], 8 );
         stream->writeInt( edge.vertex[ 0 ], 8 );
         stream->writeInt( edge.vertex[ 1 ], 8 );
      }
   }

   return retMask;
}

//-----------------------------------------------------------------------------

template< typename Base, typename P >
void ScenePolyhedralObject< Base, P >::unpackUpdate( NetConnection* connection, BitStream* stream )
{
   Parent::unpackUpdate( connection, stream );

   if( stream->readFlag() )  // PolyMask
   {
      // Read planes.

      const U32 numPlanes = stream->readInt( 8 );
      mPolyhedron.planeList.setSize( numPlanes );

      for( U32 i = 0; i < numPlanes; ++ i )
         mathRead( *stream, &mPolyhedron.planeList[ i ] );

      // Read points.

      const U32 numPoints = stream->readInt( 8 );
      mPolyhedron.pointList.setSize( numPoints );

      for( U32 i = 0; i < numPoints; ++ i )
         mathRead( *stream, &mPolyhedron.pointList[ i ] );

      // Read edges.

      const U32 numEdges = stream->readInt( 8 );
      mPolyhedron.edgeList.setSize( numEdges );

      for( U32 i = 0; i < numEdges; ++ i )
      {
         typename PolyhedronType::EdgeType& edge = mPolyhedron.edgeList[ i ];

         edge.face[ 0 ] = stream->readInt( 8 );
         edge.face[ 1 ] = stream->readInt( 8 );
         edge.vertex[ 0 ] = stream->readInt( 8 );
         edge.vertex[ 1 ] = stream->readInt( 8 );
      }
   }
}

//-----------------------------------------------------------------------------

template< typename Base, typename P >
bool ScenePolyhedralObject< Base, P >::writeField( StringTableEntry name, const char* value )
{
   StringTableEntry sPlane = StringTable->insert( "plane" );
   StringTableEntry sPoint = StringTable->insert( "point" );
   StringTableEntry sEdge = StringTable->insert( "edge" );

   if( name == sPlane || name == sPoint || name == sEdge )
      return false;

   return Parent::writeField( name, value );
}

//-----------------------------------------------------------------------------

template< typename Base, typename P >
void ScenePolyhedralObject< Base, P >::writeFields( Stream& stream, U32 tabStop )
{
   Parent::writeFields( stream, tabStop );

   // If the polyhedron is the same as our object box,
   // don't bother writing out the planes and points.

   if( mIsBox )
      return;

   stream.write( 2, "\r\n" );

   // Write all planes.
   
   const U32 numPlanes = mPolyhedron.getNumPlanes();
   for( U32 i = 0; i < numPlanes; ++ i )
   {
      const PlaneF& plane = mPolyhedron.getPlanes()[ i ];

      stream.writeTabs( tabStop );

      char buffer[ 1024 ];
      dSprintf( buffer, sizeof( buffer ), "plane = \"%g %g %g %g\";",
         plane.x, plane.y, plane.z, plane.d
      );

      stream.writeLine( reinterpret_cast< const U8* >( buffer ) );
   }

   // Write all points.

   const U32 numPoints = mPolyhedron.getNumPoints();
   for( U32 i = 0; i < numPoints; ++ i )
   {
      const Point3F& point = mPolyhedron.getPoints()[ i ];

      stream.writeTabs( tabStop );

      char buffer[ 1024 ];
      dSprintf( buffer, sizeof( buffer ), "point = \"%g %g %g\";",
         point.x, point.y, point.z
      );

      stream.writeLine( reinterpret_cast< const U8* >( buffer ) );
   }

   // Write all edges.

   const U32 numEdges = mPolyhedron.getNumEdges();
   for( U32 i = 0; i < numEdges; ++ i )
   {
      const PolyhedronData::Edge& edge = mPolyhedron.getEdges()[ i ];

      stream.writeTabs( tabStop );

      char buffer[ 1024 ];
      dSprintf( buffer, sizeof( buffer ), "edge = \"%i %i %i %i\";",
         edge.face[ 0 ], edge.face[ 1 ],
         edge.vertex[ 0 ], edge.vertex[ 1 ]
      );

      stream.writeLine( reinterpret_cast< const U8* >( buffer ) );
   }
}

//-----------------------------------------------------------------------------

template< typename Base, typename P >
bool ScenePolyhedralObject< Base, P >::_setPlane( void* object, const char* index, const char* data )
{
   ScenePolyhedralObject* obj = reinterpret_cast< ScenePolyhedralObject* >( object );

   PlaneF plane;

   dSscanf( data, "%g %g %g %g",
      &plane.x,
      &plane.y,
      &plane.z,
      &plane.d
   );

   obj->mPolyhedron.planeList.push_back( plane );
   obj->setMaskBits( PolyMask );
   obj->mIsBox = false;

   return false;
}

//-----------------------------------------------------------------------------

template< typename Base, typename P >
bool ScenePolyhedralObject< Base, P >::_setPoint( void* object, const char* index, const char* data )
{
   ScenePolyhedralObject* obj = reinterpret_cast< ScenePolyhedralObject* >( object );

   Point3F point;

   dSscanf( data, "%g %g %g %g",
      &point[ 0 ],
      &point[ 1 ],
      &point[ 2 ]
   );

   obj->mPolyhedron.pointList.push_back( point );
   obj->setMaskBits( PolyMask );
   obj->mIsBox = false;

   return false;
}

//-----------------------------------------------------------------------------

template< typename Base, typename P >
bool ScenePolyhedralObject< Base, P >::_setEdge( void* object, const char* index, const char* data )
{
   ScenePolyhedralObject* obj = reinterpret_cast< ScenePolyhedralObject* >( object );

   PolyhedronData::Edge edge;

   dSscanf( data, "%i %i %i %i",
      &edge.face[ 0 ],
      &edge.face[ 1 ],
      &edge.vertex[ 0 ],
      &edge.vertex[ 1 ]
   );

   obj->mPolyhedron.edgeList.push_back( edge );
   obj->setMaskBits( PolyMask );
   obj->mIsBox = false;

   return false;
}

#endif // _SCENEPOLYHEDRALOBJECT_IMPL_H_
