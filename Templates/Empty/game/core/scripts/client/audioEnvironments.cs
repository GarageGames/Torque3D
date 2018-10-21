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

singleton SFXEnvironment(Generic) 
{
reverbDensity = "1.000";
reverbDiffusion = "1.000";
reverbGain = "0.3162";
reverbGainHF = "0.8913";
reverbGainLF = "1.000";
reverbDecayTime = "1.4900";
reverbDecayHFRatio = "0.8300";
reverbDecayLFRatio = "1.0000";
reflectionsGain = "0.0500";
reflectionDelay = "0.0070";
reflectionsPan[ 0 ] = "0.0";
reflectionsPan[ 1 ] = "0.0";
reflectionsPan[ 2 ] = "0.0";
lateReverbGain = "1.2589";
lateReverbDelay = "0.0110";
lateReverbPan[ 0 ] = "0.0";
lateReverbPan[ 1 ] = "0.0";
lateReverbPan[ 2 ] = "0.0";
reverbEchoTime = "0.2500";
reverbEchoDepth = "0.0000";
reverbModTime = "0.2500";
reverbModDepth = "0.0000";
airAbsorbtionGainHF = "0.9943";
reverbHFRef = "5000.0000";
reverbLFRef = "250.0000";
roomRolloffFactor = "0.0000";
decayHFLimit = "1";
};

singleton SFXEnvironment(PaddedCell) 
{
reverbDensity = "0.1715";
reverbDiffusion = "1.000";
reverbGain = "0.3162";
reverbGainHF = "0.0010";
reverbGainLF = "1.000";
reverbDecayTime = "0.1700";
reverbDecayHFRatio = "0.1000";
reverbDecayLFRatio = "1.0000";
reflectionsGain = "0.2500";
reflectionDelay = "0.0010";
reflectionsPan[ 0 ] = "0.0";
reflectionsPan[ 1 ] = "0.0";
reflectionsPan[ 2 ] = "0.0";
lateReverbGain = "1.2691";
lateReverbDelay = "0.0020";
lateReverbPan[ 0 ] = "0.0";
lateReverbPan[ 1 ] = "0.0";
lateReverbPan[ 2 ] = "0.0";
reverbEchoTime = "0.2500";
reverbEchoDepth = "0.0000";
reverbModTime = "0.2500";
reverbModDepth = "0.0000";
airAbsorbtionGainHF = "0.9943";
reverbHFRef = "5000.0000";
reverbLFRef = "250.0000";
roomRolloffFactor = "0.0000";
decayHFLimit = "1";
};

singleton SFXEnvironment(PresetRoom) 
{
reverbDensity = "0.4287";
reverbDiffusion = "1.000";
reverbGain = "0.3162";
reverbGainHF = "0.5929";
reverbGainLF = "1.000";
reverbDecayTime = "0.4000";
reverbDecayHFRatio = "0.8300";
reverbDecayLFRatio = "1.0000";
reflectionsGain = "0.1503";
reflectionDelay = "0.0020";
reflectionsPan[ 0 ] = "0.0";
reflectionsPan[ 1 ] = "0.0";
reflectionsPan[ 2 ] = "0.0";
lateReverbGain = "1.0629";
lateReverbDelay = "0.0030";
lateReverbPan[ 0 ] = "0.0";
lateReverbPan[ 1 ] = "0.0";
lateReverbPan[ 2 ] = "0.0";
reverbEchoTime = "0.2500";
reverbEchoDepth = "0.0000";
reverbModTime = "0.2500";
reverbModDepth = "0.0000";
airAbsorbtionGainHF = "0.9943";
reverbHFRef = "5000.0000";
reverbLFRef = "250.0000";
roomRolloffFactor = "0.0000";
decayHFLimit = "1";
};

singleton SFXEnvironment(PresetBathroom) 
{
reverbDensity = "0.1715";
reverbDiffusion = "1.000";
reverbGain = "0.3162";
reverbGainHF = "0.2512";
reverbGainLF = "1.000";
reverbDecayTime = "1.4900";
reverbDecayHFRatio = "0.5400";
reverbDecayLFRatio = "1.0000";
reflectionsGain = "0.6531";
reflectionDelay = "0.0070";
reflectionsPan[ 0 ] = "0.0";
reflectionsPan[ 1 ] = "0.0";
reflectionsPan[ 2 ] = "0.0";
lateReverbGain = "3.2734";
lateReverbDelay = "0.0110";
lateReverbPan[ 0 ] = "0.0";
lateReverbPan[ 1 ] = "0.0";
lateReverbPan[ 2 ] = "0.0";
reverbEchoTime = "0.2500";
reverbEchoDepth = "0.0000";
reverbModTime = "0.2500";
reverbModDepth = "0.0000";
airAbsorbtionGainHF = "0.9943";
reverbHFRef = "5000.0000";
reverbLFRef = "250.0000";
roomRolloffFactor = "0.0000";
decayHFLimit = "1";
};

singleton SFXEnvironment(PresetLivingroom) 
{
reverbDensity = "0.9766";
reverbDiffusion = "1.000";
reverbGain = "0.3162";
reverbGainHF = "0.0010";
reverbGainLF = "1.0000";
reverbDecayTime = "0.0900";
reverbDecayHFRatio = "0.5000";
reverbDecayLFRatio = "1.0000";
reflectionsGain = "0.2051";
reflectionDelay = "0.0030";
reflectionsPan[ 0 ] = "0.0";
reflectionsPan[ 1 ] = "0.0";
reflectionsPan[ 2 ] = "0.0";
lateReverbGain = "0.2805";
lateReverbDelay = "0.0040";
lateReverbPan[ 0 ] = "0.0";
lateReverbPan[ 1 ] = "0.0";
lateReverbPan[ 2 ] = "0.0";
reverbEchoTime = "0.2500";
reverbEchoDepth = "0.0000";
reverbModTime = "0.2500";
reverbModDepth = "0.0000";
airAbsorbtionGainHF = "0.9943";
reverbHFRef = "5000.0000";
reverbLFRef = "250.0000";
roomRolloffFactor = "0.0000";
decayHFLimit = "1";
};

singleton SFXEnvironment(PresetStoneroom) 
{
reverbDensity = "1.000";
reverbDiffusion = "1.000";
reverbGain = "0.3162";
reverbGainHF = "0.7079";
reverbGainLF = "1.0000";
reverbDecayTime = "2.3100";
reverbDecayHFRatio = "0.6400";
reverbDecayLFRatio = "1.0000";
reflectionsGain = "0.4411";
reflectionDelay = "0.0120";
reflectionsPan[ 0 ] = "0.0";
reflectionsPan[ 1 ] = "0.0";
reflectionsPan[ 2 ] = "0.0";
lateReverbGain = "1.1003";
lateReverbDelay = "0.0170";
lateReverbPan[ 0 ] = "0.0";
lateReverbPan[ 1 ] = "0.0";
lateReverbPan[ 2 ] = "0.0";
reverbEchoTime = "0.2500";
reverbEchoDepth = "0.0000";
reverbModTime = "0.2500";
reverbModDepth = "0.0000";
airAbsorbtionGainHF = "0.9943";
reverbHFRef = "5000.0000";
reverbLFRef = "250.0000";
roomRolloffFactor = "0.0000";
decayHFLimit = "1";
};

singleton SFXEnvironment(PresetAuditorium) 
{
reverbDensity = "1.000";
reverbDiffusion = "1.000";
reverbGain = "0.3162";
reverbGainHF = "0.5781";
reverbGainLF = "1.0000";
reverbDecayTime = "4.3200";
reverbDecayHFRatio = "0.5900";
reverbDecayLFRatio = "1.0000";
reflectionsGain = "0.4032";
reflectionDelay = "0.0200";
reflectionsPan[ 0 ] = "0.0";
reflectionsPan[ 1 ] = "0.0";
reflectionsPan[ 2 ] = "0.0";
lateReverbGain = "0.7170";
lateReverbDelay = "0.0300";
lateReverbPan[ 0 ] = "0.0";
lateReverbPan[ 1 ] = "0.0";
lateReverbPan[ 2 ] = "0.0";
reverbEchoTime = "0.2500";
reverbEchoDepth = "0.0000";
reverbModTime = "0.2500";
reverbModDepth = "0.0000";
airAbsorbtionGainHF = "0.9943";
reverbHFRef = "5000.0000";
reverbLFRef = "250.0000";
roomRolloffFactor = "0.0000";
decayHFLimit = "1";
};

singleton SFXEnvironment(PresetConcerthall) 
{
reverbDensity = "1.000";
reverbDiffusion = "1.000";
reverbGain = "0.3162";
reverbGainHF = "0.5632";
reverbGainLF = "1.0000";
reverbDecayTime = "3.9200";
reverbDecayHFRatio = "0.7000";
reverbDecayLFRatio = "1.0000";
reflectionsGain = "0.2427";
reflectionDelay = "0.0200";
reflectionsPan[ 0 ] = "0.0";
reflectionsPan[ 1 ] = "0.0";
reflectionsPan[ 2 ] = "0.0";
lateReverbGain = "0.9977";
lateReverbDelay = "0.0290";
lateReverbPan[ 0 ] = "0.0";
lateReverbPan[ 1 ] = "0.0";
lateReverbPan[ 2 ] = "0.0";
reverbEchoTime = "0.2500";
reverbEchoDepth = "0.0000";
reverbModTime = "0.2500";
reverbModDepth = "0.0000";
airAbsorbtionGainHF = "0.9943";
reverbHFRef = "5000.0000";
reverbLFRef = "250.0000";
roomRolloffFactor = "0.0000";
decayHFLimit = "1";
};

singleton SFXEnvironment(PresetCave) 
{
reverbDensity = "1.000";
reverbDiffusion = "1.000";
reverbGain = "0.3162";
reverbGainHF = "1.000";
reverbGainLF = "1.0000";
reverbDecayTime = "2.9100";
reverbDecayHFRatio = "1.3000";
reverbDecayLFRatio = "1.0000";
reflectionsGain = "0.5000";
reflectionDelay = "0.0250";
reflectionsPan[ 0 ] = "0.0";
reflectionsPan[ 1 ] = "0.0";
reflectionsPan[ 2 ] = "0.0";
lateReverbGain = "0.7063";
lateReverbDelay = "0.0220";
lateReverbPan[ 0 ] = "0.0";
lateReverbPan[ 1 ] = "0.0";
lateReverbPan[ 2 ] = "0.0";
reverbEchoTime = "0.2500";
reverbEchoDepth = "0.0000";
reverbModTime = "0.2500";
reverbModDepth = "0.0000";
airAbsorbtionGainHF = "0.9943";
reverbHFRef = "5000.0000";
reverbLFRef = "250.0000";
roomRolloffFactor = "0.0000";
decayHFLimit = "0";
};

singleton SFXEnvironment(PresetArena) 
{
reverbDensity = "1.000";
reverbDiffusion = "1.000";
reverbGain = "0.3162";
reverbGainHF = "0.4477";
reverbGainLF = "1.0000";
reverbDecayTime = "7.2400";
reverbDecayHFRatio = "0.3300";
reverbDecayLFRatio = "1.0000";
reflectionsGain = "0.2612";
reflectionDelay = "0.0200";
reflectionsPan[ 0 ] = "0.0";
reflectionsPan[ 1 ] = "0.0";
reflectionsPan[ 2 ] = "0.0";
lateReverbGain = "1.0186";
lateReverbDelay = "0.0300";
lateReverbPan[ 0 ] = "0.0";
lateReverbPan[ 1 ] = "0.0";
lateReverbPan[ 2 ] = "0.0";
reverbEchoTime = "0.2500";
reverbEchoDepth = "0.0000";
reverbModTime = "0.2500";
reverbModDepth = "0.0000";
airAbsorbtionGainHF = "0.9943";
reverbHFRef = "5000.0000";
reverbLFRef = "250.0000";
roomRolloffFactor = "0.0000";
decayHFLimit = "1";
};

singleton SFXEnvironment(PresetHangar) 
{
reverbDensity = "1.000";
reverbDiffusion = "1.000";
reverbGain = "0.3162";
reverbGainHF = "0.3162";
reverbGainLF = "1.0000";
reverbDecayTime = "10.0500";
reverbDecayHFRatio = "0.2300";
reverbDecayLFRatio = "1.0000";
reflectionsGain = "0.5000";
reflectionDelay = "0.0200";
reflectionsPan[ 0 ] = "0.0";
reflectionsPan[ 1 ] = "0.0";
reflectionsPan[ 2 ] = "0.0";
lateReverbGain = "1.2560";
lateReverbDelay = "0.0300";
lateReverbPan[ 0 ] = "0.0";
lateReverbPan[ 1 ] = "0.0";
lateReverbPan[ 2 ] = "0.0";
reverbEchoTime = "0.2500";
reverbEchoDepth = "0.0000";
reverbModTime = "0.2500";
reverbModDepth = "0.0000";
airAbsorbtionGainHF = "0.9943";
reverbHFRef = "5000.0000";
reverbLFRef = "250.0000";
roomRolloffFactor = "0.0000";
decayHFLimit = "1";
};

singleton SFXEnvironment(PresetCarpetedHall) 
{
reverbDensity = "0.4287";
reverbDiffusion = "1.000";
reverbGain = "0.3162";
reverbGainHF = "0.0100";
reverbGainLF = "1.0000";
reverbDecayTime = "0.3000";
reverbDecayHFRatio = "0.1000";
reverbDecayLFRatio = "1.0000";
reflectionsGain = "0.1215";
reflectionDelay = "0.0020";
reflectionsPan[ 0 ] = "0.0";
reflectionsPan[ 1 ] = "0.0";
reflectionsPan[ 2 ] = "0.0";
lateReverbGain = "0.1531";
lateReverbDelay = "0.0300";
lateReverbPan[ 0 ] = "0.0";
lateReverbPan[ 1 ] = "0.0";
lateReverbPan[ 2 ] = "0.0";
reverbEchoTime = "0.2500";
reverbEchoDepth = "0.0000";
reverbModTime = "0.2500";
reverbModDepth = "0.0000";
airAbsorbtionGainHF = "0.9943";
reverbHFRef = "5000.0000";
reverbLFRef = "250.0000";
roomRolloffFactor = "0.0000";
decayHFLimit = "1";
};

singleton SFXEnvironment(PresetHallway) 
{
reverbDensity = "0.3645";
reverbDiffusion = "1.000";
reverbGain = "0.3162";
reverbGainHF = "0.7079";
reverbGainLF = "1.0000";
reverbDecayTime = "1.4900";
reverbDecayHFRatio = "0.5900";
reverbDecayLFRatio = "1.0000";
reflectionsGain = "0.2458";
reflectionDelay = "0.0070";
reflectionsPan[ 0 ] = "0.0";
reflectionsPan[ 1 ] = "0.0";
reflectionsPan[ 2 ] = "0.0";
lateReverbGain = "1.6615";
lateReverbDelay = "0.0110";
lateReverbPan[ 0 ] = "0.0";
lateReverbPan[ 1 ] = "0.0";
lateReverbPan[ 2 ] = "0.0";
reverbEchoTime = "0.2500";
reverbEchoDepth = "0.0000";
reverbModTime = "0.2500";
reverbModDepth = "0.0000";
airAbsorbtionGainHF = "0.9943";
reverbHFRef = "5000.0000";
reverbLFRef = "250.0000";
roomRolloffFactor = "0.0000";
decayHFLimit = "1";
};

singleton SFXEnvironment(PresetStoneCorridor) 
{
reverbDensity = "1.000";
reverbDiffusion = "1.000";
reverbGain = "0.3162";
reverbGainHF = "0.7612";
reverbGainLF = "1.0000";
reverbDecayTime = "2.7000";
reverbDecayHFRatio = "0.7900";
reverbDecayLFRatio = "1.0000";
reflectionsGain = "0.2472";
reflectionDelay = "0.0130";
reflectionsPan[ 0 ] = "0.0";
reflectionsPan[ 1 ] = "0.0";
reflectionsPan[ 2 ] = "0.0";
lateReverbGain = "1.5758";
lateReverbDelay = "0.0200";
lateReverbPan[ 0 ] = "0.0";
lateReverbPan[ 1 ] = "0.0";
lateReverbPan[ 2 ] = "0.0";
reverbEchoTime = "0.2500";
reverbEchoDepth = "0.0000";
reverbModTime = "0.2500";
reverbModDepth = "0.0000";
airAbsorbtionGainHF = "0.9943";
reverbHFRef = "5000.0000";
reverbLFRef = "250.0000";
roomRolloffFactor = "0.0000";
decayHFLimit = "1";
};

singleton SFXEnvironment(PresetStoneAlley)
{
reverbDensity = "1.000";
reverbDiffusion = "0.300";
reverbGain = "0.3162";
reverbGainHF = "0.7328";
reverbGainLF = "1.0000";
reverbDecayTime = "1.4900";
reverbDecayHFRatio = "0.8600";
reverbDecayLFRatio = "1.0000";
reflectionsGain = "0.2500";
reflectionDelay = "0.0070";
reflectionsPan[ 0 ] = "0.0";
reflectionsPan[ 1 ] = "0.0";
reflectionsPan[ 2 ] = "0.0";
lateReverbGain = "0.9954";
lateReverbDelay = "0.0110";
lateReverbPan[ 0 ] = "0.0";
lateReverbPan[ 1 ] = "0.0";
lateReverbPan[ 2 ] = "0.0";
reverbEchoTime = "0.1250";
reverbEchoDepth = "0.9500";
reverbModTime = "0.2500";
reverbModDepth = "0.0000";
airAbsorbtionGainHF = "0.9943";
reverbHFRef = "5000.0000";
reverbLFRef = "250.0000";
roomRolloffFactor = "0.0000";
decayHFLimit = "1";
};

singleton SFXEnvironment(PresetFroest)
{
reverbDensity = "1.000";
reverbDiffusion = "0.300";
reverbGain = "0.3162";
reverbGainHF = "0.0224";
reverbGainLF = "1.0000";
reverbDecayTime = "1.4900";
reverbDecayHFRatio = "0.5400";
reverbDecayLFRatio = "1.0000";
reflectionsGain = "0.0525";
reflectionDelay = "0.1620";
reflectionsPan[ 0 ] = "0.0";
reflectionsPan[ 1 ] = "0.0";
reflectionsPan[ 2 ] = "0.0";
lateReverbGain = "0.7682";
lateReverbDelay = "0.0880";
lateReverbPan[ 0 ] = "0.0";
lateReverbPan[ 1 ] = "0.0";
lateReverbPan[ 2 ] = "0.0";
reverbEchoTime = "0.1250";
reverbEchoDepth = "1.0000";
reverbModTime = "0.2500";
reverbModDepth = "0.0000";
airAbsorbtionGainHF = "0.9943";
reverbHFRef = "5000.0000";
reverbLFRef = "250.0000";
roomRolloffFactor = "0.0000";
decayHFLimit = "1";
};

singleton SFXEnvironment(PresetCity)
{
reverbDensity = "1.000";
reverbDiffusion = "0.500";
reverbGain = "0.3162";
reverbGainHF = "0.3981";
reverbGainLF = "1.0000";
reverbDecayTime = "1.4900";
reverbDecayHFRatio = "0.6700";
reverbDecayLFRatio = "1.0000";
reflectionsGain = "0.0730";
reflectionDelay = "0.0070";
reflectionsPan[ 0 ] = "0.0";
reflectionsPan[ 1 ] = "0.0";
reflectionsPan[ 2 ] = "0.0";
lateReverbGain = "0.1427";
lateReverbDelay = "0.0110";
lateReverbPan[ 0 ] = "0.0";
lateReverbPan[ 1 ] = "0.0";
lateReverbPan[ 2 ] = "0.0";
reverbEchoTime = "0.1250";
reverbEchoDepth = "0.0000";
reverbModTime = "0.2500";
reverbModDepth = "0.0000";
airAbsorbtionGainHF = "0.9943";
reverbHFRef = "5000.0000";
reverbLFRef = "250.0000";
roomRolloffFactor = "0.0000";
decayHFLimit = "1";
};

singleton SFXEnvironment(PresetMountains)
{
reverbDensity = "1.000";
reverbDiffusion = "0.2700";
reverbGain = "0.3162";
reverbGainHF = "0.0562";
reverbGainLF = "1.0000";
reverbDecayTime = "1.4900";
reverbDecayHFRatio = "0.2100";
reverbDecayLFRatio = "1.0000";
reflectionsGain = "0.0407";
reflectionDelay = "0.3000";
reflectionsPan[ 0 ] = "0.0";
reflectionsPan[ 1 ] = "0.0";
reflectionsPan[ 2 ] = "0.0";
lateReverbGain = "0.1919";
lateReverbDelay = "0.1000";
lateReverbPan[ 0 ] = "0.0";
lateReverbPan[ 1 ] = "0.0";
lateReverbPan[ 2 ] = "0.0";
reverbEchoTime = "0.1250";
reverbEchoDepth = "1.0000";
reverbModTime = "0.2500";
reverbModDepth = "0.0000";
airAbsorbtionGainHF = "0.9943";
reverbHFRef = "5000.0000";
reverbLFRef = "250.0000";
roomRolloffFactor = "0.0000";
decayHFLimit = "0";
};

singleton SFXEnvironment(PresetQuarry)
{
reverbDensity = "1.000";
reverbDiffusion = "1.0000";
reverbGain = "0.3162";
reverbGainHF = "0.3162";
reverbGainLF = "1.0000";
reverbDecayTime = "1.4900";
reverbDecayHFRatio = "0.8300";
reverbDecayLFRatio = "1.0000";
reflectionsGain = "0.0000";
reflectionDelay = "0.0610";
reflectionsPan[ 0 ] = "0.0";
reflectionsPan[ 1 ] = "0.0";
reflectionsPan[ 2 ] = "0.0";
lateReverbGain = "1.7783";
lateReverbDelay = "0.0250";
lateReverbPan[ 0 ] = "0.0";
lateReverbPan[ 1 ] = "0.0";
lateReverbPan[ 2 ] = "0.0";
reverbEchoTime = "0.1250";
reverbEchoDepth = "0.7000";
reverbModTime = "0.2500";
reverbModDepth = "0.0000";
airAbsorbtionGainHF = "0.9943";
reverbHFRef = "5000.0000";
reverbLFRef = "250.0000";
roomRolloffFactor = "0.0000";
decayHFLimit = "1";
};

singleton SFXEnvironment(PresetPlain)
{
reverbDensity = "1.000";
reverbDiffusion = "0.2100";
reverbGain = "0.3162";
reverbGainHF = "0.1000";
reverbGainLF = "1.0000";
reverbDecayTime = "1.4900";
reverbDecayHFRatio = "0.5000";
reverbDecayLFRatio = "1.0000";
reflectionsGain = "0.0585";
reflectionDelay = "0.1790";
reflectionsPan[ 0 ] = "0.0";
reflectionsPan[ 1 ] = "0.0";
reflectionsPan[ 2 ] = "0.0";
lateReverbGain = "0.1089";
lateReverbDelay = "0.1000";
lateReverbPan[ 0 ] = "0.0";
lateReverbPan[ 1 ] = "0.0";
lateReverbPan[ 2 ] = "0.0";
reverbEchoTime = "0.2500";
reverbEchoDepth = "1.0000";
reverbModTime = "0.2500";
reverbModDepth = "0.0000";
airAbsorbtionGainHF = "0.9943";
reverbHFRef = "5000.0000";
reverbLFRef = "250.0000";
roomRolloffFactor = "0.0000";
decayHFLimit = "1";
};

singleton SFXEnvironment(PresetParkinglot)
{
reverbDensity = "1.000";
reverbDiffusion = "1.0000";
reverbGain = "0.3162";
reverbGainHF = "1.0000";
reverbGainLF = "1.0000";
reverbDecayTime = "1.6500";
reverbDecayHFRatio = "1.5000";
reverbDecayLFRatio = "1.0000";
reflectionsGain = "0.2082";
reflectionDelay = "0.0080";
reflectionsPan[ 0 ] = "0.0";
reflectionsPan[ 1 ] = "0.0";
reflectionsPan[ 2 ] = "0.0";
lateReverbGain = "0.2652";
lateReverbDelay = "0.0120";
lateReverbPan[ 0 ] = "0.0";
lateReverbPan[ 1 ] = "0.0";
lateReverbPan[ 2 ] = "0.0";
reverbEchoTime = "0.2500";
reverbEchoDepth = "0.0000";
reverbModTime = "0.2500";
reverbModDepth = "0.0000";
airAbsorbtionGainHF = "0.9943";
reverbHFRef = "5000.0000";
reverbLFRef = "250.0000";
roomRolloffFactor = "0.0000";
decayHFLimit = "0";
};

singleton SFXEnvironment(PresetSewerpipe)
{
reverbDensity = "0.3071";
reverbDiffusion = "0.8000";
reverbGain = "0.3162";
reverbGainHF = "0.3162";
reverbGainLF = "1.0000";
reverbDecayTime = "2.8100";
reverbDecayHFRatio = "0.1400";
reverbDecayLFRatio = "1.0000";
reflectionsGain = "1.6387";
reflectionDelay = "0.0140";
reflectionsPan[ 0 ] = "0.0";
reflectionsPan[ 1 ] = "0.0";
reflectionsPan[ 2 ] = "0.0";
lateReverbGain = "3.2471";
lateReverbDelay = "0.0210";
lateReverbPan[ 0 ] = "0.0";
lateReverbPan[ 1 ] = "0.0";
lateReverbPan[ 2 ] = "0.0";
reverbEchoTime = "0.2500";
reverbEchoDepth = "0.0000";
reverbModTime = "0.2500";
reverbModDepth = "0.0000";
airAbsorbtionGainHF = "0.9943";
reverbHFRef = "5000.0000";
reverbLFRef = "250.0000";
roomRolloffFactor = "0.0000";
decayHFLimit = "1";
};

singleton SFXEnvironment(PresetUnderwater)
{
reverbDensity = "0.3645";
reverbDiffusion = "1.0000";
reverbGain = "0.3162";
reverbGainHF = "0.0100";
reverbGainLF = "1.0000";
reverbDecayTime = "1.4900";
reverbDecayHFRatio = "0.1000";
reverbDecayLFRatio = "1.0000";
reflectionsGain = "0.5963";
reflectionDelay = "0.0070";
reflectionsPan[ 0 ] = "0.0";
reflectionsPan[ 1 ] = "0.0";
reflectionsPan[ 2 ] = "0.0";
lateReverbGain = "7.0795";
lateReverbDelay = "0.0110";
lateReverbPan[ 0 ] = "0.0";
lateReverbPan[ 1 ] = "0.0";
lateReverbPan[ 2 ] = "0.0";
reverbEchoTime = "0.2500";
reverbEchoDepth = "0.0000";
reverbModTime = "1.1800";
reverbModDepth = "0.3480";
airAbsorbtionGainHF = "0.9943";
reverbHFRef = "5000.0000";
reverbLFRef = "250.0000";
roomRolloffFactor = "0.0000";
decayHFLimit = "1";
};

singleton SFXEnvironment(PresetDrugged)
{
reverbDensity = "0.4287";
reverbDiffusion = "0.5000";
reverbGain = "0.3162";
reverbGainHF = "1.0000";
reverbGainLF = "1.0000";
reverbDecayTime = "8.3900";
reverbDecayHFRatio = "1.3900";
reverbDecayLFRatio = "1.0000";
reflectionsGain = "0.8760";
reflectionDelay = "0.0020";
reflectionsPan[ 0 ] = "0.0";
reflectionsPan[ 1 ] = "0.0";
reflectionsPan[ 2 ] = "0.0";
lateReverbGain = "3.1081";
lateReverbDelay = "0.0300";
lateReverbPan[ 0 ] = "0.0";
lateReverbPan[ 1 ] = "0.0";
lateReverbPan[ 2 ] = "0.0";
reverbEchoTime = "0.2500";
reverbEchoDepth = "0.0000";
reverbModTime = "0.2500";
reverbModDepth = "1.0000";
airAbsorbtionGainHF = "0.9943";
reverbHFRef = "5000.0000";
reverbLFRef = "250.0000";
roomRolloffFactor = "0.0000";
decayHFLimit = "0";
};

singleton SFXEnvironment(PresetDizzy)
{
reverbDensity = "0.3645";
reverbDiffusion = "0.6000";
reverbGain = "0.3162";
reverbGainHF = "0.6310";
reverbGainLF = "1.0000";
reverbDecayTime = "17.2300";
reverbDecayHFRatio = "0.5600";
reverbDecayLFRatio = "1.0000";
reflectionsGain = "0.1392";
reflectionDelay = "0.0200";
reflectionsPan[ 0 ] = "0.0";
reflectionsPan[ 1 ] = "0.0";
reflectionsPan[ 2 ] = "0.0";
lateReverbGain = "0.4937";
lateReverbDelay = "0.0300";
lateReverbPan[ 0 ] = "0.0";
lateReverbPan[ 1 ] = "0.0";
lateReverbPan[ 2 ] = "0.0";
reverbEchoTime = "0.2500";
reverbEchoDepth = "1.0000";
reverbModTime = "0.8100";
reverbModDepth = "0.3100";
airAbsorbtionGainHF = "0.9943";
reverbHFRef = "5000.0000";
reverbLFRef = "250.0000";
roomRolloffFactor = "0.0000";
decayHFLimit = "0";
};

singleton SFXEnvironment(PresetPsychotic)
{
reverbDensity = "0.0625";
reverbDiffusion = "0.5000";
reverbGain = "0.3162";
reverbGainHF = "0.8404";
reverbGainLF = "1.0000";
reverbDecayTime = "7.5600";
reverbDecayHFRatio = "0.9100";
reverbDecayLFRatio = "1.0000";
reflectionsGain = "0.4864";
reflectionDelay = "0.0200";
reflectionsPan[ 0 ] = "0.0";
reflectionsPan[ 1 ] = "0.0";
reflectionsPan[ 2 ] = "0.0";
lateReverbGain = "2.4378";
lateReverbDelay = "0.0300";
lateReverbPan[ 0 ] = "0.0";
lateReverbPan[ 1 ] = "0.0";
lateReverbPan[ 2 ] = "0.0";
reverbEchoTime = "0.2500";
reverbEchoDepth = "0.0000";
reverbModTime = "4.0000";
reverbModDepth = "1.0000";
airAbsorbtionGainHF = "0.9943";
reverbHFRef = "5000.0000";
reverbLFRef = "250.0000";
roomRolloffFactor = "0.0000";
decayHFLimit = "0";
};

singleton SFXEnvironment(CastleSmallroom)
{
reverbDensity = "1.0000";
reverbDiffusion = "0.8900";
reverbGain = "0.3162";
reverbGainHF = "0.3981";
reverbGainLF = "0.1000";
reverbDecayTime = "1.2200";
reverbDecayHFRatio = "0.8300";
reverbDecayLFRatio = "0.3100";
reflectionsGain = "0.8913";
reflectionDelay = "0.0220";
reflectionsPan[ 0 ] = "0.0";
reflectionsPan[ 1 ] = "0.0";
reflectionsPan[ 2 ] = "0.0";
lateReverbGain = "1.9953";
lateReverbDelay = "0.0110";
lateReverbPan[ 0 ] = "0.0";
lateReverbPan[ 1 ] = "0.0";
lateReverbPan[ 2 ] = "0.0";
reverbEchoTime = "0.1380";
reverbEchoDepth = "0.0800";
reverbModTime = "0.2500";
reverbModDepth = "0.0000";
airAbsorbtionGainHF = "0.9943";
reverbHFRef = "5000.0000";
reverbLFRef = "250.0000";
roomRolloffFactor = "0.0000";
decayHFLimit = "1";
};

singleton SFXEnvironment(CastleShortPassage)
{
reverbDensity = "1.0000";
reverbDiffusion = "0.8900";
reverbGain = "0.3162";
reverbGainHF = "0.3162";
reverbGainLF = "0.1000";
reverbDecayTime = "2.3200";
reverbDecayHFRatio = "0.8300";
reverbDecayLFRatio = "0.3100";
reflectionsGain = "0.8913";
reflectionDelay = "0.0070";
reflectionsPan[ 0 ] = "0.0";
reflectionsPan[ 1 ] = "0.0";
reflectionsPan[ 2 ] = "0.0";
lateReverbGain = "1.2589";
lateReverbDelay = "0.0230";
lateReverbPan[ 0 ] = "0.0";
lateReverbPan[ 1 ] = "0.0";
lateReverbPan[ 2 ] = "0.0";
reverbEchoTime = "0.1380";
reverbEchoDepth = "0.0800";
reverbModTime = "0.2500";
reverbModDepth = "0.0000";
airAbsorbtionGainHF = "0.9943";
reverbHFRef = "5168.0001";
reverbLFRef = "139.5000";
roomRolloffFactor = "0.0000";
decayHFLimit = "1";
};

singleton SFXEnvironment(CastleMediumRoom)
{
reverbDensity = "1.0000";
reverbDiffusion = "0.9300";
reverbGain = "0.3162";
reverbGainHF = "0.2818";
reverbGainLF = "0.1000";
reverbDecayTime = "2.0400";
reverbDecayHFRatio = "0.8300";
reverbDecayLFRatio = "0.4600";
reflectionsGain = "0.6310";
reflectionDelay = "0.0220";
reflectionsPan[ 0 ] = "0.0";
reflectionsPan[ 1 ] = "0.0";
reflectionsPan[ 2 ] = "0.0";
lateReverbGain = "1.5849";
lateReverbDelay = "0.0110";
lateReverbPan[ 0 ] = "0.0";
lateReverbPan[ 1 ] = "0.0";
lateReverbPan[ 2 ] = "0.0";
reverbEchoTime = "0.1550";
reverbEchoDepth = "0.0300";
reverbModTime = "0.2500";
reverbModDepth = "0.0000";
airAbsorbtionGainHF = "0.9943";
reverbHFRef = "5168.0001";
reverbLFRef = "139.5000";
roomRolloffFactor = "0.0000";
decayHFLimit = "1";
};

singleton SFXEnvironment(CastleLargeRoom)
{
reverbDensity = "1.0000";
reverbDiffusion = "0.8200";
reverbGain = "0.3162";
reverbGainHF = "0.2818";
reverbGainLF = "0.1259";
reverbDecayTime = "2.5300";
reverbDecayHFRatio = "0.8300";
reverbDecayLFRatio = "0.5000";
reflectionsGain = "0.4467";
reflectionDelay = "0.0340";
reflectionsPan[ 0 ] = "0.0";
reflectionsPan[ 1 ] = "0.0";
reflectionsPan[ 2 ] = "0.0";
lateReverbGain = "1.2589";
lateReverbDelay = "0.0160";
lateReverbPan[ 0 ] = "0.0";
lateReverbPan[ 1 ] = "0.0";
lateReverbPan[ 2 ] = "0.0";
reverbEchoTime = "0.1850";
reverbEchoDepth = "0.0700";
reverbModTime = "0.2500";
reverbModDepth = "0.0000";
airAbsorbtionGainHF = "0.9943";
reverbHFRef = "5168.0001";
reverbLFRef = "139.5000";
roomRolloffFactor = "0.0000";
decayHFLimit = "1";
};

singleton SFXEnvironment(CastleLongPassage)
{
reverbDensity = "1.0000";
reverbDiffusion = "0.8900";
reverbGain = "0.3162";
reverbGainHF = "0.3981";
reverbGainLF = "0.1000";
reverbDecayTime = "3.4200";
reverbDecayHFRatio = "0.8300";
reverbDecayLFRatio = "0.3100";
reflectionsGain = "0.8913";
reflectionDelay = "0.0070";
reflectionsPan[ 0 ] = "0.0";
reflectionsPan[ 1 ] = "0.0";
reflectionsPan[ 2 ] = "0.0";
lateReverbGain = "1.4125";
lateReverbDelay = "0.0230";
lateReverbPan[ 0 ] = "0.0";
lateReverbPan[ 1 ] = "0.0";
lateReverbPan[ 2 ] = "0.0";
reverbEchoTime = "0.1380";
reverbEchoDepth = "0.0800";
reverbModTime = "0.2500";
reverbModDepth = "0.0000";
airAbsorbtionGainHF = "0.9943";
reverbHFRef = "5168.0001";
reverbLFRef = "139.5000";
roomRolloffFactor = "0.0000";
decayHFLimit = "1";
};

singleton SFXEnvironment(CastleHall)
{
reverbDensity = "1.0000";
reverbDiffusion = "0.8100";
reverbGain = "0.3162";
reverbGainHF = "0.2818";
reverbGainLF = "0.1778";
reverbDecayTime = "3.1400";
reverbDecayHFRatio = "0.7900";
reverbDecayLFRatio = "0.6200";
reflectionsGain = "0.1778";
reflectionDelay = "0.0560";
reflectionsPan[ 0 ] = "0.0";
reflectionsPan[ 1 ] = "0.0";
reflectionsPan[ 2 ] = "0.0";
lateReverbGain = "1.1220";
lateReverbDelay = "0.0240";
lateReverbPan[ 0 ] = "0.0";
lateReverbPan[ 1 ] = "0.0";
lateReverbPan[ 2 ] = "0.0";
reverbEchoTime = "0.2500";
reverbEchoDepth = "0.0000";
reverbModTime = "0.2500";
reverbModDepth = "0.0000";
airAbsorbtionGainHF = "0.9943";
reverbHFRef = "5168.0001";
reverbLFRef = "139.5000";
roomRolloffFactor = "0.0000";
decayHFLimit = "1";
};

singleton SFXEnvironment(CastleCupboard)
{
reverbDensity = "1.0000";
reverbDiffusion = "0.8900";
reverbGain = "0.3162";
reverbGainHF = "0.2818";
reverbGainLF = "0.1000";
reverbDecayTime = "0.6700";
reverbDecayHFRatio = "0.8700";
reverbDecayLFRatio = "0.3100";
reflectionsGain = "1.4125";
reflectionDelay = "0.0100";
reflectionsPan[ 0 ] = "0.0";
reflectionsPan[ 1 ] = "0.0";
reflectionsPan[ 2 ] = "0.0";
lateReverbGain = "3.5481";
lateReverbDelay = "0.0070";
lateReverbPan[ 0 ] = "0.0";
lateReverbPan[ 1 ] = "0.0";
lateReverbPan[ 2 ] = "0.0";
reverbEchoTime = "0.1380";
reverbEchoDepth = "0.0800";
reverbModTime = "0.2500";
reverbModDepth = "0.0000";
airAbsorbtionGainHF = "0.9943";
reverbHFRef = "5168.0001";
reverbLFRef = "139.5000";
roomRolloffFactor = "0.0000";
decayHFLimit = "1";
};

singleton SFXEnvironment(CastleCourtyard)
{
reverbDensity = "1.0000";
reverbDiffusion = "0.4200";
reverbGain = "0.3162";
reverbGainHF = "0.4467";
reverbGainLF = "0.1995";
reverbDecayTime = "2.1300";
reverbDecayHFRatio = "0.6100";
reverbDecayLFRatio = "0.2300";
reflectionsGain = "0.2239";
reflectionDelay = "0.1600";
reflectionsPan[ 0 ] = "0.0";
reflectionsPan[ 1 ] = "0.0";
reflectionsPan[ 2 ] = "0.0";
lateReverbGain = "0.7079";
lateReverbDelay = "0.0360";
lateReverbPan[ 0 ] = "0.0";
lateReverbPan[ 1 ] = "0.0";
lateReverbPan[ 2 ] = "0.0";
reverbEchoTime = "0.2500";
reverbEchoDepth = "0.3700";
reverbModTime = "0.2500";
reverbModDepth = "0.0000";
airAbsorbtionGainHF = "0.9943";
reverbHFRef = "5000.0000";
reverbLFRef = "250.0000";
roomRolloffFactor = "0.0000";
decayHFLimit = "0";
};

singleton SFXEnvironment(CastleAlcove)
{
reverbDensity = "1.0000";
reverbDiffusion = "0.8900";
reverbGain = "0.3162";
reverbGainHF = "0.5012";
reverbGainLF = "0.1000";
reverbDecayTime = "1.6400";
reverbDecayHFRatio = "0.8700";
reverbDecayLFRatio = "0.3100";
reflectionsGain = "1.0000";
reflectionDelay = "0.0070";
reflectionsPan[ 0 ] = "0.0";
reflectionsPan[ 1 ] = "0.0";
reflectionsPan[ 2 ] = "0.0";
lateReverbGain = "1.4125";
lateReverbDelay = "0.0340";
lateReverbPan[ 0 ] = "0.0";
lateReverbPan[ 1 ] = "0.0";
lateReverbPan[ 2 ] = "0.0";
reverbEchoTime = "0.1380";
reverbEchoDepth = "0.0800";
reverbModTime = "0.2500";
reverbModDepth = "0.0000";
airAbsorbtionGainHF = "0.9943";
reverbHFRef = "5168.0001";
reverbLFRef = "139.5000";
roomRolloffFactor = "0.0000";
decayHFLimit = "1";
};

singleton SFXEnvironment(FactorySmallRoom)
{
reverbDensity = "0.3645";
reverbDiffusion = "0.8200";
reverbGain = "0.3162";
reverbGainHF = "0.7943";
reverbGainLF = "0.5012";
reverbDecayTime = "1.7200";
reverbDecayHFRatio = "0.6500";
reverbDecayLFRatio = "1.3100";
reflectionsGain = "0.7079";
reflectionDelay = "0.0100";
reflectionsPan[ 0 ] = "0.0";
reflectionsPan[ 1 ] = "0.0";
reflectionsPan[ 2 ] = "0.0";
lateReverbGain = "1.7783";
lateReverbDelay = "0.0240";
lateReverbPan[ 0 ] = "0.0";
lateReverbPan[ 1 ] = "0.0";
lateReverbPan[ 2 ] = "0.0";
reverbEchoTime = "0.1190";
reverbEchoDepth = "0.0700";
reverbModTime = "0.2500";
reverbModDepth = "0.0000";
airAbsorbtionGainHF = "0.9943";
reverbHFRef = "3762.6001";
reverbLFRef = "362.5000";
roomRolloffFactor = "0.0000";
decayHFLimit = "1";
};

singleton SFXEnvironment(FactoryShortPassage)
{
reverbDensity = "0.3645";
reverbDiffusion = "0.6400";
reverbGain = "0.2512";
reverbGainHF = "0.7943";
reverbGainLF = "0.5012";
reverbDecayTime = "2.5300";
reverbDecayHFRatio = "0.6500";
reverbDecayLFRatio = "1.3100";
reflectionsGain = "1.0000";
reflectionDelay = "0.0100";
reflectionsPan[ 0 ] = "0.0";
reflectionsPan[ 1 ] = "0.0";
reflectionsPan[ 2 ] = "0.0";
lateReverbGain = "1.2589";
lateReverbDelay = "0.0380";
lateReverbPan[ 0 ] = "0.0";
lateReverbPan[ 1 ] = "0.0";
lateReverbPan[ 2 ] = "0.0";
reverbEchoTime = "0.1350";
reverbEchoDepth = "0.2300";
reverbModTime = "0.2500";
reverbModDepth = "0.0000";
airAbsorbtionGainHF = "0.9943";
reverbHFRef = "3762.6001";
reverbLFRef = "362.5000";
roomRolloffFactor = "0.0000";
decayHFLimit = "1";
};

singleton SFXEnvironment(FactoryMediumRoom)
{
reverbDensity = "0.4287";
reverbDiffusion = "0.8200";
reverbGain = "0.2512";
reverbGainHF = "0.7943";
reverbGainLF = "0.5012";
reverbDecayTime = "2.7600";
reverbDecayHFRatio = "0.6500";
reverbDecayLFRatio = "1.3100";
reflectionsGain = "0.2818";
reflectionDelay = "0.0220";
reflectionsPan[ 0 ] = "0.0";
reflectionsPan[ 1 ] = "0.0";
reflectionsPan[ 2 ] = "0.0";
lateReverbGain = "1.4125";
lateReverbDelay = "0.0230";
lateReverbPan[ 0 ] = "0.0";
lateReverbPan[ 1 ] = "0.0";
lateReverbPan[ 2 ] = "0.0";
reverbEchoTime = "0.1740";
reverbEchoDepth = "0.0700";
reverbModTime = "0.2500";
reverbModDepth = "0.0000";
airAbsorbtionGainHF = "0.9943";
reverbHFRef = "3762.6001";
reverbLFRef = "362.5000";
roomRolloffFactor = "0.0000";
decayHFLimit = "1";
};

singleton SFXEnvironment(FactoryLargeRoom)
{
reverbDensity = "0.4287";
reverbDiffusion = "0.7500";
reverbGain = "0.2512";
reverbGainHF = "0.7079";
reverbGainLF = "0.6310";
reverbDecayTime = "4.2400";
reverbDecayHFRatio = "0.5100";
reverbDecayLFRatio = "1.3100";
reflectionsGain = "0.1778";
reflectionDelay = "0.0390";
reflectionsPan[ 0 ] = "0.0";
reflectionsPan[ 1 ] = "0.0";
reflectionsPan[ 2 ] = "0.0";
lateReverbGain = "1.1220";
lateReverbDelay = "0.0230";
lateReverbPan[ 0 ] = "0.0";
lateReverbPan[ 1 ] = "0.0";
lateReverbPan[ 2 ] = "0.0";
reverbEchoTime = "0.2310";
reverbEchoDepth = "0.0700";
reverbModTime = "0.2500";
reverbModDepth = "0.0000";
airAbsorbtionGainHF = "0.9943";
reverbHFRef = "3762.6001";
reverbLFRef = "362.5000";
roomRolloffFactor = "0.0000";
decayHFLimit = "1";
};

singleton SFXEnvironment(FactoryLongPassage)
{
reverbDensity = "0.3645";
reverbDiffusion = "0.6400";
reverbGain = "0.2512";
reverbGainHF = "0.7943";
reverbGainLF = "0.5012";
reverbDecayTime = "4.0000";
reverbDecayHFRatio = "0.6500";
reverbDecayLFRatio = "1.3100";
reflectionsGain = "1.0000";
reflectionDelay = "0.0200";
reflectionsPan[ 0 ] = "0.0";
reflectionsPan[ 1 ] = "0.0";
reflectionsPan[ 2 ] = "0.0";
lateReverbGain = "1.2589";
lateReverbDelay = "0.0370";
lateReverbPan[ 0 ] = "0.0";
lateReverbPan[ 1 ] = "0.0";
lateReverbPan[ 2 ] = "0.0";
reverbEchoTime = "0.1350";
reverbEchoDepth = "0.2300";
reverbModTime = "0.2500";
reverbModDepth = "0.0000";
airAbsorbtionGainHF = "0.9943";
reverbHFRef = "3762.6001";
reverbLFRef = "362.5000";
roomRolloffFactor = "0.0000";
decayHFLimit = "1";
};

singleton SFXEnvironment(FactoryHall)
{
reverbDensity = "0.4287";
reverbDiffusion = "0.7500";
reverbGain = "0.3162";
reverbGainHF = "0.7079";
reverbGainLF = "0.6310";
reverbDecayTime = "7.4300";
reverbDecayHFRatio = "0.5100";
reverbDecayLFRatio = "1.3100";
reflectionsGain = "0.0631";
reflectionDelay = "0.0730";
reflectionsPan[ 0 ] = "0.0";
reflectionsPan[ 1 ] = "0.0";
reflectionsPan[ 2 ] = "0.0";
lateReverbGain = "0.8913";
lateReverbDelay = "0.0270";
lateReverbPan[ 0 ] = "0.0";
lateReverbPan[ 1 ] = "0.0";
lateReverbPan[ 2 ] = "0.0";
reverbEchoTime = "0.2500";
reverbEchoDepth = "0.0700";
reverbModTime = "0.2500";
reverbModDepth = "0.0000";
airAbsorbtionGainHF = "0.9943";
reverbHFRef = "3762.6001";
reverbLFRef = "362.5000";
roomRolloffFactor = "0.0000";
decayHFLimit = "1";
};

singleton SFXEnvironment(FactoryCupboard)
{
reverbDensity = "0.3071";
reverbDiffusion = "0.6300";
reverbGain = "0.2512";
reverbGainHF = "0.7943";
reverbGainLF = "0.5012";
reverbDecayTime = "0.4900";
reverbDecayHFRatio = "0.6500";
reverbDecayLFRatio = "1.3100";
reflectionsGain = "1.2589";
reflectionDelay = "0.0100";
reflectionsPan[ 0 ] = "0.0";
reflectionsPan[ 1 ] = "0.0";
reflectionsPan[ 2 ] = "0.0";
lateReverbGain = "1.9953";
lateReverbDelay = "0.0320";
lateReverbPan[ 0 ] = "0.0";
lateReverbPan[ 1 ] = "0.0";
lateReverbPan[ 2 ] = "0.0";
reverbEchoTime = "0.1070";
reverbEchoDepth = "0.0700";
reverbModTime = "0.2500";
reverbModDepth = "0.0000";
airAbsorbtionGainHF = "0.9943";
reverbHFRef = "3762.6001";
reverbLFRef = "362.5000";
roomRolloffFactor = "0.0000";
decayHFLimit = "1";
};

singleton SFXEnvironment(FactoryCourtyard)
{
reverbDensity = "0.3071";
reverbDiffusion = "0.5700";
reverbGain = "0.3162";
reverbGainHF = "0.3162";
reverbGainLF = "0.6310";
reverbDecayTime = "2.3200";
reverbDecayHFRatio = "0.2900";
reverbDecayLFRatio = "0.5600";
reflectionsGain = "0.2239";
reflectionDelay = "0.1400";
reflectionsPan[ 0 ] = "0.0";
reflectionsPan[ 1 ] = "0.0";
reflectionsPan[ 2 ] = "0.0";
lateReverbGain = "0.3981";
lateReverbDelay = "0.0390";
lateReverbPan[ 0 ] = "0.0";
lateReverbPan[ 1 ] = "0.0";
lateReverbPan[ 2 ] = "0.0";
reverbEchoTime = "0.2500";
reverbEchoDepth = "0.2900";
reverbModTime = "0.2500";
reverbModDepth = "0.0000";
airAbsorbtionGainHF = "0.9943";
reverbHFRef = "3762.6001";
reverbLFRef = "362.5000";
roomRolloffFactor = "0.0000";
decayHFLimit = "1";
};

singleton SFXEnvironment(FactoryAlcove)
{
reverbDensity = "0.3645";
reverbDiffusion = "0.5900";
reverbGain = "0.2512";
reverbGainHF = "0.7943";
reverbGainLF = "0.5012";
reverbDecayTime = "3.1400";
reverbDecayHFRatio = "0.6500";
reverbDecayLFRatio = "1.3100";
reflectionsGain = "1.4125";
reflectionDelay = "0.0100";
reflectionsPan[ 0 ] = "0.0";
reflectionsPan[ 1 ] = "0.0";
reflectionsPan[ 2 ] = "0.0";
lateReverbGain = "1.0000";
lateReverbDelay = "0.0380";
lateReverbPan[ 0 ] = "0.0";
lateReverbPan[ 1 ] = "0.0";
lateReverbPan[ 2 ] = "0.0";
reverbEchoTime = "0.1140";
reverbEchoDepth = "0.1000";
reverbModTime = "0.2500";
reverbModDepth = "0.0000";
airAbsorbtionGainHF = "0.9943";
reverbHFRef = "3762.6001";
reverbLFRef = "362.5000";
roomRolloffFactor = "0.0000";
decayHFLimit = "1";
};