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

#include "platform/input/razerHydra/razerHydraFrameStore.h"
#include "platform/input/razerHydra/razerHydraFrame.h"
#include "core/module.h"
#include "console/simSet.h"
#include "console/consoleTypes.h"

MODULE_BEGIN( RazerHydraFrameStore )

   MODULE_INIT_AFTER( RazerHydraDevice )
   MODULE_INIT_AFTER( Sim )
   MODULE_SHUTDOWN_BEFORE( Sim )
   MODULE_SHUTDOWN_BEFORE( RazerHydraDevice )

   MODULE_INIT
   {
      RazerHydraFrameStore::staticInit();
      ManagedSingleton< RazerHydraFrameStore >::createSingleton();
   }
   
   MODULE_SHUTDOWN
   {
      ManagedSingleton< RazerHydraFrameStore >::deleteSingleton();
   }

MODULE_END;

S32 RazerHydraFrameStore::smMaximumFramesStored = 30;

SimGroup* RazerHydraFrameStore::smFrameGroup = NULL;

RazerHydraFrameStore::RazerHydraFrameStore()
{
   // Set up the SimGroup to store our frames
   smFrameGroup = new SimGroup();
   smFrameGroup->registerObject("RazerHydraFrameGroup");
   smFrameGroup->setNameChangeAllowed(false);
   Sim::getRootGroup()->addObject(smFrameGroup);
}

RazerHydraFrameStore::~RazerHydraFrameStore()
{
   if(smFrameGroup)
   {
      smFrameGroup->deleteObject();
      smFrameGroup = NULL;
   }
}

void RazerHydraFrameStore::staticInit()
{
   Con::addVariable("RazerHydra::MaximumFramesStored", TypeS32, &smMaximumFramesStored, 
      "@brief The maximum number of frames to keep when $RazerHydra::GenerateWholeFrameEvents is true.\n\n"
	   "@ingroup Game");
}

S32 RazerHydraFrameStore::generateNewFrame(const sixenseAllControllerData& frame, const F32& maxAxisRadius)
{
   // Make sure our group has been created
   if(!smFrameGroup)
      return 0;

   // Either create a new frame object or pull one off the end
   S32 frameID = 0;
   if(smFrameGroup->size() >= smMaximumFramesStored)
   {
      // Make the last frame the first and update
      RazerHydraFrame* frameObj = static_cast<RazerHydraFrame*>(smFrameGroup->last());
      smFrameGroup->bringObjectToFront(frameObj);
      frameObj->copyFromFrame(frame, maxAxisRadius);
      frameID = frameObj->getId();
   }
   else
   {
      // Create a new frame and add it to the front of the list
      RazerHydraFrame* frameObj = new RazerHydraFrame();
      frameObj->registerObject();
      smFrameGroup->addObject(frameObj);
      smFrameGroup->bringObjectToFront(frameObj);
      frameObj->copyFromFrame(frame, maxAxisRadius);
      frameID = frameObj->getId();
   }

   return frameID;
}
