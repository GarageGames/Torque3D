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

#ifndef COMPONENT_H
#define COMPONENT_H

#ifndef _NETOBJECT_H_
#include "sim/netObject.h"
#endif
#ifndef ENTITY_H
#include "T3D/entity.h"
#endif
#ifndef CORE_INTERFACES_H
#include "T3D/components/coreInterfaces.h"
#endif

class Entity;

struct ComponentField
{
   StringTableEntry mFieldName;
   StringTableEntry mFieldDescription;

   S32 mFieldType;
   StringTableEntry mUserData;

   StringTableEntry mDefaultValue;

   StringTableEntry mGroup;

   StringTableEntry mDependency;

   bool mHidden;
};

//////////////////////////////////////////////////////////////////////////
/// 
/// 
//////////////////////////////////////////////////////////////////////////
class Component : public NetObject, public UpdateInterface
{
   typedef NetObject Parent;

protected:
   StringTableEntry mFriendlyName;
   StringTableEntry mDescription;

   StringTableEntry mFromResource;
   StringTableEntry mComponentGroup;
   StringTableEntry mComponentType;
   StringTableEntry mNetworkType;
   StringTableEntry mTemplateName;

   Vector<StringTableEntry> mDependencies;
   Vector<ComponentField> mFields;

   bool mNetworked;

   U32 componentIdx;

   Entity*              mOwner;
   bool					   mHidden;
   bool					   mEnabled;

public:
   Component();
   virtual ~Component();
   DECLARE_CONOBJECT(Component);

   virtual bool onAdd();
   virtual void onRemove();
   static void initPersistFields();

   virtual void packToStream(Stream &stream, U32 tabStop, S32 behaviorID, U32 flags = 0);

   //This is called when we are added to an entity
   virtual void onComponentAdd();            
   //This is called when we are removed from an entity
   virtual void onComponentRemove();         

   //This is called when a different component is added to our owner entity
   virtual void componentAddedToOwner(Component *comp);  
   //This is called when a different component is removed from our owner entity
   virtual void componentRemovedFromOwner(Component *comp);  

   virtual void ownerTransformSet(MatrixF *mat);

   void setOwner(Entity* pOwner);
   inline Entity *getOwner() { return mOwner ? mOwner : NULL; }
   static bool setOwner(void *object, const char *index, const char *data) { return true; }

   bool	isEnabled() { return mEnabled; }
   void  setEnabled(bool toggle) { mEnabled = toggle; setMaskBits(EnableMask); }

   bool isActive() { return mEnabled && mOwner != NULL; }

   static bool _setEnabled(void *object, const char *index, const char *data);

   virtual void processTick();
   virtual void interpolateTick(F32 dt){}
   virtual void advanceTime(F32 dt){}

   /// @name Adding Named Fields
   /// @{

   /// Adds a named field to a Component that can specify a description, data type, default value and userData
   ///
   /// @param   fieldName    The name of the Field
   /// @param   desc         The Description of the Field
   /// @param   type         The Type of field that this is, example 'Text' or 'Bool'
   /// @param   defaultValue The Default value of this field
   /// @param   userData     An extra optional field that can be used for user data
   void addComponentField(const char *fieldName, const char *desc, const char *type, const char *defaultValue = NULL, const char *userData = NULL, bool hidden = false);

   /// Returns the number of ComponentField's on this template
   inline S32 getComponentFieldCount() { return mFields.size(); };

   /// Gets a ComponentField by its index in the mFields vector 
   /// @param idx  The index of the field in the mField vector
   inline ComponentField *getComponentField(S32 idx)
   {
      if (idx < 0 || idx >= mFields.size())
         return NULL;

      return &mFields[idx];
   }

   ComponentField *getComponentField(const char* fieldName);

   const char* getComponentType() { return mComponentType; }

   const char *getDescriptionText(const char *desc);

   const char *getName() { return mTemplateName; }

   const char *getFriendlyName() { return mFriendlyName; }

   bool isNetworked() { return mNetworked; }

   void beginFieldGroup(const char* groupName);
   void endFieldGroup();

   void addDependency(StringTableEntry name);
   /// @}

   /// @name Description
   /// @{
   static bool setDescription(void *object, const char *index, const char *data);
   static const char* getDescription(void* obj, const char* data);

   /// @Primary usage functions
   /// @These are used by the various engine-based behaviors to integrate with the component classes
   enum NetMaskBits
   {
      InitialUpdateMask = BIT(0),
      OwnerMask = BIT(1),
      UpdateMask = BIT(2),
      EnableMask = BIT(3),
      NextFreeMask = BIT(4)
   };

   virtual U32 packUpdate(NetConnection *con, U32 mask, BitStream *stream);
   virtual void unpackUpdate(NetConnection *con, BitStream *stream);
   /// @}

   Signal< void(SimObject*, String, String) > onDataSet;
   virtual void setDataField(StringTableEntry slotName, const char *array, const char *value);

   virtual void onStaticModified(const char* slotName, const char* newValue); ///< Called when a static field is modified.
   virtual void onDynamicModified(const char* slotName, const char*newValue = NULL); ///< Called when a dynamic field is modified.

   /// This is what we actually use to check if the modified field is one of our behavior fields. If it is, we update and make the correct callbacks
   void checkComponentFieldModified(const char* slotName, const char* newValue);

   virtual void checkDependencies(){}
};

#endif // COMPONENT_H
