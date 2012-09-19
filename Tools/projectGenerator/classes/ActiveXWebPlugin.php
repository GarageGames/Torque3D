<?php
//-----------------------------------------------------------------------------
// Copyright (c) 2012 GarageGames, LLC
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------

class ActiveXWebPlugin
{
	// GUID
	var $REGISTRY_APPID_RESOURCEID = "";
	var $WEBGAME_UUID = "";
	var $IWEBGAMECTRL_UUID = "";
	var $WEBGAMELIB_UUID = "";
	
	var $WEBGAME_PLUGINNAME = "";
	var $WEBGAME_CTRLNAME = "";

    var $PLUGIN_DLL = "";
    var $PLUGIN_VERSION = "";
	
	var $FOLDER = "";	
	var $GAMEFOLDER = "";
	
	function ActiveXWebPlugin()
	{
		// Setup our GUID
		
	    //$this->REGISTRY_APPID_RESOURCEID = ActiveXWebPlugin::generateUUID();
		//$this->WEBGAME_UUID = ActiveXWebPlugin::generateUUID();
		//$this->IWEBGAMECTRL_UUID = ActiveXWebPlugin::generateUUID();
		//$this->WEBGAMELIB_UUID = ActiveXWebPlugin::generateUUID();
        
        $rootPhpBuildDir = getcwd();
		$this->FOLDER = $rootPhpBuildDir."\\web\\source\\activex\\";
		$this->GAMEFOLDER = $rootPhpBuildDir."\\game\\";

        $this->PLUGIN_DLL = WebDeploymentWindows::$axPluginName.".dll";
        $this->PLUGIN_VERSION = WebDeploymentWindows::$version;
        
        
	    $this->REGISTRY_APPID_RESOURCEID = WebDeploymentWindows::$axAppUUID;
		$this->WEBGAME_UUID = WebDeploymentWindows::$axWebGameUUID;
		$this->IWEBGAMECTRL_UUID = WebDeploymentWindows::$axWebGameCtrlUUID;
		$this->WEBGAMELIB_UUID = WebDeploymentWindows::$axWebGameLibUUID;
        
		
		$this->WEBGAME_PLUGINNAME = str_replace(" ", "", WebDeploymentWindows::$axPluginName);
		$this->WEBGAME_CTRLNAME = "IEWebGameCtrl";//str_replace(" ", "", WebDeploymentWindows::$axPluginName)."Ctrl";		
	}
	
	function process($project)
	{
      $html = 'file://'.getcwd().'/game/web/sample.html';
      $html = str_replace("\\", "/", $html);
      
      // if the someone is clever enough to change the default location of IE, they are assumed to be 
      // clever enough to edit the debug command :)
      
      $IEExe = "C:\\Program Files\\Internet Explorer\\iexplore.exe";
      if (file_exists("C:\\Program Files (x86)\\Internet Explorer\\iexplore.exe"))
        $IEExe = "C:\\Program Files (x86)\\Internet Explorer\\iexplore.exe";
      
      $project->commandRelease = $IEExe;
      $project->commandDebug = $IEExe;
      $project->commandOptimized = $IEExe;
      
      $project->argsRelease = $html;
      $project->argsDebug = $html;
      $project->argsOptimized = $html;
    
      $this->processTemplates();
	}
	
	function readTemplate($filename)
	{
	    $filename = realpath( dirname( $_SERVER[ 'PHP_SELF' ] ) ). "/templates/web/".$filename;
		$fh = fopen($filename, 'r');
		$data = fread($fh, filesize($filename));
		fclose($fh);
		return $data;
	}
	
	function writeFile($filename, $data)
	{
	    $filename = $this->FOLDER . $filename;
		$fh = fopen($filename, 'w');
		fwrite($fh, $data);
		fclose($fh);
	}

	function writeGameFile($filename, $data)
	{
	    $filename = $this->GAMEFOLDER . $filename;
		$fh = fopen($filename, 'w');
		fwrite($fh, $data);
		fclose($fh);
	}

	function processTemplates()
	{
	    // Make sure these come first so timestamp is oldest (for autogeneration)
		$gen =  $this->readTemplate("activex_IEWebGamePlugin_i_h.tpl");
		$this->writeFile("IEWebGamePlugin_i.h", $gen);
		$gen =  $this->readTemplate("activex_IEWebGamePlugin_i_c.tpl");
		$this->writeFile("IEWebGamePlugin_i.c", $gen);

		// WebGame.idl
		$idl =  $this->readTemplate("activex_IEWebGamePlugin_idl.tpl");
		$idl = str_replace("__WEBGAME_UUID__", $this->WEBGAME_UUID, $idl);
		$idl = str_replace("__IWEBGAMECTRL_UUID__", $this->IWEBGAMECTRL_UUID, $idl);
		$idl = str_replace("__WEBGAMELIB_UUID__", $this->WEBGAMELIB_UUID, $idl);
        $idl = str_replace("__WEBGAME_PLUGINNAME__", $this->WEBGAME_PLUGINNAME, $idl);
		$this->writeFile("IEWebGamePlugin.idl", $idl);

		// dllmain.h
		$dllmain =  $this->readTemplate("activex_dllmain_h.tpl");
        $dllmain = str_replace("__WEBGAME_PLUGINNAME__", $this->WEBGAME_PLUGINNAME, $dllmain);
		$dllmain = str_replace("__REGISTRY_APPID_RESOURCEID__", $this->REGISTRY_APPID_RESOURCEID, $dllmain);
		$this->writeFile("dllmain.h", $dllmain);
		
		//WebGameCtrl.rgs
		$regs =  $this->readTemplate("activex_IEWebGameCtrl_rgs.tpl");
		$regs = str_replace("__WEBGAME_UUID__", $this->WEBGAME_UUID, $regs);
		$regs = str_replace("__WEBGAMELIB_UUID__", $this->WEBGAMELIB_UUID, $regs);
		$regs = str_replace("__WEBGAME_PLUGINNAME__", $this->WEBGAME_PLUGINNAME, $regs);
		$regs = str_replace("__WEBGAME_CTRLNAME__", $this->WEBGAME_CTRLNAME, $regs);
		$this->writeFile("IEWebGameCtrl.rgs", $regs);

		//WebGamePlugin.rgs
		$regs =  $this->readTemplate("activex_IEWebGamePlugin_rgs.tpl");
		$regs = str_replace("__WEBGAME_PLUGINNAME__", $this->WEBGAME_PLUGINNAME, $regs);
		$regs = str_replace("__WEBGAME_CTRLNAME__", $this->WEBGAME_CTRLNAME, $regs);
		$this->writeFile("IEWebGamePlugin.rgs", $regs);

		
		//WebGameCtrl.rc
		$rc =  $this->readTemplate("activex_IEWebGamePlugin_rc.tpl");
        $rc = str_replace("__PLUGIN_DLL__", $this->PLUGIN_DLL, $rc);
        $rc = str_replace("__WEBGAME_PLUGINNAME__", $this->WEBGAME_PLUGINNAME, $rc);
        
        //versioning
        
        $rc = str_replace("__PLUGIN_VERSION_STRING__", $this->PLUGIN_VERSION, $rc);
        $cd = str_replace(".", "," , $this->PLUGIN_VERSION);
        $rc = str_replace("__PLUGIN_VERSION_COMMA_DELIMITED__", $cd, $rc);
        
		$this->writeFile("IEWebGamePlugin.rc", $rc);

        // handle the sample HTML (and whatever else) that is shared between plugins
        WebPlugin::processActiveXPlugin($this);

	}
	
	static function generateUUID()
	{
		$command = realpath( dirname( $_SERVER[ 'PHP_SELF' ] ) )."\\..\\..\\Engine\\bin\\uuidgen\\uuidgen.exe -c";
		exec($command, $output);
		return $output[0];
	}
	
}

?>
