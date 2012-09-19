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

#include "ts/collada/colladaExtensions.h"
#include "ts/collada/colladaAppSequence.h"


ColladaAppSequence::ColladaAppSequence(const domAnimation_clip* clip)
   : pClip(clip), clipExt(new ColladaExtension_animation_clip(clip))
{
   seqStart = pClip->getStart();
   seqEnd = pClip->getEnd();
}

ColladaAppSequence::~ColladaAppSequence()
{
   delete clipExt;
}

const char* ColladaAppSequence::getName() const
{
   return _GetNameOrId(pClip);
}

S32 ColladaAppSequence::getNumTriggers()
{
   return clipExt->triggers.size();
}

void ColladaAppSequence::getTrigger(S32 index, TSShape::Trigger& trigger)
{
   trigger.pos = clipExt->triggers[index].time;
   trigger.state = clipExt->triggers[index].state;
}

U32 ColladaAppSequence::getFlags() const
{
   U32 flags = 0;
   if (clipExt->cyclic) flags |= TSShape::Cyclic;
   if (clipExt->blend)  flags |= TSShape::Blend;
   return flags;
}

F32 ColladaAppSequence::getPriority()
{
   return clipExt->priority;
}

F32 ColladaAppSequence::getBlendRefTime()
{
   return clipExt->blendReferenceTime;
}

void ColladaAppSequence::setActive(bool active)
{
   for (int iAnim = 0; iAnim < getClip()->getInstance_animation_array().getCount(); iAnim++) {
      domAnimation* anim = daeSafeCast<domAnimation>(getClip()->getInstance_animation_array()[iAnim]->getUrl().getElement());
      if (anim)
         setAnimationActive(anim, active);
   }
}

void ColladaAppSequence::setAnimationActive(const domAnimation* anim, bool active)
{
   // Enabled/disable data channels for this animation
   for (int iChannel = 0; iChannel < anim->getChannel_array().getCount(); iChannel++) {
      domChannel* channel = anim->getChannel_array()[iChannel];
      AnimData* animData = reinterpret_cast<AnimData*>(channel->getUserData());
      if (animData)
         animData->enabled = active;
   }

   // Recurse into child animations
   for (int iAnim = 0; iAnim < anim->getAnimation_array().getCount(); iAnim++)
      setAnimationActive(anim->getAnimation_array()[iAnim], active);
}
