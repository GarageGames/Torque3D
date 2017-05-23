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

// A very simple music player.

//---------------------------------------------------------------------------------------------
// Prerequisites.

if( !isObject( GuiMusicPlayer ) )
   exec( "./guiMusicPlayer.gui" );

//---------------------------------------------------------------------------------------------
// Preferences.

$pref::GuiMusicPlayer::filePattern = "*.ogg\t*.wav";
$pref::GuiMusicPlayer::filePatternFMOD = "*.aiff\t*.asf\t*.flac\t*.it\t*.mid\t*.mod\t*.mp2\t*.mp3\t*.ogg\t*.s3m\t*.vag\t*.wav\t*.wma\t*.xm";
$pref::GuiMusicPlayer::fadeTime = "3.0";

//---------------------------------------------------------------------------------------------
// Datablocks.

singleton SFXDescription( GuiMusicPlayerStream : AudioMusic2D )
{
   volume = 1.0;
   isLooping = false;
   isStreaming = true;
   is3D = false;
};
singleton SFXDescription( GuiMusicPlayerLoopingStream : AudioMusic2D )
{
   volume = 1.0;
   isLooping = true;
   isStreaming = true;
   is3D = false;
};

//---------------------------------------------------------------------------------------------
// Functions.

function toggleMusicPlayer()
{   
   if( !GuiMusicPlayer.isAwake() )
   {
      GuiMusicPlayer.setExtent( Canvas.getExtent() );
      GuiMusicPlayer.setPosition( 0, 0 );
      
      Canvas.pushDialog( GuiMusicPlayer );
   }
   else
      Canvas.popDialog( GuiMusicPlayer );
}

//---------------------------------------------------------------------------------------------
// Methods.

function GuiMusicPlayer_onSFXSourceStatusChange( %id, %status )
{
   if( %status $= "Stopped" )
      GuiMusicPlayer.onStop();
}

function GuiMusicPlayerClass::play( %this )
{
   if( %this.status $= "Stopped"
       || %this.status $= "Paused"
       || %this.status $= "" )
   {
      %isPlaying = true;
      if( %this.status $= "Paused" && isObject( %this.sfxSource ) )
         %this.sfxSource.play();
      else
      {
         %sel = GuiMusicPlayerMusicList.getSelectedItem();
         if( %sel == -1 )
            %isPlaying = false;
         else
         {
            %desc = GuiMusicPlayerStream;
            if( GuiMusicPlayerLoopCheckBox.getValue() )
               %desc = GuiMusicPlayerLoopingStream;
               
            if( GuiMusicPlayerFadeCheckBox.getValue() )
            {
               %desc.fadeInTime = $pref::GuiMusicPlayer::fadeTime;
               %desc.fadeOutTime = $pref::GuiMusicPlayer::fadeTime;
            }
            else
            {
               %desc.fadeInTime = 0;
               %desc.fadeOutTime = 0;
            }
               
            %file = GuiMusicPlayerMusicList.getItemText( %sel );
            %this.sfxSource = sfxPlayOnce( %desc, %file );
            if( !%this.sfxSource )
               %isPlaying = false;
            else
            {
               %this.sfxSource.statusCallback = "GuiMusicPlayer_onSFXSourceStatusChange";
               GuiMusicPlayer.status = "Playing";
               
               GuiMusicPlayerScrubber.setActive( true );
               GuiMusicPlayerScrubber.setup( %this.sfxSource.getDuration() );
            }
         }
      }
      
      if( %isPlaying )
      {
         GuiMusicPlayerPlayButton.setText( "Pause" );
         GuiMusicPlayerPlayButton.command = "GuiMusicPlayer.pause();";
         GuiMusicPlayerLoopCheckBox.setActive( false );
         GuiMusicPlayerFadeCheckBox.setActive( false );
         %this.status = "Playing";
      }
   }
}

function GuiMusicPlayerClass::stop( %this )
{
   if( %this.status $= "Playing"
       || %this.status $= "Paused" )
   {
      if( isObject( %this.sfxSource ) )
         %this.sfxSource.stop( 0 ); // Stop immediately.
   }
}

function GuiMusicPlayerClass::onStop( %this )
{
   %this.sfxSource = 0;

   GuiMusicPlayerLoopCheckBox.setActive( true );
   GuiMusicPlayerFadeCheckBox.setActive( true );
   GuiMusicPlayerScrubber.setActive( false );
   GuiMusicPlayerPlayButton.setText( "Play" );
   GuiMusicPlayerPlayButton.Command = "GuiMusicPlayer.play();";
   %this.status = "Stopped";
   
   GuiMusicPlayerScrubber.setValue( 0 );   
}

function GuiMusicPlayerClass::pause( %this )
{
   if( %this.status $= "Playing" )
   {
      if( isObject( %this.sfxSource ) )
         %this.sfxSource.pause( 0 );
         
      GuiMusicPlayerPlayButton.setText( "Play" );
      GuiMusicPlayerPlayButton.command = "GuiMusicPlayer.play();";
      %this.status = "Paused";
   }
}

function GuiMusicPlayerClass::seek( %this, %playtime )
{
   if( ( %this.status $= "Playing"
         || %this.status $= "Paused" )
       && isObject( %this.sfxSource ) )
      %this.sfxSource.setPosition( %playtime );
}

function GuiMusicPlayer::onWake( %this )
{
   GuiMusicPlayerMusicList.load();
}

function GuiMusicPlayerMusicListClass::load( %this )
{
   // Remove all the files currently in the list.
   
   %this.clearItems();
   
   // Find the file matching pattern we should use.
   
   %filePattern = $pref::GuiMusicPlayer::filePattern;
   %sfxProvider = getWord( sfxGetDeviceInfo(), 0 );
   %filePatternVarName = "$pref::GuiMusicPlayer::filePattern" @ %sfxProvider;
   if( isDefined( %filePatternVarName ) )
      eval( "%filePattern = " @ %filePatternVarName @ ";" );
      
   // Find all files matching the pattern.
      
   for( %file = findFirstFileMultiExpr( %filePattern );
        %file !$= "";
        %file = findNextFileMultiExpr( %filePattern ) )
      %this.addItem( makeRelativePath( %file, getMainDotCsDir() ) );
}

function GuiMusicPlayerMusicList::onDoubleClick( %this )
{
   GuiMusicPlayer.stop();
   GuiMusicPlayer.play();
}

function GuiMusicPlayerScrubber::onMouseDragged( %this )
{
   %this.isBeingDragged = true;
}

function GuiMusicPlayerScrubberClass::setup( %this, %totalPlaytime )
{
   %this.range = "0 " @ %totalPlaytime;
   %this.ticks = %totalPlaytime / 5; // One tick per five seconds.
   
   %this.update();
}

function GuiMusicPlayerScrubberClass::update( %this )
{   
   if( GuiMusicPlayer.status $= "Playing"
       && !%this.isBeingDragged )
      %this.setValue( GuiMusicPlayer.sfxSource.getPosition() );

   if( GuiMusicPlayer.status $= "Playing"
       || GuiMusicPlayer.status $= "Paused" )
      %this.schedule( 5, "update" );
}

function GuiMusicPlayerScrubberClass::onDragComplete( %this )
{
   GuiMusicPlayer.seek( %this.getValue() );
   %this.isBeingDragged = false;
}
