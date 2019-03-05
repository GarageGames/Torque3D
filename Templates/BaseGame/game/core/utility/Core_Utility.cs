
function Core_Utility::onCreate(%this)
{
   exec("./scripts/parseArgs.cs");
   exec("./scripts/globals.cs");
   exec("./scripts/helperFunctions.cs");
   exec("./scripts/gameObjectManagement.cs");
   exec("./scripts/persistanceManagement.cs");
}

function Core_Utility::onDestroy(%this)
{
}