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
#include "gfx/gl/tGL/tGL.h"

GFXGLOcclusionQuery::GFXGLOcclusionQuery(GFXDevice* device) : 
   GFXOcclusionQuery(device), mQuery(-1)
{
   
}

GFXGLOcclusionQuery::~GFXGLOcclusionQuery()
{
   glDeleteQueries(1, &mQuery);
}

bool GFXGLOcclusionQuery::begin()
{
   if(mQuery == -1)
      glGenQueries(1, &mQuery);

   glBeginQuery(GL_SAMPLES_PASSED, mQuery);
   return true;
}

void GFXGLOcclusionQuery::end()
{
   glEndQuery(GL_SAMPLES_PASSED);
}

GFXOcclusionQuery::OcclusionQueryStatus GFXGLOcclusionQuery::getStatus(bool block, U32* data)
{
   // If this ever shows up near the top of a profile 
   // then your system is GPU bound.
   PROFILE_SCOPE(GFXGLOcclusionQuery_getStatus);

   if(mQuery == -1)
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
   glDeleteQueries(1, &mQuery);
   mQuery = 0;
}

void GFXGLOcclusionQuery::resurrect()
{
   glGenQueries(1, &mQuery);
}

const String GFXGLOcclusionQuery::describeSelf() const
{
   // We've got nothing
   return String();
}
