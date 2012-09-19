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

class SafariWebPlugin
{
   var $FOLDER = "";
   var $GAMEFOLDER = "";
   var $PLUGINNAME = "";
   var $MIMETYPE = "";
   var $DESCRIPTION = "";
   var $IDENTIFIER = "";
   var $BUNDLE = "";
   var $VERSION = "";
   
   function SafariWebPlugin()
   {
      $rootPhpBuildDir = getcwd();

      $this->FOLDER = $rootPhpBuildDir."/web/source/npplugin/mac/";
      $this->GAMEFOLDER = $rootPhpBuildDir."/game/";
      
      $this->PLUGINNAME = WebDeploymentOSX::$safariPluginName;
      $this->MIMETYPE = WebDeploymentOSX::$mimeType;
      
      $this->VERSION = WebDeploymentOSX::$version;

      $this->DESCRIPTION = WebDeploymentOSX::$description." ".$this->VERSION ;      

      $this->BUNDLE = WebDeploymentOSX::$safariPluginName;
      $this->IDENTIFIER = WebDeploymentOSX::$identifier;
   }
   
   function process($project)
   {
      
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

      echo($filename);
      
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

      $data =  $this->readTemplate("safari_Info_plist.tpl");
      $data = str_replace("__CFBundleExecutable__", $this->BUNDLE, $data);
      $data = str_replace("__CFBundleIdentifier__", $this->IDENTIFIER, $data);
      $data = str_replace("__WebPluginDescription__", $this->DESCRIPTION, $data);
      $data = str_replace("__WebPluginMIMEType__", "application/".$this->MIMETYPE, $data);
      $data = str_replace("__WebPluginName__", $this->PLUGINNAME, $data);      

      $data = str_replace("__GameName__", getGameProjectName(), $data);

      //TODO: handled by Toolbox
      $data = str_replace("__GameInstallPath__", $this->GAMEFOLDER, $data);

      $this->writeFile('Info.plist', $data);
      
      WebPlugin::processNPPlugin($this);
   }
   
}

?>
