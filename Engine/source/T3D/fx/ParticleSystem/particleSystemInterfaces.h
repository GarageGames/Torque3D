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

#ifndef _PARTICLE_SYSTEM_INTERFACES_H
#define _PARTICLE_SYSTEM_INTERFACES_H
#include <core/color.h>

/// @defgroup particlesysteminterfaces Interfaces
/// @ingroup particlesystem
/// @{

//-----------------------------------------------
//! An interface for ParticleRenderers that uses colors.
//! This interface is used for objects that wish to change
//! the colors of the particles. E.g. to match the ground
//! below the player.
//-----------------------------------------------
class IColoredParticleRenderer
{
public:
   virtual ~IColoredParticleRenderer() {}

   virtual void setColors(ColorF *colorList) = 0;
};

//-----------------------------------------------
//! An interface for ParticleRenderers that uses sizes.
//! This interface is used for objects that wish to change
//! the sizes of the particles.
//-----------------------------------------------
class ISizedParticleRenderer
{
public:
   virtual ~ISizedParticleRenderer() {}

   virtual void setSizes(F32 *sizeList) = 0;
};

/// @}

#endif // _PARTICLE_SYSTEM_INTERFACES_H

