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
#include "environment/decalRoad.h"

#include "console/consoleTypes.h"
#include "console/engineAPI.h"
#include "util/catmullRom.h"
#include "math/util/quadTransforms.h"
#include "scene/sceneRenderState.h"
#include "scene/sceneManager.h"
#include "core/stream/bitStream.h"
#include "gfx/gfxDrawUtil.h"
#include "gfx/gfxTransformSaver.h"
#include "math/mathIO.h"
#include "math/mathUtils.h"
#include "terrain/terrData.h"
#include "materials/materialDefinition.h"
#include "materials/materialManager.h"
#include "materials/baseMatInstance.h"
#include "environment/nodeListManager.h"
#include "lighting/lightQuery.h"


extern F32 gDecalBias;
extern bool gEditingMission;


//-----------------------------------------------------------------------------
// DecalRoadNodeList Struct
//-----------------------------------------------------------------------------

struct DecalRoadNodeList : public NodeListManager::NodeList
{
   Vector<Point3F>   mPositions;
   Vector<F32>       mWidths;

   DecalRoadNodeList() { }
   virtual ~DecalRoadNodeList() { }
};

//-----------------------------------------------------------------------------
// DecalRoadNodeEvent Class
//-----------------------------------------------------------------------------

class DecalRoadNodeEvent : public NodeListEvent
{
   typedef NodeListEvent Parent;

public:
   Vector<Point3F>   mPositions;
   Vector<F32>       mWidths;

public:
   DecalRoadNodeEvent() { mNodeList = NULL; }
   virtual ~DecalRoadNodeEvent() { }

   virtual void pack(NetConnection*, BitStream*);
   virtual void unpack(NetConnection*, BitStream*);

   virtual void copyIntoList(NodeListManager::NodeList* copyInto);
   virtual void padListToSize();

   DECLARE_CONOBJECT(DecalRoadNodeEvent);
};

void DecalRoadNodeEvent::pack(NetConnection* conn, BitStream* stream)
{
   Parent::pack( conn, stream );

   stream->writeInt( mPositions.size(), 16 );

   for (U32 i=0; i<mPositions.size(); ++i)
   {
      mathWrite( *stream, mPositions[i] );
      stream->write( mWidths[i] );
   }
}

void DecalRoadNodeEvent::unpack(NetConnection* conn, BitStream* stream)
{
   mNodeList = new DecalRoadNodeList();

   Parent::unpack( conn, stream );

   U32 count = stream->readInt( 16 );

   Point3F pos;
   F32 width;

   DecalRoadNodeList* list = static_cast<DecalRoadNodeList*>(mNodeList);

   for (U32 i=0; i<count; ++i)
   {
      mathRead( *stream, &pos );
      stream->read( &width );   

      list->mPositions.push_back( pos );
      list->mWidths.push_back( width );
   }

   list->mTotalValidNodes = count;

   // Do we have a complete list?
   if (list->mPositions.size() >= mTotalNodes)
      list->mListComplete = true;
}

void DecalRoadNodeEvent::copyIntoList(NodeListManager::NodeList* copyInto)
{
   DecalRoadNodeList* prevList = static_cast<DecalRoadNodeList*>(copyInto);
   DecalRoadNodeList* list = static_cast<DecalRoadNodeList*>(mNodeList);

   // Merge our list with the old list.
   for (U32 i=mLocalListStart, index=0; i<mLocalListStart+list->mPositions.size(); ++i, ++index)
   {
      prevList->mPositions[i] = list->mPositions[index];
      prevList->mWidths[i] = list->mWidths[index];
   }
}

void DecalRoadNodeEvent::padListToSize()
{
   DecalRoadNodeList* list = static_cast<DecalRoadNodeList*>(mNodeList);

   U32 totalValidNodes = list->mTotalValidNodes;

   // Pad our list front?
   if (mLocalListStart)
   {
      DecalRoadNodeList* newlist = new DecalRoadNodeList();
      newlist->mPositions.increment(mLocalListStart);
      newlist->mWidths.increment(mLocalListStart);

      newlist->mPositions.merge(list->mPositions);
      newlist->mWidths.merge(list->mWidths);

      delete list;
      mNodeList = list = newlist;
   }

   // Pad our list end?
   if (list->mPositions.size() < mTotalNodes)
   {
      U32 delta = mTotalNodes - list->mPositions.size();
      list->mPositions.increment(delta);
      list->mWidths.increment(delta);
   }

   list->mTotalValidNodes = totalValidNodes;
}

IMPLEMENT_CO_NETEVENT_V1(DecalRoadNodeEvent);

ConsoleDocClass( DecalRoadNodeEvent,
   "@brief Sends messages to the Decal Road Editor\n\n"
   "Editor use only.\n\n"
   "@internal"
);
//-----------------------------------------------------------------------------
// DecalRoadNodeListNotify Class
//-----------------------------------------------------------------------------

class DecalRoadNodeListNotify : public NodeListNotify
{
   typedef NodeListNotify Parent;

protected:
   SimObjectPtr<DecalRoad> mRoad;

public:
   DecalRoadNodeListNotify( DecalRoad* road, U32 listId ) { mRoad = road; mListId = listId; }
   virtual ~DecalRoadNodeListNotify() { mRoad = NULL; }

   virtual void sendNotification( NodeListManager::NodeList* list );
};

void DecalRoadNodeListNotify::sendNotification( NodeListManager::NodeList* list )
{
   if (mRoad.isValid())
   {
      // Build the road's nodes
      DecalRoadNodeList* roadList = dynamic_cast<DecalRoadNodeList*>( list );
      if (roadList)
         mRoad->buildNodesFromList( roadList );
   }
}

//-----------------------------------------------------------------------------
// DecalRoadUpdateEvent Class
//-----------------------------------------------------------------------------

void DecalRoadUpdateEvent::process( SimObject *object )
{
   DecalRoad *road = dynamic_cast<DecalRoad*>( object );
   AssertFatal( road, "DecalRoadRegenEvent::process - wasn't a DecalRoad" );

   // Inform clients to perform the update.
   road->setMaskBits( mMask );

   if ( !road->isProperlyAdded() )
      return;
   
   // Perform the server side update.
   if ( mMask & DecalRoad::TerrainChangedMask )
   {      
      road->_generateEdges();
   }
   if ( mMask & DecalRoad::GenEdgesMask )
   {
      // Server has already done this.
      //road->_generateEdges();
   }   
   if ( mMask & DecalRoad::ReClipMask )
   {
      // Server does not need to capture verts.
      road->_captureVerts();
   }
}


//------------------------------------------------------------------------------
// Class: DecalRoad
//------------------------------------------------------------------------------

ConsoleDocClass( DecalRoad,
   "@brief A strip shaped decal defined by spine nodes which clips against Terrain objects.\n\n"

   "DecalRoad is for representing a road or path ( or other inventive things ) across "
   "a TerrainBlock. It renders as a decal and is therefore only for features that do "
   "not need geometric depth.\n\n"

   "The Material assigned to DecalRoad should tile vertically.\n\n"

   "@ingroup Terrain"
);

// Init Statics

// Static ConsoleVars for toggling debug rendering
bool DecalRoad::smEditorOpen = false;
bool DecalRoad::smWireframe = true;
bool DecalRoad::smShowBatches = false;
bool DecalRoad::smDiscardAll = false;
bool DecalRoad::smShowSpline = true;
bool DecalRoad::smShowRoad = true;
S32 DecalRoad::smUpdateDelay = 500;

SimObjectPtr<SimSet> DecalRoad::smServerDecalRoadSet = NULL;


// Constructors

DecalRoad::DecalRoad()
 : mLoadRenderData( true ),
   mBreakAngle( 3.0f ),
   mSegmentsPerBatch( 10 ),
   mTextureLength( 5.0f ),
   mRenderPriority( 10 ),
   mMaterial( NULL ),
   mMatInst( NULL ),
   mUpdateEventId( -1 ),
   mTerrainUpdateRect( Box3F::Invalid )
{   
   // Setup NetObject.
   mTypeMask |= StaticObjectType | StaticShapeObjectType;
   mNetFlags.set(Ghostable);      
}

DecalRoad::~DecalRoad()
{   
}

IMPLEMENT_CO_NETOBJECT_V1(DecalRoad);


// ConsoleObject

void DecalRoad::initPersistFields()
{
   addGroup( "DecalRoad" );

      addField( "material", TypeMaterialName, Offset( mMaterialName, DecalRoad ), "Material used for rendering." ); 

      addProtectedField( "textureLength", TypeF32, Offset( mTextureLength, DecalRoad ), &DecalRoad::ptSetTextureLength, &defaultProtectedGetFn, 
         "The length in meters of textures mapped to the DecalRoad" );      

      addProtectedField( "breakAngle", TypeF32, Offset( mBreakAngle, DecalRoad ), &DecalRoad::ptSetBreakAngle, &defaultProtectedGetFn, 
         "Angle in degrees - DecalRoad will subdivided the spline if its curve is greater than this threshold." );      

      addField( "renderPriority", TypeS32, Offset( mRenderPriority, DecalRoad ), 
         "DecalRoad(s) are rendered in descending renderPriority order." );

   endGroup( "DecalRoad" );

   addGroup( "Internal" );

      addProtectedField( "node", TypeString, NULL, &addNodeFromField, &emptyStringProtectedGetFn, 
         "Do not modify, for internal use." );

   endGroup( "Internal" );

   Parent::initPersistFields();
}

void DecalRoad::consoleInit()
{
   Parent::consoleInit();

   // Vars for debug rendering while the RoadEditor is open, only used if smEditorOpen is true.
   Con::addVariable( "$DecalRoad::EditorOpen", TypeBool, &DecalRoad::smEditorOpen, "For use by the Decal Editor.\n\n"
	   "@ingroup Editors\n" );
   Con::addVariable( "$DecalRoad::wireframe", TypeBool, &DecalRoad::smWireframe, "For use by the Decal Editor.\n\n"
	   "@ingroup Editors\n" );
   Con::addVariable( "$DecalRoad::showBatches", TypeBool, &DecalRoad::smShowBatches, "For use by the Decal Editor.\n\n"
	   "@ingroup Editors\n" );
   Con::addVariable( "$DecalRoad::discardAll", TypeBool, &DecalRoad::smDiscardAll, "For use by the Decal Editor.\n\n"
	   "@ingroup Editors\n");
   Con::addVariable( "$DecalRoad::showSpline", TypeBool, &DecalRoad::smShowSpline, "For use by the Decal Editor.\n\n"
	   "@ingroup Editors\n" );
   Con::addVariable( "$DecalRoad::showRoad", TypeBool, &DecalRoad::smShowRoad, "For use by the Decal Editor.\n\n"
	   "@ingroup Editors\n" );
   Con::addVariable( "$DecalRoad::updateDelay", TypeS32, &DecalRoad::smUpdateDelay, "For use by the Decal Editor.\n\n"
	   "@ingroup Editors\n" );
}


// SimObject

bool DecalRoad::onAdd()
{
   if ( !Parent::onAdd() ) 
      return false;

   // DecalRoad is at position zero when created,
   // it sets its own position to the first node inside
   // _generateEdges but until it has at least one node
   // it will be at 0,0,0.

   MatrixF mat(true);
   Parent::setTransform( mat );

   // The client side calculates bounds based on clipped geometry.  It would 
   // be wasteful for the server to do this so the server uses global bounds.
   if ( isServerObject() )
   {
      setGlobalBounds();
      resetWorldBox();
   }

   // Set the Render Transform.
   setRenderTransform(mObjToWorld);

   // Add to Scene.
   addToScene();

   if ( isServerObject() )   
      getServerSet()->addObject( this );   

   //   
   TerrainBlock::smUpdateSignal.notify( this, &DecalRoad::_onTerrainChanged );

   //   
   if ( isClientObject() )
      _initMaterial();

   _generateEdges();
   _captureVerts();

   return true;
}

void DecalRoad::onRemove()
{
   SAFE_DELETE( mMatInst );

   TerrainBlock::smUpdateSignal.remove( this, &DecalRoad::_onTerrainChanged );

   removeFromScene();

   Parent::onRemove();
}

void DecalRoad::inspectPostApply()
{
   Parent::inspectPostApply();      

   setMaskBits( DecalRoadMask );
}

void DecalRoad::onStaticModified( const char* slotName, const char*newValue )
{
   Parent::onStaticModified( slotName, newValue );

   /*
   if ( isProperlyAdded() &&
        dStricmp( slotName, "material" ) == 0 )
   {
      setMaskBits( DecalRoadMask );      
   }
   */

   if ( dStricmp( slotName, "renderPriority" ) == 0 )
   {
      mRenderPriority = getMax( dAtoi(newValue), (S32)1 );
   }
}

SimSet* DecalRoad::getServerSet()
{
   if ( !smServerDecalRoadSet )
   {
      smServerDecalRoadSet = new SimSet();
      smServerDecalRoadSet->registerObject( "ServerDecalRoadSet" );
      Sim::getRootGroup()->addObject( smServerDecalRoadSet );
   }

   return smServerDecalRoadSet;
}

void DecalRoad::writeFields( Stream &stream, U32 tabStop )
{
   Parent::writeFields( stream, tabStop );

   // Now write all nodes

   stream.write(2, "\r\n");   

   for ( U32 i = 0; i < mNodes.size(); i++ )
   {
      const RoadNode &node = mNodes[i];

      stream.writeTabs(tabStop);

      char buffer[1024];
      dMemset( buffer, 0, 1024 );
      dSprintf( buffer, 1024, "Node = \"%f %f %f %f\";", node.point.x, node.point.y, node.point.z, node.width );      
      stream.writeLine( (const U8*)buffer );
   }
}

bool DecalRoad::writeField( StringTableEntry fieldname, const char *value )
{   
   if ( fieldname == StringTable->insert("node") )
      return false;

   return Parent::writeField( fieldname, value );
}

void DecalRoad::onEditorEnable()
{
}

void DecalRoad::onEditorDisable()
{
}


// NetObject

U32 DecalRoad::packUpdate(NetConnection * con, U32 mask, BitStream * stream)
{  
   // Pack Parent.
   U32 retMask = Parent::packUpdate(con, mask, stream);

   if ( stream->writeFlag( mask & DecalRoadMask ) )
   {
      // Write Texture Name.
      stream->write( mMaterialName );

      stream->write( mBreakAngle );      

      stream->write( mSegmentsPerBatch );

      stream->write( mTextureLength );

      stream->write( mRenderPriority );
   }

   if ( stream->writeFlag( mask & NodeMask ) )
   {
      //stream->writeInt( mNodes.size(), 16 );

      //for ( U32 i = 0; i < mNodes.size(); i++ )
      //{
      //   mathWrite( *stream, mNodes[i].point );
      //   stream->write( mNodes[i].width );
      //}

      const U32 nodeByteSize = 16; // Based on sending all of a node's parameters

      // Test if we can fit all of our nodes within the current stream.
      // We make sure we leave 100 bytes still free in the stream for whatever
      // may follow us.
      S32 allowedBytes = stream->getWriteByteSize() - 100;
      if ( stream->writeFlag( (nodeByteSize * mNodes.size()) < allowedBytes ) )
      {
         // All nodes should fit, so send them out now.
         stream->writeInt( mNodes.size(), 16 );

         for ( U32 i = 0; i < mNodes.size(); i++ )
         {
            mathWrite( *stream, mNodes[i].point );
            stream->write( mNodes[i].width );
         }
      }
      else
      {
         // There isn't enough space left in the stream for all of the
         // nodes.  Batch them up into NetEvents.
         U32 id = gServerNodeListManager->nextListId();
         U32 count = 0;
         U32 index = 0;
         while (count < mNodes.size())
         {
            count += NodeListManager::smMaximumNodesPerEvent;
            if (count > mNodes.size())
            {
               count = mNodes.size();
            }

            DecalRoadNodeEvent* event = new DecalRoadNodeEvent();
            event->mId = id;
            event->mTotalNodes = mNodes.size();
            event->mLocalListStart = index;

            for (; index<count; ++index)
            {
               event->mPositions.push_back( mNodes[index].point );
               event->mWidths.push_back( mNodes[index].width );
            }

            con->postNetEvent( event );
         }

         stream->write( id );
      }
   }

   stream->writeFlag( mask & GenEdgesMask );

   stream->writeFlag( mask & ReClipMask );

   stream->writeFlag( mask & TerrainChangedMask );

   // Were done ...
   return retMask;
}

void DecalRoad::unpackUpdate( NetConnection *con, BitStream *stream )
{
   // Unpack Parent.
   Parent::unpackUpdate( con, stream );

   // DecalRoadMask
   if ( stream->readFlag() )
   {
      String matName;
      stream->read( &matName );
      
      if ( matName != mMaterialName )
      {
         mMaterialName = matName;
         Material *pMat = NULL;
         if ( !Sim::findObject( mMaterialName, pMat ) )
         {
            Con::printf( "DecalRoad::unpackUpdate, failed to find Material of name %s!", mMaterialName.c_str() );
         }
         else
         {
            mMaterial = pMat;
            if ( isProperlyAdded() )
               _initMaterial(); 
         }
      }

      stream->read( &mBreakAngle );    

      stream->read( &mSegmentsPerBatch );

      stream->read( &mTextureLength );

      stream->read( &mRenderPriority );
   }

   // NodeMask
   if ( stream->readFlag() )
   {
      //U32 count = stream->readInt( 16 );

      //mNodes.clear();

      //Point3F pos;
      //F32 width;
      //for ( U32 i = 0; i < count; i++ )
      //{
      //   mathRead( *stream, &pos );
      //   stream->read( &width );         
      //   _addNode( pos, width );         
      //}

      if (stream->readFlag())
      {
         // Nodes have been passed in this update
         U32 count = stream->readInt( 16 );

         mNodes.clear();

         Point3F pos;
         F32 width;
         for ( U32 i = 0; i < count; i++ )
         {
            mathRead( *stream, &pos );
            stream->read( &width );         
            _addNode( pos, width );         
         }
      }
      else
      {
         // Nodes will arrive as events
         U32 id;
         stream->read( &id );

         // Check if the road's nodes made it here before we did.
         NodeListManager::NodeList* list = NULL;
         if ( gClientNodeListManager->findListById( id, &list, true) )
         {
            // Work with the completed list
            DecalRoadNodeList* roadList = dynamic_cast<DecalRoadNodeList*>( list );
            if (roadList)
               buildNodesFromList( roadList );

            delete list;
         }
         else
         {
            // Nodes have not yet arrived, so register our interest in the list
            DecalRoadNodeListNotify* notify = new DecalRoadNodeListNotify( this, id );
            gClientNodeListManager->registerNotification( notify );
         }
      }
   }

    // GenEdgesMask
   if ( stream->readFlag() && isProperlyAdded() )
      _generateEdges();

   // ReClipMask
   if ( stream->readFlag() && isProperlyAdded() )
      _captureVerts();

   // TerrainChangedMask
   if ( stream->readFlag() )
   {      
      if ( isProperlyAdded() )
      {
         if ( mTerrainUpdateRect.isOverlapped( getWorldBox() ) )
         {
            _generateEdges();
            _captureVerts();
            // Clear out the mTerrainUpdateRect since we have updated its
            // region and we now need to store future terrain changes
            // in it.
            mTerrainUpdateRect = Box3F::Invalid;
         }         
      }      
   }
}

void DecalRoad::prepRenderImage( SceneRenderState* state )
{
   PROFILE_SCOPE( DecalRoad_prepRenderImage );

   if (  mNodes.size() <= 1 || 
         mBatches.size() == 0 ||
         !mMatInst ||
         state->isShadowPass() )
      return;

   // If we don't have a material instance after the override then 
   // we can skip rendering all together.
   BaseMatInstance *matInst = state->getOverrideMaterial( mMatInst );
   if ( !matInst )
      return;
      
   RenderPassManager *renderPass = state->getRenderPass();

   // Debug RenderInstance
   // Only when editor is open.
   if ( smEditorOpen )
   {
      ObjectRenderInst *ri = renderPass->allocInst<ObjectRenderInst>();
      ri->type = RenderPassManager::RIT_Editor;
      ri->renderDelegate.bind( this, &DecalRoad::_debugRender );
      state->getRenderPass()->addInst( ri );
   }

   // Normal Road RenderInstance
   // Always rendered when the editor is not open
   // otherwise obey the smShowRoad flag
   if ( !smShowRoad && smEditorOpen )
      return;

   const Frustum &frustum = state->getCameraFrustum();

   MeshRenderInst coreRI;
   coreRI.clear();
   coreRI.objectToWorld = &MatrixF::Identity;
   coreRI.worldToCamera = renderPass->allocSharedXform(RenderPassManager::View);
   
   MatrixF *tempMat = renderPass->allocUniqueXform( MatrixF( true ) );   
   MathUtils::getZBiasProjectionMatrix( gDecalBias, frustum, tempMat );
   coreRI.projection = tempMat;

   coreRI.type = RenderPassManager::RIT_DecalRoad;
   coreRI.vertBuff = &mVB;
   coreRI.primBuff = &mPB;
   coreRI.matInst = matInst;

   // Make it the sort distance the max distance so that 
   // it renders after all the other opaque geometry in 
   // the prepass bin.
   coreRI.sortDistSq = F32_MAX;

	// If we need lights then set them up.
   if ( matInst->isForwardLit() )
   {
      LightQuery query;
      query.init( getWorldSphere() );
		query.getLights( coreRI.lights, 8 );
   }
   
   U32 startBatchIdx = -1;
   U32 endBatchIdx = 0;

   for ( U32 i = 0; i < mBatches.size(); i++ )   
   {
      const RoadBatch &batch = mBatches[i];
      const bool isVisible = !frustum.isCulled( batch.bounds );         
      if ( isVisible )
      {
         // If this is the start of a set of batches.
         if ( startBatchIdx == -1 )
            endBatchIdx = startBatchIdx = i;

         // Else we're extending the end batch index.
         else
            ++endBatchIdx; 

         // If this isn't the last batch then continue.
         if ( i < mBatches.size()-1 )
            continue;
      }

      // We we still don't have a start batch, so skip.
      if ( startBatchIdx == -1 )
         continue;

      // Render this set of batches.
      const RoadBatch &startBatch = mBatches[startBatchIdx];
      const RoadBatch &endBatch = mBatches[endBatchIdx]; 

      U32 startVert = startBatch.startVert;
      U32 startIdx = startBatch.startIndex;
      U32 vertCount = endBatch.endVert - startVert;
      U32 idxCount = ( endBatch.endIndex - startIdx ) + 1;
      U32 triangleCount = idxCount / 3;

      AssertFatal( startVert + vertCount <= mVertCount, "DecalRoad, bad draw call!" );
      AssertFatal( startIdx + triangleCount < mTriangleCount * 3, "DecalRoad, bad draw call!" );

      MeshRenderInst *ri = renderPass->allocInst<MeshRenderInst>();

      *ri = coreRI;

      ri->prim = renderPass->allocPrim();
      ri->prim->type = GFXTriangleList;
      ri->prim->minIndex = 0;
      ri->prim->startIndex = startIdx;
      ri->prim->numPrimitives = triangleCount;
      ri->prim->startVertex = 0;
      ri->prim->numVertices = endBatch.endVert + 1;

      // For sorting we first sort by render priority
      // and then by objectId. 
      //
      // Since a road can submit more than one render instance, we want all 
      // draw calls for a single road to occur consecutively, since they
      // could use the same vertex buffer.
      ri->defaultKey =  mRenderPriority << 0 | mId << 16;
      ri->defaultKey2 = 0;

      renderPass->addInst( ri );

      // Reset the batching.
      startBatchIdx = -1;
   }   
}

void DecalRoad::setTransform( const MatrixF &mat )
{
   // We ignore transform requests from the editor
   // right now.
}

void DecalRoad::setScale( const VectorF &scale )
{
   // We ignore scale requests from the editor
   // right now.
}


// DecalRoad Public Methods

bool DecalRoad::getClosestNode( const Point3F &pos, U32 &idx )
{
   F32 closestDist = F32_MAX;

   for ( U32 i = 0; i < mNodes.size(); i++ )
   {
      F32 dist = ( mNodes[i].point - pos ).len();
      if ( dist < closestDist )
      {
         closestDist = dist;
         idx = i;
      }      
   }

   return closestDist != F32_MAX;
}

bool DecalRoad::containsPoint( const Point3F &worldPos, U32 *nodeIdx ) const
{
   // This is just for making selections in the editor, we use the 
   // client-side road because it has the proper edge's.
   if ( isServerObject() && getClientObject() )
      return ((DecalRoad*)getClientObject())->containsPoint( worldPos, nodeIdx );

   // If point isn't in the world box, 
   // it's definitely not in the road.
   //if ( !getWorldBox().isContained( worldPos ) )
   //   return false;

   if ( mEdges.size() < 2 )
      return false;

   Point2F testPt(   worldPos.x, 
                     worldPos.y );
   Point2F poly[4];

   // Look through all edges, does the polygon
   // formed from adjacent edge's contain the worldPos?
   for ( U32 i = 0; i < mEdges.size() - 1; i++ )
   {
      const RoadEdge &edge0 = mEdges[i];
      const RoadEdge &edge1 = mEdges[i+1];

      poly[0].set( edge0.p0.x, edge0.p0.y );
      poly[1].set( edge0.p2.x, edge0.p2.y );
      poly[2].set( edge1.p2.x, edge1.p2.y );
      poly[3].set( edge1.p0.x, edge1.p0.y );

      if ( MathUtils::pointInPolygon( poly, 4, testPt ) )
      {
         if ( nodeIdx )
            *nodeIdx = edge0.parentNodeIdx;

         return true;
      }

   }

   return false;
}

bool DecalRoad::castray( const Point3F &start, const Point3F &end ) const
{
   // We just cast against the object box for the editor.
   return mWorldBox.collideLine( start, end );
}

Point3F DecalRoad::getNodePosition( U32 idx )
{
   if ( mNodes.size() - 1 < idx )
      return Point3F();

   return mNodes[idx].point;
}

void DecalRoad::setNodePosition( U32 idx, const Point3F &pos )
{
   if ( mNodes.size() - 1 < idx )
      return;

   mNodes[idx].point = pos;

   _generateEdges();
   scheduleUpdate( GenEdgesMask | ReClipMask | NodeMask );
}

U32 DecalRoad::addNode( const Point3F &pos, F32 width )
{
   U32 idx = _addNode( pos, width );   

   _generateEdges();
   scheduleUpdate( GenEdgesMask | ReClipMask | NodeMask );

   return idx;
}

U32 DecalRoad::insertNode(const Point3F &pos, const F32 &width, const U32 &idx)
{
   U32 ret = _insertNode( pos, width, idx );

   _generateEdges();
   scheduleUpdate( GenEdgesMask | ReClipMask | NodeMask );

   return ret;
}

void DecalRoad::setNodeWidth( U32 idx, F32 width )
{
   if ( mNodes.size() - 1 < idx )
      return;

   mNodes[idx].width = width;
   
   _generateEdges();
   scheduleUpdate( GenEdgesMask | ReClipMask | NodeMask );
}

F32 DecalRoad::getNodeWidth( U32 idx )
{
   if ( mNodes.size() - 1 < idx )
      return -1.0f;

   return mNodes[idx].width;
}

void DecalRoad::deleteNode( U32 idx )
{
   if ( mNodes.size() - 1 < idx )
      return;

   mNodes.erase(idx);   

   _generateEdges();
   scheduleUpdate( GenEdgesMask | ReClipMask | NodeMask );
}

void DecalRoad::buildNodesFromList( DecalRoadNodeList* list )
{
   mNodes.clear();

   for (U32 i=0; i<list->mPositions.size(); ++i)
   {
      _addNode( list->mPositions[i], list->mWidths[i] );
   }

   _generateEdges();
   _captureVerts();
}

void DecalRoad::setTextureLength( F32 meters )
{   
   meters = getMax( meters, 0.1f );   
   if ( mTextureLength == meters )
      return;

   mTextureLength = meters;

   _generateEdges();
   scheduleUpdate( DecalRoadMask | ReClipMask );
}

void DecalRoad::setBreakAngle( F32 degrees )
{
   //meters = getMax( meters, MIN_METERS_PER_SEGMENT );
   //if ( mBreakAngle == meters )
   //   return;

   mBreakAngle = degrees;

   _generateEdges();
   scheduleUpdate( DecalRoadMask | GenEdgesMask | ReClipMask );
}

void DecalRoad::scheduleUpdate( U32 updateMask )
{
   scheduleUpdate( updateMask, smUpdateDelay, true );
}

void DecalRoad::scheduleUpdate( U32 updateMask, U32 delayMs, bool restartTimer  )
{
   if ( Sim::isEventPending( mUpdateEventId ) )
   {
      if ( !restartTimer )
      {
         mLastEvent->mMask |= updateMask;
         return;
      }
      else
      {
         Sim::cancelEvent( mUpdateEventId );
      }
   }
   
   mLastEvent = new DecalRoadUpdateEvent( updateMask, delayMs );
   mUpdateEventId = Sim::postEvent( this, mLastEvent, Sim::getCurrentTime() + delayMs );
}

void DecalRoad::regenerate()
{
   _generateEdges();
   _captureVerts();   
   setMaskBits( NodeMask | GenEdgesMask | ReClipMask );
}

bool DecalRoad::addNodeFromField( void *object, const char *index, const char *data )
{
   DecalRoad *pObj = static_cast<DecalRoad*>(object);
    
   F32 x,y,z,width;      
   U32 result = dSscanf( data, "%f %f %f %f", &x, &y, &z, &width );      
   if ( result == 4 )
      pObj->_addNode( Point3F(x,y,z), width );

   return false;
}


// Internal Helper Methods

void DecalRoad::_initMaterial()
{
   SAFE_DELETE( mMatInst );

   if ( mMaterial )
      mMatInst = mMaterial->createMatInstance();
   else
      mMatInst = MATMGR->createMatInstance( "WarningMaterial" );

   GFXStateBlockDesc desc;
   desc.setZReadWrite( true, false );
   mMatInst->addStateBlockDesc( desc );

   mMatInst->init( MATMGR->getDefaultFeatures(), getGFXVertexFormat<GFXVertexPNTBT>() );
}

void DecalRoad::_debugRender( ObjectRenderInst *ri, SceneRenderState *state, BaseMatInstance* )
{
   //if ( mStateBlock.isNull() )
   //   return;

   GFX->enterDebugEvent( ColorI( 255, 0, 0 ), "DecalRoad_debugRender" );
   GFXTransformSaver saver;

   //GFX->setStateBlock( mStateBlock );      

   Point3F size(1,1,1);
   ColorI color( 255, 0, 0, 255 );

   GFXStateBlockDesc desc;
   desc.setZReadWrite( true, false );
   desc.setBlend( true );
   desc.fillMode = GFXFillWireframe;

   if ( smShowBatches )  
   {
      for ( U32 i = 0; i < mBatches.size(); i++ )   
      {
         const Box3F &box = mBatches[i].bounds;         
         GFX->getDrawUtil()->drawCube( desc, box, ColorI(255,100,100,255) );         
      }
   }

   //GFX->leaveDebugEvent();
}

void DecalRoad::_generateEdges()
{      
   PROFILE_SCOPE( DecalRoad_generateEdges );

   //Con::warnf( "%s - generateEdges", isServerObject() ? "server" : "client" );

   if ( mNodes.size() > 0 )
   {
      // Set our object position to the first node.
      const Point3F &nodePt = mNodes.first().point;
      MatrixF mat( true );
      mat.setPosition( nodePt );
      Parent::setTransform( mat );

      // The server object has global bounds, which Parent::setTransform 
      // messes up so we must reset it.
      if ( isServerObject() )
      {
         mObjBox.minExtents.set(-1e10, -1e10, -1e10);
         mObjBox.maxExtents.set( 1e10,  1e10,  1e10);
      }
   }


   if ( mNodes.size() < 2 )
      return;

   // Ensure nodes are above the terrain height at their xy position
   for ( U32 i = 0; i < mNodes.size(); i++ )
   {
      _getTerrainHeight( mNodes[i].point );
   }

   // Now start generating edges...

   U32 nodeCount = mNodes.size();
   Point3F *positions = new Point3F[nodeCount];

   for ( U32 i = 0; i < nodeCount; i++ )
   {
      const RoadNode &node = mNodes[i];
      positions[i].set( node.point.x, node.point.y, node.width );
   }

   CatmullRom<Point3F> spline;
   spline.initialize( nodeCount, positions );
   delete [] positions;

   mEdges.clear();

   Point3F lastBreakVector(0,0,0);      
   RoadEdge slice;
   Point3F lastBreakNode;
   lastBreakNode = spline.evaluate(0.0f);

   for ( U32 i = 1; i < mNodes.size(); i++ )
   {
      F32 t1 = spline.getTime(i);
      F32 t0 = spline.getTime(i-1);

      F32 segLength = spline.arcLength( t0, t1 );

      U32 numSegments = mCeil( segLength / MIN_METERS_PER_SEGMENT );
      numSegments = getMax( numSegments, (U32)1 );
      F32 tstep = ( t1 - t0 ) / numSegments; 

      U32 startIdx = 0;
      U32 endIdx = ( i == nodeCount - 1 ) ? numSegments + 1 : numSegments;

      for ( U32 j = startIdx; j < endIdx; j++ )
      {
         F32 t = t0 + tstep * j;
         Point3F splineNode = spline.evaluate(t);
         F32 width = splineNode.z;
         _getTerrainHeight( splineNode );

         Point3F toNodeVec = splineNode - lastBreakNode;
         toNodeVec.normalizeSafe();

         if ( lastBreakVector.isZero() )
            lastBreakVector = toNodeVec;

         F32 angle = mRadToDeg( mAcos( mDot( toNodeVec, lastBreakVector ) ) );

         if ( j == startIdx || 
            ( j == endIdx - 1 && i == mNodes.size() - 1 ) ||
            angle > mBreakAngle )
         {
            // Push back a spline node
            //slice.p1.set( splineNode.x, splineNode.y, 0.0f );
            //_getTerrainHeight( slice.p1 );
            slice.p1 = splineNode;
            slice.uvec.set(0,0,1);
            slice.width = width;            
            slice.parentNodeIdx = i-1;
            mEdges.push_back( slice );         

            lastBreakVector = splineNode - lastBreakNode;
            lastBreakVector.normalizeSafe();

            lastBreakNode = splineNode;
         }          
      }
   }

   /*
   for ( U32 i = 1; i < nodeCount; i++ )
   {
      F32 t0 = spline.getTime( i-1 );
      F32 t1 = spline.getTime( i );

      F32 segLength = spline.arcLength( t0, t1 );

      U32 numSegments = mCeil( segLength / mBreakAngle );
      numSegments = getMax( numSegments, (U32)1 );
      F32 tstep = ( t1 - t0 ) / numSegments;

      AssertFatal( numSegments > 0, "DecalRoad::_generateEdges, got zero segments!" );   

      U32 startIdx = 0;
      U32 endIdx = ( i == nodeCount - 1 ) ? numSegments + 1 : numSegments;

      for ( U32 j = startIdx; j < endIdx; j++ )
      {
         F32 t = t0 + tstep * j;
         Point3F val = spline.evaluate(t);

         RoadEdge edge;         
         edge.p1.set( val.x, val.y, 0.0f );    
         _getTerrainHeight( val.x, val.y, edge.p1.z );    
         edge.uvec.set(0,0,1);
         edge.width = val.z;
         edge.parentNodeIdx = i-1;
         mEdges.push_back( edge );
      }   
   }
   */

   //
   // Calculate fvec and rvec for all edges
   //
   RoadEdge *edge = NULL;
   RoadEdge *nextEdge = NULL;

   for ( U32 i = 0; i < mEdges.size() - 1; i++ )
   {
      edge = &mEdges[i];
      nextEdge = &mEdges[i+1];

      edge->fvec = nextEdge->p1 - edge->p1;
      edge->fvec.normalize();

      edge->rvec = mCross( edge->fvec, edge->uvec );
      edge->rvec.normalize();
   }

   // Must do the last edge outside the loop
   RoadEdge *lastEdge = &mEdges[mEdges.size()-1];
   RoadEdge *prevEdge = &mEdges[mEdges.size()-2];
   lastEdge->fvec = prevEdge->fvec;
   lastEdge->rvec = prevEdge->rvec;


   //
   // Calculate p0/p2 for all edges
   //      
   for ( U32 i = 0; i < mEdges.size(); i++ )
   {
      RoadEdge *edge = &mEdges[i];
      edge->p0 = edge->p1 - edge->rvec * edge->width * 0.5f;
      edge->p2 = edge->p1 + edge->rvec * edge->width * 0.5f;
      _getTerrainHeight( edge->p0 );
      _getTerrainHeight( edge->p2 );
   }   
}

void DecalRoad::_captureVerts()
{  
   PROFILE_SCOPE( DecalRoad_captureVerts );

   //Con::warnf( "%s - captureVerts", isServerObject() ? "server" : "client" );

   if ( isServerObject() )
   {
      //Con::errorf( "DecalRoad::_captureVerts - called on the server side!" );
      return;
   }

   if ( mEdges.size() == 0 )
      return;

   //
   // Construct ClippedPolyList objects for each pair
   // of roadEdges.
   // Use them to capture Terrain verts.
   //
   SphereF sphere;
   RoadEdge *edge = NULL;
   RoadEdge *nextEdge = NULL;

   mTriangleCount = 0;
   mVertCount = 0;

   Vector<ClippedPolyList> clipperList;

   for ( U32 i = 0; i < mEdges.size() - 1; i++ )
   {      
      Box3F box;
      edge = &mEdges[i];
      nextEdge = &mEdges[i+1];

      box.minExtents = edge->p1;
      box.maxExtents = edge->p1;
      box.extend( edge->p0 );
      box.extend( edge->p2 );
      box.extend( nextEdge->p0 );
      box.extend( nextEdge->p1 );
      box.extend( nextEdge->p2 );
      box.minExtents.z -= 5.0f;
      box.maxExtents.z += 5.0f;

      sphere.center = ( nextEdge->p1 + edge->p1 ) * 0.5f;
      sphere.radius = 100.0f; // NOTE: no idea how to calculate this

      ClippedPolyList clipper;
      clipper.mNormal.set(0.0f, 0.0f, 0.0f);
      VectorF n;
      PlaneF plane0, plane1;

      // Construct Back Plane
      n = edge->p2 - edge->p0;
      n.normalize();
      n = mCross( n, edge->uvec );      
      plane0.set( edge->p0, n );   
      clipper.mPlaneList.push_back( plane0 );

      // Construct Front Plane
      n = nextEdge->p2 - nextEdge->p0;
      n.normalize();
      n = -mCross( edge->uvec, n );
      plane1.set( nextEdge->p0, -n );
      //clipper.mPlaneList.push_back( plane1 );

      // Test if / where the planes intersect.
      bool discardLeft = false;
      bool discardRight = false;
      Point3F iPos; 
      VectorF iDir;

      if ( plane0.intersect( plane1, iPos, iDir ) )
      {                  
         Point2F iPos2F( iPos.x, iPos.y );
         Point2F cPos2F( edge->p1.x, edge->p1.y );
         Point2F rVec2F( edge->rvec.x, edge->rvec.y );

         Point2F iVec2F = iPos2F - cPos2F;
         F32 iLen = iVec2F.len();
         iVec2F.normalize();

         if ( iLen < edge->width * 0.5f )
         {
            F32 dot = mDot( rVec2F, iVec2F );

            // The clipping planes intersected on the right side,
            // discard the right side clipping plane.
            if ( dot > 0.0f )
               discardRight = true;         
            // The clipping planes intersected on the left side,
            // discard the left side clipping plane.
            else
               discardLeft = true;
         }
      }

      // Left Plane
      if ( !discardLeft )
      {
         n = ( nextEdge->p0 - edge->p0 );
         n.normalize();
         n = mCross( edge->uvec, n );
         clipper.mPlaneList.push_back( PlaneF(edge->p0, n) );
      }
      else
      {
         nextEdge->p0 = edge->p0;         
      }

      // Right Plane
      if ( !discardRight )
      {
         n = ( nextEdge->p2 - edge->p2 );
         n.normalize();            
         n = -mCross( n, edge->uvec );
         clipper.mPlaneList.push_back( PlaneF(edge->p2, -n) );
      }
      else
      {
         nextEdge->p2 = edge->p2;         
      }

      n = nextEdge->p2 - nextEdge->p0;
      n.normalize();
      n = -mCross( edge->uvec, n );
      plane1.set( nextEdge->p0, -n );
      clipper.mPlaneList.push_back( plane1 );

      // We have constructed the clipping planes,
      // now grab/clip the terrain geometry
      getContainer()->buildPolyList( PLC_Decal, box, TerrainObjectType, &clipper );         
      clipper.cullUnusedVerts();
      clipper.triangulate();
      clipper.generateNormals();

      // If we got something, add it to the ClippedPolyList Vector
      if ( !clipper.isEmpty() && !( smDiscardAll && ( discardRight || discardLeft ) ) )
      {
         clipperList.push_back( clipper );       
      
         mVertCount += clipper.mVertexList.size();
         mTriangleCount += clipper.mPolyList.size();
      }
   }

   //
   // Set the roadEdge height to be flush with terrain
   // This is not really necessary but makes the debug spline rendering better.
   //
   for ( U32 i = 0; i < mEdges.size() - 1; i++ )
   {            
      edge = &mEdges[i];      
      
      _getTerrainHeight( edge->p0.x, edge->p0.y, edge->p0.z );

      _getTerrainHeight( edge->p2.x, edge->p2.y, edge->p2.z );
   }

   //
   // Allocate the RoadBatch(s)   
   //

   // If we captured no verts, then we can return here without
   // allocating any RoadBatches or the Vert/Index Buffers.
   // PreprenderImage will not allocate a render instance while
   // mBatches.size() is zero.
   U32 numClippers = clipperList.size();
   if ( numClippers == 0 )
      return;

   mBatches.clear();

   // Allocate the VertexBuffer and PrimitiveBuffer
   mVB.set( GFX, mVertCount, GFXBufferTypeStatic );
   mPB.set( GFX, mTriangleCount * 3, 0, GFXBufferTypeStatic );

   // Lock the VertexBuffer
   GFXVertexPNTBT *vertPtr = mVB.lock();
   if(!vertPtr) return;
   U32 vertIdx = 0;

   //
   // Fill the VertexBuffer and vertex data for the RoadBatches
   // Loop through the ClippedPolyList Vector   
   //
   RoadBatch *batch = NULL;
   F32 texStart = 0.0f;
   F32 texEnd;
   
   for ( U32 i = 0; i < clipperList.size(); i++ )
   {   
      ClippedPolyList *clipper = &clipperList[i];
      RoadEdge &edge = mEdges[i];
      RoadEdge &nextEdge = mEdges[i+1];      

      VectorF segFvec = nextEdge.p1 - edge.p1;
      F32 segLen = segFvec.len();
      segFvec.normalize();

      F32 texLen = segLen / mTextureLength;
      texEnd = texStart + texLen;

      BiQuadToSqr quadToSquare(  Point2F( edge.p0.x, edge.p0.y ), 
                                 Point2F( edge.p2.x, edge.p2.y ), 
                                 Point2F( nextEdge.p2.x, nextEdge.p2.y ), 
                                 Point2F( nextEdge.p0.x, nextEdge.p0.y ) );  

      //
      if ( i % mSegmentsPerBatch == 0 )
      {
         mBatches.increment();
         batch = &mBatches.last();

         batch->bounds.minExtents = clipper->mVertexList[0].point;
         batch->bounds.maxExtents = clipper->mVertexList[0].point;
         batch->startVert = vertIdx;
      }

      // Loop through each ClippedPolyList        
      for ( U32 j = 0; j < clipper->mVertexList.size(); j++ )
      {
         // Add each vert to the VertexBuffer
         Point3F pos = clipper->mVertexList[j].point;
         vertPtr[vertIdx].point = pos;
         vertPtr[vertIdx].normal = clipper->mNormalList[j];                     
                  
         Point2F uv = quadToSquare.transform( Point2F(pos.x,pos.y) );
         vertPtr[vertIdx].texCoord.x = uv.x;
         vertPtr[vertIdx].texCoord.y = -(( texEnd - texStart ) * uv.y + texStart);   

         vertPtr[vertIdx].tangent = mCross( segFvec, clipper->mNormalList[j] );
         vertPtr[vertIdx].binormal = segFvec;

         vertIdx++;

         // Expand the RoadBatch bounds to contain this vertex   
         batch->bounds.extend( pos );
      }    

      batch->endVert = vertIdx - 1;

      texStart = texEnd;
   }   

   // Unlock the VertexBuffer, we are done filling it.
   mVB.unlock();

   // Lock the PrimitiveBuffer
   U16 *idxBuff;
   mPB.lock(&idxBuff);     
   U32 curIdx = 0;
   U16 vertOffset = 0;   
   batch = NULL;
   S32 batchIdx = -1;

   // Fill the PrimitiveBuffer   
   // Loop through each ClippedPolyList in the Vector
   for ( U32 i = 0; i < clipperList.size(); i++ )
   {      
      ClippedPolyList *clipper = &clipperList[i];

      if ( i % mSegmentsPerBatch == 0 )
      {
         batchIdx++;
         batch = &mBatches[batchIdx];
         batch->startIndex = curIdx;         
      }        

      for ( U32 j = 0; j < clipper->mPolyList.size(); j++ )
      {
         // Write indices for each Poly
         ClippedPolyList::Poly *poly = &clipper->mPolyList[j];                  

         AssertFatal( poly->vertexCount == 3, "Got non-triangle poly!" );

         idxBuff[curIdx] = clipper->mIndexList[poly->vertexStart] + vertOffset;         
         curIdx++;
         idxBuff[curIdx] = clipper->mIndexList[poly->vertexStart + 1] + vertOffset;            
         curIdx++;
         idxBuff[curIdx] = clipper->mIndexList[poly->vertexStart + 2] + vertOffset;                
         curIdx++;
      } 

      batch->endIndex = curIdx - 1;

      vertOffset += clipper->mVertexList.size();
   }

   // Unlock the PrimitiveBuffer, we are done filling it.
   mPB.unlock();

   // Generate the object/world bounds
   // Is the union of all batch bounding boxes.

   Box3F box;
   for ( U32 i = 0; i < mBatches.size(); i++ )
   {
      const RoadBatch &batch = mBatches[i];

      if ( i == 0 )      
         box = batch.bounds;               
      else      
         box.intersect( batch.bounds );               
   }

   Point3F pos = getPosition();

   mWorldBox = box;
   resetObjectBox();

   // Make sure we are in the correct bins given our world box.
   if( getSceneManager() != NULL )
      getSceneManager()->notifyObjectDirty( this );
}

U32 DecalRoad::_addNode( const Point3F &pos, F32 width )
{
   mNodes.increment();
   RoadNode &node = mNodes.last();

   node.point = pos;
   node.width = width;

   return mNodes.size() - 1;
}

U32 DecalRoad::_insertNode( const Point3F &pos, const F32 &width, const U32 &idx )
{
   U32 ret;
   RoadNode *node;

   if ( idx == U32_MAX )
   {
      mNodes.increment();
      node = &mNodes.last();
      ret = mNodes.size() - 1;
   }
   else
   {
      mNodes.insert( idx );
      node = &mNodes[idx];
      ret = idx;
   }

   node->point = pos;
   //node->t = -1.0f;
   //node->rot.identity();   
   node->width = width;     

   return ret;
}

bool DecalRoad::_getTerrainHeight( Point3F &pos )
{
   return _getTerrainHeight( pos.x, pos.y, pos.z );
}

bool DecalRoad::_getTerrainHeight( const Point2F &pos, F32 &height )
{     
   return _getTerrainHeight( pos.x, pos.y, height );
}

bool DecalRoad::_getTerrainHeight( const F32 &x, const F32 &y, F32 &height )
{
   Point3F startPnt( x, y, 10000.0f );
   Point3F endPnt( x, y, -10000.0f );

   RayInfo ri;
   bool hit;         

   hit = getContainer()->castRay(startPnt, endPnt, TerrainObjectType, &ri);   

   if ( hit )
      height = ri.point.z;

   return hit;
}

void DecalRoad::_onTerrainChanged( U32 type, TerrainBlock* tblock, const Point2I &min, const Point2I &max )
{
   // The client side object just stores the area that has changed
   // and waits for the (delayed) update event from the server
   // to actually perform the update.
   if ( isClientObject() && tblock->isClientObject() )
   {
      // Convert the min and max into world space.
      const F32 size = tblock->getSquareSize();
      const Point3F pos = tblock->getPosition();

      // TODO: I don't think this works right with tiling!
      Box3F dirty( F32( min.x * size ) + pos.x, F32( min.y * size ) + pos.y, -F32_MAX,
         F32( max.x * size ) + pos.x, F32( max.y * size ) + pos.y, F32_MAX );

      if ( !mTerrainUpdateRect.isValidBox() )
         mTerrainUpdateRect = dirty;
      else
         mTerrainUpdateRect.intersect( dirty );
   }
   // The server object only updates edges (doesn't clip to geometry)
   // and schedules an update to be sent to the client.
   else if ( isServerObject() && tblock->isServerObject() )
   {
      //_generateEdges();
      scheduleUpdate( TerrainChangedMask );
   }
}


// Static protected field set methods

bool DecalRoad::ptSetBreakAngle( void *object, const char *index, const char *data )
{
   DecalRoad *road = static_cast<DecalRoad*>( object );
   F32 val = dAtof( data );

   road->setBreakAngle( val );

   // we already set the field
   return false;
}

bool DecalRoad::ptSetTextureLength( void *object, const char *index, const char *data )
{
   DecalRoad *road = static_cast<DecalRoad*>( object );
   F32 val = dAtof( data );

   road->setTextureLength( val );

   // we already set the field
   return false;
}


// ConsoleMethods

DefineEngineMethod( DecalRoad, regenerate, void, (),,
                   "Intended as a helper to developers and editor scripts.\n"
                   "Force DecalRoad to update it's spline and reclip geometry."
                  )
{
   object->regenerate();
}

DefineEngineMethod( DecalRoad, postApply, void, (),,
                   "Intended as a helper to developers and editor scripts.\n"
                   "Force trigger an inspectPostApply. This will transmit "
                   "the material and other fields ( not including nodes ) "
                   "to client objects."
                  )
{
   object->inspectPostApply();
}
