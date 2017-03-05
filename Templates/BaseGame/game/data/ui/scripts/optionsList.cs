new SimGroup( MeshQualityGroup )
{ 
   class = "GraphicsOptionsMenuGroup";
   
   new ArrayObject()
   {
      class = "GraphicsQualityLevel";
      caseSensitive = true;
      
      displayName = "High";

      key["$pref::TS::detailAdjust"] = 1.5;
      key["$pref::TS::skipRenderDLs"] = 0;      
   }; 
   new ArrayObject( )
   {
      class = "GraphicsQualityLevel";
      caseSensitive = true;
      
      displayName = "Medium";

      key["$pref::TS::detailAdjust"] = 1.0;
      key["$pref::TS::skipRenderDLs"] = 0;      
   };
   new ArrayObject()
   {
      class = "GraphicsQualityLevel";
      caseSensitive = true;
      
      displayName = "Low";
            
      key["$pref::TS::detailAdjust"] = 0.75;
      key["$pref::TS::skipRenderDLs"] = 0;      
   };
   new ArrayObject()
   {
      class = "GraphicsQualityLevel";
      caseSensitive = true;
      
      displayName = "Lowest";
      
      key["$pref::TS::detailAdjust"] = 0.5;
      key["$pref::TS::skipRenderDLs"] = 1;      
  };
};

new SimGroup( TextureQualityGroup )
{
   class = "GraphicsOptionsMenuGroup";
   
   new ArrayObject()
   {
      class = "GraphicsQualityLevel";
      caseSensitive = true;
      
      displayName = "High";

      key["$pref::Video::textureReductionLevel"] = 0;
      key["$pref::Reflect::refractTexScale"] = 1.25;
   }; 
   new ArrayObject()
   {
      class = "GraphicsQualityLevel";
      caseSensitive = true;
      
      displayName = "Medium";

      key["$pref::Video::textureReductionLevel"] = 0;
      key["$pref::Reflect::refractTexScale"] = 1;
   };
   new ArrayObject()
   {
      class = "GraphicsQualityLevel";
      caseSensitive = true;
      
      displayName = "Low";
            
      key["$pref::Video::textureReductionLevel"] = 1;
      key["$pref::Reflect::refractTexScale"] = 0.75;
   };
   new ArrayObject()
   {
      class = "GraphicsQualityLevel";
      caseSensitive = true;
      
      displayName = "Lowest";
      
      key["$pref::Video::textureReductionLevel"] = 2;
      key["$pref::Reflect::refractTexScale"] = 0.5;
   };
};

new SimGroup( GroundCoverDensityGroup )
{ 
   class = "GraphicsOptionsMenuGroup";
   
   new ArrayObject()
   {
      class = "GraphicsQualityLevel";
      caseSensitive = true;
      
      displayName = "High";

      key["$pref::GroundCover::densityScale"] = 1.0;
   };
   new ArrayObject()
   {
      class = "GraphicsQualityLevel";
      caseSensitive = true;
      
      displayName = "Medium";

      key["$pref::GroundCover::densityScale"] = 0.75;
   };
   new ArrayObject()
   {
      class = "GraphicsQualityLevel";
      caseSensitive = true;
      
      displayName = "Low";
            
      key["$pref::GroundCover::densityScale"] = 0.5;
   };
   new ArrayObject()
   {
      class = "GraphicsQualityLevel";
      caseSensitive = true;
      
      displayName = "Lowest";

      key["$pref::GroundCover::densityScale"] = 0.25;
   };
};

new SimGroup( DecalLifetimeGroup )
{ 
   class = "GraphicsOptionsMenuGroup";
   
   new ArrayObject()
   {
      class = "GraphicsQualityLevel";
      caseSensitive = true;
      
      displayName = "High";
      
      key["$pref::decalMgr::enabled"] = true;
      key["$pref::Decals::lifeTimeScale"] = 1;
   };
   new ArrayObject()
   {
      class = "GraphicsQualityLevel";
      caseSensitive = true;
      
      displayName = "Medium";
      
      key["$pref::decalMgr::enabled"] = true;
      key["$pref::Decals::lifeTimeScale"] = 0.5;
   };
   new ArrayObject()
   {
      class = "GraphicsQualityLevel";
      caseSensitive = true;
      
      displayName = "Low";
      
      key["$pref::decalMgr::enabled"] = true;
      key["$pref::Decals::lifeTimeScale"] = 0.25;
   };
   new ArrayObject()
   {
      class = "GraphicsQualityLevel";
      caseSensitive = true;
      
      displayName = "None";
      
      key["$pref::decalMgr::enabled"] = false;
   };
};

new SimGroup( TerrainQualityGroup )
{ 
   class = "GraphicsOptionsMenuGroup";
   
   new ArrayObject()
   {
      class = "GraphicsQualityLevel";
      caseSensitive = true;
      
      displayName = "High";

      key["$pref::Terrain::lodScale"] = 0.75;
      key["$pref::Terrain::detailScale"] = 1.5;
   }; 
   new ArrayObject()
   {
      class = "GraphicsQualityLevel";
      caseSensitive = true;
      
      displayName = "Medium";

      key["$pref::Terrain::lodScale"] = 1.0;
      key["$pref::Terrain::detailScale"] = 1.0;
   };
   new ArrayObject()
   {
      class = "GraphicsQualityLevel";
      caseSensitive = true;
      
      displayName = "Low";
            
      key["$pref::Terrain::lodScale"] = 1.5;
      key["$pref::Terrain::detailScale"] = 0.75;
   };
   new ArrayObject()
   {
      class = "GraphicsQualityLevel";
      caseSensitive = true;
      
      displayName = "Lowest";
      
      key["$pref::Terrain::lodScale"] = 2.0;
      key["$pref::Terrain::detailScale"] = 0.5; 
   };   
};

//Shadows and Lighting
new SimGroup( ShadowQualityList )
{
   class = "GraphicsOptionsMenuGroup";
   
   new ArrayObject()
   {
      class = "GraphicsQualityLevel";
      caseSensitive = true;
      
      displayName = "High";
      
      key["$pref::lightManager"] = "Advanced Lighting";
      key["$pref::Shadows::disable"] = false;
      key["$pref::Shadows::textureScalar"] = 1.0;
   };
   new ArrayObject()
   {
      class = "GraphicsQualityLevel";
      caseSensitive = true;
      
      displayName = "Medium";

      key["$pref::lightManager"] = "Advanced Lighting";
      key["$pref::Shadows::disable"] = false;
      key["$pref::Shadows::textureScalar"] = 0.5;
   };
   new ArrayObject()
   {
      class = "GraphicsQualityLevel";
      caseSensitive = true;
      
      displayName = "Low";
                  
      key["$pref::lightManager"] = "Advanced Lighting";
      key["$pref::Shadows::disable"] = false;
      key["$pref::Shadows::textureScalar"] = 0.25;

   };
   new ArrayObject()
   {
      class = "GraphicsQualityLevel";
      caseSensitive = true;
      
      displayName = "None";
      
      key["$pref::lightManager"] = "Advanced Lighting";
      key["$pref::Shadows::disable"] = true;
      key["$pref::Shadows::textureScalar"] = 0.5;
   };
};

new SimGroup( ShadowDistanceList )
{
   class = "GraphicsOptionsMenuGroup";
   
   new ArrayObject()
   {
      class = "GraphicsQualityLevel";
      caseSensitive = true;
      
      displayName = "Highest";
      
      key["$pref::Shadows::drawDistance"] = 2; 
   }; 
   new ArrayObject()
   {
      class = "GraphicsQualityLevel";
      caseSensitive = true;
      
      displayName = "High";
      
      key["$pref::Shadows::drawDistance"] = 1.5;
   };
   new ArrayObject()
   {
      class = "GraphicsQualityLevel";
      caseSensitive = true;
      
      displayName = "Medium";

      key["$pref::Shadows::drawDistance"] = 1; 
   };
   new ArrayObject()
   {
      class = "GraphicsQualityLevel";
      caseSensitive = true;
      
      displayName = "Low";
                  
      key["$pref::Shadows::drawDistance"] = 0.5;  
   };
   new ArrayObject()
   {
      class = "GraphicsQualityLevel";
      caseSensitive = true;
      
      displayName = "Lowest";
      
      key["$pref::Shadows::drawDistance"] = 0.25;
   };   
};

new SimGroup( SoftShadowList )
{
   class = "GraphicsOptionsMenuGroup";
   
   new ArrayObject()
   {
      class = "GraphicsQualityLevel";
      caseSensitive = true;
      
      displayName = "High";
                  
      key["$pref::Shadows::filterMode"] = "SoftShadowHighQuality"; 
   };
   new ArrayObject()
   {
      class = "GraphicsQualityLevel";
      caseSensitive = true;
      
      displayName = "Low";
                  
      key["$pref::Shadows::filterMode"] = "SoftShadow"; 
   };
   new ArrayObject()
   {
      class = "GraphicsQualityLevel";
      caseSensitive = true;
      
      displayName = "Off";
      
      key["$pref::Shadows::filterMode"] = "None"; 
   };
};

new SimGroup( LightDistanceList )
{
   class = "GraphicsOptionsMenuGroup";
   
   new ArrayObject()
   {
      class = "GraphicsQualityLevel";
      caseSensitive = true;
      
      displayName = "Highest";
      
      key["$pref::Lights::drawDistance"] = 2; 
   };
   new ArrayObject()
   {
      class = "GraphicsQualityLevel";
      caseSensitive = true;
      
      displayName = "High";
      
      key["$pref::Lights::drawDistance"] = 1.5;
   };
   new ArrayObject()
   {
      class = "GraphicsQualityLevel";
      caseSensitive = true;
      
      displayName = "Medium";

      key["$pref::Lights::drawDistance"] = 1; 
   };
   new ArrayObject()
   {
      class = "GraphicsQualityLevel";
      caseSensitive = true;
      
      displayName = "Low";
                  
      key["$pref::Lights::drawDistance"] = 0.5;  
   };
   new ArrayObject()
   {
      class = "GraphicsQualityLevel";
      caseSensitive = true;
      
      displayName = "Lowest";
      
      key["$pref::Lights::drawDistance"] = 0.25;
   };   
};

new SimGroup( ShaderQualityGroup )
{
   class = "GraphicsOptionsMenuGroup";
   
   new ArrayObject()
   {
      class = "GraphicsQualityLevel";
      caseSensitive = true;
      
      displayName = "High";
      
      key["$pref::Video::disablePixSpecular"] = false;
      key["$pref::Video::disableNormalmapping"] = false;
   };
   new ArrayObject()
   {
      class = "GraphicsQualityLevel";
      caseSensitive = true;
      
      displayName = "Low";
      
      key["$pref::Video::disablePixSpecular"] = true;
      key["$pref::Video::disableNormalmapping"] = true;
   };
};