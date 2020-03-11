//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

singleton CubemapData( BlankSkyCubemap )
{
   cubeFace[0] = "./skybox_1";
   cubeFace[1] = "./skybox_2";
   cubeFace[2] = "./skybox_3";
   cubeFace[3] = "./skybox_4";
   cubeFace[4] = "./skybox_5";
   cubeFace[5] = "./skybox_6";
};

singleton Material( BlankSkyMat )
{
   cubemap = BlankSkyCubemap;
};

singleton Material(White)
{
   diffuseMap[0] = "./white.png";
};

singleton Material(Gray)
{
   diffuseMap[0] = "./gray.png";
};

singleton Material(Black)
{
   diffuseMap[0] = "./black.png";
};

singleton Material(Grid_512_Black)
{
   diffuseMap[0] = "./512_black.png";
};

singleton Material(Grid_512_ForestGreen)
{
   diffuseMap[0] = "./512_forestgreen.png";
};

singleton Material(Grid_512_ForestGreen_Lines)
{
   diffuseMap[0] = "./512_forestgreen_lines.png";
};

singleton Material(Grid_512_Green)
{
   diffuseMap[0] = "./512_green.png";
};

singleton Material(Grid_512_Grey)
{
   diffuseMap[0] = "./512_grey.png";
};

singleton Material(Grid_512_Grey_Base)
{
   diffuseMap[0] = "./512_grey_base.png";
};

singleton Material(Grid_512_Orange)
{
   diffuseMap[0] = "./512_orange.png";
};

singleton Material(Grid_512_Orange_Lines)
{
   diffuseMap[0] = "./512_orange_lines.png";
};

singleton Material(Grid_512_Red)
{
   diffuseMap[0] = "./512_red.png";
};