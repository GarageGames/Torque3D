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

#include "console/engineAPI.h"
#include "platform/platform.h"
#include "gui/editor/inspector/field.h"
#include "gui/buttons/guiIconButtonCtrl.h"
#include "gui/editor/guiInspector.h"
#include "core/util/safeDelete.h"
#include "gfx/gfxDrawUtil.h"
#include "math/mathTypes.h"
#include "core/strings/stringUnit.h"


IMPLEMENT_CONOBJECT(GuiInspectorField);

ConsoleDocClass( GuiInspectorField,
   "@brief The GuiInspectorField control is a representation of a single abstract "
   "field for a given ConsoleObject derived object.\n\n"
   "Editor use only.\n\n"
   "@internal"
);

//-----------------------------------------------------------------------------

GuiInspectorField::GuiInspectorField( GuiInspector* inspector,
                                      GuiInspectorGroup* parent, 
                                      AbstractClassRep::Field* field ) 
 : mInspector( inspector ),
   mParent( parent ), 
   mField( field ), 
   mFieldArrayIndex( NULL ), 
   mEdit( NULL )
{
   if( field != NULL )
      mCaption    = field->pFieldname;
   else
      mCaption    = StringTable->EmptyString();

   setCanSave( false );
   setBounds(0,0,100,18);
   
   if( field != NULL )
      _setFieldDocs( field->pFieldDocs );
}

//-----------------------------------------------------------------------------

GuiInspectorField::GuiInspectorField() 
 : mInspector( NULL ),
   mParent( NULL ), 
   mEdit( NULL ),
   mField( NULL ), 
   mFieldArrayIndex( NULL ),
   mCaption( StringTable->EmptyString() ),
   mHighlighted( false )
{
   setCanSave( false );
}

//-----------------------------------------------------------------------------

GuiInspectorField::~GuiInspectorField()
{
}

//-----------------------------------------------------------------------------

void GuiInspectorField::init( GuiInspector *inspector, GuiInspectorGroup *group )
{   
   mInspector = inspector;
   mParent = group;
}

//-----------------------------------------------------------------------------

bool GuiInspectorField::onAdd()
{   
   setInspectorProfile();   

   if ( !Parent::onAdd() )
      return false;

   if ( !mInspector )
      return false;   

   mEdit = constructEditControl();
   if ( mEdit == NULL )
      return false;

   setBounds(0,0,100,18);

   // Add our edit as a child
   addObject( mEdit );

   // Calculate Caption and EditCtrl Rects
   updateRects();   

   // Force our editField to set it's value
   updateValue();

   return true;
}

//-----------------------------------------------------------------------------

bool GuiInspectorField::resize( const Point2I &newPosition, const Point2I &newExtent )
{
   if ( !Parent::resize( newPosition, newExtent ) )
      return false;

   return updateRects();
}

//-----------------------------------------------------------------------------

void GuiInspectorField::onRender( Point2I offset, const RectI &updateRect )
{
   RectI ctrlRect(offset, getExtent());
   
   // Render fillcolor...
   if ( mProfile->mOpaque )
      GFX->getDrawUtil()->drawRectFill(ctrlRect, mProfile->mFillColor);   

   // Render caption...
   if ( mCaption && mCaption[0] )
   {      
      // Backup current ClipRect
      RectI clipBackup = GFX->getClipRect();

      RectI clipRect = updateRect;

      // The rect within this control in which our caption must fit.
      RectI rect( offset + mCaptionRect.point + mProfile->mTextOffset, mCaptionRect.extent + Point2I(1,1) - Point2I(5,0) );

      // Now clipRect is the amount of our caption rect that is actually visible.
      bool hit = clipRect.intersect( rect );

      if ( hit )
      {
         GFX->setClipRect( clipRect );
         GFXDrawUtil *drawer = GFX->getDrawUtil();

         // Backup modulation color
         ColorI currColor;
         drawer->getBitmapModulation( &currColor );

         // Draw caption background...
         if( !isActive() )
            GFX->getDrawUtil()->drawRectFill( clipRect, mProfile->mFillColorNA );
         else if ( mHighlighted )         
            GFX->getDrawUtil()->drawRectFill( clipRect, mProfile->mFillColorHL );             

         // Draw caption text...

         drawer->setBitmapModulation( !isActive() ? mProfile->mFontColorNA : mHighlighted ? mProfile->mFontColorHL : mProfile->mFontColor );
         
         // Clip text with '...' if too long to fit
         String clippedText( mCaption );
         clipText( clippedText, clipRect.extent.x );

         renderJustifiedText( offset + mProfile->mTextOffset, getExtent(), clippedText );

         // Restore modulation color
         drawer->setBitmapModulation( currColor );

         // Restore previous ClipRect
         GFX->setClipRect( clipBackup );
      }
   }

   // Render Children...
   renderChildControls(offset, updateRect);

   // Render border...
   if ( mProfile->mBorder )
      renderBorder(ctrlRect, mProfile);   

   // Render divider...
   Point2I worldPnt = mEditCtrlRect.point + offset;
   GFX->getDrawUtil()->drawLine( worldPnt.x - 5,
      worldPnt.y, 
      worldPnt.x - 5,
      worldPnt.y + getHeight(),
      !isActive() ? mProfile->mBorderColorNA : mHighlighted ? mProfile->mBorderColorHL : mProfile->mBorderColor );
}

//-----------------------------------------------------------------------------

void GuiInspectorField::setFirstResponder( GuiControl *firstResponder )
{
   Parent::setFirstResponder( firstResponder );

   if ( firstResponder == this || firstResponder == mEdit )
   {
      mInspector->setHighlightField( this );      
   }   
}

//-----------------------------------------------------------------------------

void GuiInspectorField::onMouseDown( const GuiEvent &event )
{
   if ( mCaptionRect.pointInRect( globalToLocalCoord( event.mousePoint ) ) )  
   {
      if ( mEdit )
         //mEdit->onMouseDown( event );
         mInspector->setHighlightField( this );
   }
   else
      Parent::onMouseDown( event );
}

//-----------------------------------------------------------------------------

void GuiInspectorField::onRightMouseUp( const GuiEvent &event )
{
   if ( mCaptionRect.pointInRect( globalToLocalCoord( event.mousePoint ) ) ) 
      Con::executef( mInspector, "onFieldRightClick", getIdString() );
   else
      Parent::onMouseDown( event );
}

//-----------------------------------------------------------------------------

void GuiInspectorField::setData( const char* data, bool callbacks )
{
   if( mField == NULL )
      return;

   if( verifyData( data ) )
   {
      String strData = data;
      const U32 numTargets = mInspector->getNumInspectObjects();
      
      if( callbacks && numTargets > 1 )
         Con::executef( mInspector, "onBeginCompoundEdit" );
            
      for( U32 i = 0; i < numTargets; ++ i )
      {
         SimObject* target = mInspector->getInspectObject( i );
         
         String oldValue = target->getDataField( mField->pFieldname, mFieldArrayIndex);
         
         // For numeric fields, allow input expressions.
         
         String newValue = strData;
         S32 type= mField->type;
         if( type == TypeS8 || type == TypeS32 || type == TypeF32 )
         {
            char buffer[ 2048 ];
            expandEscape( buffer, newValue );
            newValue = (const char*)Con::evaluatef( "%%f = \"%s\"; return ( %s );", oldValue.c_str(), buffer );
         }
         else if(    type == TypeS32Vector
                  || type == TypeF32Vector
                  || type == TypeColorI
                  || type == TypeColorF
                  || type == TypePoint2I
                  || type == TypePoint2F
                  || type == TypePoint3F
                  || type == TypePoint4F
                  || type == TypeRectI
                  || type == TypeRectF
                  || type == TypeMatrixPosition
                  || type == TypeMatrixRotation
                  || type == TypeBox3F
                  || type == TypeRectUV
                  || type == TypeRotationF)
         {
            //TODO: we should actually take strings into account and not chop things up between quotes

            U32 numNewUnits = StringUnit::getUnitCount( newValue, " \t\n\r" );
            
            StringBuilder strNew;
            bool isFirst = true;
            for( U32 n = 0; n < numNewUnits; ++ n )
            {
               char oldComponentVal[ 1024 ];
               StringUnit::getUnit( oldValue, n, " \t\n\r", oldComponentVal, sizeof( oldComponentVal ) );
               
               char newComponentExpr[ 1024 ];
               StringUnit::getUnit( newValue, n, " \t\n\r", newComponentExpr, sizeof( newComponentExpr ) );
               
               char buffer[ 2048 ];
               expandEscape( buffer, newComponentExpr );

               const char* newComponentVal = Con::evaluatef( "%%f = \"%s\"; %%v = \"%s\"; return ( %s );",
                  oldComponentVal, oldValue.c_str(), buffer );
               
               if( !isFirst )
                  strNew.append( ' ' );
               strNew.append( newComponentVal );
               
               isFirst = false;
            }
            
            newValue = strNew.end();
         }
            
         target->inspectPreApply();
         
         // Fire callback single-object undo.
         
         if( callbacks )
            Con::executef( mInspector, "onInspectorFieldModified", 
                                          target->getIdString(), 
                                          mField->pFieldname, 
                                          mFieldArrayIndex ? mFieldArrayIndex : "(null)", 
                                          oldValue.c_str(), 
                                          newValue.c_str() );

         target->setDataField( mField->pFieldname, mFieldArrayIndex, newValue );
         
         // Give the target a chance to validate.
         target->inspectPostApply();
      }
      
      if( callbacks && numTargets > 1 )
         Con::executef( mInspector, "onEndCompoundEdit" );
   }

   // Force our edit to update
   updateValue();
}

//-----------------------------------------------------------------------------

const char* GuiInspectorField::getData( U32 inspectObjectIndex )
{
   if( mField == NULL )
      return "";

   return mInspector->getInspectObject( inspectObjectIndex )->getDataField( mField->pFieldname, mFieldArrayIndex );
}

//-----------------------------------------------------------------------------

void GuiInspectorField::resetData()
{
   if( !mField )
      return;
      
   SimObject* inspectObject = getInspector()->getInspectObject();
   
   SimObject* tempObject = static_cast< SimObject* >( inspectObject->getClassRep()->create() );
   setData( tempObject->getDataField( mField->pFieldname, mFieldArrayIndex ) );
   delete tempObject;
}

//-----------------------------------------------------------------------------

void GuiInspectorField::setInspectorField( AbstractClassRep::Field *field, StringTableEntry caption, const char*arrayIndex ) 
{
   mField = field; 

   if ( arrayIndex != NULL )   
      mFieldArrayIndex = StringTable->insert( arrayIndex );

   if ( !caption || !caption[0] )
      mCaption = getFieldName(); 
   else
      mCaption = caption;

   if ( mField != NULL )
      _setFieldDocs( mField->pFieldDocs );
}

//-----------------------------------------------------------------------------

StringTableEntry GuiInspectorField::getRawFieldName()
{
   if( !mField )
      return StringTable->EmptyString();
      
   return mField->pFieldname;
}

//-----------------------------------------------------------------------------

StringTableEntry GuiInspectorField::getFieldName() 
{ 
   // Sanity
   if ( mField == NULL )
      return StringTable->EmptyString();

   // Array element?
   if( mFieldArrayIndex != NULL )
   {
      S32 frameTempSize = dStrlen( mField->pFieldname ) + 32;
      FrameTemp<char> valCopy( frameTempSize );
      dSprintf( (char *)valCopy, frameTempSize, "%s[%s]", mField->pFieldname, mFieldArrayIndex );

      // Return formatted element
      return StringTable->insert( valCopy );
   }

   // Plain field name.
   return mField->pFieldname;
}

//-----------------------------------------------------------------------------

StringTableEntry GuiInspectorField::getFieldType()
{
   if( !mField )
      return StringTable->EmptyString();
      
   return ConsoleBaseType::getType( mField->type )->getTypeName();
}

//-----------------------------------------------------------------------------

GuiControl* GuiInspectorField::constructEditControl()
{
   GuiControl* retCtrl = new GuiTextEditCtrl();

   static StringTableEntry sProfile = StringTable->insert( "profile" );
   retCtrl->setDataField( sProfile, NULL, "GuiInspectorTextEditProfile" );

   _registerEditControl( retCtrl );

   char szBuffer[512];
   dSprintf( szBuffer, 512, "%d.apply(%d.getText());",getId(), retCtrl->getId() );
   
   // Suffices to hook on to "validate" as regardless of whether we lose
   // focus through the user pressing enter or clicking away on another
   // keyboard control, we will see a validate call.
   
   retCtrl->setField("validate", szBuffer );

   return retCtrl;
}

//-----------------------------------------------------------------------------

void GuiInspectorField::setInspectorProfile()
{
   GuiControlProfile *profile = NULL;   
   
   if( mInspector && (mInspector->getNumInspectObjects() > 1) )
   {
      if( !hasSameValueInAllObjects() )
         Sim::findObject( "GuiInspectorMultiFieldDifferentProfile", profile );
      else
         Sim::findObject( "GuiInspectorMultiFieldProfile", profile );
   }
   
   if( !profile )
      Sim::findObject( "GuiInspectorFieldProfile", profile );
   
   if( profile )
      setControlProfile( profile );
}

//-----------------------------------------------------------------------------

void GuiInspectorField::setValue( StringTableEntry newValue )
{
   GuiTextEditCtrl *ctrl = dynamic_cast<GuiTextEditCtrl*>( mEdit );
   if( ctrl != NULL )
      ctrl->setText( newValue );
}

//-----------------------------------------------------------------------------

bool GuiInspectorField::updateRects()
{
   S32 dividerPos, dividerMargin;
   mInspector->getDivider( dividerPos, dividerMargin );   

   Point2I fieldExtent = getExtent();
   Point2I fieldPos = getPosition();

   S32 editWidth = dividerPos - dividerMargin;

   mEditCtrlRect.set( fieldExtent.x - dividerPos + dividerMargin, 1, editWidth, fieldExtent.y - 1 );
   mCaptionRect.set( 0, 0, fieldExtent.x - dividerPos - dividerMargin, fieldExtent.y );
   
   if ( !mEdit )
      return false;

   return mEdit->resize( mEditCtrlRect.point, mEditCtrlRect.extent );
}

//-----------------------------------------------------------------------------

void GuiInspectorField::updateValue()
{
   if( mInspector->getNumInspectObjects() > 1 )
   {
      setInspectorProfile();
         
      if( !hasSameValueInAllObjects() )
         setValue( StringTable->EmptyString() );
      else
         setValue( getData() );
   }
   else
      setValue( getData() );
}

//-----------------------------------------------------------------------------

void GuiInspectorField::setHLEnabled( bool enabled )
{
   mHighlighted = enabled;
   if ( mHighlighted )
   {
      if ( mEdit && !mEdit->isFirstResponder() )
      {
         mEdit->setFirstResponder();
         GuiTextEditCtrl *edit = dynamic_cast<GuiTextEditCtrl*>( mEdit );
         if ( edit )
         {
            mouseUnlock();
            edit->mouseLock();
            edit->setCursorPos(0);
         }
      }
      _executeSelectedCallback();
   }
}

//-----------------------------------------------------------------------------

bool GuiInspectorField::hasSameValueInAllObjects()
{
   char value1[ 2048 ];
   
   // Get field value from first object.
   
   const char* data1 = getData( 0 );
   if( data1 )
   {
      dStrncpy( value1, data1, sizeof( value1 ) );
      value1[ sizeof( value1 ) - 1 ] = 0;
   }
   else
      value1[ 0 ] = 0;
   
   // Check if all other objects have the same value.

   const U32 numObjects = mInspector->getNumInspectObjects();
   for( U32 i = 1; i < numObjects; ++ i )
   {
      const char* value2 = getData( i );
      if( !value2 )
         value2 = "";

      if( dStrcmp( value1, value2 ) != 0 )
         return false;
   }
         
   return true;
}

//-----------------------------------------------------------------------------

void GuiInspectorField::_executeSelectedCallback()
{
   if( mField )
      Con::executef( mInspector, "onFieldSelected", mField->pFieldname, ConsoleBaseType::getType(mField->type)->getTypeName(), mFieldDocs.c_str() );
}

//-----------------------------------------------------------------------------

void GuiInspectorField::_registerEditControl( GuiControl *ctrl )
{
   char szName[512];
   dSprintf( szName, 512, "IE_%s_%d_%s_Field", ctrl->getClassName(), mInspector->getInspectObject()->getId(), mCaption);

   // Register the object
   ctrl->registerObject( szName );
}

//-----------------------------------------------------------------------------

void GuiInspectorField::_setFieldDocs( StringTableEntry docs )
{
   mFieldDocs = String();
   if( docs && docs[ 0 ] )
   {
      // Only accept first line of docs for brevity.
      
      const char* newline = dStrchr( docs, '\n' );
      if( newline )
         mFieldDocs = String( docs, newline - docs );
      else
         mFieldDocs = docs;
   }
}

//=============================================================================
//    Console Methods.
//=============================================================================
// MARK: ---- Console Methods ----

//-----------------------------------------------------------------------------

DefineConsoleMethod( GuiInspectorField, getInspector, S32, (), , "() - Return the GuiInspector to which this field belongs." )
{
   return object->getInspector()->getId();
}

//-----------------------------------------------------------------------------

DefineConsoleMethod( GuiInspectorField, getInspectedFieldName, const char*, (), , "() - Return the name of the field edited by this inspector field." )
{
   return object->getFieldName();
}

//-----------------------------------------------------------------------------

DefineConsoleMethod( GuiInspectorField, getInspectedFieldType, const char*, (), , "() - Return the type of the field edited by this inspector field." )
{
   return object->getFieldType();
}

//-----------------------------------------------------------------------------

DefineConsoleMethod( GuiInspectorField, apply, void, ( const char * newValue, bool callbacks ), (true), "( string newValue, bool callbacks=true ) - Set the field's value. Suppress callbacks for undo if callbacks=false." )
{
   object->setData( newValue, callbacks );
}

//-----------------------------------------------------------------------------

DefineConsoleMethod( GuiInspectorField, applyWithoutUndo, void, (const char * data), , "() - Set field value without recording undo (same as 'apply( value, false )')." )
{
   object->setData( data, false );
}

//-----------------------------------------------------------------------------

DefineConsoleMethod( GuiInspectorField, getData, const char*, (), , "() - Return the value currently displayed on the field." )
{
   return object->getData();
}

//-----------------------------------------------------------------------------

DefineConsoleMethod( GuiInspectorField, reset, void, (), , "() - Reset to default value." )
{
   object->resetData();
}
