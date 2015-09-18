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

#ifndef _TSFORESTCELLBATCH_H_
#define _TSFORESTCELLBATCH_H_

#ifndef _IMPOSTERRENDERMGR_H_
#include "renderInstance/renderImposterMgr.h"
#endif
#ifndef _FORESTCELLBATCH_H_
#include "forest/forestCellBatch.h"
#endif
#ifndef _GFXTEXTUREHANDLE_H_
#include "gfx/gfxTextureHandle.h"
#endif
#ifndef _GFXVERTEXBUFFER_H_
#include "gfx/gfxVertexBuffer.h"
#endif
#ifndef _GFXSTRUCTS_H_
#include "gfx/gfxStructs.h"
#endif

class GFXShader;


class TSForestCellBatch : public ForestCellBatch
{
protected:
   
   /// We use the same shader and vertex format as TSLastDetail.
   GFXVertexBufferHandle<ImposterState> mVB;

   TSLastDetail *mDetail;

   // ForestCellBatch
   virtual bool _prepBatch( const ForestItem &item );
   virtual void _rebuildBatch();
   virtual void _render( const SceneRenderState *state );

public:
   
   TSForestCellBatch( TSLastDetail *detail );

   virtual ~TSForestCellBatch();

};


#endif // _TSFORESTCELLBATCH_H_
