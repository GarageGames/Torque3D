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

#include "platform/platform.h"
#include "T3D/convexShape.h"

#include "math/mathIO.h"
#include "math/mathUtils.h"
#include "scene/sceneRenderState.h"
#include "console/consoleTypes.h"
#include "core/stream/bitStream.h"
#include "materials/materialManager.h"
#include "lighting/lightQuery.h"
#include "renderInstance/renderPassManager.h"
#include "gfx/gfxTransformSaver.h"
#include "gfx/primBuilder.h"
#include "gfx/gfxDrawUtil.h"
#include "core/bitVector.h"
#include "materials/materialFeatureTypes.h"
#include "materials/baseMatInstance.h"
#include "collision/optimizedPolyList.h"
#include "T3D/physics/physicsPlugin.h"
#include "T3D/physics/physicsBody.h"
#include "T3D/physics/physicsCollision.h"
#include "console/engineAPI.h"

IMPLEMENT_CO_NETOBJECT_V1( ConvexShape );

ConsoleDocClass( ConvexShape,
   "@brief A renderable, collidable convex shape defined by a collection of surface planes.\n\n"
   
   "%ConvexShape is intended to be used as a temporary asset for quickly "
   "blocking out a scene or filling in approximate shapes to be later replaced "
   "with final assets. This is most easily done by using the WorldEditor's "
   "Sketch Tool.\n\n"
   
   "@ingroup enviroMisc"   
);


Point3F ConvexShapeCollisionConvex::support( const VectorF &vec ) const
{
	const Vector< Point3F > &pointList = pShape->mGeometry.points;

	if ( pointList.empty() )
		return pShape->getObjBox().getCenter();
	
	// This doesn't deal with the case that the farthest plane along vec is also
	// perpendicular to it, but in that case maybe it doesn't matter which point we return
	// anyway.

	F32 bestDot = mDot( pointList[0], vec );

	const Point3F *bestP = &pointList[0];

	for ( S32 i = 1; i < pointList.size(); i++ )
	{
		F32 newD = mDot( pointList[i], vec );

		if ( newD > bestDot )
		{
			bestDot = newD;
			bestP = &pointList[i];
		}
	}

	return *bestP;
}

void ConvexShapeCollisionConvex::getFeatures( const MatrixF &mat, const VectorF &n, ConvexFeature *cf )
{
	if ( pShape->mGeometry.points.empty() )
	{
		cf->material = 0;
		cf->object = NULL;
		return;
	}

	cf->material = 0;
	cf->object = mObject;

	// Simple implementation... Add all Points, Edges and Faces.


	// Points...

	S32 firstVert = cf->mVertexList.size();
	
	const Vector< Point3F > &pointList = pShape->mGeometry.points;
	const U32 pointListCount = pointList.size();

	cf->mVertexList.increment( pointListCount );

	for ( S32 i = 0; i < pointListCount; i++ )	
		mat.mulP( pointList[i], &(cf->mVertexList[ firstVert + i ]) );	
	

	// Edges and Triangles for each face...

	const Vector< ConvexShape::Face > &faceList = pShape->mGeometry.faces;

	for ( S32 i = 0; i < faceList.size(); i++ )
	{
		// Add this Face's Edges.

		const Vector< ConvexShape::Edge > &edgeList = faceList[i].edges;		
		const U32 edgeCount = edgeList.size();
		const S32 firstEdge = cf->mEdgeList.size();

		cf->mEdgeList.increment( edgeCount );

		for ( S32 j = 0; j < edgeCount; j++ )
		{
			cf->mEdgeList[ firstEdge + j ].vertex[0] = faceList[i].points[ edgeList[j].p0 ];
			cf->mEdgeList[ firstEdge + j ].vertex[1] = faceList[i].points[ edgeList[j].p1 ];
		}

		// Add this face's Triangles. 

		// Note that ConvexFeature calls triangles 'faces' but a ConvexShape 'Face' is not
		// necessarily a single triangle.
				
		const Vector< ConvexShape::Triangle > &triangleList = faceList[i].triangles;
		const U32 triangleCount = triangleList.size();
		S32 firstTriangle = cf->mFaceList.size();

		cf->mFaceList.increment( triangleCount );

		for ( S32 j = 0; j < triangleCount; j++ )
		{			
			ConvexFeature::Face &cft = cf->mFaceList[ firstTriangle + j ];

			cft.normal = faceList[i].normal;
			cft.vertex[0] = triangleList[j].p0;
			cft.vertex[1] = triangleList[j].p1;
			cft.vertex[2] = triangleList[j].p2;
		}
	}	
}


void ConvexShapeCollisionConvex::getPolyList( AbstractPolyList* list )
{
	SphereF sphere( Point3F::Zero, 0.0f );
	
	pShape->buildPolyList( PLC_Collision, list, Box3F::Invalid, sphere );	
}


GFXImplementVertexFormat( ConvexVert )
{
   addElement( "POSITION", GFXDeclType_Float3 );
   addElement( "COLOR", GFXDeclType_Color );
   addElement( "NORMAL", GFXDeclType_Float3 );
   addElement( "TANGENT", GFXDeclType_Float3 );
   addElement( "TEXCOORD", GFXDeclType_Float2, 0 );
};

static const U32 sgConvexFaceColorCount = 16;
static const ColorI sgConvexFaceColors[ sgConvexFaceColorCount ] = 
{
   ColorI( 239, 131, 201 ),
   ColorI( 124, 255, 69 ),
   ColorI( 255, 65, 77 ),
   ColorI( 33, 118, 235 ),
   ColorI( 114, 227, 110 ),
   ColorI( 197, 50, 237 ),
   ColorI( 236, 255, 255 ),
   ColorI( 139, 225, 192 ),
   ColorI( 215, 9, 65 ),
   ColorI( 249, 114, 93 ),
   ColorI( 255, 255, 90 ),
   ColorI( 93, 104, 97 ),
   ColorI( 255, 214, 192 ),
   ColorI( 122, 44, 198 ),
   ColorI( 137, 141, 194 ),
   ColorI( 164, 114, 43 )
};

bool ConvexShape::smRenderEdges = false;

bool ConvexShape::protectedSetSurface( void *object, const char *index, const char *data )
{
   ConvexShape *shape = static_cast< ConvexShape* >( object );

   QuatF quat;
	Point3F pos;
	//MatrixF mat;

	/*
   dSscanf( data, "%g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g", 
      &mat[0], &mat[1], &mat[2], &mat[3], 
      &mat[4], &mat[5], &mat[6], &mat[7], 
      &mat[8], &mat[9], &mat[10], &mat[11],
      &mat[12], &mat[13], &mat[14], &mat[15] );
	*/

	dSscanf( data, "%g %g %g %g %g %g %g", &quat.x, &quat.y, &quat.z, &quat.w, &pos.x, &pos.y, &pos.z );

	MatrixF surface;
	quat.setMatrix( &surface );
	surface.setPosition( pos );

   shape->mSurfaces.push_back( surface );   

   return false;
}


ConvexShape::ConvexShape()
 : mMaterialName( "Grid512_OrangeLines_Mat" ),
   mMaterialInst( NULL ),
   mVertCount( 0 ),
   mPrimCount( 0 ),
   mPhysicsRep( NULL ),
   mNormalLength( 0.3f )
{   
   mNetFlags.set( Ghostable | ScopeAlways );
   
   mTypeMask |= StaticObjectType | 
                StaticShapeObjectType;    

   mConvexList = new Convex;
}

ConvexShape::~ConvexShape()
{
   if ( mMaterialInst )
      SAFE_DELETE( mMaterialInst );

   delete mConvexList;
   mConvexList = NULL;
}

void ConvexShape::initPersistFields()
{
   addGroup( "Rendering" );

      addField( "material", TypeMaterialName, Offset( mMaterialName, ConvexShape ), "Material used to render the ConvexShape surface." );

   endGroup( "Rendering" );

   addGroup( "Internal" );

      addProtectedField( "surface", TypeRealString, NULL, &protectedSetSurface, &defaultProtectedGetFn, 
         "Do not modify, for internal use.", AbstractClassRep::FIELD_HideInInspectors );

   endGroup( "Internal" );

   Parent::initPersistFields();
}

void ConvexShape::inspectPostApply()
{
   Parent::inspectPostApply();

   _updateGeometry( true );

   setMaskBits( UpdateMask );
}

bool ConvexShape::onAdd()
{
   if ( !Parent::onAdd() )
      return false;
   
   //mObjBox.set( -0.5f, -0.5f, -0.5f, 0.5f, 0.5f, 0.5f );
   //resetWorldBox();   

   // Face Order:
   // Top, Bottom, Front, Back, Left, Right

   // X Axis
   static const Point3F cubeTangents[6] =
   {
      Point3F( 1,  0,  0 ),
      Point3F(-1,  0,  0 ),
      Point3F( 1,  0,  0 ),
      Point3F(-1,  0,  0 ),
      Point3F( 0,  1,  0 ),
      Point3F( 0, -1,  0 )      
   };

   // Y Axis
   static const Point3F cubeBinormals[6] =
   {
      Point3F( 0,  1,  0 ),
      Point3F( 0,  1,  0 ),
      Point3F( 0,  0, -1 ),
      Point3F( 0,  0, -1 ),
      Point3F( 0,  0, -1 ),
      Point3F( 0,  0, -1 )
   };

   // Z Axis
   static const Point3F cubeNormals[6] = 
   {
      Point3F( 0,  0,  1),
      Point3F( 0,  0, -1),
      Point3F( 0,  1,  0),
      Point3F( 0, -1,  0),
      Point3F(-1,  0,  0),
      Point3F( 1,  0,  0),      
   };

   if ( mSurfaces.empty() )
   {      
      for ( S32 i = 0; i < 6; i++ )
      {
         mSurfaces.increment();
         MatrixF &surf = mSurfaces.last();

         surf.identity();
         
         surf.setColumn( 0, cubeTangents[i] );
         surf.setColumn( 1, cubeBinormals[i] );
         surf.setColumn( 2, cubeNormals[i] );
         surf.setPosition( cubeNormals[i] * 0.5f );         
      }
   }

   if ( isClientObject() )   
      _updateMaterial();      
   
   _updateGeometry( true );

   addToScene();

   return true;
}

void ConvexShape::onRemove()
{
   removeFromScene();

   mConvexList->nukeList();

   SAFE_DELETE( mPhysicsRep );

   Parent::onRemove();
}

void ConvexShape::writeFields( Stream &stream, U32 tabStop )
{
   Parent::writeFields( stream, tabStop );

   // Now write all planes.

   stream.write(2, "\r\n");   

   S32 count = mSurfaces.size();
   if ( count > smMaxSurfaces )
   {
       Con::errorf( "ConvexShape has too many surfaces to save! Truncated value %d to maximum value of %d", count, smMaxSurfaces );
       count = smMaxSurfaces;
   }

   for ( U32 i = 0; i < count; i++ )
   {      
      const MatrixF &mat = mSurfaces[i];

		QuatF quat( mat );
		Point3F pos( mat.getPosition() );

      stream.writeTabs(tabStop);

      char buffer[1024];
      dMemset( buffer, 0, 1024 );      
      
      dSprintf( buffer, 1024, "surface = \"%g %g %g %g %g %g %g\";", 
         quat.x, quat.y, quat.z, quat.w, pos.x, pos.y, pos.z );      

      stream.writeLine( (const U8*)buffer );
   }
}

bool ConvexShape::writeField( StringTableEntry fieldname, const char *value )
{   
   if ( fieldname == StringTable->insert("surface") )
      return false;

   return Parent::writeField( fieldname, value );
}

void ConvexShape::onScaleChanged()
{
   if ( isProperlyAdded() )
      _updateCollision();
}

void ConvexShape::setTransform( const MatrixF &mat )
{   
   Parent::setTransform( mat );

   if ( mPhysicsRep )
      mPhysicsRep->setTransform( mat );
 
   setMaskBits( TransformMask );
}

U32 ConvexShape::packUpdate( NetConnection *conn, U32 mask, BitStream *stream )
{
   U32 retMask = Parent::packUpdate( conn, mask, stream );
   
   if ( stream->writeFlag( mask & TransformMask ) )
   {
      mathWrite(*stream, getTransform());
      mathWrite(*stream, getScale());
   }

   if ( stream->writeFlag( mask & UpdateMask ) )
   {
      stream->write( mMaterialName );
      
      U32 surfCount = mSurfaces.size();
      stream->writeInt( surfCount, 32 );

      for ( S32 i = 0; i < surfCount; i++ )    
      {
         QuatF quat( mSurfaces[i] );
		 Point3F pos( mSurfaces[i].getPosition() );

         mathWrite( *stream, quat );
         mathWrite( *stream, pos );                    
      }
   }

   return retMask;
}

void ConvexShape::unpackUpdate( NetConnection *conn, BitStream *stream )
{
   Parent::unpackUpdate( conn, stream );

   if ( stream->readFlag() )  // TransformMask
   {
      mathRead(*stream, &mObjToWorld);
      mathRead(*stream, &mObjScale);

      setTransform( mObjToWorld );
      setScale( mObjScale );
   }

   if ( stream->readFlag() ) // UpdateMask
   {
      stream->read( &mMaterialName );      

      if ( isProperlyAdded() )
         _updateMaterial();

      mSurfaces.clear();

      const U32 surfCount = stream->readInt( 32 );
      for ( S32 i = 0; i < surfCount; i++ )
      {
         mSurfaces.increment();
         MatrixF &mat = mSurfaces.last();

         QuatF quat;
         Point3F pos;

         mathRead( *stream, &quat );
         mathRead( *stream, &pos ); 

         quat.setMatrix( &mat );
         mat.setPosition( pos );
      }

      if ( isProperlyAdded() )
         _updateGeometry( true );
   }
}

void ConvexShape::prepRenderImage( SceneRenderState *state )
{   
   /*
   if ( state->isDiffusePass() )
   {
      ObjectRenderInst *ri2 = state->getRenderPass()->allocInst<ObjectRenderInst>();
      ri2->renderDelegate.bind( this, &ConvexShape::_renderDebug );
      ri2->type = RenderPassManager::RIT_Editor;
      state->getRenderPass()->addInst( ri2 );
   }
   */

   if ( mVertexBuffer.isNull() || !state)
      return;

   // If we don't have a material instance after the override then 
   // we can skip rendering all together.
   BaseMatInstance *matInst = state->getOverrideMaterial( mMaterialInst ? mMaterialInst : MATMGR->getWarningMatInstance() );
   if ( !matInst )
      return;

   // Get a handy pointer to our RenderPassmanager
   RenderPassManager *renderPass = state->getRenderPass();

   // Allocate an MeshRenderInst so that we can submit it to the RenderPassManager
   MeshRenderInst *ri = renderPass->allocInst<MeshRenderInst>();

   // Set our RenderInst as a standard mesh render
   ri->type = RenderPassManager::RIT_Mesh;

   // Calculate our sorting point
   if ( state )
   {
      // Calculate our sort point manually.
      const Box3F& rBox = getRenderWorldBox();
      ri->sortDistSq = rBox.getSqDistanceToPoint( state->getCameraPosition() );      
   } 
   else 
      ri->sortDistSq = 0.0f;

   // Set up our transforms
   MatrixF objectToWorld = getRenderTransform();
   objectToWorld.scale( getScale() );

   ri->objectToWorld = renderPass->allocUniqueXform( objectToWorld );
   ri->worldToCamera = renderPass->allocSharedXform(RenderPassManager::View);
   ri->projection    = renderPass->allocSharedXform(RenderPassManager::Projection);

	// If we need lights then set them up.
   if ( matInst->isForwardLit() )
   {
      LightQuery query;
      query.init( getWorldSphere() );
		query.getLights( ri->lights, 8 );
   }

   // Make sure we have an up-to-date backbuffer in case
   // our Material would like to make use of it
   // NOTICE: SFXBB is removed and refraction is disabled!
   //ri->backBuffTex = GFX->getSfxBackBuffer();

   // Set our Material
   ri->matInst = matInst;
   if ( matInst->getMaterial()->isTranslucent() )
   {
      ri->translucentSort = true;
      ri->type = RenderPassManager::RIT_Translucent;
   }

   // Set up our vertex buffer and primitive buffer
   ri->vertBuff = &mVertexBuffer;
   ri->primBuff = &mPrimitiveBuffer;

   ri->prim = renderPass->allocPrim();
   ri->prim->type = GFXTriangleList;
   ri->prim->minIndex = 0;
   ri->prim->startIndex = 0;
   ri->prim->numPrimitives = mPrimCount;
   ri->prim->startVertex = 0;
   ri->prim->numVertices = mVertCount;

   // We sort by the material then vertex buffer.
   ri->defaultKey = matInst->getStateHint();
   ri->defaultKey2 = (uintptr_t)ri->vertBuff; // Not 64bit safe!

   // Submit our RenderInst to the RenderPassManager
   state->getRenderPass()->addInst( ri );
}

void ConvexShape::buildConvex( const Box3F &box, Convex *convex )
{
   if ( mGeometry.faces.empty() )
      return;

   mConvexList->collectGarbage();

   Box3F realBox = box;
   mWorldToObj.mul( realBox );
   realBox.minExtents.convolveInverse( mObjScale );
   realBox.maxExtents.convolveInverse( mObjScale );

   if ( realBox.isOverlapped( getObjBox() ) == false )
      return;

   // See if this convex exists in the working set already...
   Convex *cc = 0;
   CollisionWorkingList &wl = convex->getWorkingList();
   for ( CollisionWorkingList* itr = wl.wLink.mNext; itr != &wl; itr = itr->wLink.mNext ) 
   {
      if ( itr->mConvex->getType() == ConvexShapeCollisionConvexType )
      {
         ConvexShapeCollisionConvex *pConvex = static_cast<ConvexShapeCollisionConvex*>(itr->mConvex);

         if ( pConvex->pShape == this )              
         {
            cc = itr->mConvex;
            return;
         }
      }
   }

   // Set up the convex...

   ConvexShapeCollisionConvex *cp = new ConvexShapeCollisionConvex();

   mConvexList->registerObject( cp );
   convex->addToWorkingList( cp );

   cp->mObject = this;
   cp->pShape  = this;   
}

bool ConvexShape::buildPolyList( PolyListContext context, AbstractPolyList *plist, const Box3F &box, const SphereF &sphere )
{
   if ( mGeometry.points.empty() )	
      return false;

   // If we're exporting deal with that first.
   if ( context == PLC_Export )
   {
      AssertFatal( dynamic_cast<OptimizedPolyList*>( plist ), "ConvexShape::buildPolyList - Bad polylist for export!" );
      _export( (OptimizedPolyList*)plist, box, sphere );
      return true;
   }

   plist->setTransform( &mObjToWorld, mObjScale );
   plist->setObject( this );


   // Add points...

   const Vector< Point3F > pointList = mGeometry.points;

   S32 base = plist->addPoint( pointList[0] );

   for ( S32 i = 1; i < pointList.size(); i++ )	
      plist->addPoint( pointList[i] );


   // Add Surfaces...

   const Vector< ConvexShape::Face > faceList = mGeometry.faces;

   if(context == PLC_Navigation)
   {
      for(S32 i = 0; i < faceList.size(); i++)
      {
         const ConvexShape::Face &face = faceList[i];

         S32 s = face.triangles.size();
         for(S32 j = 0; j < s; j++)
         {
            plist->begin(0, s*i + j);

            plist->plane(PlaneF(face.centroid, face.normal));

            plist->vertex(base + face.points[face.triangles[j].p0]);
            plist->vertex(base + face.points[face.triangles[j].p1]);
            plist->vertex(base + face.points[face.triangles[j].p2]);

            plist->end();
         }
      }
      return true;
   }

   for ( S32 i = 0; i < faceList.size(); i++ )
   {
      const ConvexShape::Face &face = faceList[i];		

      plist->begin( 0, i );

      plist->plane( PlaneF( face.centroid, face.normal ) );

      for ( S32 j = 0; j < face.triangles.size(); j++ )
      {
         plist->vertex( base + face.points[ face.triangles[j].p0 ] );
         plist->vertex( base + face.points[ face.triangles[j].p1 ] );
         plist->vertex( base + face.points[ face.triangles[j].p2 ] );
      }      

      plist->end();
   }

   return true;
}

void ConvexShape::_export( OptimizedPolyList *plist, const Box3F &box, const SphereF &sphere )
{
   BaseMatInstance *matInst = mMaterialInst;
   if ( isServerObject() && getClientObject() )
      matInst = dynamic_cast<ConvexShape*>(getClientObject())->mMaterialInst;
   
   MatrixF saveMat;
   Point3F saveScale;
   plist->getTransform( &saveMat, &saveScale );

   plist->setTransform( &mObjToWorld, mObjScale );
   plist->setObject( this );   

   const Vector< ConvexShape::Face > faceList = mGeometry.faces;
   const Vector< Point3F > &pointList = mGeometry.points;

   for ( S32 i = 0; i < faceList.size(); i++ )
   {
      const ConvexShape::Face &face = faceList[i];		

      plist->begin( matInst, i, OptimizedPolyList::TriangleList );

      plist->plane( PlaneF( face.centroid, face.normal ) );

      for ( S32 j = 0; j < face.triangles.size(); j++ )
      {                  
         for ( S32 k = 0; k < 3; k++ )         
         {
            U32 vertId = face.triangles[j][k];
            plist->vertex( pointList[ face.points[ vertId ] ], face.normal, face.texcoords[ vertId ] );         
         }
      }      

      plist->end();
   }

   plist->setTransform( &saveMat, saveScale );
}

bool ConvexShape::castRay( const Point3F &start, const Point3F &end, RayInfo *info )
{
   if ( mPlanes.empty() )
      return false;   

   const Vector< PlaneF > &planeList = mPlanes;
   const U32 planeCount = planeList.size();  

   F32 t;
   F32 tmin = F32_MAX;
   S32 hitFace = -1;
   Point3F hitPnt, pnt;
   VectorF rayDir( end - start );
   rayDir.normalizeSafe();

   if ( false )
   {
      PlaneF plane( Point3F(0,0,0), Point3F(0,0,1) );
      Point3F sp( 0,0,-1 );
      Point3F ep( 0,0,1 );

      F32 t = plane.intersect( sp, ep );
      Point3F hitPnt;
      hitPnt.interpolate( sp, ep, t );
   }

   for ( S32 i = 0; i < planeCount; i++ )
   {
      // Don't hit the back-side of planes.
      if ( mDot( rayDir, planeList[i] ) >= 0.0f )
         continue;

      t = planeList[i].intersect( start, end );

      if ( t >= 0.0f && t <= 1.0f && t < tmin )
      {
         pnt.interpolate( start, end, t );

         S32 j = 0;
         for ( ; j < planeCount; j++ )
         {
            if ( i == j )
               continue;

            F32 dist = planeList[j].distToPlane( pnt );
            if ( dist > 1.0e-004f )
               break;
         }

         if ( j == planeCount )
         {
            tmin = t;
            hitFace = i;
         }
      }
   }

   if ( hitFace == -1 )
      return false;

   info->face = hitFace;            
   info->material = mMaterialInst;
   info->normal = planeList[ hitFace ];
   info->object = this;
   info->t = tmin;

   //mObjToWorld.mulV( info->normal );

   return true;
}

bool ConvexShape::collideBox( const Point3F &start, const Point3F &end, RayInfo *info )
{
   return Parent::collideBox( start, end, info );
}

void ConvexShape::updateBounds( bool recenter )
{
   if ( mGeometry.points.size() == 0 )
      return;

   Vector<Point3F> &pointListOS = mGeometry.points;
   U32 pointCount = pointListOS.size();

   Point3F volumnCenter( 0,0,0 );
   F32 areaSum = 0.0f;

   F32 faceCount = mGeometry.faces.size();

   for ( S32 i = 0; i < faceCount; i++ )   
   {
      volumnCenter += mGeometry.faces[i].centroid * mGeometry.faces[i].area;         
      areaSum += mGeometry.faces[i].area;
   }

   if ( areaSum == 0.0f )
      return;

   volumnCenter /= areaSum;
   
   mObjBox.minExtents = mObjBox.maxExtents = Point3F::Zero;
   mObjBox.setCenter( volumnCenter );

   for ( S32 i = 0; i < pointCount; i++ )      
      mObjBox.extend( pointListOS[i] );

   resetWorldBox();
}

void ConvexShape::recenter()
{
   if ( mGeometry.points.size() == 0 )
      return;
  
   Point3F volCenterOS( 0,0,0 );
   F32 areaSum = 0.0f;

   F32 faceCount = mGeometry.faces.size();

   for ( S32 i = 0; i < faceCount; i++ )   
   {
      volCenterOS += mGeometry.faces[i].centroid * mGeometry.faces[i].area;         
      areaSum += mGeometry.faces[i].area;
   }

   volCenterOS /= areaSum;

   for ( S32 i = 0; i < mSurfaces.size(); i++ )   
      mSurfaces[i].setPosition( mSurfaces[i].getPosition() - volCenterOS );
   
   Point3F volCenterWS;
   MatrixF objToWorld( mObjToWorld );
   objToWorld.scale( mObjScale );
   objToWorld.mulP( volCenterOS, &volCenterWS );

   setPosition( volCenterWS );

   _updateGeometry(true);   
}

MatrixF ConvexShape::getSurfaceWorldMat( S32 surfId, bool scaled ) const
{
   if ( surfId < 0 || surfId >= mSurfaces.size() )
      return MatrixF::Identity;      

   MatrixF objToWorld( mObjToWorld );

   if ( scaled )
      objToWorld.scale( mObjScale );

   MatrixF surfMat;
   surfMat.mul( objToWorld, mSurfaces[surfId] );   

   return surfMat;
}

// Used in cullEmptyPlanes.
S32 QSORT_CALLBACK sortDescendingU32( const void *a, const void *b )
{
   U32 *aa = (U32*)(a);
   U32 *bb = (U32*)(b);

   return (S32)(*bb) - (S32)(*aa);
}

void ConvexShape::cullEmptyPlanes( Vector< U32 > *removedPlanes )
{
   //if ( mPlanes.size() == mGeometry.faces.size() )
   //   return;

   removedPlanes->clear();
   const U32 startPlaneCount = mPlanes.size();

   const Vector< ConvexShape::Face > &faceList = mGeometry.faces;
   const U32 faceCount = faceList.size();

   S32 *used = new S32[ startPlaneCount ];

   for ( S32 i = 0; i < startPlaneCount; i++ )   
      used[i] = i;

   for ( S32 i = 0; i < faceCount; i++ )
   {
      if ( faceList[i].area > 0.001f )
         used[ faceList[i].id ] = -1;
   }

   for ( S32 i = 0; i < startPlaneCount; i++ )
   {
      if ( used[i] != -1 )      
         removedPlanes->push_back( used[i] );
   }
   
   dQsort( removedPlanes->address(), removedPlanes->size(), sizeof( U32 ), sortDescendingU32 );

   for ( S32 i = 0; i < removedPlanes->size(); i++ )
   {
      mPlanes.erase( (*removedPlanes)[i] );
      mSurfaces.erase( (*removedPlanes)[i] );      
   }

   delete [] used;
}

void ConvexShape::exportToCollada()
{
	if ( mSurfaces.size() == 0 )
	{
		Con::errorf( "ConvexShape::exportToCollada() - has no surfaces to export!" );
		return;
	}
}

void ConvexShape::resizePlanes( const Point3F &size )
{
   //Point3F nSize;
   //mWorldToObj.mulV( nSize );

   for ( S32 i = 0; i < mSurfaces.size(); i++ )
   {
      MatrixF objToPlane( mSurfaces[i] );
      objToPlane.inverse();

      Point3F lim;
      objToPlane.mulV( size, &lim );

      F32 sign = ( mPlanes[i].d > 0.0f ) ? 1.0f : -1.0f;
      mPlanes[i].d = mFabs(lim.z) * 0.5f * sign;
      
      //mPlanes[i].d = -lim.z * 0.5f;      

      mSurfaces[i].setPosition( mPlanes[i].getPosition() );
   }   
}

void ConvexShape::getSurfaceLineList( S32 surfId, Vector< Point3F > &lineList )
{
   if ( surfId < 0 || surfId > mSurfaces.size() - 1 )
      return;

   S32 faceId = -1;

   for ( S32 i = 0; i < mGeometry.faces.size(); i++ )
   {
      if ( mGeometry.faces[i].id == surfId )
      {
         faceId = i;
         break;
      }
   }

   if ( faceId == -1 )
      return;

   ConvexShape::Face &face = mGeometry.faces[faceId];
   const Vector< Point3F > &pointList = mGeometry.points;

   if ( pointList.size() == 0 )
      return;

   for ( S32 i = 0; i < face.winding.size(); i++ )   
      lineList.push_back( pointList[ face.points[ face.winding[i] ] ] );
   
   lineList.push_back( pointList[ face.points[ face.winding.first() ] ] );
}

void ConvexShape::_updateMaterial()
{   
   // If the material name matches then don't bother updating it.
   if ( mMaterialInst && mMaterialName.equal( mMaterialInst->getMaterial()->getName(), String::NoCase ) )
      return;

   SAFE_DELETE( mMaterialInst );

   Material *material;
   
   if ( !Sim::findObject( mMaterialName, material ) )
      Sim::findObject( "WarningMaterial", material );

   mMaterialInst = material->createMatInstance();

   //GFXStateBlockDesc desc;
   //desc.setCullMode( GFXCullNone );
   //desc.setBlend( false );

   //mMaterialInst->addStateBlockDesc( desc );

   FeatureSet features = MATMGR->getDefaultFeatures();
   //features.addFeature( MFT_DiffuseVertColor );

   mMaterialInst->init( features, getGFXVertexFormat<VertexType>() );

   if ( !mMaterialInst->isValid() )
   {
      SAFE_DELETE( mMaterialInst );
   }
}

void ConvexShape::_updateGeometry( bool updateCollision )
{
   mPlanes.clear();

   for ( S32 i = 0; i < mSurfaces.size(); i++ )   
      mPlanes.push_back( PlaneF( mSurfaces[i].getPosition(), mSurfaces[i].getUpVector() ) );

	Vector< Point3F > tangents;
	for ( S32 i = 0; i < mSurfaces.size(); i++ )
		tangents.push_back( mSurfaces[i].getRightVector() );
   
   mGeometry.generate( mPlanes, tangents );

   AssertFatal( mGeometry.faces.size() <= mSurfaces.size(), "Got more faces than planes?" );

   const Vector< ConvexShape::Face > &faceList = mGeometry.faces;
   const Vector< Point3F > &pointList = mGeometry.points;

   // Reset our surface center points.

   for ( S32 i = 0; i < faceList.size(); i++ )
		mSurfaces[ faceList[i].id ].setPosition( faceList[i].centroid );

   mPlanes.clear();

   for ( S32 i = 0; i < mSurfaces.size(); i++ )   
      mPlanes.push_back( PlaneF( mSurfaces[i].getPosition(), mSurfaces[i].getUpVector() ) );

   // Update bounding box.   
   updateBounds( false );

	mVertexBuffer = NULL;
	mPrimitiveBuffer = NULL;
	mVertCount = 0;
	mPrimCount = 0;

   if ( updateCollision )
      _updateCollision();

   // Server does not need to generate vertex/prim buffers.
   if ( isServerObject() )
      return;

   if ( faceList.empty() )   
      return;


	// Get total vert and prim count.

	for ( S32 i = 0; i < faceList.size(); i++ )	
	{
		U32 count = faceList[i].triangles.size();
		mPrimCount += count;
		mVertCount += count * 3;		
	}

	// Allocate VB and copy in data.

	mVertexBuffer.set( GFX, mVertCount, GFXBufferTypeStatic );
	VertexType *pVert = mVertexBuffer.lock();

	for ( S32 i = 0; i < faceList.size(); i++ )
	{
		const ConvexShape::Face &face = faceList[i];
		const Vector< U32 > &facePntMap = face.points;
		const Vector< ConvexShape::Triangle > &triangles = face.triangles;
		const ColorI &faceColor = sgConvexFaceColors[ i % sgConvexFaceColorCount ];

		for ( S32 j = 0; j < triangles.size(); j++ )
		{
			for ( S32 k = 0; k < 3; k++ )
			{
				pVert->normal = face.normal;
				pVert->tangent = face.tangent;
				pVert->color = faceColor;			
				pVert->point = pointList[ facePntMap[ triangles[j][k] ] ];
				pVert->texCoord = face.texcoords[ triangles[j][k] ];

				pVert++;
			}
		}		
	}	

	mVertexBuffer.unlock();

	// Allocate PB

   mPrimitiveBuffer.set( GFX, mPrimCount * 3, mPrimCount, GFXBufferTypeStatic );

   U16 *pIndex;
   mPrimitiveBuffer.lock( &pIndex );

   for ( U16 i = 0; i < mPrimCount * 3; i++ )
   {
      *pIndex = i;
      pIndex++;
   }

   mPrimitiveBuffer.unlock();
}

void ConvexShape::_updateCollision()
{
   SAFE_DELETE( mPhysicsRep );

   if ( !PHYSICSMGR )
      return;

   PhysicsCollision *colShape = PHYSICSMGR->createCollision();

   // We need the points untransformed!
   Vector<Point3F> rawPoints;
   MatrixF xfm( getWorldTransform() );
   xfm.setPosition( Point3F::Zero );
   for ( U32 i=0; i < mGeometry.points.size(); i++ )
   {
      Point3F p = mGeometry.points[i];
      xfm.mulP( p );
      rawPoints.push_back( p );
   }

   // The convex generation from a point cloud 
   // can fail at times... give up in that case.
   if ( !colShape->addConvex(    mGeometry.points.address(), 
                                 mGeometry.points.size(), 
                                 MatrixF::Identity ) )
   {
      delete colShape;
      return;
   }

   PhysicsWorld *world = PHYSICSMGR->getWorld( isServerObject() ? "server" : "client" );

   mPhysicsRep = PHYSICSMGR->createBody();
   mPhysicsRep->init( colShape, 0, 0, this, world );

   mPhysicsRep->setTransform( getTransform() );
}

void ConvexShape::_renderDebug( ObjectRenderInst *ri, SceneRenderState *state, BaseMatInstance *mat )
{   
   GFXDrawUtil *drawer = GFX->getDrawUtil();

   GFX->setTexture( 0, NULL );

   // Render world box.
   if ( false )
   {
      Box3F wbox( mWorldBox );
      //if ( getServerObject() )      
      //   Box3F wbox = static_cast<ConvexShape*>( getServerObject() )->mWorldBox;      
      GFXStateBlockDesc desc;
      desc.setCullMode( GFXCullNone );
      desc.setFillModeWireframe();
      drawer->drawCube( desc, wbox, ColorI::RED );
   }


   const Vector< Point3F > &pointList = mGeometry.points;
	const Vector< ConvexShape::Face > &faceList = mGeometry.faces;

   // Render Edges.
   if ( false )
   {
      GFXTransformSaver saver;
      //GFXFrustumSaver fsaver;

      MatrixF xfm( getRenderTransform() );
      xfm.scale( getScale() );
      GFX->multWorld( xfm );

      GFXStateBlockDesc desc;
      desc.setZReadWrite( true, false );
      desc.setBlend( true );
      GFX->setStateBlockByDesc( desc );

      //MathUtils::getZBiasProjectionMatrix( 0.01f, state->getFrustum(), )

      const Point3F &camFvec = state->getCameraTransform().getForwardVector();



      for ( S32 i = 0; i < faceList.size(); i++ )
      {         
         const ConvexShape::Face &face = faceList[i];
         
         const Vector< ConvexShape::Edge > &edgeList = face.edges;

         const Vector< U32 > &facePntList = face.points;

         PrimBuild::begin( GFXLineList, edgeList.size() * 2 );
         
         PrimBuild::color( ColorI::WHITE * 0.8f );

         for ( S32 j = 0; j < edgeList.size(); j++ )         
         {
            PrimBuild::vertex3fv( pointList[ facePntList[ edgeList[j].p0 ] ] - camFvec * 0.001f );
            PrimBuild::vertex3fv( pointList[ facePntList[ edgeList[j].p1 ] ] - camFvec * 0.001f );
         }
         
         PrimBuild::end();
      }
   }

   ColorI faceColorsx[4] = 
   {
      ColorI( 255, 0, 0 ),
      ColorI( 0, 255, 0 ),
      ColorI( 0, 0, 255 ),
      ColorI( 255, 0, 255 )
   };

   MatrixF objToWorld( mObjToWorld );
   objToWorld.scale( mObjScale );

   // Render faces centers/colors.
   if ( false )
   {
      GFXStateBlockDesc desc;
      desc.setCullMode( GFXCullNone );
      
      Point3F size( 0.1f );

      for ( S32 i = 0; i < faceList.size(); i++ )
      {
         ColorI color = faceColorsx[ i % 4 ];
         S32 div = ( i / 4 ) * 4;
         if ( div > 0 )
            color /= div;
         color.alpha = 255;
         
         Point3F pnt;
         objToWorld.mulP( faceList[i].centroid, &pnt );
         drawer->drawCube( desc, size, pnt, color, NULL );
      }
   }

   // Render winding order.
   if ( false )
   {
      GFXStateBlockDesc desc;
      desc.setCullMode( GFXCullNone );
      desc.setZReadWrite( true, false );
      GFX->setStateBlockByDesc( desc );  

      U32 pointCount = 0;
      for ( S32 i = 0; i < faceList.size(); i++ )      
         pointCount += faceList[i].winding.size();      

      PrimBuild::begin( GFXLineList, pointCount * 2 );
      
      for ( S32 i = 0; i < faceList.size(); i++ )
      {
         for ( S32 j = 0; j < faceList[i].winding.size(); j++ )
         {
            Point3F p0 = pointList[ faceList[i].points[ faceList[i].winding[j] ] ];
            Point3F p1 = p0 + mSurfaces[ faceList[i].id ].getUpVector() * 0.75f * ( Point3F::One / mObjScale );

            objToWorld.mulP( p0 );
            objToWorld.mulP( p1 );

            ColorI color = faceColorsx[ j % 4 ];
            S32 div = ( j / 4 ) * 4;
            if ( div > 0 )
               color /= div;
            color.alpha = 255;
            
            PrimBuild::color( color );
            PrimBuild::vertex3fv( p0 );            
            PrimBuild::color( color );
            PrimBuild::vertex3fv( p1 );                        
         }
      }

      PrimBuild::end();
   }

   // Render Points.
   if ( false )
   {      
      /*
      GFXTransformSaver saver;

      MatrixF xfm( getRenderTransform() );
      xfm.scale( getScale() );
      GFX->multWorld( xfm );

      GFXStateBlockDesc desc;
      Point3F size( 0.05f );
      */
   }

   // Render surface transforms.
   if ( false )
   {
      GFXStateBlockDesc desc;
      desc.setBlend( false );
      desc.setZReadWrite( true, true );

      Point3F scale(mNormalLength);

      for ( S32 i = 0; i < mSurfaces.size(); i++ )
      {
         MatrixF objToWorld( mObjToWorld );
         objToWorld.scale( mObjScale );

         MatrixF renderMat;
         renderMat.mul( objToWorld, mSurfaces[i] );

         renderMat.setPosition( renderMat.getPosition() + renderMat.getUpVector() * 0.001f );
              
         drawer->drawTransform( desc, renderMat, &scale, NULL );
      }
   }
}

void ConvexShape::renderFaceEdges( S32 faceid, const ColorI &color /*= ColorI::WHITE*/, F32 lineWidth /*= 1.0f */ )
{
   const Vector< ConvexShape::Face > &faceList = mGeometry.faces;

   if ( faceid >= faceList.size() )
      return;

   GFXTransformSaver saver;
   MatrixF xfm( mObjToWorld );
   xfm.scale( mObjScale );
   GFX->multWorld( xfm );

   GFXStateBlockDesc desc;
   desc.setBlend( true );
   GFX->setStateBlockByDesc( desc );

   MatrixF projBias(true);
   const Frustum& frustum = GFX->getFrustum();
   MathUtils::getZBiasProjectionMatrix( 0.001f, frustum, &projBias );
   GFX->setProjectionMatrix( projBias );

   S32 s = faceid;
   S32 e = faceid + 1;

   if ( faceid == -1 )
   {
      s = 0;
      e = faceList.size();
   }

   for ( S32 i = s; i < e; i++ )
   {
      const ConvexShape::Face &face = faceList[i];
      const Vector< ConvexShape::Edge > &edgeList = face.edges;
      const Vector< U32 > &facePntList = face.points;
      const Vector< Point3F > &pointList = mGeometry.points;

      PrimBuild::begin( GFXLineList, edgeList.size() * 2 );

      PrimBuild::color( color );

      for ( S32 j = 0; j < edgeList.size(); j++ )         
      {
         PrimBuild::vertex3fv( pointList[ facePntList[ edgeList[j].p0 ] ] );
         PrimBuild::vertex3fv( pointList[ facePntList[ edgeList[j].p1 ] ] );
      }

      PrimBuild::end();
   }
}

void ConvexShape::getSurfaceTriangles( S32 surfId, Vector< Point3F > *outPoints, Vector< Point2F > *outCoords, bool worldSpace )
{
   S32 faceId = -1;
   for ( S32 i = 0; i < mGeometry.faces.size(); i++ )
   {
      if ( mGeometry.faces[i].id == surfId )
      {
         faceId = i;
         break;
      }
   }

   if ( faceId == -1 )
      return;

   const ConvexShape::Face &face = mGeometry.faces[ faceId ];
   const Vector< Point3F > &pointList = mGeometry.points;

   const MatrixF &surfToObj = mSurfaces[ faceId ];
   MatrixF objToSurf( surfToObj );
   objToSurf.inverse();

   Point3F surfScale( 1.5f, 1.5f, 1.0f );

   for ( S32 i = 0; i < face.triangles.size(); i++ )
   {
      for ( S32 j = 0; j < 3; j++ )
      {
         Point3F pnt( pointList[ face.points[ face.triangles[i][j] ] ] );
         
         objToSurf.mulP( pnt );
         pnt *= surfScale;
         surfToObj.mulP( pnt );

         outPoints->push_back( pnt );

         if ( outCoords )
            outCoords->push_back( face.texcoords[ face.triangles[i][j] ] );
      }
   }

   if ( worldSpace )
   {
      MatrixF objToWorld( mObjToWorld );
      objToWorld.scale( mObjScale );

      for ( S32 i = 0; i < outPoints->size(); i++ )      
         objToWorld.mulP( (*outPoints)[i] );      
   }
}
void ConvexShape::Geometry::generate( const Vector< PlaneF > &planes, const Vector< Point3F > &tangents )
{
   PROFILE_SCOPE( Geometry_generate );

   points.clear();
   faces.clear();	

   AssertFatal( planes.size() == tangents.size(), "ConvexShape - incorrect plane/tangent count." );

#ifdef TORQUE_ENABLE_ASSERTS
   for ( S32 i = 0; i < planes.size(); i++ )
   {
      F32 dt = mDot( planes[i], tangents[i] );
      AssertFatal( mIsZero( dt, 0.0001f ), "ConvexShape - non perpendicular input vectors." );
      AssertFatal( planes[i].isUnitLength() && tangents[i].isUnitLength(), "ConvexShape - non unit length input vector." );
   }
#endif

   const U32 planeCount = planes.size();

   Point3F linePt, lineDir;   

   for ( S32 i = 0; i < planeCount; i++ )
   {      
      Vector< MathUtils::Line > collideLines;

      // Find the lines defined by the intersection of this plane with all others.

      for ( S32 j = 0; j < planeCount; j++ )
      {         
         if ( i == j )
            continue;

         if ( planes[i].intersect( planes[j], linePt, lineDir ) )
         {
            collideLines.increment();
            MathUtils::Line &line = collideLines.last();
            line.origin = linePt;
            line.direction = lineDir;   
         }         
      }

      if ( collideLines.empty() )
         continue;

      // Find edges and points defined by the intersection of these lines.
      // As we find them we fill them into our working ConvexShape::Face
      // structure.
      
      Face newFace;

      for ( S32 j = 0; j < collideLines.size(); j++ )
      {
         Vector< Point3F > collidePoints;

         for ( S32 k = 0; k < collideLines.size(); k++ )
         {
            if ( j == k )
               continue;

            MathUtils::LineSegment segment;
            MathUtils::mShortestSegmentBetweenLines( collideLines[j], collideLines[k], &segment );

            F32 dist = ( segment.p0 - segment.p1 ).len();

            if ( dist < 0.0005f )
            {
               S32 l = 0;
               for ( ; l < planeCount; l++ )
               {
                  if ( planes[l].whichSide( segment.p0 ) == PlaneF::Front )
                     break;
               }

               if ( l == planeCount )
                  collidePoints.push_back( segment.p0 );
            }
         }

         //AssertFatal( collidePoints.size() <= 2, "A line can't collide with more than 2 other lines in a convex shape..." );

         if ( collidePoints.size() != 2 )
            continue;

         // Push back collision points into our points vector
         // if they are not duplicates and determine the id
         // index for those points to be used by Edge(s).    

         const Point3F &pnt0 = collidePoints[0];
         const Point3F &pnt1 = collidePoints[1];
         S32 idx0 = -1;
         S32 idx1 = -1;

         for ( S32 k = 0; k < points.size(); k++ )
         {
            if ( pnt0.equal( points[k] ) )
            {
               idx0 = k;
               break;
            }
         }

         for ( S32 k = 0; k < points.size(); k++ )
         {
            if ( pnt1.equal( points[k] ) )
            {
               idx1 = k;
               break;
            }
         }

         if ( idx0 == -1 )
         {
            points.push_back( pnt0 );               
            idx0 = points.size() - 1;
         }

         if ( idx1 == -1 )
         {
            points.push_back( pnt1 );
            idx1 = points.size() - 1;
         }

         // Construct the Face::Edge defined by this collision.

         S32 localIdx0 = newFace.points.push_back_unique( idx0 );
         S32 localIdx1 = newFace.points.push_back_unique( idx1 );

         newFace.edges.increment();
         ConvexShape::Edge &newEdge = newFace.edges.last();
         newEdge.p0 = localIdx0;
         newEdge.p1 = localIdx1;
      }    

      if ( newFace.points.size() < 3 )
         continue;

      //AssertFatal( newFace.points.size() == newFace.edges.size(), "ConvexShape - face point count does not equal edge count." );


		// Fill in some basic Face information.

		newFace.id = i;
		newFace.normal = planes[i];
		newFace.tangent = tangents[i];


		// Make a working array of Point3Fs on this face.

		U32 pntCount = newFace.points.size();		
		Point3F *workPoints = new Point3F[ pntCount ];

		for ( S32 j = 0; j < pntCount; j++ )
			workPoints[j] = points[ newFace.points[j] ];


      // Calculate the average point for calculating winding order.

      Point3F averagePnt = Point3F::Zero;

		for ( S32 j = 0; j < pntCount; j++ )
			averagePnt += workPoints[j];

		averagePnt /= pntCount;		


		// Sort points in correct winding order.

		U32 *vertMap = new U32[pntCount];

      MatrixF quadMat( true );
      quadMat.setPosition( averagePnt );
      quadMat.setColumn( 0, newFace.tangent );
      quadMat.setColumn( 1, mCross( newFace.normal, newFace.tangent ) );
      quadMat.setColumn( 2, newFace.normal );
		quadMat.inverse();

      // Transform working points into quad space 
      // so we can work with them as 2D points.

      for ( S32 j = 0; j < pntCount; j++ )
         quadMat.mulP( workPoints[j] );

		MathUtils::sortQuadWindingOrder( true, workPoints, vertMap, pntCount );

      // Save points in winding order.

      for ( S32 j = 0; j < pntCount; j++ )
         newFace.winding.push_back( vertMap[j] );

      // Calculate the area and centroid of the face.

      newFace.area = 0.0f;
      for ( S32 j = 0; j < pntCount; j++ )
      {
         S32 k = ( j + 1 ) % pntCount;
         const Point3F &p0 = workPoints[ vertMap[j] ];
         const Point3F &p1 = workPoints[ vertMap[k] ];
         
         // Note that this calculation returns positive area for clockwise winding
         // and negative area for counterclockwise winding.
         newFace.area += p0.y * p1.x;
         newFace.area -= p0.x * p1.y;                  
      }

      //AssertFatal( newFace.area > 0.0f, "ConvexShape - face area was not positive." );
      if ( newFace.area > 0.0f )
         newFace.area /= 2.0f;      

      F32 factor;
      F32 cx = 0.0f, cy = 0.0f;
      
      for ( S32 j = 0; j < pntCount; j++ )
      {
         S32 k = ( j + 1 ) % pntCount;
         const Point3F &p0 = workPoints[ vertMap[j] ];
         const Point3F &p1 = workPoints[ vertMap[k] ];

         factor = p0.x * p1.y - p1.x * p0.y;
         cx += ( p0.x + p1.x ) * factor;
         cy += ( p0.y + p1.y ) * factor;
      }
      
      factor = 1.0f / ( newFace.area * 6.0f );
      newFace.centroid.set( cx * factor, cy * factor, 0.0f );
      quadMat.inverse();
      quadMat.mulP( newFace.centroid );

      delete [] workPoints;
      workPoints = NULL;

		// Make polygons / triangles for this face.

		const U32 polyCount = pntCount - 2;

		newFace.triangles.setSize( polyCount );

		for ( S32 j = 0; j < polyCount; j++ )
		{
			ConvexShape::Triangle &poly = newFace.triangles[j];

			poly.p0 = vertMap[0];

			if ( j == 0 )
			{
				poly.p1 = vertMap[ 1 ];
				poly.p2 = vertMap[ 2 ];
			}
			else
			{
				poly.p1 = vertMap[ 1 + j ];
				poly.p2 = vertMap[ 2 + j ];
			}
		}

		delete [] vertMap;


		// Calculate texture coordinates for each point in this face.

		const Point3F binormal = mCross( newFace.normal, newFace.tangent );
		PlaneF planey( newFace.centroid - 0.5f * binormal, binormal );
		PlaneF planex( newFace.centroid - 0.5f * newFace.tangent, newFace.tangent );

		newFace.texcoords.setSize( newFace.points.size() );

		for ( S32 j = 0; j < newFace.points.size(); j++ )
		{
			F32 x = planex.distToPlane( points[ newFace.points[ j ] ] );
			F32 y = planey.distToPlane( points[ newFace.points[ j ] ] );

			newFace.texcoords[j].set( -x, -y );
		}

      // Data verification tests.
#ifdef TORQUE_ENABLE_ASSERTS
      //S32 triCount = newFace.triangles.size();
      //S32 edgeCount = newFace.edges.size();
      //AssertFatal( triCount == edgeCount - 2, "ConvexShape - triangle/edge count do not match." );

      /*
      for ( S32 j = 0; j < triCount; j++ )
      {
         F32 area = MathUtils::mTriangleArea( points[ newFace.points[ newFace.triangles[j][0] ] ], 
                                              points[ newFace.points[ newFace.triangles[j][1] ] ],
                                              points[ newFace.points[ newFace.triangles[j][2] ] ] );
         AssertFatal( area > 0.0f, "ConvexShape - triangle winding bad." );
      }*/
#endif


      // Done with this Face.
      
      faces.push_back( newFace );
   }
}
