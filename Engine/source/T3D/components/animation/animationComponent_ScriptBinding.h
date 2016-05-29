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
#include "T3D/components/animation/animationComponent.h"

DefineEngineMethod(AnimationComponent, playThread, bool, (S32 slot, const char* name, bool transition, F32 transitionTime), (-1, "", true, 0.5),
   "@brief Start a new animation thread, or restart one that has been paused or "
   "stopped.\n\n"

   "@param slot thread slot to play. Valid range is 0 - 3)\n"  // 3 = AnimationComponent::MaxScriptThreads-1
   "@param name name of the animation sequence to play in this slot. If not "
   "specified, the paused or stopped thread in this slot will be resumed.\n"
   "@return true if successful, false if failed\n\n"

   "@tsexample\n"
   "%obj.playThread( 0, \"ambient\" );      // Play the ambient sequence in slot 0\n"
   "%obj.setThreadTimeScale( 0, 0.5 );    // Play at half-speed\n"
   "%obj.pauseThread( 0 );                // Pause the sequence\n"
   "%obj.playThread( 0 );                 // Resume playback\n"
   "%obj.playThread( 0, \"spin\" );         // Replace the sequence in slot 0\n"
   "@endtsexample\n"

   "@see pauseThread()\n"
   "@see stopThread()\n"
   "@see setThreadDir()\n"
   "@see setThreadTimeScale()\n"
   "@see destroyThread()\n")
{
   return object->playThread(slot, name, transition, transitionTime);
}

DefineEngineMethod(AnimationComponent, setThreadDir, bool, (S32 slot, bool fwd), ,
   "@brief Set the playback direction of an animation thread.\n\n"

   "@param slot thread slot to modify\n"
   "@param fwd true to play the animation forwards, false to play backwards\n"
   "@return true if successful, false if failed\n\n"

   "@see playThread()\n")
{
   if (slot >= 0 && slot < AnimationComponent::MaxScriptThreads) 
   {
      if (object->setThreadDir(slot, fwd))
         return true;
   }
   return false;
}

DefineEngineMethod(AnimationComponent, setThreadTimeScale, bool, (S32 slot, F32 scale), ,
   "@brief Set the playback time scale of an animation thread.\n\n"

   "@param slot thread slot to modify\n"
   "@param scale new thread time scale (1=normal speed, 0.5=half speed etc)\n"
   "@return true if successful, false if failed\n\n"

   "@see playThread\n")
{
   if (slot >= 0 && slot < AnimationComponent::MaxScriptThreads)
   {
      if (object->setThreadTimeScale(slot, scale))
         return true;
   }
   return false;
}

DefineEngineMethod(AnimationComponent, setThreadPosition, bool, (S32 slot, F32 pos), ,
   "@brief Set the position within an animation thread.\n\n"

   "@param slot thread slot to modify\n"
   "@param pos position within thread\n"
   "@return true if successful, false if failed\n\n"

   "@see playThread\n")
{
   if (slot >= 0 && slot < AnimationComponent::MaxScriptThreads) 
   {
      if (object->setThreadPosition(slot, pos))
         return true;
   }
   return false;
}

DefineEngineMethod(AnimationComponent, setThreadAnimation, bool, (S32 slot, const char* name), (""),
   "@brief Force-sets the animation in a particular thread without starting it playing."

   "@param slot thread slot to play. Valid range is 0 - 3)\n"  // 3 = AnimationComponent::MaxScriptThreads-1
   "@param name name of the animation sequence to play in this slot. If not "
   "specified, the paused or stopped thread in this slot will be resumed.\n"
   "@return true if successful, false if failed\n\n")
{
   return object->setThreadAnimation(slot, name);
}

DefineEngineMethod(AnimationComponent, getThreadAnimation, String, (S32 slot), ,
   "@brief Force-sets the animation in a particular thread without starting it playing."

   "@param slot thread slot to play. Valid range is 0 - 3)\n"  // 3 = AnimationComponent::MaxScriptThreads-1
   "@param name name of the animation sequence to play in this slot. If not "
   "specified, the paused or stopped thread in this slot will be resumed.\n"
   "@return true if successful, false if failed\n\n")
{
   if (slot >= 0 && slot < AnimationComponent::MaxScriptThreads)
   {
      if (TSShape* shape = object->getShape())
      {
         S32 seq = object->getThreadSequenceID(slot);
         if (seq != -1)
         {
            String animationName = object->getAnimationName(seq);
            return animationName;
         }
      }
   }

   return "";
}

DefineEngineMethod(AnimationComponent, stopThread, bool, (S32 slot), ,
   "@brief Stop an animation thread.\n\n"

   "If restarted using playThread, the animation "
   "will start from the beginning again.\n"
   "@param slot thread slot to stop\n"
   "@return true if successful, false if failed\n\n"

   "@see playThread\n")
{
   if (slot >= 0 && slot < AnimationComponent::MaxScriptThreads) 
   {
      if (object->stopThread(slot))
         return true;
   }
   return false;
}

DefineEngineMethod(AnimationComponent, destroyThread, bool, (S32 slot), ,
   "@brief Destroy an animation thread, which prevents it from playing.\n\n"

   "@param slot thread slot to destroy\n"
   "@return true if successful, false if failed\n\n"

   "@see playThread\n")
{
   if (slot >= 0 && slot < AnimationComponent::MaxScriptThreads) 
   {
      if (object->destroyThread(slot))
         return true;
   }
   return false;
}

DefineEngineMethod(AnimationComponent, pauseThread, bool, (S32 slot), ,
   "@brief Pause an animation thread.\n\n"

   "If restarted using playThread, the animation "
   "will resume from the paused position.\n"
   "@param slot thread slot to stop\n"
   "@return true if successful, false if failed\n\n"

   "@see playThread\n")
{
   if (slot >= 0 && slot < AnimationComponent::MaxScriptThreads) 
   {
      if (object->pauseThread(slot))
         return true;
   }
   return false;
}

DefineEngineMethod(AnimationComponent, getAnimationCount, S32, (), ,
   "Get the total number of sequences in the shape.\n"
   "@return the number of sequences in the shape\n\n")
{
   return object->getAnimationCount();
}

DefineEngineMethod(AnimationComponent, getAnimationIndex, S32, (const char* name), ,
   "Find the index of the sequence with the given name.\n"
   "@param name name of the sequence to lookup\n"
   "@return index of the sequence with matching name, or -1 if not found\n\n"
   "@tsexample\n"
   "// Check if a given sequence exists in the shape\n"
   "if ( %this.getSequenceIndex( \"walk\" ) == -1 )\n"
   "   echo( \"Could not find 'walk' sequence\" );\n"
   "@endtsexample\n")
{
   return object->getAnimationIndex(name);
}

DefineEngineMethod(AnimationComponent, getAnimationName, const char*, (S32 index), ,
   "Get the name of the indexed sequence.\n"
   "@param index index of the sequence to query (valid range is 0 - getSequenceCount()-1)\n"
   "@return the name of the sequence\n\n"
   "@tsexample\n"
   "// print the name of all sequences in the shape\n"
   "%count = %this.getSequenceCount();\n"
   "for ( %i = 0; %i < %count; %i++ )\n"
   "   echo( %i SPC %this.getSequenceName( %i ) );\n"
   "@endtsexample\n")
{
   return object->getAnimationName(index);
}