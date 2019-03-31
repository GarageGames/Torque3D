
function Core_Lighting::onCreate(%this)
{
   exec("./scripts/lighting.cs");
   
   //Advanced/Deferred
   exec("./scripts/advancedLighting_Shaders.cs");
   exec("./scripts/deferredShading.cs");
   exec("./scripts/advancedLighting_Init.cs");
   
   //Basic/Forward
   exec("./scripts/basicLighting_shadowFilter.cs");
   exec("./scripts/shadowMaps_Init.cs");
   exec("./scripts/basicLighting_Init.cs");
   
}

function Core_Lighting::onDestroy(%this)
{
}