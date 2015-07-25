singleton Material(w_swarmgun)
{
	mapTo = "w_swarmgun";

	diffuseMap[0] = "art/shapes/weapons/shared/w_swarmgun";
	normalMap[0] = "";
	specularMap[0] = "";

	diffuseColor[0] = "1 1 1 1";
	specular[0] = "0.996078 0.988235 0.921569 0.954";
	specularPower[0] = "19";

	doubleSided = false;
	translucent = false;
	translucentBlendOp = "None";
   pixelSpecular[0] = "1";
   materialTag0 = "Weapon";
};

//--- RifleShell.DAE MATERIALS BEGIN ---
singleton Material(RifleShell_RifleShell)
{
	mapTo = "RifleShell";

	diffuseMap[0] = "art/shapes/weapons/shared/RifleShell_diff";
	normalMap[0] = "";
	specularMap[0] = "";

	diffuseColor[0] = "1 1 1 1";
	specular[0] = "0.996078 0.988235 0.921569 0.954";
	specularPower[0] = "19";

	doubleSided = false;
	translucent = false;
	translucentBlendOp = "None";
   pixelSpecular[0] = "1";
   materialTag0 = "Weapon";
};

singleton Material(RifleShell_ColorEffectR88G88B225_material)
{
	mapTo = "ColorEffectR88G88B225-material";

	diffuseMap[0] = "";
	normalMap[0] = "";
	specularMap[0] = "";

	diffuseColor[0] = "0.345098 0.345098 0.882353 1";
	specular[0] = "1 1 1 1";
	specularPower[0] = 10;

	doubleSided = false;
	translucent = false;
	translucentBlendOp = "None";
	materialTag0 = "Weapon";
};

//--- RifleShell.DAE MATERIALS END ---