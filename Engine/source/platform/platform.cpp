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

#include <limits>

#include "platform/platform.h"
#include "console/console.h"
#include "console/consoleTypes.h"
#include "platform/threads/mutex.h"
#include "app/mainLoop.h"
#include "platform/input/event.h"
#include "platform/typetraits.h"
#include "core/volume.h"


const F32 TypeTraits< F32 >::MIN = - F32_MAX;
const F32 TypeTraits< F32 >::MAX = F32_MAX;
const F32 TypeTraits< F32 >::ZERO = 0;
const F32 Float_Inf = std::numeric_limits< F32 >::infinity();

// The tools prefer to allow the CPU time to process
#ifndef TORQUE_TOOLS
S32 sgBackgroundProcessSleepTime = 25;
#else
S32 sgBackgroundProcessSleepTime = 200;
#endif
S32 sgTimeManagerProcessInterval = 1;

Vector<Platform::KeyboardInputExclusion> gKeyboardExclusionList;
bool gInitKeyboardExclusionList = false;
static bool gWebDeployment = false;

void Platform::initConsole()
{
   Con::addVariable("$platform::backgroundSleepTime", TypeS32, &sgBackgroundProcessSleepTime, "Controls processor time usage when the game window is out of focus.\n"
	   "@ingroup Platform\n");
   Con::addVariable("$platform::timeManagerProcessInterval", TypeS32, &sgTimeManagerProcessInterval, "Controls processor time usage when the game window is in focus.\n"
	   "@ingroup Platform\n");
}

S32 Platform::getBackgroundSleepTime()
{
   return sgBackgroundProcessSleepTime;
}

ConsoleToolFunction(restartInstance, void, 1, 1, "restartInstance()")
{
   StandardMainLoop::setRestart(true);
   Platform::postQuitMessage( 0 );
}

void Platform::clearKeyboardInputExclusion()
{
   gKeyboardExclusionList.clear();
   gInitKeyboardExclusionList = true;
}

void Platform::addKeyboardInputExclusion(const KeyboardInputExclusion &kie)
{
   gKeyboardExclusionList.push_back(kie);
}

const bool Platform::checkKeyboardInputExclusion(const InputEventInfo *info)
{
   // Do one-time initialization of platform defaults.
   if(!gInitKeyboardExclusionList)
   {
      gInitKeyboardExclusionList = true;

      // CodeReview Looks like we don't even need to do #ifdefs here since
      // things like cmd-tab don't appear on windows, and alt-tab is an unlikely
      // desired bind on other platforms - might be best to simply have a 
      // global exclusion list and keep it standard on all platforms.
      // This might not be so, but it's the current assumption. [bjg 5/4/07]

      // Alt-tab
      {
         KeyboardInputExclusion kie;
         kie.key = KEY_TAB;
         kie.andModifierMask = SI_ALT;
         addKeyboardInputExclusion(kie);
      }

      // ... others go here...
   }

   // Walk the list and look for matches.
   for(S32 i=0; i<gKeyboardExclusionList.size(); i++)
   {
      if(gKeyboardExclusionList[i].checkAgainstInput(info))
         return true;
   }

   return false;
}

const bool Platform::KeyboardInputExclusion::checkAgainstInput( const InputEventInfo *info ) const
{
   if(info->objType != SI_KEY)
      return false;

   if(info->objInst != key)
      return false;

   if((info->modifier & andModifierMask) != andModifierMask)
      return false;

   if(info->modifier & !(info->modifier & orModifierMask))
      return false;

   return true;
}

S32 Platform::compareModifiedTimes( const char *firstPath, const char *secondPath )
{
   FileTime firstModTime;
   if ( !getFileTimes( firstPath, NULL, &firstModTime ) ) {
      //The reason we failed to get file times could be cause it is in a zip.  Lets check.
      return Torque::FS::CompareModifiedTimes(firstPath, secondPath);
   }

   FileTime secondModTime;
   if ( !getFileTimes( secondPath, NULL, &secondModTime ) )
      return -1;

   return compareFileTimes( firstModTime, secondModTime );
}

bool Platform::getWebDeployment()
{
   return gWebDeployment;
}

void Platform::setWebDeployment(bool v)
{
   gWebDeployment = v;
}


