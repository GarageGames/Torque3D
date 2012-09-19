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
#include "lighting/lightQuery.h"

#include "lighting/lightManager.h"
#include "platform/profiler.h"


LightQuery::LightQuery( U32 maxLights )
   : mMaxLights( maxLights )
{
}

LightQuery::~LightQuery()
{
}

void LightQuery::init(  const Point3F &cameraPos,
                        const Point3F &cameraDir, 
                        F32 viewDist )
{
   mVolume.center = cameraPos;
   mVolume.radius = viewDist;
   mLights.clear();
}

void LightQuery::init( const SphereF &bounds )
{
   mVolume = bounds;
   mLights.clear();
}

void LightQuery::init( const Box3F &bounds )
{
   bounds.getCenter( &mVolume.center );
   mVolume.radius = ( bounds.maxExtents - mVolume.center ).len();
   mLights.clear();
}

void LightQuery::getLights( LightInfo** outLights, U32 maxLights )
{
   PROFILE_SCOPE( LightQuery_getLights );

   // Gather lights if we haven't already.
   if ( mLights.empty() )
      _scoreLights();

   U32 lightCount = getMin( (U32)mLights.size(), getMin( mMaxLights, maxLights ) );

   // Copy them over.
   for ( U32 i = 0; i < lightCount; i++ )
   {
      LightInfo *light = mLights[i];

      // If the score reaches zero then we got to
      // the end of the valid lights for this object.
      if ( light->getScore() <= 0.0f )
         break;

      outLights[i] = light;
   }
}

void LightQuery::_scoreLights()
{
   PROFILE_SCOPE( LightQuery_scoreLights );

   if ( !LIGHTMGR )
      return;

   // Get all the lights.
   LIGHTMGR->getAllUnsortedLights( &mLights );
   LightInfo *sun = LIGHTMGR->getSpecialLight( LightManager::slSunLightType );

   const Point3F lumDot( 0.2125f, 0.7154f, 0.0721f );

   Vector<LightInfo*>::iterator iter = mLights.begin();
   for ( ; iter != mLights.end(); iter++ )
   {
      // Get the light.
      LightInfo *light = (*iter);

      F32 luminace = 0.0f;
      F32 dist = 0.0f;
      F32 weight = 0.0f;

      const bool isSpot = light->getType() == LightInfo::Spot;
      const bool isPoint = light->getType() == LightInfo::Point;

      if ( isPoint || isSpot )
      {
         // Get the luminocity.
         luminace = mDot( light->getColor(), lumDot ) * light->getBrightness();

         // Get the distance to the light... score it 1 to 0 near to far.
         F32 lenSq = ( mVolume.center - light->getPosition() ).lenSquared();

         F32 radiusSq = mSquared( light->getRange().x + mVolume.radius );
         F32 distSq = radiusSq - lenSq;
         
         if ( distSq > 0.0f )
            dist = mClampF( distSq / ( 1000.0f * 1000.0f ), 0.0f, 1.0f );

         // TODO: This culling is broken... it culls spotlights 
         // that are actually visible.
         if ( false && isSpot && dist > 0.0f )
         {
            // TODO: I cannot test to see if we're within
            // the cone without a more detailed test... so
            // just reject if we're behind the spot direction.

            Point3F toCenter = mVolume.center - light->getPosition();
            F32 angDot = mDot( toCenter, light->getDirection() );
            if ( angDot < 0.0f )
               dist = 0.0f;
         }

         weight = light->getPriority();
      }
      else
      {
         // The sun always goes first 
         // regardless of the settings.
         if ( light == sun )
         {
            weight = F32_MAX;
            dist = 1.0f;
            luminace = 1.0f;
         }
         else
         {
            // TODO: When we have multiple directional 
            // lights we should score them here.
         }
      }
      
      // TODO: Manager ambient lights here too!

      light->setScore( luminace * weight * dist );
   }

   // Sort them!
   mLights.sort( _lightScoreCmp );
}

S32 LightQuery::_lightScoreCmp( LightInfo* const *a, LightInfo* const *b )
{
   F32 diff = (*a)->getScore() - (*b)->getScore();
   return diff < 0 ? 1 : diff > 0 ? -1 : 0;
}
