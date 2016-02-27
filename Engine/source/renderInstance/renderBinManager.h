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
#ifndef _RENDERBINMANAGER_H_
#define _RENDERBINMANAGER_H_

#ifndef _CONSOLEOBJECT_H_
#include "console/consoleObject.h"
#endif
#ifndef _RENDERPASSMANAGER_H_
#include "renderInstance/renderPassManager.h"
#endif
#ifndef _BASEMATINSTANCE_H_
#include "materials/baseMatInstance.h"
#endif
#ifndef _UTIL_DELEGATE_H_
#include "core/util/delegate.h"
#endif

class SceneRenderState;


/// This delegate is used in derived RenderBinManager classes
/// to allow material instances to be overriden.
typedef Delegate<BaseMatInstance*(BaseMatInstance*)> MaterialOverrideDelegate;


/// The RenderBinManager manages and renders lists of MainSortElem, which
/// is a light wrapper around RenderInst.
class RenderBinManager : public SimObject
{
   typedef SimObject Parent;

   friend class RenderPassManager;

public:

   RenderBinManager( const RenderInstType& ritype = RenderInstType::Invalid,
                     F32 renderOrder = 1.0f, 
                     F32 processAddOrder  = 1.0f );
   virtual ~RenderBinManager() {}
   
   // SimObject
   void onRemove();

   virtual void addElement( RenderInst *inst );
   virtual void sort();
   virtual void render( SceneRenderState *state ) {}
   virtual void clear();

   // Manager info
   F32 getProcessAddOrder() const { return mProcessAddOrder; }
   void setProcessAddOrder(F32 processAddOrder) { mProcessAddOrder = processAddOrder; }
   F32 getRenderOrder() const { return mRenderOrder; } 
   void setRenderOrder(F32 renderOrder) { mRenderOrder = renderOrder; }

   /// Returns the primary render instance type.
   const RenderInstType& getRenderInstType() { return mRenderInstType; }

   /// Returns the render pass this bin is registered to.
   RenderPassManager* getRenderPass() const { return mRenderPass; }

   /// QSort callback function
   static S32 FN_CDECL cmpKeyFunc(const void* p1, const void* p2);

   DECLARE_CONOBJECT(RenderBinManager);
   static void initPersistFields();

   MaterialOverrideDelegate& getMatOverrideDelegate() { return mMatOverrideDelegate; }

protected:

   struct MainSortElem
   {
      RenderInst *inst;
      U32 key;
      U32 key2;
   };

   void setRenderPass( RenderPassManager *rpm );

   /// Called from derived bins to add additional
   /// render instance types to be notified about.
   void notifyType( const RenderInstType &type );

   Vector< MainSortElem > mElementList; // List of our instances
   F32 mProcessAddOrder;   // Where in the list do we process RenderInstance additions?
   F32 mRenderOrder;       // Where in the list do we render?

   /// The primary render instance type this bin supports.
   RenderInstType mRenderInstType;

   /// The list of additional render instance types 
   /// this bin wants to process.
   Vector<RenderInstType> mOtherTypes;

   /// The render pass manager this bin is registered with.
   RenderPassManager *mRenderPass;

   MaterialOverrideDelegate mMatOverrideDelegate;

   virtual void setupSGData(MeshRenderInst *ri, SceneData &data );
   virtual void internalAddElement(RenderInst* inst);

   /// A inlined helper method for testing if the next 
   /// MeshRenderInst requires a new batch/pass.
   inline bool newPassNeeded( MeshRenderInst *ri, MeshRenderInst* nextRI ) const;

   /// Inlined utility function which gets the material from the 
   /// RenderInst if available, otherwise, return NULL.
   inline BaseMatInstance* getMaterial( RenderInst *inst ) const;

   // Limits bin to rendering in basic lighting only.
   bool mBasicOnly;
};


inline bool RenderBinManager::newPassNeeded( MeshRenderInst *ri, MeshRenderInst* nextRI ) const
{
   if ( ri == nextRI )
      return false;

   // We can depend completely on the state hint to check
   // for changes in the material as it uniquely identifies it.
   if ( ri->matInst->getStateHint() != nextRI->matInst->getStateHint() )
      return true;

   if (  ri->vertBuff != nextRI->vertBuff ||
         ri->primBuff != nextRI->primBuff ||
         ri->prim != nextRI->prim ||
         ri->primBuffIndex != nextRI->primBuffIndex ||

         // NOTE: Keep an eye on this... should we find a more
         // optimal test for light set changes?
         //
         dMemcmp( ri->lights, nextRI->lights, sizeof( ri->lights ) ) != 0 )

      return true;

   return false;
}

inline BaseMatInstance* RenderBinManager::getMaterial( RenderInst *inst ) const
{
   if (  inst->type == RenderPassManager::RIT_Mesh || 
         inst->type == RenderPassManager::RIT_Decal ||
         inst->type == RenderPassManager::RIT_DecalRoad ||         
         inst->type == RenderPassManager::RIT_Translucent )
      return static_cast<MeshRenderInst*>(inst)->matInst;

   return NULL;      
}

#endif // _RENDERBINMANAGER_H_
