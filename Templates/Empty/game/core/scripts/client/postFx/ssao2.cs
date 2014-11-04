///
$SSAO2PostFx::intensity = 5.0;
$SSAO2PostFx::radius = 1.0;
$SSAO2PostFx::scale = 1.0;
$SSAO2PostFx::bias = 0.35;

///
$SSAO2PostFx::targetScale = "1 1";


function SSAO2PostFx::onAdd( %this )
{
   %this.wasVis = "Uninitialized";
}

function SSAO2PostFx::preProcess( %this )
{
   %this.targetScale = $SSAO2PostFx::targetScale;
}

function SSAO2PostFx::setShaderConsts( %this )
{
   %this.setShaderConst( "$cameraFOV",      $Pref::Player::DefaultFOV );

   %ao = %this->ao;
   %ao.setShaderConst( "$intensity", $SSAO2PostFx::intensity );
   %ao.setShaderConst( "$radius",    $SSAO2PostFx::radius );
   %ao.setShaderConst( "$scale",     $SSAO2PostFx::scale );
   %ao.setShaderConst( "$bias",      $SSAO2PostFx::bias );
}

function SSAO2PostFx::onEnabled( %this )
{
   // This tells the AL shaders to reload and sample
   // from our #ssaoMask texture target.
   $AL::UseSSAOMask = true;

   return true;
}

function SSAO2PostFx::onDisabled( %this )
{
  $AL::UseSSAOMask = false;
}


//-----------------------------------------------------------------------------
// GFXStateBlockData / ShaderData
//-----------------------------------------------------------------------------

singleton GFXStateBlockData( SSAO2StateBlock : PFX_DefaultStateBlock )
{
   samplersDefined = true;
   samplerStates[0] = SamplerClampPoint;
   samplerStates[1] = SamplerClampPoint;
   samplerStates[2] = SamplerWrapLinear;
};

singleton GFXStateBlockData( SSAO2PosStateBlock : PFX_DefaultStateBlock )
{
   samplersDefined = true;
   samplerStates[0] = SamplerClampPoint;
};

singleton ShaderData( SSAO2Shader )
{
   DXVertexShaderFile 	= "shaders/common/postFx/postFxV.hlsl";
   DXPixelShaderFile 	= "shaders/common/postFx/ssao2/SSAO_P.hlsl";
   pixVersion = 3.0;
};

singleton ShaderData( SSAO2PosShader )
{
   DXVertexShaderFile 	= "shaders/common/postFx/ssao2/SSAO_Pos_V.hlsl";
   DXPixelShaderFile 	= "shaders/common/postFx/ssao2/SSAO_Pos_P.hlsl";
   pixVersion = 3.0;
};

//-----------------------------------------------------------------------------
// PostEffects
//-----------------------------------------------------------------------------

singleton PostEffect( SSAO2PostFx )
{
   allowReflectPass = false;

   renderTime = "PFXBeforeBin";
   renderBin = "AL_LightBinMgr";
   renderPriority = 10;

   shader = SSAO2PosShader;
   stateBlock = SSAO2PosStateBlock;

   texture[0] = "#prepass";

   target = "$outTex";
   targetFormat = "GFXFormatR16G16B16A16F";
   targetScale = "0.5 0.5";

   singleton PostEffect()
   {
      internalName = "ao";

      shader = SSAO2Shader;
      stateBlock = SSAO2StateBlock;

      texture[0] = "$inTex";
      texture[1] = "#prepass";
      texture[2] = "randNorm.png";

      target = "#ssaoMask";
   };

};


/// Just here for debug visualization of the
/// SSAO mask texture used during lighting.
singleton PostEffect( SSAO2VizPostFx )
{
   allowReflectPass = false;

   shader = PFX_PassthruShader;
   stateBlock = PFX_DefaultStateBlock;

   texture[0] = "#ssaoMask";

   target = "$backbuffer";
};