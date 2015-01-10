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

// Reverb environment presets.
//
// For customized presets, best derive from one of these presets.

singleton SFXEnvironment( AudioEnvOff )
{
   envSize = "7.5";
   envDiffusion = "1.0";
   room = "-10000";
   roomHF = "-10000";
   roomLF = "0";
   decayTime = "1.0";
   decayHFRatio = "1.0";
   decayLFRatio = "1.0";
   reflections = "-2602";
   reflectionsDelay = "0.007";
   reflectionsPan[ 0 ] = "0.0";
   reflectionsPan[ 1 ] = "0.0";
   reflectionsPan[ 2 ] = "0.0";
   reverb = "200";
   reverbDelay = "0.011";
   reverbPan[ 0 ] = "0.0";
   reverbPan[ 1 ] = "0.0";
   reverbPan[ 2 ] = "0.0";
   echoTime = "0.25";
   echoDepth = "0.0";
   modulationTime = "0.25";
   modulationDepth = "0.0";
   airAbsorptionHF = "-5.0";
   HFReference = "5000.0";
   LFReference = "250.0";
   roomRolloffFactor = "0.0";
   diffusion = "0.0";
   density = "0.0";
   flags = 0x33;
};

singleton SFXEnvironment( AudioEnvGeneric )
{
   envSize = "7.5";
   envDiffusion = "1.0";
   room = "-1000";
   roomHF = "-100";
   roomLF = "0";
   decayTime = "1.49";
   decayHFRatio = "0.83";
   decayLFRatio = "1.0";
   reflections = "-2602";
   reflectionsDelay = "0.007";
   reflectionsPan[ 0 ] = "0.0";
   reflectionsPan[ 1 ] = "0.0";
   reflectionsPan[ 2 ] = "0.0";
   reverb = "200";
   reverbDelay = "0.011";
   reverbPan[ 0 ] = "0.0";
   reverbPan[ 1 ] = "0.0";
   reverbPan[ 2 ] = "0.0";
   echoTime = "0.25";
   echoDepth = "0.0";
   modulationTime = "0.25";
   modulationDepth = "0.0";
   airAbsorptionHF = "-5.0";
   HFReference = "5000.0";
   LFReference = "250.0";
   roomRolloffFactor = "0.0";
   diffusion = "100.0";
   density = "100.0";
   flags = 0x3f;
};

singleton SFXEnvironment( AudioEnvRoom )
{
   envSize = "1.9";
   envDiffusion = "1.0";
   room = "-1000";
   roomHF = "-454";
   roomLF = "0";
   decayTime = "0.4";
   decayHFRatio = "0.83";
   decayLFRatio = "1.0";
   reflections = "-1646";
   reflectionsDelay = "0.002";
   reflectionsPan[ 0 ] = "0.0";
   reflectionsPan[ 1 ] = "0.0";
   reflectionsPan[ 2 ] = "0.0";
   reverb = "53";
   reverbDelay = "0.003";
   reverbPan[ 0 ] = "0.0";
   reverbPan[ 1 ] = "0.0";
   reverbPan[ 2 ] = "0.0";
   echoTime = "0.25";
   echoDepth = "0.0";
   modulationTime = "0.25";
   modulationDepth = "0.0";
   airAbsorptionHF = "-5.0";
   HFReference = "5000.0";
   LFReference = "250.0";
   roomRolloffFactor = "0.0";
   diffusion = "100.0";
   density = "100.0";
   flags = 0x3f;
};

singleton SFXEnvironment( AudioEnvPaddedCell )
{
   envSize = "1.4";
   envDiffusion = "1.0";
   room = "-1000";
   roomHF = "-6000";
   roomLF = "0";
   decayTime = "0.17";
   decayHFRatio = "0.1";
   decayLFRatio = "1.0";
   reflections = "-1204";
   reflectionsDelay = "0.001";
   reflectionsPan[ 0 ] = "0.0";
   reflectionsPan[ 1 ] = "0.0";
   reflectionsPan[ 2 ] = "0.0";
   reverb = "207";
   reverbDelay = "0.002";
   reverbPan[ 0 ] = "0.0";
   reverbPan[ 1 ] = "0.0";
   reverbPan[ 2 ] = "0.0";
   echoTime = "0.25";
   echoDepth = "0.0";
   modulationTime = "0.25";
   modulationDepth = "0.0";
   airAbsorptionHF = "-5.0";
   HFReference = "5000.0";
   LFReference = "250.0";
   roomRolloffFactor = "0.0";
   diffusion = "100.0";
   density = "100.0";
   flags = 0x3f;
};

singleton SFXEnvironment( AudioEnvBathroom )
{
   envSize = "1.4";
   envDiffusion = "1.0";
   room = "-1000";
   roomHF = "-1200";
   roomLF = "0";
   decayTime = "1.49";
   decayHFRatio = "0.54";
   decayLFRatio = "1.0";
   reflections = "-370";
   reflectionsDelay = "0.007";
   reflectionsPan[ 0 ] = "0.0";
   reflectionsPan[ 1 ] = "0.0";
   reflectionsPan[ 2 ] = "0.0";
   reverb = "1030";
   reverbDelay = "0.011";
   reverbPan[ 0 ] = "0.0";
   reverbPan[ 1 ] = "0.0";
   reverbPan[ 2 ] = "0.0";
   echoTime = "0.25";
   echoDepth = "0.0";
   modulationTime = "0.25";
   modulationDepth = "0.0";
   airAbsorptionHF = "-5.0";
   HFReference = "5000.0";
   LFReference = "250.0";
   roomRolloffFactor = "0.0";
   diffusion = "100.0";
   density = "60.0";
   flags = 0x3f;
};

singleton SFXEnvironment( AudioEnvLivingRoom )
{
   envSize = "2.5";
   envDiffusion = "1.0";
   room = "-1000";
   roomHF = "-6000";
   roomLF = "0";
   decayTime = "0.5";
   decayHFRatio = "0.1";
   decayLFRatio = "1.0";
   reflections = "-1376";
   reflectionsDelay = "0.003";
   reflectionsPan[ 0 ] = "0.0";
   reflectionsPan[ 1 ] = "0.0";
   reflectionsPan[ 2 ] = "0.0";
   reverb = "-1104";
   reverbDelay = "0.004";
   reverbPan[ 0 ] = "0.0";
   reverbPan[ 1 ] = "0.0";
   reverbPan[ 2 ] = "0.0";
   echoTime = "0.25";
   echoDepth = "0.0";
   modulationTime = "0.25";
   modulationDepth = "0.0";
   airAbsorptionHF = "-5.0";
   HFReference = "5000.0";
   LFReference = "250.0";
   roomRolloffFactor = "0.0";
   diffusion = "100.0";
   density = "100.0";
   flags = 0x3f;
};

singleton SFXEnvironment( AudioEnvStoneRoom )
{
   envSize = "11.6";
   envDiffusion = "1.0";
   room = "-1000";
   roomHF = "300";
   roomLF = "0";
   decayTime = "2.31";
   decayHFRatio = "0.64";
   decayLFRatio = "1.0";
   reflections = "-711";
   reflectionsDelay = "0.012";
   reflectionsPan[ 0 ] = "0.0";
   reflectionsPan[ 1 ] = "0.0";
   reflectionsPan[ 2 ] = "0.0";
   reverb = "83";
   reverbDelay = "0.017";
   reverbPan[ 0 ] = "0.0";
   reverbPan[ 1 ] = "0.0";
   reverbPan[ 2 ] = "0.0";
   echoTime = "0.25";
   echoDepth = "0.0";
   modulationTime = "0.25";
   modulationDepth = "0.0";
   airAbsorptionHF = "-5.0";
   HFReference = "-5000.0";
   LFReference = "250.0";
   roomRolloffFactor = "0.0";
   diffusion = "100.0";
   density = "100.0";
   flags = 0x3f;
};

singleton SFXEnvironment( AudioEnvAuditorium )
{
   envSize = "21.6";
   envDiffusion = "1.0";
   room = "-1000";
   roomHF = "-476";
   roomLF = "0";
   decayTime = "4.32";
   decayHFRatio = "0.59";
   decayLFRatio = "1.0";
   reflections = "0.789";
   reflectionsDelay = "0.02";
   reflectionsPan[ 0 ] = "0.0";
   reflectionsPan[ 1 ] = "0.0";
   reflectionsPan[ 2 ] = "0.0";
   reverb = "-289";
   reverbDelay = "0.03";
   reverbPan[ 0 ] = "0.0";
   reverbPan[ 1 ] = "0.0";
   reverbPan[ 2 ] = "0.0";
   echoTime = "0.25";
   echoDepth = "0.0";
   modulationTime = "0.25";
   modulationDepth = "0.0";
   airAbsorptionHF = "-5.0";
   HFReference = "5000.0";
   LFReference = "250.0";
   roomRolloffFactor = "0.0";
   diffusion = "100.0";
   density = "100.0";
   flags = 0x3f;
};

singleton SFXEnvironment( AudioEnvConcertHall )
{
   envSize = "19.6";
   envDiffusion = "1.0";
   room = "-1000";
   roomHF = "-500";
   roomLF = "0";
   decayTime = "3.92";
   decayHFRatio = "0.7";
   decayLFRatio = "1.0";
   reflections = "-1230";
   reflectionsDelay = "0.02";
   reflectionsPan[ 0 ] = "0.0";
   reflectionsPan[ 1 ] = "0.0";
   reflectionsPan[ 2 ] = "0.0";
   reverb = "-2";
   reverbDelay = "0.029";
   reverbPan[ 0 ] = "0.0";
   reverbPan[ 1 ] = "0.0";
   reverbPan[ 2 ] = "0.0";
   echoTime = "0.25";
   echoDepth = "0.0";
   modulationTime = "0.25";
   modulationDepth = "0.0";
   airAbsorptionHF = "-5.0";
   HFReference = "5000.0";
   LFReference = "250.0";
   roomRolloffFactor = "0.0";
   diffusion = "100.0";
   density = "100.0";
   flags = 0x3f;
};

singleton SFXEnvironment( AudioEnvCave )
{
   envSize = "14.6";
   envDiffusion = "1.0";
   room = "-1000";
   roomHF = "0";
   roomLF = "0";
   decayTime = "2.91";
   decayHFRatio = "1.3";
   decayLFRatio = "1.0";
   reflections = "-602";
   reflectionsDelay = "0.015";
   reflectionsPan[ 0 ] = "0.0";
   reflectionsPan[ 1 ] = "0.0";
   reflectionsPan[ 2 ] = "0.0";
   reverb = "-302";
   reverbDelay = "0.022";
   reverbPan[ 0 ] = "0.0";
   reverbPan[ 1 ] = "0.0";
   reverbPan[ 2 ] = "0.0";
   echoTime = "0.25";
   echoDepth = "0.0";
   modulationTime = "0.25";
   modulationDepth = "0.0";
   airAbsorptionHF = "-5.0";
   HFReference = "5000.0";
   LFReference = "250.0";
   roomRolloffFactor = "0.0";
   diffusion = "100.0";
   density = "100.0";
   flags = 0x1f;
};

singleton SFXEnvironment( AudioEnvArena )
{
   envSize = "36.2f";
   envDiffusion = "1.0";
   room = "-1000";
   roomHF = "-698";
   roomLF = "0";
   decayTime = "7.24";
   decayHFRatio = "0.33";
   decayLFRatio = "1.0";
   reflections = "-1166";
   reflectionsDelay = "0.02";
   reflectionsPan[ 0 ] = "0.0";
   reflectionsPan[ 1 ] = "0.0";
   reflectionsPan[ 2 ] = "0.0";
   reverb = "16";
   reverbDelay = "0.03";
   reverbPan[ 0 ] = "0.0";
   reverbPan[ 1 ] = "0.0";
   reverbPan[ 2 ] = "0.0";
   echoTime = "0.25";
   echoDepth = "0.0";
   modulationTime = "0.25";
   modulationDepth = "0.0";
   airAbsorptionHF = "-5.0";
   HFReference = "5000.0";
   LFReference = "250.0";
   roomRolloffFactor = "0.0";
   diffusion = "100.0";
   density = "100.0";
   flags = 0x3f;
};

singleton SFXEnvironment( AudioEnvHangar )
{
   envSize = "50.3";
   envDiffusion = "1.0";
   room = "-1000";
   roomHF = "-1000";
   roomLF = "0";
   decayTime = "10.05";
   decayHFRatio = "0.23";
   decayLFRatio = "1.0";
   reflections = "-602";
   reflectionsDelay = "0.02";
   reflectionsPan[ 0 ] = "0.0";
   reflectionsPan[ 1 ] = "0.0";
   reflectionsPan[ 2 ] = "0.0";
   reverb = "198";
   reverbDelay = "0.03";
   reverbPan[ 0 ] = "0.0";
   reverbPan[ 1 ] = "0.0";
   reverbPan[ 2 ] = "0.0";
   echoTime = "0.25";
   echoDepth = "0.0";
   modulationTime = "0.25";
   modulationDepth = "0.0";
   airAbsorptionHF = "-5.0";
   HFReference = "5000.0";
   LFReference = "250.0";
   roomRolloffFactor = "0.0";
   diffusion = "100.0";
   density = "100.0";
   flags = 0x3f;
};

singleton SFXEnvironment( AudioEnvCarpettedHallway )
{
   envSize = "1.9";
   envDiffusion = "1.0";
   room = "-1000";
   roomHF = "-4000";
   roomLF = "0";
   decayTime = "0.3";
   decayHFRatio = "0.1";
   decayLFRatio = "1.0";
   reflections = "-1831";
   reflectionsDelay = "0.002";
   reflectionsPan[ 0 ] = "0.0";
   reflectionsPan[ 1 ] = "0.0";
   reflectionsPan[ 2 ] = "0.0";
   reverb = "-1630";
   reverbDelay = "0.03";
   reverbPan[ 0 ] = "0.0";
   reverbPan[ 1 ] = "0.0";
   reverbPan[ 2 ] = "0.0";
   echoTime = "0.25";
   echoDepth = "0.0";
   modulationTime = "0.25";
   modulationDepth = "0.0";
   airAbsorptionHF = "-5.0";
   HFReference = "5000.0";
   LFReference = "250.0";
   roomRolloffFactor = "0.0";
   diffusion = "100.0";
   density = "100.0";
   flags = 0x3f;
};

singleton SFXEnvironment( AudioEnvHallway )
{
   envSize = "1.8";
   envDiffusion = "1.0";
   room = "-1000";
   roomHF = "-300";
   roomLF = "0";
   decayTime = "1.49";
   decayHFRatio = "0.59";
   decayLFRatio = "1.0";
   reflections = "-1219";
   reflectionsDelay = "0.007";
   reflectionsPan[ 0 ] = "0.0";
   reflectionsPan[ 1 ] = "0.0";
   reflectionsPan[ 2 ] = "0.0";
   reverb = "441";
   reverbDelay = "0.011";
   reverbPan[ 0 ] = "0.0";
   reverbPan[ 1 ] = "0.0";
   reverbPan[ 2 ] = "0.0";
   echoTime = "0.25";
   echoDepth = "0.0";
   modulationTime = "0.25";
   modulationDepth = "0.0";
   airAbsorptionHF = "-5.0";
   HFReference = "5000.0";
   LFReference = "250.0";
   roomRolloffFactor = "0.0";
   diffusion = "100.0";
   density = "100.0";
   flags = 0x3f;
};

singleton SFXEnvironment( AudioEnvStoneCorridor )
{
   envSize = "13.5";
   envDiffusion = "1.0";
   room = "-1000";
   roomHF = "-237";
   roomLF = "0";
   decayTime = "2.7";
   decayHFRatio = "0.79";
   decayLFRatio = "1.0";
   reflections = "-1214";
   reflectionsDelay = "0.013";
   reflectionsPan[ 0 ] = "0.0";
   reflectionsPan[ 1 ] = "0.0";
   reflectionsPan[ 2 ] = "0.0";
   reverb = "395";
   reverbDelay = "0.02";
   reverbPan[ 0 ] = "0.0";
   reverbPan[ 1 ] = "0.0";
   reverbPan[ 2 ] = "0.0";
   echoTime = "0.25";
   echoDepth = "0.0";
   modulationTime = "0.25";
   modulationDepth = "0.0";
   airAbsorptionHF = "-5.0";
   HFReference = "5000.0";
   LFReference = "250.0";
   roomRolloffFactor = "0.0";
   diffusion = "100.0";
   density = "100.0";
   flags = 0x3f;
};

singleton SFXEnvironment( AudioEnvAlley )
{
   envSize = "7.5";
   envDiffusion = "0.3";
   room = "-1000";
   roomHF = "-270";
   roomLF = "0";
   decayTime = "1.49";
   decayHFRatio = "0.86";
   decayLFRatio = "1.0";
   reflections = "-1204";
   reflectionsDelay = "0.007";
   reflectionsPan[ 0 ] = "0.0";
   reflectionsPan[ 1 ] = "0.0";
   reflectionsPan[ 2 ] = "0.0";
   reverb = "-4";
   reverbDelay = "0.011";
   reverbPan[ 0 ] = "0.0";
   reverbPan[ 1 ] = "0.0";
   reverbPan[ 2 ] = "0.0";
   echoTime = "0.125";
   echoDepth = "0.95";
   modulationTime = "0.25";
   modulationDepth = "0.0";
   airAbsorptionHF = "-5.0";
   HFReference = "5000.0";
   LFReference = "250.0";
   roomRolloffFactor = "0.0";
   diffusion = "100.0";
   density = "100.0";
   flags = 0x3f;
};

singleton SFXEnvironment( AudioEnvForest )
{
   envSize = "38.0";
   envDiffusion = "0.3";
   room = "-1000";
   roomHF = "-3300";
   roomLF = "0";
   decayTime = "1.49";
   decayHFRatio = "0.54";
   decayLFRatio = "1.0";
   reflections = "-2560";
   reflectionsDelay = "0.162";
   reflectionsPan[ 0 ] = "0.0";
   reflectionsPan[ 1 ] = "0.0";
   reflectionsPan[ 2 ] = "0.0";
   reverb = "-229";
   reverbDelay = "0.088";
   reverbPan[ 0 ] = "0.0";
   reverbPan[ 1 ] = "0.0";
   reverbPan[ 2 ] = "0.0";
   echoTime = "0.125";
   echoDepth = "1.0";
   modulationTime = "0.25";
   modulationDepth = "0.0";
   airAbsorptionHF = "-5.0";
   HFReference = "5000.0";
   LFReference = "250.0";
   roomRolloffFactor = "0.0";
   diffusion = "79.0";
   density = "100.0";
   flags = 0x3f;
};

singleton SFXEnvironment( AudioEnvCity )
{
  envSize = "7.5";
  envDiffusion = "0.5";
  room = "-1000";
  roomHF = "-800";
  roomLF = "0";
  decayTime = "1.49";
  decayHFRatio = "0.67";
  decayLFRatio = "1.0";
  reflections = "-2273";
  reflectionsDelay = "0.007";
  reflectionsPan[ 0 ] = "0.0";
  reflectionsPan[ 1 ] = "0.0";
  reflectionsPan[ 2 ] = "0.0";
  reverb = "-1691";
  reverbDelay = "0.011";
  reverbPan[ 0 ] = "0.0";
  reverbPan[ 1 ] = "0.0";
  reverbPan[ 2 ] = "0.0";
  echoTime = "0.25";
  echoDepth = "0.0";
  modulationTime = "0.25";
  modulationDepth = "0.0";
  airAbsorptionHF = "-5.0";
  HFReference = "5000.0";
  LFReference = "250.0";
  roomRolloffFactor = "0.0";
  diffusion = "50.0";
  density = "100.0";
  flags = 0x3f;
};

singleton SFXEnvironment( AudioEnvMountains )
{
  envSize = "100.0";
  envDiffusion = "0.27";
  room = "-1000";
  roomHF = "-2500";
  roomLF = "0";
  decayTime = "1.49";
  decayHFRatio = "0.21";
  decayLFRatio = "1.0";
  reflections = "-2780";
  reflectionsDelay = "0.3";
  reflectionsPan[ 0 ] = "0.0";
  reflectionsPan[ 1 ] = "0.0";
  reflectionsPan[ 2 ] = "0.0";
  reverb = "-1434";
  reverbDelay = "0.1";
  reverbPan[ 0 ] = "0.0";
  reverbPan[ 1 ] = "0.0";
  reverbPan[ 2 ] = "0.0";
  echoTime = "0.25";
  echoDepth = "1.0";
  modulationTime = "0.25";
  modulationDepth = "0.0";
  airAbsorptionHF = "-5.0";
  HFReference = "5000.0";
  LFReference = "250.0";
  roomRolloffFactor = "0.0";
  diffusion = "27.0";
  density = "100.0";
  flags = 0x1f;
};

singleton SFXEnvironment( AudioEnvQuary )
{
  envSize = "17.5";
  envDiffusion = "1.0";
  room = "-1000";
  roomHF = "-1000";
  roomLF = "0";
  decayTime = "1.49";
  decayHFRatio = "0.83";
  decayLFRatio = "1.0";
  reflections = "-10000";
  reflectionsDelay = "0.061";
  reflectionsPan[ 0 ] = "0.0";
  reflectionsPan[ 1 ] = "0.0";
  reflectionsPan[ 2 ] = "0.0";
  reverb = "500";
  reverbDelay = "0.025";
  reverbPan[ 0 ] = "0.0";
  reverbPan[ 1 ] = "0.0";
  reverbPan[ 2 ] = "0.0";
  echoTime = "0.125";
  echoDepth = "0.7";
  modulationTime = "0.25";
  modulationDepth = "0.0";
  airAbsorptionHF = "-5.0";
  HFReference = "5000.0";
  LFReference = "250.0";
  roomRolloffFactor = "0.0";
  diffusion = "100.0";
  density = "100.0";
  flags = 0x3f;
};

singleton SFXEnvironment( AudioEnvPlain )
{
  envSize = "42.5";
  envDiffusion = "0.21";
  room = "-1000";
  roomHF = "-2000";
  roomLF = "0";
  decayTime = "1.49";
  decayHFRatio = "0.5";
  decayLFRatio = "1.0";
  reflections = "-2466";
  reflectionsDelay = "0.179";
  reflectionsPan[ 0 ] = "0.0";
  reflectionsPan[ 1 ] = "0.0";
  reflectionsPan[ 2 ] = "0.0";
  reverb = "-1926";
  reverbDelay = "0.1";
  reverbPan[ 0 ] = "0.0";
  reverbPan[ 1 ] = "0.0";
  reverbPan[ 2 ] = "0.0";
  echoTime = "0.25";
  echoDepth = "1.0";
  modulationTime = "0.25";
  modulationDepth = "0.0";
  airAbsorptionHF = "-5.0";
  HFReference = "5000.0";
  LFReference = "250.0";
  roomRolloffFactor = "0.0";
  diffusion = "21.0";
  density = "100.0";
  flags = 0x3f;
};

singleton SFXEnvironment( AudioEnvParkingLot )
{
  envSize = "8.3";
  envDiffusion = "1.0";
  room = "-1000";
  roomHF = "0";
  roomLF = "0";
  decayTime = "1.65";
  decayHFRatio = "1.5";
  decayLFRatio = "1.0";
  reflections = "-1363";
  reflectionsDelay = "0.008";
  reflectionsPan[ 0 ] = "0.0";
  reflectionsPan[ 1 ] = "0.0";
  reflectionsPan[ 2 ] = "0.0";
  reverb = "-1153";
  reverbDelay = "0.012";
  reverbPan[ 0 ] = "0.0";
  reverbPan[ 1 ] = "0.0";
  reverbPan[ 2 ] = "0.0";
  echoTime = "0.25";
  echoDepth = "0.0";
  modulationTime = "0.25";
  modulationDepth = "0.0";
  airAbsorptionHF = "-5.0";
  HFReference = "5000.0";
  LFReference = "250.0";
  roomRolloffFactor = "0.0";
  diffusion = "100.0";
  density = "100.0";
  flags = 0x1f;
};

singleton SFXEnvironment( AudioEnvSewerPipe )
{
  envSize = "1.7";
  envDiffusion = "0.8";
  room = "-1000";
  roomHF = "-1000";
  roomLF = "0";
  decayTime = "2.81";
  decayHFRatio = "0.14";
  decayLFRatio = "1.0";
  reflections = "429";
  reflectionsDelay = "0.014";
  reflectionsPan[ 0 ] = "0.0";
  reflectionsPan[ 1 ] = "0.0";
  reflectionsPan[ 2 ] = "0.0";
  reverb = "1023";
  reverbDelay = "0.21";
  reverbPan[ 0 ] = "0.0";
  reverbPan[ 1 ] = "0.0";
  reverbPan[ 2 ] = "0.0";
  echoTime = "0.25";
  echoDepth = "0.0";
  modulationTime = "0.25";
  modulationDepth = "0.0";
  airAbsorptionHF = "-5.0";
  HFReference = "5000.0";
  LFReference = "250.0";
  roomRolloffFactor = "0.0";
  diffusion = "80.0";
  density = "60.0";
  flags = 0x3f;
};

singleton SFXEnvironment( AudioEnvUnderwater )
{
  envSize = "1.8";
  envDiffusion = "1.0";
  room = "-1000";
  roomHF = "-4000";
  roomLF = "0";
  decayTime = "1.49";
  decayHFRatio = "0.1";
  decayLFRatio = "1.0";
  reflections = "-449";
  reflectionsDelay = "0.007";
  reflectionsPan[ 0 ] = "0.0";
  reflectionsPan[ 1 ] = "0.0";
  reflectionsPan[ 2 ] = "0.0";
  reverb = "1700";
  reverbDelay = "0.011";
  reverbPan[ 0 ] = "0.0";
  reverbPan[ 1 ] = "0.0";
  reverbPan[ 2 ] = "0.0";
  echoTime = "0.25";
  echoDepth = "0.0";
  modulationTime = "1.18";
  modulationDepth = "0.348";
  airAbsorptionHF = "-5.0";
  HFReference = "5000.0";
  LFReference = "250.0";
  roomRolloffFactor = "0.0";
  diffusion = "100.0";
  density = "100.0";
  flags = 0x3f;
};

singleton SFXEnvironment( AudioEnvDrugged )
{
  envSize = "1.9";
  envDiffusion = "0.5";
  room = "-1000";
  roomHF = "0";
  roomLF = "0";
  decayTime = "8.39";
  decayHFRatio = "1.39";
  decayLFRatio = "1.0";
  reflections = "-115";
  reflectionsDelay = "0.002";
  reflectionsPan[ 0 ] = "0.0";
  reflectionsPan[ 1 ] = "0.0";
  reflectionsPan[ 2 ] = "0.0";
  reverb = "985";
  reverbDelay = "0.03";
  reverbPan[ 0 ] = "0.0";
  reverbPan[ 1 ] = "0.0";
  reverbPan[ 2 ] = "0.0";
  echoTime = "0.25";
  echoDepth = "0.0";
  modulationTime = "0.25";
  modulationDepth = "1.0";
  airAbsorptionHF = "-5.0";
  HFReference = "5000.0";
  LFReference = "250.0";
  roomRolloffFactor = "0.0";
  diffusion = "100.0";
  density = "100.0";
  flags = 0x1f;
};

singleton SFXEnvironment( AudioEnvDizzy )
{
  envSize = "1.8";
  envDiffusion = "0.6";
  room = "-1000.0";
  roomHF = "-400";
  roomLF = "0";
  decayTime = "17.23";
  decayHFRatio = "0.56";
  decayLFRatio = "1.0";
  reflections = "-1713";
  reflectionsDelay = "0.02";
  reflectionsPan[ 0 ] = "0.0";
  reflectionsPan[ 1 ] = "0.0";
  reflectionsPan[ 2 ] = "0.0";
  reverb = "-613";
  reverbDelay = "0.03";
  reverbPan[ 0 ] = "0.0";
  reverbPan[ 1 ] = "0.0";
  reverbPan[ 2 ] = "0.0";
  echoTime = "0.25";
  echoDepth = "1.0";
  modulationTime = "0.81";
  modulationDepth = "0.31";
  airAbsorptionHF = "-5.0";
  HFReference = "5000.0";
  LFReference = "250.0";
  roomRolloffFactor = "0.0";
  diffusion = "100.0";
  density = "100.0";
  flags = 0x1f;
};

singleton SFXEnvironment( AudioEnvPsychotic )
{
  envSize = "1.0";
  envDiffusion = "0.5";
  room = "-1000";
  roomHF = "-151";
  roomLF = "0";
  decayTime = "7.56";
  decayHFRatio = "0.91";
  decayLFRatio = "1.0";
  reflections = "-626";
  reflectionsDelay = "0.02";
  reflectionsPan[ 0 ] = "0.0";
  reflectionsPan[ 1 ] = "0.0";
  reflectionsPan[ 2 ] = "0.0";
  reverb = "774";
  reverbDelay = "0.03";
  reverbPan[ 0 ] = "0.0";
  reverbPan[ 1 ] = "0.0";
  reverbPan[ 2 ] = "0.0";
  echoTime = "0.25";
  echoDepth = "0.0";
  modulationTime = "4.0";
  modulationDepth = "1.0";
  airAbsorptionHF = "-5.0";
  HFReference = "5000.0";
  LFReference = "250.0";
  roomRolloffFactor = "0.0";
  diffusion = "100.0";
  density = "100.0";
  flags = 0x1f;
};
