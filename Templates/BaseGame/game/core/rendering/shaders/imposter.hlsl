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

#include "torque.hlsl"


static float sCornerRight[4] = { -1, 1, 1, -1 };
static float sCornerUp[4] = { -1, -1, 1, 1 };
static float2 sUVCornerExtent[4] =
{ 
   float2( 0, 1 ),
   float2( 1, 1 ), 
   float2( 1, 0 ), 
   float2( 0, 0 )
};

#define IMPOSTER_MAX_UVS 64


void imposter_v(
                    // These parameters usually come from the vertex.
                    float3 center,
                    int corner,
                    float halfSize,
                    float3 imposterUp,
                    float3 imposterRight,

                    // These are from the imposter shader constant.
                    int numEquatorSteps,
                    int numPolarSteps,
                    float polarAngle,
                    bool includePoles,

                    // Other shader constants.
                    float3 camPos,
                    float4 uvs[IMPOSTER_MAX_UVS],
                    
                    // The outputs of this function.
                    out float3 outWsPosition,
                    out float2 outTexCoord,
                    out float3x3 outWorldToTangent
    )
{
    // TODO: This could all be calculated on the CPU.
    float equatorStepSize = M_2PI_F / numEquatorSteps;
    float equatorHalfStep = ( equatorStepSize / 2.0 ) - 0.0001;
    float polarStepSize = M_PI_F / numPolarSteps;
    float polarHalfStep = ( polarStepSize / 2.0 ) - 0.0001;

    // The vector between the camera and the billboard.
    float3 lookVec = normalize( camPos - center );

    // Generate the camera up and right vectors from
    // the object transform and camera forward.
    float3 camUp = imposterUp;
    float3 camRight = normalize( cross( -lookVec, camUp ) );

    // The billboarding is based on the camera directions.
    float3 rightVec  = camRight * sCornerRight[corner];
    float3 upVec     = camUp * sCornerUp[corner];

    float lookPitch = acos( dot( imposterUp, lookVec ) );

    // First check to see if we need to render the top billboard.   
    int index;
    /*
    if ( includePoles && ( lookPitch < polarAngle || lookPitch > sPi - polarAngle ) )
    {
      index = numEquatorSteps * 3; 

      // When we render the top/bottom billboard we always use
      // a fixed vector that matches the rotation of the object.
      rightVec = float3( 1, 0, 0 ) * sCornerRight[corner];
      upVec = float3( 0, 1, 0 ) * sCornerUp[corner];

      if ( lookPitch > sPi - polarAngle )
      {
         upVec = -upVec;
         index++;
      }
    }
    else
    */
    {
        // Calculate the rotation around the z axis then add the
        // equator half step.  This gets the images to switch a
        // half step before the captured angle is met.
        float lookAzimuth = atan2( lookVec.y, lookVec.x );
        float azimuth = atan2( imposterRight.y, imposterRight.x );
        float rotZ = ( lookAzimuth - azimuth ) + equatorHalfStep;

        // The y rotation is calculated from the look vector and 
        // the object up vector.
        float rotY = lookPitch - polarHalfStep;

        // TODO: How can we do this without conditionals?
        // Normalize the result to 0 to 2PI.
        if ( rotZ < 0 )
            rotZ += M_2PI_F;
        if ( rotZ > M_2PI_F )
            rotZ -= M_2PI_F;
        if ( rotY < 0 )
            rotY += M_2PI_F;
        if ( rotY > M_PI_F ) // Not M_2PI_F?
            rotY -= M_2PI_F;

        float polarIdx = round( abs( rotY ) / polarStepSize );

        // Get the index to the start of the right polar
        // images for this viewing angle.
        int numPolarOffset = numEquatorSteps * polarIdx;

        // Calculate the final image index for lookup 
        // of the texture coords.
        index = ( rotZ / equatorStepSize ) + numPolarOffset;
    }

    // Generate the final world space position.         
    outWsPosition = center + ( upVec * halfSize ) + ( rightVec * halfSize );

    // Grab the uv set and setup the texture coord.
    float4 uvSet = uvs[index];
    outTexCoord.x = uvSet.x + ( uvSet.z * sUVCornerExtent[corner].x );
    outTexCoord.y = uvSet.y + ( uvSet.w * sUVCornerExtent[corner].y );

    // Needed for normal mapping and lighting.    
    outWorldToTangent[0] = float3( 1, 0, 0 );
    outWorldToTangent[1] = float3( 0, 1, 0 );
    outWorldToTangent[2] = float3( 0, 0, -1 );
}
