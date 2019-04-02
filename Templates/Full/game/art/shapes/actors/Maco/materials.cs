singleton Material(DECAL_PlayerFootprint)
{
	diffuseMap[0] = "art/shapes/actors/Maco/footprint.png";
	normalMap[0] = "art/shapes/actors/Maco/footprint_normal.png";
	translucent = "1";
	//parallaxScale[0] = "0.05";
};

singleton Material( base_maco_mat_body_diffusetex )
{
	mapTo = "base_maco_mat_body_diffusetex";
	diffuseMap[0] = "art/shapes/actors/Maco/base_maco_mat_body_diffusetex";
	doubleSided = "1";	//our normals are backwards (oops!) so we turn this on
};

singleton Material( cnst_maco_mat_face_diffusetex )
{
	mapTo = "cnst_maco_mat_face_diffusetex";
	diffuseMap[0] = "art/shapes/actors/Maco/cnst_maco_mat_face_diffusetex";
	doubleSided = "1";	//our normals are backwards (oops!) so we turn this on
	subSurface[0] = "1";
	subSurfaceRolloff[0] = "0.1";
};

//Blue Maco skin
singleton Material( blue_maco_mat_body_diffusetex : base_maco_mat_body_diffusetex )
{
   mapTo = "blue_maco_mat_body_diffusetex";
   diffuseMap[0] = "art/shapes/actors/Maco/blue_maco_mat_body_diffusetex";
};