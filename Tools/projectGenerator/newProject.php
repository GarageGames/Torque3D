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

// Yes, I still need to comment most of the stuff in here. I tried to do a quick port over of the stuff from the bat and
// skipped laying it out
function copyDir( $source, $target, $exclude = array())
{
   if ( is_dir( $source ) )
   {
      @mkdir( $target, 0777, true );

      $dirRef = dir( $source );

      while ( FALSE !== ( $entry = $dirRef->read() ) )
      {
         // Read through exclude list. If the current directory is on it, skip it
         $skip = false;
         foreach($exclude as $exclFile)
         {
            if($entry == $exclFile)
            { 
               $skip = true;
               continue;
            }
         }

         if ( $entry == '.' || $entry == '..' || $skip )
            continue;

         $entryFullPath = $source . '/' . $entry;           
         if ( is_dir( $entryFullPath ) )
         {
            copyDir( $entryFullPath, $target . '/' . $entry, $exclude );
            continue;
         }

         copy( $entryFullPath, $target . '/' . $entry );
         echo("Copying " . $entry . " to " . $target . "/\n");
      }

      $dirRef->close();
   }
   else
      copy( $source, $target );
}

// Function taken from PHP.net
function rm_recursive($filepath)
{
   if (is_dir($filepath) && !is_link($filepath))
   {
      if ($dirRef = opendir($filepath))
      {
         while (($curfile = readdir($dirRef)) !== false)
         {
            if ($curfile == '.' || $curfile == '..')
                continue;
 
            if (!rm_recursive($filepath.'/'.$curfile))
               throw new Exception($filepath.'/'.$curfile.' could not be deleted.');
         }

         closedir($dirRef);
      }

      return rmdir($filepath);
   }

   return unlink($filepath);
}

// Replaces all instances of the given text string from the given file
function ReplaceTextInFile($file, $string, $replace)
{
   $buf = "";
   $confg = file($file);

   foreach ($confg as $line)
   {
      $line = str_replace($string, $replace, $line);
      $buf .= $line;
   }

   $confg = fopen($file, "w"); 
   fwrite($confg, $buf); 
   fclose($confg);
}

// Flag that notes whether a simple (non-fatal) error has occurred
$withError = false;

// Minimum number of passed arguments is 3
if($argc < 4)
   exit("\nError! Invalid number of arguments! Correct format is Project_Name Torque_Root Project_Destination_Path (optional -O overwrite existing files)\n");
// If there are 5 args and the last one isn't the override flag, it's a fatal error
else if($argc == 5 && $argv[4] != "-O")
   exit("\nInvalid argument of $argv[4]! Should be -O (optional overwrite existing files flag)\n");

// Define our main working paths
$torqueDir = $argv[2];
$newPath = $argv[3] . "/".$argv[1] . "/";
$templPath = $torqueDir . "/Templates/Full/";

// Aesthetics...php doesn't care
$newPath = str_replace("//", "/", $newPath);
$templPath = str_replace("//", "/", $templPath);

// Check if template path exists (ie user didn't do something wrong)
if(is_dir($templPath))
{
   // If the destination path doesn't exist, make it
   if(!is_dir($argv[3]))
      mkdir($argv[3], 0777, true);

   // If the project folder exists, check for -O
   if(is_dir($newPath))
   {
      //If there's no -O, we exit out
      if($argv[4] != "-O")
         exit("\nProject folder already exists! Use -O as 4th argument is you wish to overwrite");

      // Otherwise we delete the other .exe and .torsion as they won't otherwise be overwritten
      else
      {
         if(!unlink($newPath . "game/" . $argv[1] . ".exe") ||
            !unlink($newPath . "game/" . $argv[1] . ".torsion"))
         {
            echo("\nError deleting old files " . $argv[1] . ".exe and " . $argv[1] . ".torsion\n");
            $withError = true;
         }
      }
   }

   // Read directory exclusion list from file (most likely provided by GUI prompt)
   $exclude = file($templPath . "exclude.txt", FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);

   // By default exclude .svn, config, and buildFiles
   if($exclude == false)
   {
      $exclude = array(".svn", "Link");
   }

   else
   {
      $exclude[] = ".svn";
      $exclude[] = "Link";
   }

   copyDir($templPath, $newPath, $exclude);
}

// If the given template directory does not exist, we exit
else{
   exit("\nTemplate directory does not exist at $templPath!");
}

//Replace references to template
ReplaceTextInFile($newPath . "buildFiles/config/project.conf", "Full", $argv[1]);
ReplaceTextInFile($newPath . "buildFiles/config/project.360.conf", "Full", $argv[1]);
ReplaceTextInFile($newPath . "buildFiles/config/project.mac.conf", "Full", $argv[1]);
ReplaceTextInFile($newPath . "game/Template.torsion", "Full", $argv[1]);
ReplaceTextInFile($newPath . "game/main.cs", "Full", $argv[1]);
ReplaceTextInFile($newPath . "source/torqueConfig.h", "Full", $argv[1]);

//Rename the executable and the torsion project file
if(!rename($newPath . "game/Full.exe", $newPath . "game/" . $argv[1] . ".exe"))
{ 
   echo("\n\nCould not rename Full.exe! You may need to rename manually\n");
   $withError = true;
}

if(!rename($newPath . "game/Full.torsion", $newPath . "game/" . $argv[1] . ".torsion"))
{ 
   echo("\n\nCould not rename Full.torsion! You may need to rename manually\n");
   $withError = true;
}

chdir($newPath);

// Generate the projects and solutions using the generateProjects.bat
// We don't hand in the $torqueDir if our output path is in the Torque folders
// Probably should make this smarter so that it can handle directories that
// are in the the Torque folder but are not at the same level as GameExamples and Projects
if (stristr($newPath, $torqueDir))
   passthru("generateProjects.bat noPause");
else
   passthru("generateProjects.bat noPause $torqueDir");

// If there wasn't an error, print happy message
if(!$withError)
   echo("\nProject creation complete!\nYou can find your new project at $newPath.");
// If there was, print ultra mega sad message
else
   echo("\nProject creation completed, however there were some errors. Please verify that your new project is correct.\nYou can find your new project at $newPath.");

?>