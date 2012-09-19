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
 
 
#define NUM_TAPS 12 
 
#define NUM_PRE_TAPS 4 
 
/// The non-uniform poisson disk used in the 
/// high quality shadow filtering. 
vec2 sNonUniformTaps[NUM_TAPS];

void initNonUniformTaps()
{
   // These first 4 taps are located around the edges 
   // of the disk and are used to predict fully shadowed 
   // or unshadowed areas. 
   sNonUniformTaps[0] = vec2( 0.992833, 0.979309 );
   sNonUniformTaps[1] = vec2( -0.998585, 0.985853 );
   sNonUniformTaps[2] = vec2( 0.949299, -0.882562 ); 
   sNonUniformTaps[3] = vec2( -0.941358, -0.893924 ); 
 
   // The rest of the samples. 
   sNonUniformTaps[4] = vec2( 0.545055, -0.589072 );
   sNonUniformTaps[5] = vec2( 0.346526, 0.385821 );
   sNonUniformTaps[6] = vec2( -0.260183, 0.334412 ); 
   sNonUniformTaps[7] = vec2( 0.248676, -0.679605 ); 
   sNonUniformTaps[8] = vec2( -0.569502, -0.390637 ); 
   sNonUniformTaps[9] = vec2( -0.014096, 0.012577 ); 
   sNonUniformTaps[10] = vec2( -0.259178, 0.876272 ); 
   sNonUniformTaps[11] = vec2( 0.649526, 0.664333 ); 
}
 
/// The texture used to do per-pixel pseudorandom 
/// rotations of the filter taps. 
uniform sampler2D gTapRotationTex; 
 
 
float softShadow_sampleTaps(  sampler2D shadowMap, 
                              vec2 sinCos, 
                              vec2 shadowPos, 
                              float filterRadius, 
                              float distToLight, 
                              float esmFactor, 
                              int startTap, 
                              int endTap ) 
{ 
   initNonUniformTaps();
   float shadow = 0.0; 
 
   vec2 tap = vec2(0.0); 
   for ( int t = startTap; t < endTap; t++ ) 
   { 
      tap.x = ( sNonUniformTaps[t].x * sinCos.y - sNonUniformTaps[t].y * sinCos.x ) * filterRadius; 
      tap.y = ( sNonUniformTaps[t].y * sinCos.y + sNonUniformTaps[t].x * sinCos.x ) * filterRadius; 
      float occluder = texture2DLod( shadowMap, shadowPos + tap, 0.0 ).r; 
 
      float esm = clamp( exp( esmFactor * ( occluder - distToLight ) ), 0.0, 1.0 ); 
      shadow += esm / float( endTap - startTap ); 
   } 
 
   return shadow; 
} 
 
 
// HACK! HACK! HACK!
// We take the noise texture directly as the second parameter to ensure that it
// is the "last used" sampler, and thus doesn't collide with the prepass buffer
// or shadow map.  If we use gTapRotationTex directly here, then it is the first
// used sampler and will collide with the prepass buffer.
float softShadow_filter(   sampler2D shadowMap, 
                           sampler2D noiseTexture,
                           vec2 vpos, 
                           vec2 shadowPos, 
                           float filterRadius, 
                           float distToLight, 
                           float dotNL, 
                           float esmFactor ) 
{ 
   // Lookup the random rotation for this screen pixel. 
   vec2 sinCos = ( texture2DLod( noiseTexture, vpos * 16.0, 0.0 ).rg - 0.5 ) * 2.0; 
 
   // Do the prediction taps first. 
   float shadow = softShadow_sampleTaps(  shadowMap, 
                                          sinCos, 
                                          shadowPos, 
                                          filterRadius, 
                                          distToLight, 
                                          esmFactor, 
                                          0, 
                                          NUM_PRE_TAPS ); 
 
   // Only do the expensive filtering if we're really 
   // in a partially shadowed area. 
   if ( shadow * ( 1.0 - shadow ) * max( dotNL, 0.0 ) > 0.06 ) 
   { 
      shadow += softShadow_sampleTaps( shadowMap, 
                                       sinCos, 
                                       shadowPos, 
                                       filterRadius, 
                                       distToLight, 
                                       esmFactor, 
                                       NUM_PRE_TAPS, 
                                       NUM_TAPS ); 
                                        
      // This averages the taps above with the results 
      // of the prediction samples. 
      shadow *= 0.5;                               
   } 
 
   return shadow; 
} 