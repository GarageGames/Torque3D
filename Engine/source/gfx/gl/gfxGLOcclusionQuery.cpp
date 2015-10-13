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
#include "gfx/gl/gfxGLOcclusionQuery.h"

GFXGLOcclusionQuery::GFXGLOcclusionQuery(GFXDevice* device) :
GFXOcclusionQuery(device), mQuery(NULL), mTesting(false)
{
}

GFXGLOcclusionQuery::~GFXGLOcclusionQuery()
{
   if (!glIsQuery(mQuery))
      glDeleteQueries(1, &mQuery);
}

bool GFXGLOcclusionQuery::begin()
{
   if (!glIsQuery(mQuery))
      glGenQueries(1, &mQuery);

   if (!mTesting)
   {
      glBeginQuery(GL_SAMPLES_PASSED, mQuery);
      mTesting = true;
   }
   return true;
}

void GFXGLOcclusionQuery::end()
{
   if (!glIsQuery(mQuery))
      return;
   glEndQuery(GL_SAMPLES_PASSED);
   mTesting = false;
}

GFXOcclusionQuery::OcclusionQueryStatus GFXGLOcclusionQuery::getStatus(bool block, U32* data)
{
   // If this ever shows up near the top of a profile 
   // then your system is GPU bound.
   PROFILE_SCOPE(GFXGLOcclusionQuery_getStatus);

   if (!glIsQuery(mQuery))
      return NotOccluded;

   GLint numPixels = 0;
   GLint queryDone = false;

   if (block)
      queryDone = true;
   else
      glGetQueryObjectiv(mQuery, GL_QUERY_RESULT_AVAILABLE, &queryDone);

   if (queryDone)
      glGetQueryObjectiv(mQuery, GL_QUERY_RESULT, &numPixels);
   else
      return Waiting;

   if (data)
      *data = numPixels;

   return numPixels > 0 ? NotOccluded : Occluded;
}

void GFXGLOcclusionQuery::zombify()
{
   if (!glIsQuery(mQuery))
      return;

   glDeleteQueries(1, &mQuery);
   mQuery = NULL;
}

void GFXGLOcclusionQuery::resurrect()
{
   if (!glIsQuery(mQuery))
      glGenQueries(1, &mQuery);
}

const String GFXGLOcclusionQuery::describeSelf() const
{
   // We've got nothing
   return String();
}