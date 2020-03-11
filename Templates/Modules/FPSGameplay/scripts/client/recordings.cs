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
// RecordingsGui is the main TSControl through which the a demo game recording
// is viewed. 
//-----------------------------------------------------------------------------

function recordingsDlg::onWake()
{
   RecordingsDlgList.clear();
   %i = 0;
   %filespec = $currentMod @ "/recordings/*.rec";
   echo(%filespec);
   for(%file = findFirstFile(%filespec); %file !$= ""; %file = findNextFile(%filespec)) 
      RecordingsDlgList.addRow(%i++, fileBase(%file));
   RecordingsDlgList.sort(0);
   RecordingsDlgList.setSelectedRow(0);
   RecordingsDlgList.scrollVisible(0);
}

function StartSelectedDemo()
{
   // first unit is filename
   %sel = RecordingsDlgList.getSelectedId();
   %rowText = RecordingsDlgList.getRowTextById(%sel);

   %file = $currentMod @ "/recordings/" @ getField(%rowText, 0) @ ".rec";

   new GameConnection(ServerConnection);
   RootGroup.add(ServerConnection);

   // Start up important client-side stuff, such as the group
   // for particle emitters.  This doesn't get launched during a demo
   // as we short circuit the whole mission loading sequence.
   clientStartMission();

   if(ServerConnection.playDemo(%file))
   {
      Canvas.setContent(PlayGui);
      Canvas.popDialog(RecordingsDlg);
      ServerConnection.prepDemoPlayback();
   }
   else 
   {
      MessageBoxOK("Playback Failed", "Demo playback failed for file '" @ %file @ "'.");
      if (isObject(ServerConnection)) {
         ServerConnection.delete();
      }
   }
}

function startDemoRecord()
{
   // make sure that current recording stream is stopped
   ServerConnection.stopRecording();
   
   // make sure we aren't playing a demo
   if(ServerConnection.isDemoPlaying())
      return;
   
   for(%i = 0; %i < 1000; %i++)
   {
      %num = %i;
      if(%num < 10)
         %num = "0" @ %num;
      if(%num < 100)
         %num = "0" @ %num;

      %file = $currentMod @ "/recordings/demo" @ %num @ ".rec";
      if(!isfile(%file))
         break;
   }
   if(%i == 1000)
      return;

   $DemoFileName = %file;

   ChatHud.AddLine( "\c4Recording to file [\c2" @ $DemoFileName @ "\cr].");

   ServerConnection.startRecording($DemoFileName);

   // make sure start worked
   if(!ServerConnection.isDemoRecording())
   {
      deleteFile($DemoFileName);
      ChatHud.AddLine( "\c3 *** Failed to record to file [\c2" @ $DemoFileName @ "\cr].");
      $DemoFileName = "";
   }
}

function stopDemoRecord()
{
   // make sure we are recording
   if(ServerConnection.isDemoRecording())
   {
      ChatHud.AddLine( "\c4Recording file [\c2" @ $DemoFileName @ "\cr] finished.");
      ServerConnection.stopRecording();
   }
}

function demoPlaybackComplete()
{
   disconnect();

   // Clean up important client-side stuff, such as the group
   // for particle emitters and the decal manager.  This doesn't get 
   // launched during a demo as we short circuit the whole mission 
   // handling functionality.
   clientEndMission();

   if (isObject( MainMenuGui ))
      Canvas.setContent( MainMenuGui );

   Canvas.pushDialog(RecordingsDlg);
}

function deleteDemoRecord()
{
   %sel = RecordingsDlgList.getSelectedId();
   %rowText = RecordingsDlgList.getRowTextById(%sel);
   %file = $currentMod @ "/recordings/" @ getField(%rowText, 0) @ ".rec";
   
   if(!isfile(%file))
   {
      RecordingsDlgList.removeRowById(%sel);
      return;
   }
   
   RecordingsDlgList.removeRowById(%sel);
   fileDelete(%file);
}
