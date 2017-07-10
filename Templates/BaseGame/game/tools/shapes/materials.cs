//--- octahedron.dts MATERIALS BEGIN ---
singleton Material(OctahedronMat)
{
    mapTo = "green";
    
    diffuseMap[0] = "camera";
    translucent = "1";
    translucentBlendOp = "LerpAlpha";
    emissive = "0";
    castShadows = "0";
    colorMultiply[0] = "0 1 0 1";
    materialTag0 = "WorldEditor";
};

//--- octahedron.dts MATERIALS END ---

//--- simplecone.dts MATERIALS BEGIN ---
singleton Material(SimpleConeMat)
{
    mapTo = "blue";

    diffuseMap[0] = "blue";
    translucent = "0";
    emissive = "1";
    castShadows = "0";
    materialTag0 = "WorldEditor";
};
//--- simplecone.dts MATERIALS END ---

//--- camera.dts MATERIALS BEGIN ---
singleton Material(CameraMat)
{
    mapTo = "pasted__phongE1";

    diffuseMap[0] = "camera";
    diffuseColor[0] = "0 0.627451 1 1";
    specular[0] = "1 1 1 1";
    specularPower[0] = 211;
    pixelSpecular[0] = 1;
    emissive[0] = 1;
    doubleSided = 1;
    translucent = true;
    translucentBlendOp = "LerpAlpha";
    castShadows = false;
    materialTag0 = "WorldEditor";
};
//--- camera.dts MATERIALS END ---
