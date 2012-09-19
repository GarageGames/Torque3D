<!-- saved from url=(0013)about:internet -->
<!-- see, http://technet.microsoft.com/en-us/library/bb457150.aspx#EHAA for how, why, this works for local controls -->
<!-- Please note that the saved from line must end in CR LF. Some HTML editors only insert a LF. (Thanks Microsoft) -->
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">

<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta name="description" content="Web Game Template" />
<script type="text/javascript" src="jquery-1.3.2.min.js"></script>
<link rel="stylesheet" href="styles.css" type="text/css" />

<script type="text/javascript" language="javascript">

// Relative path on web server to installer
var installerPath = "MyGameInstaller"

ua = navigator.userAgent.toLowerCase();
if( ua.indexOf('os x') != -1 || ua.indexOf('osx') != -1)
   installerPath += ".pkg";
else
    installerPath += ".exe";

// Firefox/Chrome/Safari
var mimeType = "application/__MIMETYPE__";

// Internet Explorer
var projId = "__PROJID__";
var clsId = "__CLSID__";

var pluginInstalled = false;
var minimumPluginVersion = 1.0;

// You can set this to true if you want the page to automatically reload
// once the plugin is installed.  However, this can be better handled by
// the installer process in most cases (by opening up web page at end of
// install process, setting up desktop/start menu shortcuts to launch web page
// etc
var autoReload = false;

// Default client screen (overridden below from actual browser window)
var cscreenW = 800;
var cscreenH = 600;

function getClientScreenSize() {
  if( typeof( window.innerWidth ) == 'number' ) {
    //Non-IE
    cscreenW = window.innerWidth;
    cscreenH = window.innerHeight;
  } else if( document.documentElement && ( document.documentElement.clientWidth || document.documentElement.clientHeight ) ) {
    //IE 6+ in 'standards compliant mode'
    cscreenW = document.documentElement.clientWidth;
    cscreenH = document.documentElement.clientHeight;
  } else if( document.body && ( document.body.clientWidth || document.body.clientHeight ) ) {
    //IE 4 compatible
    cscreenW = document.body.clientWidth;
    cscreenH = document.body.clientHeight;
  }
}

function gameObjectResize(){
    if (!pluginInstalled)
        return;

    getClientScreenSize();
    var aspectRatio = 800.0/600.0;
    var w = cscreenW - 50;
    var h = cscreenH - 125;
    if( w/h > aspectRatio )
        w = h*aspectRatio;
    else
        h = w/aspectRatio;
    var minW = 640;
    if( w < minW ){
        w = minW;
        h = w/aspectRatio;
    }
    $('#gameobject').width( Math.floor(w) );
    $('#gameobject').height( Math.floor(h) );
    $('#main').width( Math.floor(w + 2) );
    var mygame = document.getElementById("MyGame");
    mygame.width = Math.floor(w);
    mygame.height = Math.floor(h);
}

$(window).resize( gameObjectResize );

$(document).ready(function(){
    if (pluginInstalled)
     {
         var mygame = document.getElementById("MyGame");
         // Let the game object know the page is all loaded
         // This is important to register any TorqueScript <-> JavaScript callbacks, etc
         mygame.startup();
         // Export the TorqueScript -> JavaScript bridge test function 
         mygame.exportFunction("bridgeCallback",3);
         // Enable the bridge test button now that we're all set
         var bridgetest = document.getElementById("bridgetest");
         bridgetest.disabled = false;
         gameObjectResize();     
     }
});

// Returns the version of Internet Explorer 
// or -1 for non-IE browser
// or -2 for 64 bit IE browser
// (indicating the use of another browser).
function getInternetExplorerVersion()
{
  var rv = -1; // Return value for non-IE browser
  
  if (navigator.appName == 'Microsoft Internet Explorer')
  {
    var ua = navigator.userAgent;
    
    if (ua.search("Win64") != -1 || ua.search("x64") != -1)
       return -2; // Return value for IE 64 bit
    
    var re  = new RegExp("MSIE ([0-9]{1,}[\.0-9]{0,})");
    if (re.exec(ua) != null)
      rv = parseFloat( RegExp.$1 );
  }
  return rv;
}

// Checks whether the NPPlugin is installed (under Firefox/Chrome/Safari)
function nppluginIsInstalled() {

    if (!navigator || !navigator.mimeTypes) {
        return -1;
    }
    
    var mt = navigator.mimeTypes[mimeType];
    if (mt && mt.enabledPlugin)
    {
        var desc = mt.enabledPlugin.description;
        var descArray = desc.split(" ");
        var version = descArray[descArray.length - 1]
        return Number(version);
    }
    return -1;
}

function nppluginReload () {
    navigator.plugins.refresh();
    if (nppluginIsInstalled() < 0)
        window.location.reload();
    setTimeout('nppluginReload()', 500);
}

function activexReload () {
    
    if (activexIsInstalled() < 0)
        window.location.reload();
    setTimeout('activexReload()', 500);
}

function onTestBridge()
{
    var mygame = document.getElementById("MyGame");
    
    // set/get console variables test
    // variables are automatically stored Torque 3D side in the Javascript:: namespace
    // for security reasons.
    mygame.setVariable("$TestBridge", 42);
    var everything = mygame.getVariable("$TestBridge");
    
    // this tests bidirectional calling of JavaScript <-> TorqueScript including arguments and return values
    // note that testJavaScriptBridge must be specified in webConfig.h as a secure function
    var result = mygame.callScript("testJavaScriptBridge('one', 'two', 'three');");
    if (parseInt(everything) != 42)
        alert("JavaScript <-> TorqueScript: Failed, get/set console variable doesn't match");
    else if (result == "0")
        alert("JavaScript <-> TorqueScript: All Tests Passed!");
    else if (result == "1")
        alert("JavaScript -> TorqueScript: Failed, incorrect number of arguments");
    else if (result == "2")
        alert("JavaScript -> TorqueScript: Failed, incorrect argument");
    else if (result == "3")
        alert("TorqueScript -> JavaScript: Failed, incorrect return");
    else
        alert("JavaScript -> TorqueScript: Failed, unknown error");
}

// Called from TorqueScript console -> JavaScript during bridge test
function bridgeCallback(arg1, arg2, arg3)
{
    if (arg1 != "one" || arg2 != "two" || arg3 != "three")
    {
        alert("TorqueScript -> JavaScript: Failed, incorrect argument");
        return "0";
    }
    return "42";
}
</script>

<script language='VBScript'>
function activexIsInstalled()
    on error resume next
    dim gameControl
    dim version
    version = -1
    set gameControl = CreateObject(projId)
    if IsObject(gameControl) then
        version = CDbl(gameControl.getVariable("$version"))
    end if
    activexIsInstalled = version
end function
</script>

</head>
<body>
<img id = "torqueLogo" src = "./torque3D_logo.jpg">	
<div id="main">
    <div style = "height: 20px;"></div>
    <center>Web Game Template</center>
    <div id="gameobject">
        <script type="text/javascript" language="javascript">
        
        // ActiveX
        var ie = getInternetExplorerVersion();
        
        if ( ie == -2) {
              document.write('<center><h3>This plugin is not currently supported on Internet Explorer 64 bit<br><br>Please use Internet Explorer 32 bit to access this site.</h3><centre>');            
           }
        else if ( ie != -1) {
           if (activexIsInstalled() >= minimumPluginVersion) {
              pluginInstalled = true;
              document.write('<OBJECT ID="MyGame" CLASSID="CLSID:'+clsId+'"  WIDTH="100%" HEIGHT="100%"></OBJECT>');
           }
        else {
              document.write('<center><a href="'+installerPath+'"><img src="getplugin.jpg" /></a></center>');
              if (autoReload) 
                 activexReload();
           }
        }		   
        // Firefox/Chrome/Safari
        else {
            
            //we do an initial refresh in case the plugin information has changed (new DLL location, newly installed, etc)
            navigator.plugins.refresh();

            if (nppluginIsInstalled() >= minimumPluginVersion) {
                pluginInstalled = true;
                document.write('<object id="MyGame" type="'+mimeType+'" width="100% height="100%" ></object>');
            }
            else {
              document.write('<center><a href="'+installerPath+'"><img src="getplugin.jpg" /></a></center>');
              if (autoReload)
                 nppluginReload();
           }	
        }   
        </script>        
    </div>
    <center>(Press ESC to show the mouse cursor if hidden)</center>
    <center><button id="bridgetest" disabled="true" onclick="onTestBridge();">Test JavaScript <-> TorqueScript Bridge</button></center>
</div>
</body>
</html>