
function Core_Console::onCreate(%this)
{
    exec("./scripts/profiles.cs");
    exec("./scripts/console.cs");

    exec("./guis/console.gui");
}

function Core_Console::onDestroy(%this)
{
}