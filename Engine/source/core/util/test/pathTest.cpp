//-----------------------------------------------------------------------------
// Copyright (c) 2014 GarageGames, LLC
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

#ifdef TORQUE_TESTS_ENABLED
#include "testing/unitTesting.h"
#include "core/util/path.h"

TEST(MakeRelativePath, MakeRelativePath)
{
   EXPECT_EQ(Torque::Path::MakeRelativePath("art/interiors/burg/file.png", "art/interiors/"), "burg/file.png");
   EXPECT_EQ(Torque::Path::MakeRelativePath("art/interiors/file.png", "art/interiors/burg/"), "../file.png");
   EXPECT_EQ(Torque::Path::MakeRelativePath("art/file.png", "art/interiors/burg/"), "../../file.png");
   EXPECT_EQ(Torque::Path::MakeRelativePath("file.png", "art/interiors/burg/"), "../../../file.png");
   EXPECT_EQ(Torque::Path::MakeRelativePath("art/interiors/burg/file.png", "art/interiors/burg/"), "file.png");
   EXPECT_EQ(Torque::Path::MakeRelativePath("art/interiors/camp/file.png", "art/interiors/burg/"), "../camp/file.png");
   EXPECT_EQ(Torque::Path::MakeRelativePath("art/interiors/burg/file.png", "art/shapes/"), "../interiors/burg/file.png");
   EXPECT_EQ(Torque::Path::MakeRelativePath("levels/den/file.png", "art/interiors/burg/"), "../../../levels/den/file.png");
   EXPECT_EQ(Torque::Path::MakeRelativePath("art/interiors/burg/file.png", "art/dts/burg/"), "../../interiors/burg/file.png");
};

#endif