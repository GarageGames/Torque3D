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

class NPWebPlugin
{
   var $FOLDER = "";
   var $GAMEFOLDER = "";
   var $PLUGINNAME = "";
   
   var $COMPANY = "";
   var $COMPANYKEY = "";
   var $PLUGIN = "";
   var $PRODUCTNAME = "";
   var $VERSION = "";
   var $MIMETYPE = "";
   var $DESCRIPTION = "";
   
   function NPWebPlugin()
   {
      $rootPhpBuildDir = getcwd();
      
      $this->FOLDER = $rootPhpBuildDir."\\web\\source\\npplugin\\windows\\";
      $this->GAMEFOLDER = $rootPhpBuildDir."\\game\\";
      
      $this->PLUGINNAME = WebDeploymentWindows::$npPluginName;
      
      $this->COMPANY = WebDeploymentWindows::$company;
      $this->COMPANYKEY = WebDeploymentWindows::$companyKey;
      $this->PLUGIN = WebDeploymentWindows::$plugin;
      $this->PRODUCTNAME = WebDeploymentWindows::$productName;
      $this->VERSION = WebDeploymentWindows::$version;
      $this->MIMETYPE = WebDeploymentWindows::$mimeType;
      $this->DESCRIPTION = WebDeploymentWindows::$description." ".$this->VERSION;      
   }
   
   function process($project)
   {
      $windowsRegistry = new WindowsRegistry();

      $MozillaVersion = $windowsRegistry->ReadValue('HKEY_LOCAL_MACHINE\\SOFTWARE\\Mozilla\\Mozilla Firefox', 'CurrentVersion', TRUE);
      $MozillaExe = $windowsRegistry->ReadValue('HKEY_LOCAL_MACHINE\\SOFTWARE\\Mozilla\\Mozilla Firefox\\'.$MozillaVersion.'\\Main', 'PathToExe', TRUE);

      // FireFox registry settings
      $MozillaKey = 'HKEY_LOCAL_MACHINE\\SOFTWARE\\MozillaPlugins\\@';
      
      $html = 'file://'.getcwd().'/game/web/sample.html';
      $html = str_replace("\\", "/", $html);
      $html = str_replace(" ", "%20", $html);
      
      $project->commandRelease = $MozillaExe;
      $project->commandDebug = $MozillaExe;
      $project->commandOptimized = $MozillaExe;
      
      $project->argsRelease = $html;
      $project->argsDebug = $html;
      $project->argsOptimized = $html;

      //$windowsRegistry->WriteValue($MozillaKey.$this->COMPANYKEY.'/'.$this->PLUGIN, 'Path', $this->GAMEFOLDER.$this->PLUGINNAME.'.dll');
      //$windowsRegistry->WriteValue($MozillaKey.$this->COMPANYKEY.'/'.$this->PLUGIN, 'ProductName', $this->PRODUCTNAME);
      //$windowsRegistry->WriteValue($MozillaKey.$this->COMPANYKEY.'/'.$this->PLUGIN, 'Vendor', $this->COMPANY);
      //$windowsRegistry->WriteValue($MozillaKey.$this->COMPANYKEY.'/'.$this->PLUGIN, 'Version', $this->VERSION);
      //$windowsRegistry->WriteValue($MozillaKey.$this->COMPANYKEY.'/'.$this->PLUGIN.'\\MimeTypes\\application/'.$this->MIMETYPE, 'Description', $this->DESCRIPTION);

      unset($windowsRegistry);
      
      $this->processTemplates();
      
      setProjectModuleDefinitionFile('../../../web/source/npplugin/windows/npWebGamePlugin.def');
      
      /// Prefs
      addProjectDefine( 'TORQUE_SHADERGEN' );
      addProjectDefine( 'TORQUE_SHARED' );
      addProjectDefine( 'TORQUE_WEBDEPLOY' );

      // Firefox	
      addProjectDefine( 'FIREFOXPLUGIN_EXPORTS' );
      addProjectDefine( 'MOZILLA_STRICT_API' );
      addProjectDefine( 'XP_WIN' );
      
      addProjectDefine( 'WIN32' );
      addProjectDefine( '_WINDOWS' );
      addProjectDefine( '_USRDLL' );
      
      addSrcDir( '../web/source/common' );
      addSrcDir( '../web/source/npplugin' );
      addSrcDir( '../web/source/npplugin/windows' );
            
      // Additional includes
      addIncludePath( "../../web/source/npplugin/windows" );
      
      addProjectLibDir( getAppLibSrcDir() . 'SDL/win32' );
      addProjectLibDir( getAppLibSrcDir() . 'unicode' );
      // addProjectLibDir( getAppLibSrcDir() . 'mozilla/lib' );
      
      addProjectLibInput('COMCTL32.LIB');
      addProjectLibInput('COMDLG32.LIB');
      addProjectLibInput('USER32.LIB');
      addProjectLibInput('ADVAPI32.LIB');
      addProjectLibInput('GDI32.LIB');
      addProjectLibInput('WINMM.LIB');
      addProjectLibInput('WSOCK32.LIB');
      addProjectLibInput('vfw32.lib');
      addProjectLibInput('Imm32.lib');
      addProjectLibInput('UnicoWS.lib');
      addProjectLibInput('opengl32.lib');
      addProjectLibInput('glu32.lib');
      addProjectLibInput('ole32.lib');
      addProjectLibInput('shell32.lib');
      addProjectLibInput('oleaut32.lib');
      addProjectLibInput('version.lib');

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
   
      $data =  $this->readTemplate("firefox_rc.tpl");
      $cd = str_replace(".", "," , $this->VERSION);
      $data = str_replace("__PRODUCTVERSION__", $cd, $data);
      $data = str_replace("__FILEDESCRIPTION__", $this->DESCRIPTION, $data);
      $data = str_replace("__INTERNALNAME__", $this->PRODUCTNAME, $data);
      $data = str_replace("__MIMETYPE__", "application/".$this->MIMETYPE, $data);
      $data = str_replace("__ORIGINALFILENAME__", $this->PLUGINNAME.'.dll', $data);
      $data = str_replace("__PRODUCTNAME__", $this->PRODUCTNAME, $data);
      $data = str_replace("__COMPANY__", $this->COMPANY, $data);
      $data = str_replace("__COMPANYKEY__", $this->COMPANYKEY, $data);
      $data = str_replace("__PLUGIN__", $this->PLUGIN, $data);      
      $this->writeFile('NPWebGamePlugin.rc', $data);
      
      // handle the sample HTML (and whatever else) that is shared between plugins
      WebPlugin::processNPPlugin($this);
      
   }
   
}

?>
