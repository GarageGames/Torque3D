
function Core_SFX::onCreate(%this)
{
   exec("./scripts/audio.cs");
   exec("./scripts/audioData.cs");
   exec("./scripts/audioAmbience.cs");
   exec("./scripts/audioDescriptions.cs");
   exec("./scripts/audioEnvironments.cs");
   exec("./scripts/audioStates.cs");

}

function Core_SFX::onDestroy(%this)
{
}