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

// settings/logic shared by plugins

class WebPlugin
{
   static $SAMPLE_HTML = "";
   static $GAME_FOLDER = "";
   
   static function readTemplate()
   {
      $filename = realpath( dirname( $_SERVER[ 'PHP_SELF' ] ) ). "/templates/web/sample_html.tpl";
      $fh = fopen($filename, 'r');
      $data = fread($fh, filesize($filename));
      fclose($fh);
      
      if (!$data)
        trigger_error( "WebPlugin::readTemplate() - cannot read HTML template!", E_USER_ERROR );
      
      self::$SAMPLE_HTML = $data;
   }
   
   static function processNPPlugin($np)
   {
   
      if (!self::$SAMPLE_HTML)
         self::readTemplate();
         
      $rootPhpBuildDir = getcwd();      
      self::$GAME_FOLDER = $rootPhpBuildDir."/game/";

      self::$SAMPLE_HTML = str_replace("__MIMETYPE__", $np->MIMETYPE, self::$SAMPLE_HTML);
      
   }

   static function processActiveXPlugin($ax)
   {
      if (!self::$SAMPLE_HTML)
        self::readTemplate();   

      $rootPhpBuildDir = getcwd();      
      self::$GAME_FOLDER = $rootPhpBuildDir."/game/";
      
      $projID = $ax->WEBGAME_PLUGINNAME.'.'.$ax->WEBGAME_CTRLNAME.'.'.'1';
      
      self::$SAMPLE_HTML = str_replace("__PROJID__", $projID, self::$SAMPLE_HTML);
      self::$SAMPLE_HTML = str_replace("__CLSID__", $ax->WEBGAMELIB_UUID, self::$SAMPLE_HTML);
      
    }
   
   static function writeSampleHtml()
   {
      // no plugins defined
      if (!self::$SAMPLE_HTML)
         return;
         
      $filename = self::$GAME_FOLDER . "web/sample.html";
      $fh = fopen($filename, 'w');
      fwrite($fh, self::$SAMPLE_HTML);
      fclose($fh);
   }
}
   
?>
