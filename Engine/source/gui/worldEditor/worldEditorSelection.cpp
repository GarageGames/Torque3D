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

#include "gui/worldEditor/worldEditorSelection.h"
#include "gui/worldEditor/worldEditor.h"
#include "scene/sceneObject.h"


IMPLEMENT_CONOBJECT( WorldEditorSelection );

ConsoleDocClass( WorldEditorSelection,
   "@brief Specialized simset that stores the objects selected by the World Editor\n\n"
   "Editor use only.\n\n"
   "@internal"
);

//-----------------------------------------------------------------------------

WorldEditorSelection::WorldEditorSelection()
   :  mCentroidValid(false),
      mAutoSelect(false),
      mPrevCentroid(0.0f, 0.0f, 0.0f),
      mContainsGlobalBounds(false)
{
   // Selections are transient by default.
   setCanSave( false );
   setEditorOnly( true );
}

//-----------------------------------------------------------------------------

WorldEditorSelection::~WorldEditorSelection()
{
}

//-----------------------------------------------------------------------------

void WorldEditorSelection::initPersistFields()
{
   Parent::initPersistFields();
}

//-----------------------------------------------------------------------------

void WorldEditorSelection::setCanSave( bool value )
{
   if( getCanSave() == value )
      return;
      
   Parent::setCanSave( value );
   
   // If we went from being transient to being persistent,
   // make sure all objects in the selection have persistent IDs.
   
   if( getCanSave() )
      for( iterator iter = begin(); iter != end(); ++ iter )
         ( *iter )->getOrCreatePersistentId();
}

//-----------------------------------------------------------------------------

bool WorldEditorSelection::objInSet( SimObject* obj )
{
   if( !mIsResolvingPIDs )
      resolvePIDs();
      
   lock();
      
   bool result = false;
   for( iterator iter = begin(); iter != end(); ++ iter )
   {
      if( obj == *iter )
      {
         result = true;
         break;
      }
         
      WorldEditorSelection* set = dynamic_cast< WorldEditorSelection* >( *iter );
      if( set && set->objInSet( obj ) )
      {
         result = true;
         break;
      }
   }
   
   unlock();
   
   return result;
}

//-----------------------------------------------------------------------------

void WorldEditorSelection::addObject( SimObject* obj )
{
   // Return if object is already in selection.
   
   if( objInSet( obj ) )
      return;
      
   // Refuse to add object if this selection is locked.
      
   if( isLocked() )
      return;
      
   // Prevent adding us to ourselves.
      
   if( obj == this )
      return;
      
   // If the object is itself a selection set, make sure we
   // don't create a cycle.
   
   WorldEditorSelection* selection = dynamic_cast< WorldEditorSelection* >( obj );
   if( selection && !selection->objInSet( this ) )
      return;
      
   // Refuse to add any of our parents.
   
   for( SimGroup* group = getGroup(); group != NULL; group = group->getGroup() )
      if( obj == group )
         return;
      
   invalidateCentroid();
   
   Parent::addObject( obj );
      
   if( mAutoSelect )
      WorldEditor::markAsSelected( obj, true );

   return;
}

//-----------------------------------------------------------------------------

void WorldEditorSelection::removeObject( SimObject* obj )
{
   if( !objInSet( obj ) )
      return;
      
   // Refuse to remove object if this selection is locked.
      
   if( isLocked() )
      return;

   invalidateCentroid();
   
   Parent::removeObject( obj );

   if( mAutoSelect )
      WorldEditor::markAsSelected( obj, false );

   return;
}

//-----------------------------------------------------------------------------

bool WorldEditorSelection::containsGlobalBounds()
{
   updateCentroid();
   return mContainsGlobalBounds;
}

//-----------------------------------------------------------------------------

void WorldEditorSelection::updateCentroid()
{
   if( mCentroidValid )
      return;
      
   resolvePIDs();

   mCentroidValid = true;

   mCentroid.set(0,0,0);
   mBoxCentroid = mCentroid;
   mBoxBounds.minExtents.set(1e10, 1e10, 1e10);
   mBoxBounds.maxExtents.set(-1e10, -1e10, -1e10);

   mContainsGlobalBounds = false;

   if( empty() )
      return;

   //
   for( SimSet::iterator iter = begin(); iter != end(); ++ iter )
   {
      SceneObject* obj = dynamic_cast<SceneObject*>( *iter );
      if( !obj )
         continue;

      const MatrixF & mat = obj->getTransform();
      Point3F wPos;
      mat.getColumn(3, &wPos);

      //
      mCentroid += wPos;

      //
      const Box3F& bounds = obj->getWorldBox();
      mBoxBounds.minExtents.setMin(bounds.minExtents);
      mBoxBounds.maxExtents.setMax(bounds.maxExtents);

      if(obj->isGlobalBounds())
         mContainsGlobalBounds = true;
   }

   mCentroid /= (F32) size();
   mBoxCentroid = mBoxBounds.getCenter();
}

//-----------------------------------------------------------------------------

const Point3F & WorldEditorSelection::getCentroid()
{
   updateCentroid();
   return(mCentroid);
}

//-----------------------------------------------------------------------------

const Point3F & WorldEditorSelection::getBoxCentroid()
{
   updateCentroid();
   return(mBoxCentroid);
}

//-----------------------------------------------------------------------------

const Box3F & WorldEditorSelection::getBoxBounds()
{
   updateCentroid();
   return(mBoxBounds);
}

//-----------------------------------------------------------------------------

Point3F WorldEditorSelection::getBoxBottomCenter()
{
   updateCentroid();
   
   Point3F bottomCenter = mBoxCentroid;
   bottomCenter.z -= mBoxBounds.len_z() * 0.5f;
   
   return bottomCenter;
}

//-----------------------------------------------------------------------------

void WorldEditorSelection::enableCollision()
{
   for( iterator iter = begin(); iter != end(); ++ iter )
   {
      SceneObject* object = dynamic_cast<SceneObject*>( *iter );
      if( object )
         object->enableCollision();
   }
}

//-----------------------------------------------------------------------------

void WorldEditorSelection::disableCollision()
{
   for( iterator iter = begin(); iter != end(); ++ iter )
   {
      SceneObject* object = dynamic_cast< SceneObject* >( *iter );
      if( object )
         object->disableCollision();
   }
}

//-----------------------------------------------------------------------------

void WorldEditorSelection::offset( const Point3F& offset, F32 gridSnap )
{
   for( iterator iter = begin(); iter != end(); ++ iter )
   {
      SceneObject* obj = dynamic_cast<SceneObject*>( *iter );
      if( !obj )
         continue;

      MatrixF mat = obj->getTransform();
      Point3F wPos;
      mat.getColumn(3, &wPos);

      // adjust
      wPos += offset;
      
      if( gridSnap != 0.f )
      {
         wPos.x -= mFmod( wPos.x, gridSnap );
         wPos.y -= mFmod( wPos.y, gridSnap );
         wPos.z -= mFmod( wPos.z, gridSnap );
      }
      
      mat.setColumn(3, wPos);
      obj->setTransform(mat);
   }

   mCentroidValid = false;
}

//-----------------------------------------------------------------------------

void WorldEditorSelection::setPosition(const Point3F & pos)
{
   for( iterator iter = begin(); iter != end(); ++ iter )
   {
      SceneObject* object = dynamic_cast<SceneObject*>( *iter );
      if( object )
         object->setPosition(pos);
   }

   mCentroidValid = false;
}

//-----------------------------------------------------------------------------

void WorldEditorSelection::setCentroidPosition(bool useBoxCenter, const Point3F & pos)
{
   Point3F centroid;
   if( containsGlobalBounds() )
   {
      centroid = getCentroid();
   }
   else
   {
      centroid = useBoxCenter ? getBoxCentroid() : getCentroid();
   }

   offset(pos - centroid);
}

//-----------------------------------------------------------------------------

void WorldEditorSelection::orient(const MatrixF & rot, const Point3F & center)
{
   // Orient all the selected objects to the given rotation
   for( iterator iter = begin(); iter != end(); ++ iter )
   {
      SceneObject* object = dynamic_cast< SceneObject* >( *iter );
      if( !object )
         continue;
         
      MatrixF mat = rot;
      mat.setPosition( object->getPosition() );
      object->setTransform(mat);
   }

   mCentroidValid = false;
}

//-----------------------------------------------------------------------------

void WorldEditorSelection::rotate(const EulerF &rot)
{
   for( iterator iter = begin(); iter != end(); ++ iter )
   {
      SceneObject* object = dynamic_cast< SceneObject* >( *iter );
      if( !object )
         continue;

         MatrixF mat = object->getTransform();

         MatrixF transform(rot);
         mat.mul(transform);

         object->setTransform(mat);
   }
}

//-----------------------------------------------------------------------------

void WorldEditorSelection::rotate(const EulerF & rot, const Point3F & center)
{
   // single selections will rotate around own axis, multiple about world
   if(size() == 1)
   {
      SceneObject* object = dynamic_cast< SceneObject* >( at( 0 ) );
      if( object )
      {
         MatrixF mat = object->getTransform();

         Point3F pos;
         mat.getColumn(3, &pos);

         // get offset in obj space
         Point3F offset = pos - center;
         MatrixF wMat = object->getWorldTransform();
         wMat.mulV(offset);

         //
         MatrixF transform(EulerF(0,0,0), -offset);
         transform.mul(MatrixF(rot));
         transform.mul(MatrixF(EulerF(0,0,0), offset));
         mat.mul(transform);

         object->setTransform(mat);
      }
   }
   else
   {
      for( iterator iter = begin(); iter != end(); ++ iter )
      {
         SceneObject* object = dynamic_cast< SceneObject* >( *iter );
         if( !object )
            continue;
            
         MatrixF mat = object->getTransform();

         Point3F pos;
         mat.getColumn(3, &pos);

         // get offset in obj space
         Point3F offset = pos - center;

         MatrixF transform(rot);
         Point3F wOffset;
         transform.mulV(offset, &wOffset);

         MatrixF wMat = object->getWorldTransform();
         wMat.mulV(offset);

         //
         transform.set(EulerF(0,0,0), -offset);

         mat.setColumn(3, Point3F(0,0,0));
         wMat.setColumn(3, Point3F(0,0,0));

         transform.mul(wMat);
         transform.mul(MatrixF(rot));
         transform.mul(mat);
         mat.mul(transform);

         mat.normalize();
         mat.setColumn(3, wOffset + center);

         object->setTransform(mat);
      }
   }

   mCentroidValid = false;
}

//-----------------------------------------------------------------------------

void WorldEditorSelection::setRotate(const EulerF & rot)
{
   for( iterator iter = begin(); iter != end(); ++ iter )
   {
      SceneObject* object = dynamic_cast< SceneObject* >( *iter );
      if( !object )
         continue;

      MatrixF mat = object->getTransform();
      Point3F pos;
      mat.getColumn(3, &pos);

      MatrixF rmat(rot);
      rmat.setPosition(pos);

      object->setTransform(rmat);
   }
}

//-----------------------------------------------------------------------------

void WorldEditorSelection::scale(const VectorF & scale)
{
   for( iterator iter = begin(); iter != end(); ++ iter )
   {
      SceneObject* object = dynamic_cast< SceneObject* >( *iter );
      if( !object )
         continue;
         
      VectorF current = object->getScale();
      current.convolve(scale);

      // clamp scale to sensible limits
      current.setMax( Point3F( 0.01f ) );
      current.setMin( Point3F( 1000.0f ) );

      object->setScale(current);
   }

   mCentroidValid = false;
}

//-----------------------------------------------------------------------------

void WorldEditorSelection::scale(const VectorF & scale, const Point3F & center)
{
   for( iterator iter = begin(); iter != end(); ++ iter )
   {
      SceneObject* object = dynamic_cast< SceneObject* >( *iter );
      if( !object )
         continue;

      VectorF current = object->getScale();
      current.convolve(scale);

      // clamp scale to sensible limits
      current.setMax( Point3F( 0.01f ) );
      current.setMin( Point3F( 1000.0f ) );

      // Apply the scale first.  If the object's scale doesn't change with
      // this operation then this object doesn't scale.  In this case
      // we don't want to continue with the offset operation.
      VectorF prevScale = object->getScale();
      object->setScale(current);
      if( !object->getScale().equal(current) )
         continue;

      // determine the actual scale factor to apply to the object offset
      // need to account for the scale limiting above to prevent offsets
      // being reduced to 0 which then cannot be restored by unscaling
      VectorF adjustedScale = current / prevScale;

      MatrixF mat = object->getTransform();

      Point3F pos;
      mat.getColumn(3, &pos);

      Point3F offset = pos - center;
      offset *= adjustedScale;

      object->setPosition(offset + center);
   }
}

//-----------------------------------------------------------------------------

void WorldEditorSelection::setScale(const VectorF & scale)
{
   for( iterator iter = begin(); iter != end(); ++ iter )
   {
      SceneObject* object = dynamic_cast< SceneObject* >( *iter );
      if( object )
         object->setScale( scale );
   }

   mCentroidValid = false;
}

//-----------------------------------------------------------------------------

void WorldEditorSelection::setScale(const VectorF & scale, const Point3F & center)
{
   for( iterator iter = begin(); iter != end(); ++ iter )
   {
      SceneObject* object = dynamic_cast< SceneObject* >( *iter );
      if( !object )
         continue;
         
      MatrixF mat = object->getTransform();

      Point3F pos;
      mat.getColumn(3, &pos);

      Point3F offset = pos - center;
      offset *= scale;

      object->setPosition(offset + center);
      object->setScale(scale);
   }
}

//-----------------------------------------------------------------------------

void WorldEditorSelection::addSize(const VectorF & newsize)
{
   for( iterator iter = begin(); iter != end(); ++ iter )
   {
      SceneObject* object = dynamic_cast< SceneObject* >( *iter );
      if( !object )
         continue;

      if( object->isGlobalBounds() )
         continue;

      const Box3F& bounds = object->getObjBox();
      VectorF extent = bounds.getExtents();
      VectorF scaledextent = object->getScale() * extent;

      VectorF scale = (newsize + scaledextent) / scaledextent;
      object->setScale( object->getScale() * scale );
   }
}

//-----------------------------------------------------------------------------

void WorldEditorSelection::setSize(const VectorF & newsize)
{
   for( iterator iter = begin(); iter != end(); ++ iter )
   {
      SceneObject* object = dynamic_cast< SceneObject* >( *iter );
      if( !object )
         continue;

      if( object->isGlobalBounds() )
         continue;

      const Box3F& bounds = object->getObjBox();
      VectorF extent = bounds.getExtents();

      VectorF scale = newsize / extent;
      object->setScale( scale );
   }
}

//=============================================================================
//    Console Methods.
//=============================================================================
// MARK: ---- Console Methods ----

//-----------------------------------------------------------------------------

ConsoleMethod( WorldEditorSelection, containsGlobalBounds, bool, 2, 2, "() - True if an object with global bounds is contained in the selection." )
{
   return object->containsGlobalBounds();
}

//-----------------------------------------------------------------------------

ConsoleMethod( WorldEditorSelection, getCentroid, const char*, 2, 2, "() - Return the median of all object positions in the selection." )
{
   char* buffer = Con::getReturnBuffer( 256 );
   const Point3F& centroid = object->getCentroid();
   
   dSprintf( buffer, 256, "%g %g %g", centroid.x, centroid.y, centroid.z );
   return buffer;
}

//-----------------------------------------------------------------------------

ConsoleMethod( WorldEditorSelection, getBoxCentroid, const char*, 2, 2, "() - Return the center of the bounding box around the selection." )
{
   char* buffer = Con::getReturnBuffer( 256 );
   const Point3F& boxCentroid = object->getBoxCentroid();
   
   dSprintf( buffer, 256, "%g %g %g", boxCentroid.x, boxCentroid.y, boxCentroid.z );
   return buffer;
}

//-----------------------------------------------------------------------------

ConsoleMethod( WorldEditorSelection, offset, void, 3, 4, "( vector delta, float gridSnap=0 ) - Move all objects in the selection by the given delta." )
{
   F32 x, y, z;
   dSscanf( argv[ 3 ], "%g %g %g", &x, &y, &z );
   
   F32 gridSnap = 0.f;
   if( argc > 3 )
      gridSnap = dAtof( argv[ 3 ] );
      
   object->offset( Point3F( x, y, z ), gridSnap );
   WorldEditor::updateClientTransforms( object );
}

//-----------------------------------------------------------------------------

ConsoleMethod( WorldEditorSelection, union, void, 3, 3, "( SimSet set ) - Add all objects in the given set to this selection." )
{
   SimSet* selection;
   if( !Sim::findObject( argv[ 2 ], selection ) )
   {
      Con::errorf( "WorldEditorSelection::union - no SimSet '%s'", argv[ 2 ] );
      return;
   }
   
   const U32 numObjects = selection->size();
   for( U32 i = 0; i < numObjects; ++ i )
      object->addObject( selection->at( i ) );
}

//-----------------------------------------------------------------------------

ConsoleMethod( WorldEditorSelection, subtract, void, 3, 3, "( SimSet ) - Remove all objects in the given set from this selection." )
{
   SimSet* selection;
   if( !Sim::findObject( argv[ 2 ], selection ) )
   {
      Con::errorf( "WorldEditorSelection::subtract - no SimSet '%s'", argv[ 2 ] );
      return;
   }
   
   const U32 numObjects = selection->size();
   for( U32 i = 0; i < numObjects; ++ i )
      object->removeObject( selection->at( i ) );
}
