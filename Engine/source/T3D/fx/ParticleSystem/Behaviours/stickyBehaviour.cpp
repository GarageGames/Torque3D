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

#include "stickyBehaviour.h"
#include "../particleSystem.h"
#include "console/consoleTypes.h"
#include "core/stream/bitStream.h"

IMPLEMENT_CO_DATABLOCK_V1(StickyBehaviour);

//---------------------------------------------------------------
// Constructor
//---------------------------------------------------------------
StickyBehaviour::StickyBehaviour()
{

}

//----------------------IParticleBehaviour-----------------------
//---------------------------------------------------------------
// updateParticle
// --Description
//--------------------------------------------------------------
void StickyBehaviour::updateParticle(ParticleSystem* system, Particle* part, F32 time)
{
   // Set the particle position to be the relative position
   part->pos = system->getPosition() + part->relPos;
}

//----------------------- SimDataBlock -------------------------
//---------------------------------------------------------------
// InitPersistFields
//--------------------------------------------------------------
void StickyBehaviour::initPersistFields()
{
   Parent::initPersistFields();
}

//---------------------------------------------------------------
// OnAdd
//---------------------------------------------------------------
bool StickyBehaviour::onAdd()
{
   if (!Parent::onAdd())
      return false;
   return true;
}

//---------------------------------------------------------------
// PackData
//--------------------------------------------------------------
void StickyBehaviour::packData(BitStream* stream)
{
   Parent::packData(stream);
}

//---------------------------------------------------------------
// UnpackData
//--------------------------------------------------------------
void StickyBehaviour::unpackData(BitStream* stream)
{
   Parent::unpackData(stream);
}

//---------------------------------------------------------------
// Preload
//--------------------------------------------------------------
bool StickyBehaviour::preload(bool server, String &errorStr)
{
   if (Parent::preload(server, errorStr) == false)
      return false;
   // Verify variables

   return true;
}