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

#include "T3D/components/game/stateMachine.h"

StateMachine::StateMachine()
{
   mStateStartTime = -1;
   mStateTime = 0;

   mStartingState = "";

   mCurCreateState = NULL;
}

StateMachine::~StateMachine()
{
}

void StateMachine::loadStateMachineFile()
{
   if (!mXMLReader)
   {
      SimXMLDocument *xmlrdr = new SimXMLDocument();
      xmlrdr->registerObject();

      mXMLReader = xmlrdr;
   }

   bool hasStartState = false;

   if (!dStrIsEmpty(mStateMachineFile))
   {
      //use our xml reader to parse the file!
      SimXMLDocument *reader = mXMLReader.getObject();
      if (!reader->loadFile(mStateMachineFile))
         Con::errorf("Could not load state machine file: &s", mStateMachineFile);

      if (!reader->pushFirstChildElement("StateMachine"))
         return;

      //find our starting state
      if (reader->pushFirstChildElement("StartingState"))
      {
         mStartingState = reader->getData();
         reader->popElement();
         hasStartState = true;
      }

      readStates();
   }

   if (hasStartState)
      mCurrentState = getStateByName(mStartingState);

   mStateStartTime = -1;
   mStateTime = 0;
}

void StateMachine::readStates()
{
   SimXMLDocument *reader = mXMLReader.getObject();

   //iterate through our states now!
   if (reader->pushFirstChildElement("State"))
   {
      //get our first state
      State firstState;

      readStateName(&firstState, reader);
      readStateScriptFunction(&firstState, reader);

      readTransitions(firstState);

      mStates.push_back(firstState);

      //now, iterate the siblings
      while (reader->nextSiblingElement("State"))
      {
         State newState;
         readStateName(&newState, reader);
         readStateScriptFunction(&newState, reader);

         readTransitions(newState);

         mStates.push_back(newState);
      }
   }
}

void StateMachine::readTransitions(State &currentState)
{
   SimXMLDocument *reader = mXMLReader.getObject();

   //iterate through our states now!
   if (reader->pushFirstChildElement("Transition"))
   {
      //get our first state
      StateTransition firstTransition;

      readTransitonTarget(&firstTransition, reader);

      readConditions(firstTransition);

      currentState.mTransitions.push_back(firstTransition);

      //now, iterate the siblings
      while (reader->nextSiblingElement("Transition"))
      {
         StateTransition newTransition;
         readTransitonTarget(&newTransition, reader);

         readConditions(newTransition);

         currentState.mTransitions.push_back(newTransition);
      }

      reader->popElement();
   }
}

void StateMachine::readConditions(StateTransition &currentTransition)
{
   SimXMLDocument *reader = mXMLReader.getObject();

   //iterate through our states now!
   if (reader->pushFirstChildElement("Rule"))
   {
      //get our first state
      StateTransition::Condition firstCondition;
      StateField firstField;
      bool fieldRead = false;
      
      readFieldName(&firstField, reader);
      firstCondition.field = firstField;

      readFieldComparitor(&firstCondition, reader);

      readFieldValue(&firstCondition.field, reader);

      currentTransition.mTransitionRules.push_back(firstCondition);

      //now, iterate the siblings
      while (reader->nextSiblingElement("Transition"))
      {
         StateTransition::Condition newCondition;
         StateField newField;

         readFieldName(&newField, reader);
         newCondition.field = newField;

         readFieldComparitor(&newCondition, reader);

         readFieldValue(&newCondition.field, reader);

         currentTransition.mTransitionRules.push_back(newCondition);
      }

      reader->popElement();
   }
}

S32 StateMachine::parseComparitor(const char* comparitorName)
{
   S32 targetType = -1;

   if (!dStrcmp("GreaterThan", comparitorName))
      targetType = StateMachine::StateTransition::Condition::GeaterThan;
   else if (!dStrcmp("GreaterOrEqual", comparitorName))
      targetType = StateMachine::StateTransition::Condition::GreaterOrEqual;
   else if (!dStrcmp("LessThan", comparitorName))
      targetType = StateMachine::StateTransition::Condition::LessThan;
   else if (!dStrcmp("LessOrEqual", comparitorName))
      targetType = StateMachine::StateTransition::Condition::LessOrEqual;
   else if (!dStrcmp("Equals", comparitorName))
      targetType = StateMachine::StateTransition::Condition::Equals;
   else if (!dStrcmp("True", comparitorName))
      targetType = StateMachine::StateTransition::Condition::True;
   else if (!dStrcmp("False", comparitorName))
      targetType = StateMachine::StateTransition::Condition::False;
   else if (!dStrcmp("Negative", comparitorName))
      targetType = StateMachine::StateTransition::Condition::Negative;
   else if (!dStrcmp("Positive", comparitorName))
      targetType = StateMachine::StateTransition::Condition::Positive;
   else if (!dStrcmp("DoesNotEqual", comparitorName))
      targetType = StateMachine::StateTransition::Condition::DoesNotEqual;

   return targetType;
}

void StateMachine::update()
{
   //we always check if there's a timout transition, as that's the most generic transition possible.
   F32 curTime = Sim::getCurrentTime();

   if (mStateStartTime == -1)
      mStateStartTime = curTime;

   mStateTime = curTime - mStateStartTime;

   char buffer[64];
   dSprintf(buffer, sizeof(buffer), "%g", mStateTime);

   checkTransitions("stateTime", buffer);
}

void StateMachine::checkTransitions(const char* slotName, const char* newValue)
{
   //because we use our current state's fields as dynamic fields on the instance
   //we'll want to catch any fields being set so we can treat changes as transition triggers if
   //any of the transitions on this state call for it

   //One example would be in order to implement burst fire on a weapon state machine.
   //The behavior instance has a dynamic variable set up like: GunStateMachine.burstShotCount = 0;

   //We also have a transition in our fire state, as: GunStateMachine.addTransition("FireState", "burstShotCount", "DoneShooting", 3);
   //What that does is for our fire state, we check the dynamicField burstShotCount if it's equal or greater than 3. If it is, we perform the transition.

   //As state fields are handled as dynamicFields for the instance, regular dynamicFields are processed as well as state fields. So we can use the regular 
   //dynamic fields for our transitions, to act as 'global' variables that are state-agnostic. Alternately, we can use state-specific fields, such as a transition
   //like this:
   //GunStateMachine.addTransition("IdleState", "Fidget", "Timeout", ">=", 5000);

   //That uses the the timeout field, which is reset each time the state changes, and so state-specific, to see if it's been 5 seconds. If it has been, we transition
   //to our fidget state

   //so, lets check our current transitions
   //now that we have the type, check our transitions!
   for (U32 t = 0; t < mCurrentState.mTransitions.size(); t++)
   {
      //if (!dStrcmp(mCurrentState.mTransitions[t]., slotName))
      {
         //found a transition looking for this variable, so do work
         //first, figure out what data type thie field is
         //S32 type = getVariableType(newValue);

         bool fail = false;
         bool match = false;
         S32 ruleCount = mCurrentState.mTransitions[t].mTransitionRules.size();

         for (U32 r = 0; r < ruleCount; r++)
         {
            const char* fieldName = mCurrentState.mTransitions[t].mTransitionRules[r].field.name;
            if (!dStrcmp(fieldName, slotName))
            {
               match = true;
               //now, check the value with the comparitor and see if we do the transition.
               if (!passComparitorCheck(newValue, mCurrentState.mTransitions[t].mTransitionRules[r]))
               {
                  fail = true;
                  break;
               }
            }
         }

         //If we do have a transition rule for this field, and we didn't fail on the condition, go ahead and switch states
         if (match && !fail)
         {
            setState(mCurrentState.mTransitions[t].mStateTarget);

            return;
         }
      }
   }
}

bool StateMachine::passComparitorCheck(const char* var, StateTransition::Condition transitionRule)
{
   F32 num = dAtof(var);
   switch (transitionRule.field.fieldType)
   {
   case StateField::Type::VectorType:
      switch (transitionRule.triggerComparitor)
      {
      case StateTransition::Condition::Equals:
      case StateTransition::Condition::GeaterThan:
      case StateTransition::Condition::GreaterOrEqual:
      case StateTransition::Condition::LessThan:
      case StateTransition::Condition::LessOrEqual:
      case StateTransition::Condition::DoesNotEqual:
         //do
         break;
      default:
         return false;
      };
   case StateField::Type::StringType:
      switch (transitionRule.triggerComparitor)
      {
      case StateTransition::Condition::Equals:
         if (!dStrcmp(var, transitionRule.field.triggerStringVal))
            return true;
         else
            return false;
      case StateTransition::Condition::DoesNotEqual:
         if (dStrcmp(var, transitionRule.field.triggerStringVal))
            return true;
         else
            return false;
      default:
         return false;
      };
   case StateField::Type::BooleanType:
      switch (transitionRule.triggerComparitor)
      {
      case StateTransition::Condition::TriggerValueTarget::True:
         if (dAtob(var))
            return true;
         else
            return false;
      case StateTransition::Condition::TriggerValueTarget::False:
         if (dAtob(var))
            return false;
         else
            return true;
      default:
         return false;
      };
   case StateField::Type::NumberType:
      switch (transitionRule.triggerComparitor)
      {
      case StateTransition::Condition::TriggerValueTarget::Equals:
         if (num == transitionRule.field.triggerNumVal)
            return true;
         else
            return false;
      case StateTransition::Condition::TriggerValueTarget::GeaterThan:
         if (num > transitionRule.field.triggerNumVal)
            return true;
         else
            return false;
      case StateTransition::Condition::TriggerValueTarget::GreaterOrEqual:
         if (num >= transitionRule.field.triggerNumVal)
            return true;
         else
            return false;
      case StateTransition::Condition::TriggerValueTarget::LessThan:
         if (num < transitionRule.field.triggerNumVal)
            return true;
         else
            return false;
      case StateTransition::Condition::TriggerValueTarget::LessOrEqual:
         if (num <= transitionRule.field.triggerNumVal)
            return true;
         else
            return false;
      case StateTransition::Condition::TriggerValueTarget::DoesNotEqual:
         if (num != transitionRule.field.triggerNumVal)
            return true;
         else
            return false;
      case StateTransition::Condition::TriggerValueTarget::Positive:
         if (num > 0)
            return true;
         else
            return false;
      case StateTransition::Condition::TriggerValueTarget::Negative:
         if (num < 0)
            return true;
         else
            return false;
      default:
         return false;
      };
   default:
      return false;
   };
}

void StateMachine::setState(const char* stateName, bool clearFields)
{
   State oldState = mCurrentState;
   StringTableEntry sName = StringTable->insert(stateName);
   for (U32 i = 0; i < mStates.size(); i++)
   {
      //if(!dStrcmp(mStates[i]->stateName, stateName))
      if (!dStrcmp(mStates[i].stateName,sName))
      {
         mCurrentState = mStates[i];
         mStateStartTime = Sim::getCurrentTime();

         onStateChanged.trigger(this, i);
         return;
      }
   }
}

const char* StateMachine::getStateByIndex(S32 index)
{
   if (index >= 0 && mStates.size() > index)
      return mStates[index].stateName;
   else
      return "";
}

StateMachine::State& StateMachine::getStateByName(const char* name)
{
   StringTableEntry stateName = StringTable->insert(name);

   for (U32 i = 0; i < mStates.size(); i++)
   {
      if (!dStrcmp(stateName, mStates[i].stateName))
         return mStates[i];
   }
}

S32 StateMachine::findFieldByName(const char* name)
{
   for (U32 i = 0; i < mFields.size(); i++)
   {
      if (!dStrcmp(mFields[i].name, name))
         return i;
   }

   return -1;
}