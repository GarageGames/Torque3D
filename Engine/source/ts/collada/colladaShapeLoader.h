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

#ifndef _COLLADA_SHAPELOADER_H_
#define _COLLADA_SHAPELOADER_H_

#ifndef _TSSHAPELOADER_H_
#include "ts/loader/tsShapeLoader.h"
#endif

class domCOLLADA;
class domAnimation;
struct AnimChannels;

//-----------------------------------------------------------------------------
class ColladaShapeLoader : public TSShapeLoader
{
   friend TSShape* loadColladaShape(const Torque::Path &path);

   domCOLLADA*             root;
   Vector<AnimChannels*>   animations;       ///< Holds all animation channels for deletion after loading

   void processAnimation(const domAnimation* anim, F32& maxEndTime, F32& minFrameTime);

   void cleanup();

public:
   ColladaShapeLoader(domCOLLADA* _root);
   ~ColladaShapeLoader();

   void enumerateScene();
   bool ignoreNode(const String& name);
   bool ignoreMesh(const String& name);
   void computeBounds(Box3F& bounds);

   static bool canLoadCachedDTS(const Torque::Path& path);
   static bool checkAndMountSketchup(const Torque::Path& path, String& mountPoint, Torque::Path& daePath);
   static domCOLLADA* getDomCOLLADA(const Torque::Path& path);
   static domCOLLADA* readColladaFile(const String& path);
};

#endif // _COLLADA_SHAPELOADER_H_
