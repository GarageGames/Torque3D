
function Core_GUI::onCreate(%this)
{
   exec("./scripts/profiles.cs");
   exec("./scripts/canvas.cs");
   exec("./scripts/cursor.cs");
}

function Core_GUI::onDestroy(%this)
{
}