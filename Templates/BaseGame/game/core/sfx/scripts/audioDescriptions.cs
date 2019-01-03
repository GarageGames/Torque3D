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

// Always declare SFXDescription's (the type of sound) before SFXProfile's (the
// sound itself) when creating new ones

//-----------------------------------------------------------------------------
// 3D Sounds
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Single shot sounds
//-----------------------------------------------------------------------------

singleton SFXDescription( AudioDefault3D : AudioEffect )
{
   is3D              = true;
   ReferenceDistance = 20.0;
   MaxDistance       = 100.0;
};

singleton SFXDescription( AudioSoft3D : AudioEffect )
{
   is3D              = true;
   ReferenceDistance = 20.0;
   MaxDistance       = 100.0;
   volume = 0.4;
};

singleton SFXDescription( AudioClose3D : AudioEffect )
{
   is3D              = true;
   ReferenceDistance = 10.0;
   MaxDistance       = 60.0;
};

singleton SFXDescription( AudioClosest3D : AudioEffect )
{
   is3D              = true;
   ReferenceDistance = 5.0;
   MaxDistance       = 10.0;
};

//-----------------------------------------------------------------------------
// Looping sounds
//-----------------------------------------------------------------------------

singleton SFXDescription( AudioDefaultLoop3D : AudioEffect )
{
   isLooping         = true;
   is3D              = true;
   ReferenceDistance = 20.0;
   MaxDistance       = 100.0;
};

singleton SFXDescription( AudioCloseLoop3D : AudioEffect )
{
   isLooping         = true;
   is3D              = true;
   ReferenceDistance = 18.0;
   MaxDistance       = 25.0;
};

singleton SFXDescription( AudioClosestLoop3D : AudioEffect )
{
   isLooping         = true;
   is3D              = true;
   ReferenceDistance = 5.0;
   MaxDistance       = 10.0;
};

//-----------------------------------------------------------------------------
// 2d sounds
//-----------------------------------------------------------------------------

// Used for non-looping environmental sounds (like power on, power off)
singleton SFXDescription( Audio2D : AudioEffect )
{
   isLooping         = false;
};

// Used for Looping Environmental Sounds
singleton SFXDescription( AudioLoop2D : AudioEffect )
{
   isLooping         = true;
};

singleton SFXDescription( AudioStream2D : AudioEffect )
{
   isStreaming       = true;
};
singleton SFXDescription( AudioStreamLoop2D : AudioEffect )
{
   isLooping         = true;
   isStreaming       = true;
};

//-----------------------------------------------------------------------------
// Music
//-----------------------------------------------------------------------------

singleton SFXDescription( AudioMusic2D : AudioMusic )
{
   isStreaming       = true;
};

singleton SFXDescription( AudioMusicLoop2D : AudioMusic )
{
   isLooping         = true;
   isStreaming       = true;
};

singleton SFXDescription( AudioMusic3D : AudioMusic )
{
   isStreaming       = true;
   is3D              = true;
};

singleton SFXDescription( AudioMusicLoop3D : AudioMusic )
{
   isStreaming       = true;
   is3D              = true;
   isLooping         = true;
};
