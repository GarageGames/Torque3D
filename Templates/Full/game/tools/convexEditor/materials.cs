singleton Material( ZoneProxyMaterial )
{
   mapTo = "ZoneProxyMaterial";
   diffuseMap[0] = "./images/zoneProxyImage";
   materialTag0 = "TestMaterial";
   translucent = true;
   translucentBlendOp = "LerpAlpha";
   castShadows = false;
};

singleton Material( TriggerProxyMaterial )
{
   mapTo = "TriggerProxyMaterial";
   diffuseMap[0] = "./images/triggerProxyImage";
   materialTag0 = "TestMaterial";
   translucent = true;
   translucentBlendOp = "LerpAlpha";
   castShadows = false;
};

singleton Material( PortalProxyMaterial )
{
   mapTo = "PortalProxyMaterial";
   diffuseMap[0] = "./images/portalProxyImage";
   materialTag0 = "TestMaterial";
   translucent = true;
   translucentBlendOp = "LerpAlpha";
   castShadows = false;
};

singleton Material( OccluderProxyMaterial )
{
   mapTo = "OccluderProxyMaterial";
   diffuseMap[0] = "./images/occluderProxyImage";
   materialTag0 = "TestMaterial";
   translucent = true;
   translucentBlendOp = "LerpAlpha";
   castShadows = false;
};