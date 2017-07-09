//--- noshape.dts MATERIALS BEGIN ---
singleton Material(noshape_NoShape)
{
    mapTo = "NoShape";

    diffuseMap[0] = "";
    diffuseColor[0] = "0.8 0.003067 0 .8";
    emissive[0] = 0;
    doubleSided = false;
    translucent = 1;
    translucentBlendOp = "LerpAlpha";
    castShadows = false;
    materialTag0 = "WorldEditor";
};

//--- noshape.dts MATERIALS END ---

//--- noshapetext.dae MATERIALS BEGIN ---
singleton Material(noshapetext_lambert1)
{
    mapTo = "lambert1";

    diffuseMap[0] = "";
    diffuseColor[0] = "0.4 0.4 0.4 1";
    specular[0] = "1 1 1 1";
    specularPower[0] = 8;
    pixelSpecular[0] = false;
    emissive[0] = true;
    doubleSided = false;
    translucent = false;
    translucentBlendOp = "None";
    materialTag0 = "WorldEditor";
};

singleton Material(noshapetext_noshape_mat)
{
   mapTo = "noshape_mat";

	diffuseMap[0] = "";

	diffuseColor[0] = "0.4 0.3504 0.363784 0.33058";
	specular[0] = "1 1 1 1";
	specularPower[0] = 8;
	pixelSpecular[0] = false;
	emissive[0] = true;

	doubleSided = false;
	translucent = true;
	translucentBlendOp = "None";
    materialTag0 = "WorldEditor";
};

//--- noshapetext.dae MATERIALS END ---