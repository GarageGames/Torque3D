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

//-----------------------------------------------------------------------------
// Support functions used to manage the directory list

function pushFront(%list, %token, %delim)
{
   if (%list !$= "")
      return %token @ %delim @ %list;
   return %token;
}

function pushBack(%list, %token, %delim)
{
   if (%list !$= "")
      return %list @ %delim @ %token;
   return %token;
}

function popFront(%list, %delim)
{
   return nextToken(%list, unused, %delim);
}

//-----------------------------------------------------------------------------
// The default global argument parsing

function defaultParseArgs()
{
   for ($i = 1; $i < $Game::argc ; $i++)
   {
      $arg = $Game::argv[$i];
      $nextArg = $Game::argv[$i+1];
      $hasNextArg = $Game::argc - $i > 1;
      $logModeSpecified = false;

      // Check for dedicated run
      if( stricmp($arg,"-dedicated") == 0  )
      {
         $userDirs = $defaultGame;
         $dirCount = 1;
         $isDedicated = true;
      }

      switch$ ($arg)
      {
         //--------------------
         case "-log":
            $argUsed[$i]++;
            if ($hasNextArg)
            {
               // Turn on console logging
               if ($nextArg != 0)
               {
                  // Dump existing console to logfile first.
                  $nextArg += 4;
               }
               setLogMode($nextArg);
               $logModeSpecified = true;
               $argUsed[$i+1]++;
               $i++;
            }
            else
               error("Error: Missing Command Line argument. Usage: -log <Mode: 0,1,2>");

         //--------------------
         case "-dir":
            $argUsed[$i]++;
            if ($hasNextArg)
            {
               // Append the mod to the end of the current list
               $userDirs = strreplace($userDirs, $nextArg, "");
               $userDirs = pushFront($userDirs, $nextArg, ";");
               $argUsed[$i+1]++;
               $i++;
               $dirCount++;
            }
            else
               error("Error: Missing Command Line argument. Usage: -dir <dir_name>");

         //--------------------
         // changed the default behavior of this command line arg. It now
         // defaults to ONLY loading the game, not tools 
         // default auto-run already loads in tools --SRZ 11/29/07
         case "-game":
            $argUsed[$i]++;
            if ($hasNextArg)
            {
               // Set the selected dir --NOTE: we no longer allow tools with this argument
               /* 
               if( $isDedicated )
               {
                  $userDirs = $nextArg;
                  $dirCount = 1;
               }
               else
               {
                  $userDirs = "tools;" @ $nextArg;
                  $dirCount = 2;
               }
               */
               $userDirs = $nextArg;
               $dirCount = 1;
               $argUsed[$i+1]++;
               $i++;
               error($userDirs);
            }
            else
               error("Error: Missing Command Line argument. Usage: -game <game_name>");

   /* deprecated SRZ 11/29/07
         //--------------------
         case "-show":
            // A useful shortcut for -mod show
            $userMods = strreplace($userMods, "show", "");
            $userMods = pushFront($userMods, "show", ";");
            $argUsed[$i]++;
            $modcount++;
   */
         //--------------------
         case "-console":
            enableWinConsole(true);
            $argUsed[$i]++;

         //--------------------
         case "-jSave":
            $argUsed[$i]++;
            if ($hasNextArg)
            {
               echo("Saving event log to journal: " @ $nextArg);
               saveJournal($nextArg);
               $argUsed[$i+1]++;
               $i++;
            }
            else
               error("Error: Missing Command Line argument. Usage: -jSave <journal_name>");

         //--------------------
         case "-jPlay":
            $argUsed[$i]++;
            if ($hasNextArg)
            {
               playJournal($nextArg,false);
               $argUsed[$i+1]++;
               $i++;
            }
            else
               error("Error: Missing Command Line argument. Usage: -jPlay <journal_name>");
               
         //--------------------
         case "-jPlayToVideo":
            $argUsed[$i]++;
            if ($hasNextArg)
            {
               $VideoCapture::journalName = $nextArg;
               $VideoCapture::captureFromJournal = true;
               $argUsed[$i+1]++;
               $i++;
            }
            else
               error("Error: Missing Command Line argument. Usage: -jPlayToVideo <journal_name>");
               
         //--------------------
         case "-vidCapFile":
            $argUsed[$i]++;
            if ($hasNextArg)
            {
               $VideoCapture::fileName = $nextArg;
               $argUsed[$i+1]++;
               $i++;
            }
            else
               error("Error: Missing Command Line argument. Usage: -vidCapFile <ouput_video_name>");
               
         //--------------------
         case "-vidCapFPS":
            $argUsed[$i]++;
            if ($hasNextArg)
            {
               $VideoCapture::fps = $nextArg;
               $argUsed[$i+1]++;
               $i++;
            }
            else
               error("Error: Missing Command Line argument. Usage: -vidCapFPS <ouput_video_framerate>");
               
         //--------------------
         case "-vidCapEncoder":
            $argUsed[$i]++;
            if ($hasNextArg)
            {
               $VideoCapture::encoder = $nextArg;
               $argUsed[$i+1]++;
               $i++;
            }
            else
               error("Error: Missing Command Line argument. Usage: -vidCapEncoder <ouput_video_encoder>");
               
         //--------------------
         case "-vidCapWidth":
            $argUsed[$i]++;
            if ($hasNextArg)
            {
               $videoCapture::width = $nextArg;
               $argUsed[$i+1]++;
               $i++;
            }
            else
               error("Error: Missing Command Line argument. Usage: -vidCapWidth <ouput_video_width>");
               
         //--------------------
         case "-vidCapHeight":
            $argUsed[$i]++;
            if ($hasNextArg)
            {
               $videoCapture::height = $nextArg;
               $argUsed[$i+1]++;
               $i++;
            }
            else
               error("Error: Missing Command Line argument. Usage: -vidCapHeight <ouput_video_height>");

         //--------------------
         case "-jDebug":
            $argUsed[$i]++;
            if ($hasNextArg)
            {
               playJournal($nextArg,true);
               $argUsed[$i+1]++;
               $i++;
            }
            else
               error("Error: Missing Command Line argument. Usage: -jDebug <journal_name>");

         //--------------------
         case "-level":
            $argUsed[$i]++;
            if ($hasNextArg)
            {
               %hasExt = strpos($nextArg, ".mis");
               if(%hasExt == -1)
               {
                  $levelToLoad = $nextArg @ " ";
                  
                  for(%i = $i + 2; %i < $Game::argc; %i++)
                  {
                     %arg = $Game::argv[%i];
                     %hasExt = strpos(%arg, ".mis");
                     
                     if(%hasExt == -1)
                     {
                        $levelToLoad = $levelToLoad @ %arg @ " ";
                     } else
                     {
                        $levelToLoad = $levelToLoad @ %arg;
                        break;
                     }
                  }
               } else
               $levelToLoad = $nextArg;
               $argUsed[$i+1]++;
               $i++;
            }
            else
               error("Error: Missing Command Line argument. Usage: -level <level file name (no path), with or without extension>");

         //-------------------
         case "-worldeditor":
            $startWorldEditor = true;
            $argUsed[$i]++;

         //-------------------
         case "-guieditor":
            $startGUIEditor = true;
            $argUsed[$i]++;

         //-------------------
         case "-help":
            $displayHelp = true;
            $argUsed[$i]++;

         //-------------------
         case "-compileAll":
            $compileAll = true;
            $argUsed[$i]++;
            
         //-------------------
         case "-compileTools":
            $compileTools = true;
            $argUsed[$i]++;

         //-------------------
         case "-genScript":
            $genScript = true;
            $argUsed[$i]++;
            
         //-------------------
         default:
            $argUsed[$i]++;
            if($userDirs $= "")
               $userDirs = $arg;
      }
   }
   
   //-----------------------------------------------
   // Play journal to video file?
   if ($VideoCapture::captureFromJournal && $VideoCapture::journalName !$= "")
   {         
      if ($VideoCapture::fileName $= "")
         $VideoCapture::fileName = $VideoCapture::journalName;     
      
      if ($VideoCapture::encoder $= "")
         $VideoCapture::encoder = "THEORA";
            
      if ($VideoCapture::fps $= "")
         $VideoCapture::fps = 30;
               
      if ($videoCapture::width $= "")
         $videoCapture::width = 0;
         
      if ($videoCapture::height $= "")
         $videoCapture::height = 0;
         
      playJournalToVideo(  $VideoCapture::journalName, $VideoCapture::fileName, 
                           $VideoCapture::encoder, $VideoCapture::fps, 
                           $videoCapture::width SPC $videoCapture::height );
   }
}
