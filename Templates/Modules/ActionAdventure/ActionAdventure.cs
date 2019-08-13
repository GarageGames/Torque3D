//Action Adventure game template

function ActionAdventure::create( %this )
{
   //server scripts
   exec("./scripts/server/aiPlayer.cs");
   exec("./scripts/server/camera.cs");
   exec("./scripts/server/commands.cs");
   //exec("./scripts/server/centerPrint.cs");
   exec("./scripts/server/ActionAdventureGame.cs");
   exec("./scripts/server/player.cs");
   exec("./scripts/server/shapeBase.cs");
   exec("./scripts/server/spawn.cs");
   exec("./scripts/server/triggers.cs");
   
   //add DBs
   if(isObject(DatablockFilesList))
   {
      for( %file = findFirstFile( "data/ActionAdventure/scripts/datablocks/*.cs.dso" );
      %file !$= "";
      %file = findNextFile( "data/ActionAdventure/scripts/datablocks/*.cs.dso" ))
      {
         // Only execute, if we don't have the source file.
         %csFileName = getSubStr( %file, 0, strlen( %file ) - 4 );
         if( !isFile( %csFileName ) )
            DatablockFilesList.add(%csFileName);
      }
      
      // Load all source material files.
      for( %file = findFirstFile( "data/ActionAdventure/scripts/datablocks/*.cs" );
      %file !$= "";
      %file = findNextFile( "data/ActionAdventure/scripts/datablocks/*.cs" ))
      {
         DatablockFilesList.add(%file);
      }
   }
   
   if(isObject(LevelFilesList))
   {
      for( %file = findFirstFile( "data/ActionAdventure/levels/*.mis" );
      %file !$= "";
      %file = findNextFile( "data/ActionAdventure/levels/*.mis" ))
      {
         LevelFilesList.add(%file);
      }
   }
   
   if (!$Server::Dedicated)
   {
      //exec("data/ActionAdventure/scripts/client/gameProfiles.cs");
      
      //client scripts
      $KeybindPath = "data/ActionAdventure/scripts/client/default.keybinds.cs";
      exec($KeybindPath);
      
      %prefPath = getPrefpath();
      if(isFile(%prefPath @ "/keybinds.cs"))
         exec(%prefPath @ "/keybinds.cs");
         
      exec("data/ActionAdventure/scripts/client/inputCommands.cs");
      
      //guis
      exec("./scripts/gui/playGui.gui");
      
      exec("data/ActionAdventure/scripts/client/playGui.cs");

      exec("data/ActionAdventure/scripts/client/clientCommands.cs");
      exec("data/ActionAdventure/scripts/client/centerPrint.cs");
      exec("data/ActionAdventure/scripts/client/recordings.cs");
      
      exec("data/ActionAdventure/scripts/client/screenshot.cs");
   }
}

function ActionAdventure::destroy( %this )
{
   
}