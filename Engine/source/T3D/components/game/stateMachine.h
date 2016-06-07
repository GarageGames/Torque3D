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

#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#ifndef _SIMBASE_H_
#include "console/simBase.h"
#endif
#ifndef _OBJECTTYPES_H_
#include "T3D/objectTypes.h"
#endif
#ifndef _MMATH_H_
#include "math/mMath.h"
#endif
#ifndef _XMLDOC_H_
#include "console/SimXMLDocument.h"
#endif

class StateMachine
{
public:
   struct StateField
   {
      StringTableEntry name;
      
      bool 			 triggerBoolVal;
      float 		 triggerNumVal;
      Point3F 		 triggerVectorVal;
      String 		 triggerStringVal;

      enum Type
      {
         BooleanType = 0,
         NumberType,
         VectorType,
         StringType
      }fieldType;
   };

   struct UniqueReference
   {
      SimObject* referenceObj;
      const char* referenceVar;
      const char* uniqueName;
   };

   struct StateTransition
   {
      struct Condition
      {
         enum TriggerValueTarget
         {
            Equals = 0,
            GeaterThan,
            LessThan,
            GreaterOrEqual,
            LessOrEqual,
            True,
            False,
            Positive,
            Negative,
            DoesNotEqual
         };

         StateField field;

         TriggerValueTarget   triggerComparitor;

         UniqueReference      *valUniqueRef;
      };

      StringTableEntry	mName;
      StringTableEntry	mStateTarget;
      Vector<Condition>	mTransitionRules;
   };

   struct State 
   {
      Vector<StateTransition> mTransitions;

      StringTableEntry stateName;

      StringTableEntry callbackName;
   };

   StringTableEntry		mStateMachineFile;

protected:
   Vector<State> mStates;

   Vector<StateField> mFields;

   Vector<UniqueReference> mUniqueReferences;

   State mCurrentState;

   F32 mStateStartTime;
   F32 mStateTime;

   StringTableEntry mStartingState;

   State *mCurCreateSuperState;
   State *mCurCreateState;

   SimObjectPtr<SimXMLDocument> mXMLReader;

public:
   StateMachine();
   virtual ~StateMachine();

   void update();

   void loadStateMachineFile();
   void readStates();
   void readTransitions(State &currentState);
   void readConditions(StateTransition &newTransition);

   void setState(const char* stateName, bool clearFields = true);

   const char* getCurrentStateName() { return mCurrentState.stateName; }
   State& getCurrentState() {
      return mCurrentState;
   }

   S32 getStateCount() { return mStates.size(); }
   const char* getStateByIndex(S32 index);
   State& getStateByName(const char* name);

   void checkTransitions(const char* slotName, const char* newValue);

   bool passComparitorCheck(const char* var, StateTransition::Condition transitionRule);

   S32 findFieldByName(const char* name);

   S32 getFieldsCount() { return mFields.size(); }

   StateField getField(U32 index)
   {
      if (index <= mFields.size())
         return mFields[index];
   }

   Signal< void(StateMachine*, S32 stateIdx) > onStateChanged;

   //
   inline bool readStateName(State* state, SimXMLDocument* reader)
   {
      if (reader->pushFirstChildElement("Name"))
      {
         state->stateName = reader->getData();
         reader->popElement();

         return true;
      }

      return false;
   }
   inline bool readStateScriptFunction(State* state, SimXMLDocument* reader)
   {
      if (reader->pushFirstChildElement("ScriptFunction"))
      {
         state->callbackName = reader->getData();
         reader->popElement();

         return true;
      }

      return false;
   }
   inline bool readTransitonTarget(StateTransition* transition, SimXMLDocument* reader)
   {
      if (reader->pushFirstChildElement("StateTarget"))
      {
         transition->mStateTarget = reader->getData();
         reader->popElement();

         return true;
      }

      return false;
   }
   //
   inline bool readFieldName(StateField* newField, SimXMLDocument* reader)
   {
      if (reader->pushFirstChildElement("FieldName"))
      {
         newField->name = reader->getData();
         reader->popElement();

         return true;
      }

      return false;
   }
   inline bool readFieldComparitor(StateTransition::Condition* condition, SimXMLDocument* reader)
   {
      if (reader->pushFirstChildElement("Comparitor"))
      {
         S32 compIdx = parseComparitor(reader->getData());
         condition->triggerComparitor = static_cast<StateTransition::Condition::TriggerValueTarget>(compIdx);
         reader->popElement();

         return true;
      }

      return false;
   }
   inline bool readFieldValue(StateField* field, SimXMLDocument* reader)
   {
      if (reader->pushFirstChildElement("NumValue"))
      {
         field->fieldType = StateField::NumberType;
         field->triggerNumVal = dAtof(reader->getData());
         reader->popElement();
         return true;
      }
      else if (reader->pushFirstChildElement("StringValue"))
      {
         field->fieldType = StateField::StringType;
         field->triggerStringVal = reader->getData();
         reader->popElement();
         return true;
      }
      else if (reader->pushFirstChildElement("BoolValue"))
      {
         field->fieldType = StateField::BooleanType;
         field->triggerBoolVal = dAtob(reader->getData());
         reader->popElement();
         return true;
      }

      return false;
   }

private:
   S32 parseComparitor(const char* comparitorName);
};

#endif