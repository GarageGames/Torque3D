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


//------------------------------------------------------------------------------
// LightAnimData
//------------------------------------------------------------------------------

datablock LightAnimData( NullLightAnim )
{   
   animEnabled = false;
};

datablock LightAnimData( PulseLightAnim )
{   
   brightnessA = 0;
   brightnessZ = 1;
   brightnessPeriod = 1;
   brightnessKeys = "aza";
   brightnessSmooth = true;
};

datablock LightAnimData( SubtlePulseLightAnim )
{
   brightnessA = 0.5;
   brightnessZ = 1;
   brightnessPeriod = 1;
   brightnessKeys = "aza";
   brightnessSmooth = true;
};

datablock LightAnimData( FlickerLightAnim )
{
   brightnessA = 1;
   brightnessZ = 0;
   brightnessPeriod = 5;
   brightnessKeys = "aaazaaaaaazaaazaaazaaaaazaaaazzaaaazaaaaaazaaaazaaaza";
   brightnessSmooth = false;
};

datablock LightAnimData( BlinkLightAnim )
{
   brightnessA = 0;
   brightnessZ = 1;
   brightnessPeriod = 5;
   brightnessKeys = "azaaaazazaaaaaazaaaazaaaazzaaaaaazaazaaazaaaaaaa";
   brightnessSmooth = false;
};

datablock LightAnimData( FireLightAnim )
{
   brightnessA = 0.75;
   brightnessZ = 1;
   brightnessPeriod = 0.7;
   brightnessKeys = "annzzznnnzzzaznzzzz";
   brightnessSmooth = 0;
   offsetA[0] = "-0.05";
   offsetA[1] = "-0.05";
   offsetA[2] = "-0.05";
   offsetZ[0] = "0.05";
   offsetZ[1] = "0.05";
   offsetZ[2] = "0.05";
   offsetPeriod[0] = "1.25";
   offsetPeriod[1] = "1.25";
   offsetPeriod[2] = "1.25";
   offsetKeys[0] = "ahahaazahakayajza";
   offsetKeys[1] = "ahahaazahakayajza";
   offsetKeys[2] = "ahahaazahakayajza";
   rotKeys[0] = "";
   rotKeys[1] = "";
   rotKeys[2] = "";
   colorKeys[0] = "";
   colorKeys[1] = "";
   colorKeys[2] = "";
};

datablock LightAnimData( SpinLightAnim )
{
   rotA[2] = "0";
   rotZ[2] = "360";
   rotPeriod[2] = "1";
   rotKeys[2] = "az";
   rotSmooth[2] = true;
};


//------------------------------------------------------------------------------
// LightFlareData
//------------------------------------------------------------------------------

datablock LightFlareData( NullLightFlare )
{
   flareEnabled = false;
};

datablock LightFlareData( SunFlareExample )
{      
   overallScale = 4.0;
   flareEnabled = true;
   renderReflectPass = false;
   flareTexture = "art/lights/lensFlareSheet0";  

   elementRect[0] = "512 0 512 512";
   elementDist[0] = 0.0;
   elementScale[0] = 2.0;
   elementTint[0] = "0.6 0.6 0.6";
   elementRotate[0] = true;
   elementUseLightColor[0] = true;
   
   elementRect[1] = "1152 0 128 128";
   elementDist[1] = 0.3;
   elementScale[1] = 0.7;
   elementTint[1] = "1.0 1.0 1.0";
   elementRotate[1] = true;
   elementUseLightColor[1] = true;
   
   elementRect[2] = "1024 0 128 128";
   elementDist[2] = 0.5;
   elementScale[2] = 0.25;   
   elementTint[2] = "1.0 1.0 1.0";
   elementRotate[2] = true;
   elementUseLightColor[2] = true;
   
   elementRect[3] = "1024 128 128 128";
   elementDist[3] = 0.8;
   elementScale[3] = 0.7;   
   elementTint[3] = "1.0 1.0 1.0";
   elementRotate[3] = true;
   elementUseLightColor[3] = true;
   
   elementRect[4] = "1024 0 128 128";
   elementDist[4] = 1.18;
   elementScale[4] = 0.5;   
   elementTint[4] = "1.0 1.0 1.0";
   elementRotate[4] = true;
   elementUseLightColor[4] = true;
   
   elementRect[5] = "1152 128 128 128";
   elementDist[5] = 1.25;
   elementScale[5] = 0.25;   
   elementTint[5] = "1.0 1.0 1.0";
   elementRotate[5] = true;
   elementUseLightColor[5] = true;
   
   elementRect[6] = "1024 0 128 128";
   elementDist[6] = 2.0;
   elementScale[6] = 0.25;      
   elementTint[6] = "1.0 1.0 1.0";
   elementRotate[6] = true;
   elementUseLightColor[6] = true;
   occlusionRadius = "0.25";
};

datablock LightFlareData( SunFlareExample2 )
{      
   overallScale = 2.0;
   flareEnabled = true;
   renderReflectPass = false;
   flareTexture = "art/lights/lensFlareSheet0";  
   
   elementRect[0] = "1024 0 128 128";
   elementDist[0] = 0.5;
   elementScale[0] = 0.25;   
   elementTint[0] = "1.0 1.0 1.0";
   elementRotate[0] = true;
   elementUseLightColor[0] = true;
   
   elementRect[1] = "1024 128 128 128";
   elementDist[1] = 0.8;
   elementScale[1] = 0.7;   
   elementTint[1] = "1.0 1.0 1.0";
   elementRotate[1] = true;
   elementUseLightColor[1] = true;
   
   elementRect[2] = "1024 0 128 128";
   elementDist[2] = 1.18;
   elementScale[2] = 0.5;   
   elementTint[2] = "1.0 1.0 1.0";
   elementRotate[2] = true;
   elementUseLightColor[2] = true;
   
   elementRect[3] = "1152 128 128 128";
   elementDist[3] = 1.25;
   elementScale[3] = 0.25;   
   elementTint[3] = "1.0 1.0 1.0";
   elementRotate[3] = true;
   elementUseLightColor[3] = true;
   
   elementRect[4] = "1024 0 128 128";
   elementDist[4] = 2.0;
   elementScale[4] = 0.25;      
   elementTint[4] = "0.7 0.7 0.7";
   elementRotate[4] = true;
   elementUseLightColor[4] = true;
   occlusionRadius = "0.25";
};

datablock LightFlareData(SunFlareExample3)
{
   overallScale = 2.0;
   flareEnabled = true;
   renderReflectPass = false;
   flareTexture = "art/lights/lensflareSheet3.png";  
   
   elementRect[0] = "0 256 256 256";
   elementDist[0] = "-0.6";
   elementScale[0] = "3.5";   
   elementTint[0] = "0.537255 0.537255 0.537255 1";
   elementRotate[0] = true;
   elementUseLightColor[0] = true;
   
   elementRect[1] = "128 128 128 128";
   elementDist[1] = "0.1";
   elementScale[1] = "1.5";   
   elementTint[1] = "0.996078 0.976471 0.721569 1";
   elementRotate[1] = true;
   elementUseLightColor[1] = true;
   
   elementRect[2] = "0 0 64 64";
   elementDist[2] = "0.4";
   elementScale[2] = "0.25";   
   elementTint[2] = "0 0 1 1";
   elementRotate[2] = true;
   elementUseLightColor[2] = true;
   
   elementRect[3] = "0 0 64 64";
   elementDist[3] = "0.45";
   elementScale[3] = 0.25;   
   elementTint[3] = "0 1 0 1";
   elementRotate[3] = true;
   elementUseLightColor[3] = true;
   
   elementRect[4] = "0 0 64 64";
   elementDist[4] = "0.5";
   elementScale[4] = 0.25;      
   elementTint[4] = "1 0 0 1";
   elementRotate[4] = true;
   elementUseLightColor[4] = true;
   elementRect[9] = "256 0 256 256";
   elementDist[3] = "0.45";
   elementScale[3] = "0.25";
   elementScale[9] = "2";
   elementRect[4] = "0 0 64 64";
   elementRect[5] = "128 0 128 128";
   elementDist[4] = "0.5";
   elementDist[5] = "1.2";
   elementScale[1] = "1.5";
   elementScale[4] = "0.25";
   elementScale[5] = "0.5";
   elementTint[1] = "0.996078 0.976471 0.721569 1";
   elementTint[2] = "0 0 1 1";
   elementTint[5] = "0.721569 0 1 1";
   elementRotate[5] = "0";
   elementUseLightColor[5] = "1";
   elementRect[0] = "0 256 256 256";
   elementRect[1] = "128 128 128 128";
   elementRect[2] = "0 0 64 64";
   elementRect[3] = "0 0 64 64";
   elementDist[0] = "-0.6";
   elementDist[1] = "0.1";
   elementDist[2] = "0.4";
   elementScale[0] = "3.5";
   elementScale[2] = "0.25";
   elementTint[0] = "0.537255 0.537255 0.537255 1";
   elementTint[3] = "0 1 0 1";
   elementTint[4] = "1 0 0 1";
   elementRect[6] = "64 64 64 64";
   elementDist[6] = "0.9";
   elementScale[6] = "4";
   elementTint[6] = "0.00392157 0.721569 0.00392157 1";
   elementRotate[6] = "0";
   elementUseLightColor[6] = "1";
   elementRect[7] = "64 64 64 64";
   elementRect[8] = "64 64 64 64";
   elementDist[7] = "0.25";
   elementDist[8] = "0.18";
   elementDist[9] = "0";
   elementScale[7] = "2";
   elementScale[8] = "0.5";
   elementTint[7] = "0.6 0.0117647 0.741176 1";
   elementTint[8] = "0.027451 0.690196 0.0117647 1";
   elementTint[9] = "0.647059 0.647059 0.647059 1";
   elementRotate[9] = "0";
   elementUseLightColor[7] = "1";
   elementUseLightColor[8] = "1";
   elementRect[10] = "256 256 256 256";
   elementRect[11] = "0 64 64 64";
   elementRect[12] = "0 64 64 64";
   elementRect[13] = "64 0 64 64";
   elementDist[10] = "0";
   elementDist[11] = "-0.3";
   elementDist[12] = "-0.32";
   elementDist[13] = "1";
   elementScale[10] = "10";
   elementScale[11] = "2.5";
   elementScale[12] = "0.3";
   elementScale[13] = "0.4";
   elementTint[10] = "0.321569 0.321569 0.321569 1";
   elementTint[11] = "0.443137 0.0431373 0.00784314 1";
   elementTint[12] = "0.00784314 0.996078 0.0313726 1";
   elementTint[13] = "0.996078 0.94902 0.00784314 1";
   elementUseLightColor[10] = "1";
   elementUseLightColor[11] = "1";
   elementUseLightColor[13] = "1";
   elementRect[14] = "0 0 64 64";
   elementDist[14] = "0.15";
   elementScale[14] = "0.8";
   elementTint[14] = "0.505882 0.0470588 0.00784314 1";
   elementRotate[14] = "1";
   elementUseLightColor[9] = "1";
   elementUseLightColor[14] = "1";
   elementRect[15] = "64 64 64 64";
   elementRect[16] = "0 64 64 64";
   elementRect[17] = "0 0 64 64";
   elementRect[18] = "0 64 64 64";
   elementRect[19] = "256 0 256 256";
   elementDist[15] = "0.8";
   elementDist[16] = "0.7";
   elementDist[17] = "1.4";
   elementDist[18] = "-0.5";
   elementDist[19] = "-1.5";
   elementScale[15] = "3";
   elementScale[16] = "0.3";
   elementScale[17] = "0.2";
   elementScale[18] = "1";
   elementScale[19] = "35";
   elementTint[15] = "0.00784314 0.00784314 0.996078 1";
   elementTint[16] = "0.992157 0.992157 0.992157 1";
   elementTint[17] = "0.996078 0.603922 0.00784314 1";
   elementTint[18] = "0.2 0.00392157 0.47451 1";
   elementTint[19] = "0.607843 0.607843 0.607843 1";
   elementUseLightColor[15] = "1";
   elementUseLightColor[18] = "1";
   elementUseLightColor[19] = "1";
   occlusionRadius = "0.25";
};



datablock LightFlareData(SunFlarePacificIsland)
{
   overallScale = 2.0;
   flareEnabled = true;
   renderReflectPass = false;
   flareTexture = "art/lights/lensflareSheet3.png";  
   
   elementRect[0] = "0 256 256 256";
   elementDist[0] = "-0.6";
   elementScale[0] = "3.5";   
   elementTint[0] = "0.537255 0.537255 0.537255 1";
   elementRotate[0] = true;
   elementUseLightColor[0] = true;
   
   elementRect[1] = "128 128 128 128";
   elementDist[1] = "0.1";
   elementScale[1] = "1.5";   
   elementTint[1] = "0.996078 0.976471 0.721569 1";
   elementRotate[1] = true;
   elementUseLightColor[1] = true;
   
   elementRect[2] = "0 0 64 64";
   elementDist[2] = "0.4";
   elementScale[2] = "0.25";   
   elementTint[2] = "0 0 1 1";
   elementRotate[2] = true;
   elementUseLightColor[2] = true;
   
   elementRect[3] = "0 0 64 64";
   elementDist[3] = "0.45";
   elementScale[3] = 0.25;   
   elementTint[3] = "0 1 0 1";
   elementRotate[3] = true;
   elementUseLightColor[3] = true;
   
   elementRect[4] = "0 0 64 64";
   elementDist[4] = "0.5";
   elementScale[4] = 0.25;      
   elementTint[4] = "1 0 0 1";
   elementRotate[4] = true;
   elementUseLightColor[4] = true;
   elementRect[9] = "256 0 256 256";
   elementDist[3] = "0.45";
   elementScale[3] = "0.25";
   elementScale[9] = "2";
   elementRect[4] = "0 0 64 64";
   elementRect[5] = "128 0 128 128";
   elementDist[4] = "0.5";
   elementDist[5] = "1.2";
   elementScale[1] = "1.5";
   elementScale[4] = "0.25";
   elementScale[5] = "0.5";
   elementTint[1] = "0.996078 0.976471 0.721569 1";
   elementTint[2] = "0 0 1 1";
   elementTint[5] = "0.721569 0 1 1";
   elementRotate[5] = "0";
   elementUseLightColor[5] = "1";
   elementRect[0] = "0 256 256 256";
   elementRect[1] = "128 128 128 128";
   elementRect[2] = "0 0 64 64";
   elementRect[3] = "0 0 64 64";
   elementDist[0] = "-0.6";
   elementDist[1] = "0.1";
   elementDist[2] = "0.4";
   elementScale[0] = "3.5";
   elementScale[2] = "0.25";
   elementTint[0] = "0.537255 0.537255 0.537255 1";
   elementTint[3] = "0 1 0 1";
   elementTint[4] = "1 0 0 1";
   elementRect[6] = "64 64 64 64";
   elementDist[6] = "0.9";
   elementScale[6] = "4";
   elementTint[6] = "0.00392157 0.721569 0.00392157 1";
   elementRotate[6] = "0";
   elementUseLightColor[6] = "1";
   elementRect[7] = "64 64 64 64";
   elementRect[8] = "64 64 64 64";
   elementDist[7] = "0.25";
   elementDist[8] = "0.18";
   elementDist[9] = "0";
   elementScale[7] = "2";
   elementScale[8] = "0.5";
   elementTint[7] = "0.6 0.0117647 0.741176 1";
   elementTint[8] = "0.027451 0.690196 0.0117647 1";
   elementTint[9] = "0.647059 0.647059 0.647059 1";
   elementRotate[9] = "0";
   elementUseLightColor[7] = "1";
   elementUseLightColor[8] = "1";
   elementRect[10] = "256 256 256 256";
   elementRect[11] = "0 64 64 64";
   elementRect[12] = "0 64 64 64";
   elementRect[13] = "64 0 64 64";
   elementDist[10] = "0";
   elementDist[11] = "-0.3";
   elementDist[12] = "-0.32";
   elementDist[13] = "1";
   elementScale[10] = "10";
   elementScale[11] = "2.5";
   elementScale[12] = "0.3";
   elementScale[13] = "0.4";
   elementTint[10] = "0.321569 0.321569 0.321569 1";
   elementTint[11] = "0.443137 0.0431373 0.00784314 1";
   elementTint[12] = "0.00784314 0.996078 0.0313726 1";
   elementTint[13] = "0.996078 0.94902 0.00784314 1";
   elementUseLightColor[10] = "1";
   elementUseLightColor[11] = "1";
   elementUseLightColor[13] = "1";
   elementRect[14] = "0 0 64 64";
   elementDist[14] = "0.15";
   elementScale[14] = "0.8";
   elementTint[14] = "0.505882 0.0470588 0.00784314 1";
   elementRotate[14] = "1";
   elementUseLightColor[9] = "1";
   elementUseLightColor[14] = "1";
   elementRect[15] = "64 64 64 64";
   elementRect[16] = "0 64 64 64";
   elementRect[17] = "0 0 64 64";
   elementRect[18] = "0 64 64 64";
   elementRect[19] = "256 0 256 256";
   elementDist[15] = "0.8";
   elementDist[16] = "0.7";
   elementDist[17] = "1.4";
   elementDist[18] = "-0.5";
   elementDist[19] = "-1.5";
   elementScale[15] = "3";
   elementScale[16] = "0.3";
   elementScale[17] = "0.2";
   elementScale[18] = "1";
   elementScale[19] = "35";
   elementTint[15] = "0.00784314 0.00784314 0.996078 1";
   elementTint[16] = "0.992157 0.992157 0.992157 1";
   elementTint[17] = "0.996078 0.603922 0.00784314 1";
   elementTint[18] = "0.2 0.00392157 0.47451 1";
   elementTint[19] = "0.607843 0.607843 0.607843 1";
   elementUseLightColor[15] = "1";
   elementUseLightColor[18] = "1";
   elementUseLightColor[19] = "1";
};



datablock LightFlareData( LightFlareExample0 )
{
   overallScale = 2.0;
   flareEnabled = true;
   renderReflectPass = true;
   flareTexture = "art/lights/lensFlareSheet1";
   
   elementRect[0] = "0 512 512 512";
   elementDist[0] = 0.0;
   elementScale[0] = 0.5;
   elementTint[0] = "1.0 1.0 1.0";
   elementRotate[0] = false;
   elementUseLightColor[0] = false;
   
   elementRect[1] = "512 0 512 512";
   elementDist[1] = 0.0;
   elementScale[1] = 2.0;
   elementTint[1] = "0.5 0.5 0.5";
   elementRotate[1] = false;
   elementUseLightColor[1] = false;
   occlusionRadius = "0.25";
};

datablock LightFlareData( LightFlareExample1 )
{
   overallScale = 2.0;
   flareEnabled = true;
   renderReflectPass = true;
   flareTexture = "art/lights/lensFlareSheet1";
   
   elementRect[0] = "512 512 512 512";
   elementDist[0] = 0.0;
   elementScale[0] = 0.5;
   elementTint[0] = "1.0 1.0 1.0";
   elementRotate[0] = false;
   elementUseLightColor[0] = false;
   
   elementRect[1] = "512 0 512 512";
   elementDist[1] = 0.0;
   elementScale[1] = 2.0;
   elementTint[1] = "0.5 0.5 0.5";
   elementRotate[1] = false;
   elementUseLightColor[1] = false;
   occlusionRadius = "0.25";
};

datablock LightFlareData( LightFlareExample2 )
{
   overallScale = 2.0;
   flareEnabled = true;
   renderReflectPass = true;
   flareTexture = "art/lights/lensFlareSheet0";  

   elementRect[0] = "512 512 512 512";
   elementDist[0] = 0.0;
   elementScale[0] = 0.5;
   elementTint[0] = "1.0 1.0 1.0";
   elementRotate[0] = true;
   elementUseLightColor[0] = true;
   
   elementRect[1] = "512 0 512 512";
   elementDist[1] = 0.0;
   elementScale[1] = 2.0;
   elementTint[1] = "0.7 0.7 0.7";
   elementRotate[1] = true;
   elementUseLightColor[1] = true;
   
   elementRect[2] = "1152 0 128 128";
   elementDist[2] = 0.3;
   elementScale[2] = 0.5;
   elementTint[2] = "1.0 1.0 1.0";
   elementRotate[2] = true;
   elementUseLightColor[2] = true;
   
   elementRect[3] = "1024 0 128 128";
   elementDist[3] = 0.5;
   elementScale[3] = 0.25;   
   elementTint[3] = "1.0 1.0 1.0";
   elementRotate[3] = true;
   elementUseLightColor[3] = true;
   
   elementRect[4] = "1024 128 128 128";
   elementDist[4] = 0.8;
   elementScale[4] = 0.6;   
   elementTint[4] = "1.0 1.0 1.0";
   elementRotate[4] = true;
   elementUseLightColor[4] = true;
   
   elementRect[5] = "1024 0 128 128";
   elementDist[5] = 1.18;
   elementScale[5] = 0.5;   
   elementTint[5] = "0.7 0.7 0.7";
   elementRotate[5] = true;
   elementUseLightColor[5] = true;
   
   elementRect[6] = "1152 128 128 128";
   elementDist[6] = 1.25;
   elementScale[6] = 0.35;   
   elementTint[6] = "0.8 0.8 0.8";
   elementRotate[6] = true;
   elementUseLightColor[6] = true;
   
   elementRect[7] = "1024 0 128 128";
   elementDist[7] = 2.0;
   elementScale[7] = 0.25;      
   elementTint[7] = "1.0 1.0 1.0";
   elementRotate[7] = true;
   elementUseLightColor[7] = true;
};
