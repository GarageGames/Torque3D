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
#include "postFx/postEffectVis.h"
#include "gui/containers/guiWindowCtrl.h"
#include "gui/controls/guiBitmapCtrl.h"
#include "gui/core/guiCanvas.h"
#include "postFx/postEffectManager.h"
#include "core/module.h"

ConsoleDoc(
   "@class PfxVis\n"
   "@brief Singleton class that exposes ConsoleStaticFunctions for debug visualizing PostEffects.\n\n"
   "@tsexample\n"
   "// Script interface...\n"
   "PfxVis::open( PostEffect ) // Multiple PostEffects can be visualized at the same time\n"
   "PfxVis::clear()      // Clear all visualizer windows\n"
   "PfxVis::hide()       // Hide all windows (are not destroyed)\n"
   "PfxVis::show()\n"
   "@endtsexample\n\n"
   "@ingroup GFX\n"
);

MODULE_BEGIN( PostEffectVis )

   MODULE_INIT
   {
      ManagedSingleton< PostEffectVis >::createSingleton();
   }
   
   MODULE_SHUTDOWN
   {
      ManagedSingleton< PostEffectVis >::deleteSingleton();
   }

MODULE_END;


PostEffectVis::PostEffectVis()
   : mContent( NULL )
{
}

PostEffectVis::~PostEffectVis()
{
}

void PostEffectVis::open( PostEffect *pfx )
{
   GuiControl *content = _getContentControl();   

   // If we already have this PostEffect added
   // remove it first so we can recreate its controls.
   VisVector::iterator itr = mWindows.begin();
   for ( ; itr != mWindows.end(); itr++ )
   {
      if ( itr->pfx == pfx )
      {
         for ( U32 i = 0; i < TexCount; i++ )
         {
            // Deleting the GuiWindowCtrl will automatically also delete
            // any child controls we have allocated.
            if ( itr->window[i] )
               itr->window[i]->deleteObject();            
         }

         mWindows.erase_fast( itr );
         break;
      }
   }

   // Allocate VisWindow struct.
   mWindows.increment();
   VisWindow &window = mWindows.last();
   window.pfx = pfx;

   for ( U32 i = 0; i < TexCount; i++ )
   {      
      // Only allocate window/bitmaps for input textures that are actually used.
      if ( i > Target )
      {
         if ( pfx->mTexFilename[i-1].isEmpty() )
         {
            window.window[i] = NULL;
            window.bmp[i] = NULL;
            continue;
         }
      }

      // Allocate GuiWindowCtrl
      GuiWindowCtrl *winCtrl = new GuiWindowCtrl();
      winCtrl->setPosition( Point2I( 50, 50 ) + Point2I( 15, 15 ) * i );
      winCtrl->setExtent( 347, 209 );
      winCtrl->setMinExtent( Point2I( 150, 100 ) );
      winCtrl->setMobility( true, true, true, true, false, false );
      winCtrl->setCanResize( true, true );
      winCtrl->setDataField( StringTable->insert( "closeCommand" ), NULL, "PfxVis::onWindowClosed( $ThisControl );" );
      winCtrl->registerObject();      

      window.window[i] = winCtrl;

      _setDefaultCaption( window, i );

      // Allocate background GuiBitmapCtrl
      GuiBitmapCtrl *bmpCtrl = new GuiBitmapCtrl();
      bmpCtrl->setPosition( 3, 23 );
      bmpCtrl->setSizing( GuiControl::horizResizeWidth, GuiControl::vertResizeHeight );
      bmpCtrl->setExtent( 341, 181 );
      bmpCtrl->setDataField( StringTable->insert( "wrap" ), NULL, "1" );
      bmpCtrl->setBitmap( "tools/gui/images/transp_grid" );
      bmpCtrl->registerObject();      
      winCtrl->addObject( bmpCtrl );

      // Allocate GuiBitmapCtrl
      bmpCtrl = new GuiBitmapCtrl();
      bmpCtrl->setPosition( 3, 23 );
      bmpCtrl->setSizing( GuiControl::horizResizeWidth, GuiControl::vertResizeHeight );
      bmpCtrl->setExtent( 341, 181 );
      bmpCtrl->registerObject();
      winCtrl->addObject( bmpCtrl );

      window.bmp[i] = bmpCtrl;      

      content->addObject( winCtrl );      
   }

   // Make sure we visible.
   setVisible( true );
}

void PostEffectVis::setVisible( bool visible )
{
   GuiCanvas *canvas = NULL;
   if ( !Sim::findObject( "Canvas", canvas ) )
   {
      Con::errorf( "PostEffectVis::setVisible, Canvas was not found." );
      return;
   }

   GuiControl *content = _getContentControl();   
   
   if ( visible && !content->isAwake() )
      canvas->pushDialogControl( content, 100 );   
   
   if ( !visible && content->isAwake() )
      canvas->popDialogControl( content );
}

void PostEffectVis::clear()
{
   GuiControl *content = _getContentControl(); 

   content->clear();   
   mWindows.clear();
}

void PostEffectVis::onStartOfFrame()
{
   if ( mWindows.empty() )
      return;
   if ( !_getContentControl()->isAwake() )
      return;

   // Restore vis windows to a default state.
   // This ensures to users that open PostEffects that are not
   // actively being processed are obvious.

   VisVector::iterator itr = mWindows.begin();
   for ( ; itr != mWindows.end(); itr++ )
   {
      for ( U32 i = 0; i < TexCount; i++ )
      {
         if ( !itr->bmp[i] || itr->pfx->getRenderTime() == PFXTexGenOnDemand )
            continue;

         itr->bmp[i]->setBitmap( NULL );
         _setDefaultCaption( *itr, i );
      }
   }
}

void PostEffectVis::onPFXProcessed( PostEffect *pfx )
{
   // If we have no windows we can early out before even testing
   // isAwake so we avoid creating the content control unnecessarily.
   if ( mWindows.empty() )
      return;

   if ( !_getContentControl()->isAwake() )
      return;

   VisVector::iterator itr = mWindows.begin();
   for ( ; itr != mWindows.end(); itr++ )
   {
      if ( itr->pfx == pfx )
      {
         GuiBitmapCtrl *pBmpCtrl = NULL;
         GuiWindowCtrl *pWinCtrl = NULL;

         if ( itr->bmp[Target] != NULL )
         {            
            pBmpCtrl = itr->bmp[Target];
            pWinCtrl = itr->window[Target];

            GFXTextureObject *tex;

            if ( pfx->mTargetTex )
               tex = pfx->mTargetTex;         
            else
               tex = PFXMGR->getBackBufferTex();

            pBmpCtrl->setBitmapHandle( tex );

            char caption[256];           
            char name[256];

            if ( pfx->getName() == NULL || dStrlen( pfx->getName() ) == 0 )
               dSprintf( name, 256, "(none)" );
            else
               dSprintf( name, 256, "%s", pfx->getName() );


            if ( tex )
               dSprintf( caption, 256, "%s[%i] target - %s [ %ix%i ]", name, pfx->getId(), pfx->mTargetName.c_str(), tex->getWidth(), tex->getHeight() );               
            else
               dSprintf( caption, 256, "%s[%i] target", name, pfx->getId() );


            pWinCtrl->setDataField( StringTable->insert("text"), NULL, caption );
         }

         for ( U32 i = Input1; i < TexCount; i++ )
         {
            if ( itr->bmp[i] == NULL )
               continue;

            pBmpCtrl = itr->bmp[i];
            pWinCtrl = itr->window[i];            

            GFXTextureObject *tex = pfx->mActiveTextures[i-1];

            pBmpCtrl->setBitmapHandle( tex );

            char caption[256];            
            char name[256];

            if ( pfx->getName() == NULL || dStrlen( pfx->getName() ) == 0 )
               dSprintf( name, 256, "(none)" );
            else
               dSprintf( name, 256, "%s", pfx->getName() );


            if ( tex )
               dSprintf( caption, 256, "%s[%i] input%i - %s [ %ix%i ]", name, pfx->getId(), i-1, pfx->mTexFilename[i-1].c_str(), tex->getWidth(), tex->getHeight() );               
            else
               dSprintf( caption, 256, "%s[%i] input%i - %s", name, pfx->getId(), i-1, pfx->mTexFilename[i-1].c_str() );

            pWinCtrl->setDataField( StringTable->insert("text"), NULL, caption );
         }
      }
   }
}

void PostEffectVis::onWindowClosed( GuiWindowCtrl *ctrl )
{
   VisVector::iterator itr = mWindows.begin();

   for ( ; itr != mWindows.end(); itr++ )
   {
      for ( U32 i = 0; i < TexCount; i++ )
      {
         if ( itr->window[i] == ctrl )
         {
            itr->window[i] = NULL;
            itr->bmp[i] = NULL;
            ctrl->setVisible( false );

            // Avoid deleting immediately since this happens in response to a
            // script callback.
            Con::evaluate( "%i.schedule( 1, \"delete\" );" );

            return;
         }
      }
   }

   Con::errorf( "PostEffectVis::onWindowClosed, passed window (%s) [%i] was found.", StringTable->insert( ctrl->getName() ), ctrl->getId() );   
}

GuiControl* PostEffectVis::_getContentControl()
{
   if ( mContent == NULL )
   {      
      GuiCanvas *canvas = NULL;
      if ( !Sim::findObject( "Canvas", canvas ) )
      {
         AssertFatal( false, "PostEffectVis::_getContentControl, Canvas not found." );
         return NULL;
      }

      mContent = new GuiControl();
      mContent->setPosition( 0, 0 );
      mContent->setExtent( 1024, 768 );
      mContent->setDataField( StringTable->insert( "noCursor" ), NULL, "1" );
      mContent->setDataField( StringTable->insert( "profile" ), NULL, "GuiModelessDialogProfile" );   
      mContent->registerObject( "PfxVisContent" );

      canvas->pushDialogControl( mContent, 100 );
   }

   return mContent;
}

void PostEffectVis::_setDefaultCaption( VisWindow &vis, U32 texIndex )
{
   PostEffect *pfx = vis.pfx;
   GuiWindowCtrl *winCtrl = vis.window[texIndex];
   
   if ( texIndex == Target )
   {
      char caption[256];           
      char name[256];

      if ( pfx->getName() == NULL || dStrlen( pfx->getName() ) == 0 )
         dSprintf( name, 256, "(none)" );
      else
         dSprintf( name, 256, "%s", pfx->getName() );

      dSprintf( caption, 256, "%s[%i] target [NOT ENABLED]", name, pfx->getId() );

      winCtrl->setDataField( StringTable->insert("text"), NULL, caption );
   }
   else
   {
      char caption[256];            
      char name[256];

      if ( pfx->getName() == NULL || dStrlen( pfx->getName() ) == 0 )
         dSprintf( name, 256, "(none)" );
      else
         dSprintf( name, 256, "%s", pfx->getName() );

      dSprintf( caption, 256, "%s[%i] input%i - %s [NOT ENABLED]", name, pfx->getId(), texIndex-1, pfx->mTexFilename[texIndex-1].c_str() );

      winCtrl->setDataField( StringTable->insert("text"), NULL, caption );
   }
}

static ConsoleDocFragment _PfxVisclear(
   "@brief Close all visualization windows.\n\n"
   "@tsexample\n"
   "PfxVis::clear();"
   "@endtsexample\n\n",
   "PfxVis",
   "void clear();" );

ConsoleStaticMethod( PfxVis, clear, void, 1, 1, "()"
					"@hide")
{
   PFXVIS->clear();
}

static ConsoleDocFragment _PfxVisopen(
   "@brief Open visualization windows for all input and target textures.\n\n"
   "@param effect Name of the PostEffect to open\n"
   "@param clear True to close all visualization windows before opening the effect\n\n"
   "@tsexample\n"
   "// Multiple PostEffects can be visualized at the same time\n"
   "PfxVis::open( PostEffect )\n"
   "@endtsexample\n\n",
   "PfxVis",
   "void open(PostEffect effect, bool clear);" );

ConsoleStaticMethod( PfxVis, open, void, 2, 3, "( PostEffect, [bool clear = false] )"
					"@hide")
{
   if ( argc == 3 && dAtob( argv[2] ) )
      PFXVIS->clear();
   
   PostEffect *pfx;
   if ( !Sim::findObject( argv[1], pfx ) )
   {
      Con::errorf( "PfxVis::add, argument %s was not a PostEffect", argv[1] );
      return;
   }

   PFXVIS->open( pfx );
}

static ConsoleDocFragment _PfxVishide(
   "@brief Hide all visualization windows (they are not destroyed).\n\n"
   "@tsexample\n"
   "PfxVis::hide();"
   "@endtsexample\n\n",
   "PfxVis",
   "void hide();" );

ConsoleStaticMethod( PfxVis, hide, void, 1, 1, "()"
					"@hide")
{
   PFXVIS->setVisible( false );
}

static ConsoleDocFragment _PfxVisshow(
   "@brief Show all visualization windows.\n\n"
   "@tsexample\n"
   "PfxVis::show();"
   "@endtsexample\n\n",
   "PfxVis",
   "void show();" );

ConsoleStaticMethod( PfxVis, show, void, 1, 1, "()"
					"@hide")
{
   PFXVIS->setVisible( true );
}

static ConsoleDocFragment _PfxVisonWindowClosed(
   "@brief Callback when a visualization window is closed.\n\n"
   "@param ctrl Name of the GUI control being closed\n"
   "@tsexample\n"
   "PfxVis::onWindowClosed( VisWindow )\n"
   "@endtsexample\n\n",
   "PfxVis",
   "void onWindowClosed(GuiWindowCtrl *ctrl);" );

ConsoleStaticMethod( PfxVis, onWindowClosed, void, 2, 2, "( GuiWindowCtrl )"
					"@hide")
{
   GuiWindowCtrl *ctrl;
   if ( !Sim::findObject( argv[1], ctrl ) )
   {
      Con::errorf( "PfxVis::onWindowClosed, argument %s was not a GuiWindowCtrl", argv[1] );
      return;
   }

   PFXVIS->onWindowClosed( ctrl );
}