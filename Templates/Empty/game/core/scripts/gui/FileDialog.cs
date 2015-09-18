exec("./FileDialog.gui");

function PlatformFileDialog::buildFilters(%this)
{
   %str = strreplace( %this.data.filters, "|", "\t");
   %this.filterCount = getFieldCount( %str ) / 2;
   //echo( "Filter count: " @  %str );
   for( %i = 0; %i < %this.filterCount; %i++ )
   {
      %this.filterName[%i] = GetField( %str, (%i*2) + 0 );
      %this.filter[%i] = strreplace( GetField( %str, (%i*2) + 1 ), ";", "\t");
      //echo( "Filter: " @  %this.filterName[%i] @ " - " @ %this.filter[%i]);
   }   
}

function PlatformFileDialog::handleFlags(%this, %flags)
{
   %this.FDS_OPEN = false;
   %this.FDS_SAVE = false;
   %this.FDS_OVERWRITEPROMPT = false;
   %this.FDS_MUSTEXIST = false;
   %this.FDS_BROWSEFOLDER = false;
   
   %flagCount = getFieldCount( %flags );
   
   //echo( "flag count: " @ %flagCount );   
   
   for( %i = 0; %i < %flagCount; %i++ )
   {
      %flag = GetField( %flags, %i );
      //echo(%flag);
      if( %flag $= "FDS_OPEN" )
      {
         %this.FDS_OPEN = true;
         %this-->Button.setText( "OPEN" );
         %this-->Button.command = "PlatformFileDialog.tryFile();";
         %this-->window.text = "Select file to OPEN";
      }
      else if( %flag $= "FDS_SAVE" )
      {
         %this.FDS_SAVE = true;
         %this-->Button.setText( "SAVE" );
         %this-->Button.command = "PlatformFileDialog.tryFile();";
         %this-->window.text = "Select file to Save";
      }
      else if( %flag $= "FDS_OVERWRITEPROMPT" )
      {
         %this.FDS_OVERWRITEPROMPT = true;
      }
      else if( %flag $= "FDS_MUSTEXIST" )
      {
         %this.FDS_MUSTEXIST = true;
      }
      else if( %flag $= "FDS_BROWSEFOLDER" )
      {
         %this.FDS_BROWSEFOLDER = true;
         %this-->window.text = "Select folder to OPEN";
      }
   }   
}

function OpenPlatformFileDialog(%data, %flags)
{
   PlatformFileDialog.searchDir = "";
   PlatformFileDialog-->fileNameEdit.setText( "" );
   PlatformFileDialog.data = %data;
   PlatformFileDialog.data.finished = 0;
   
   PlatformFileDialog.handleFlags( %flags );
   
   if( !isObject(PlatformFileDialog.freeItemSet) )
   {
      PlatformFileDialog.freeItemSet = new SimGroup();
   }
   
   PlatformFileDialog.buildFilters();
   
   Canvas.pushDialog(PlatformFileDialog);
}

function PlatformFileDialog::changeDir( %this, %newDir )
{     
   %this.searchDir = %newDir;
   %this.update();
}

function PlatformFileDialog::navigateUp( %this )
{
   //echo( "PlatformFileDialog::navigateUp " @ %this.searchDir );
   if( %this.searchDir !$= "" )
   {      
      %str = strreplace( %this.searchDir, "/", "\t");
      %count = getFieldCount( %str );
      
      if ( %count == 0 )
         return;
         
      if ( %count == 1 )
         %address = "";
      else      
         %address = getFields( %str, 0, %count - 2 );
         
      %newDir = strreplace( %address, "\t", "/" );
      
      if( %newDir !$= "" )
         %newDir = %newDir @ "/";
      
      %this.changeDir( %newDir );
      
   }
}

function PlatformFileDialog::cancel( %this )
{
   %this.data.files[0] = "";
   %this.data.fileCount = 0;
   %this.data.finished = 1;
   
   Canvas.popDialog(%this);
}

function FileDialogItem::onClick( %this )
{
   PlatformFileDialog-->fileNameEdit.setText( "" );
   
   if( %this.isDir && %this.FDS_BROWSEFOLDER)
   {
      PlatformFileDialog-->fileNameEdit.setText( %this.text );
   }
   else if( !%this.isDir && !%this.FDS_BROWSEFOLDER )
   {
      PlatformFileDialog-->fileNameEdit.setText( %this.text );
   }
}

function FileDialogItem::onDoubleClick( %this )
{
   PlatformFileDialog-->fileNameEdit.setText( "" );
   
   if( %this.isDir )
   {
      PlatformFileDialog.changeDir( PlatformFileDialog.searchDir @ %this.text @ "/" );
   }
}

function PlatformFileDialog::tryFile( %this )
{
   %file = %this-->fileNameEdit.getText();
   if( %file $= "" )
      return;
      
   if( %this.FDS_OVERWRITEPROMPT )
   {
      %callback = "PlatformFileDialog.onFile( \"" @ %file @ "\" );";
      MessageBoxOKCancel("Confirm overwrite", "Confirm overwrite", %callback, "");
      return;
   }
   
   %this.onFile( %file );
}

function PlatformFileDialog::onFile( %this, %file )
{
   %this.data.files[0] = "";
   %this.data.fileCount = 0;   
   
   if( %file !$= "" )
   {
      %file = %this.searchDir @ %file;
      %this.data.fileCount = 1;
   }
   
   if( %this.FDS_BROWSEFOLDER && !isDirectory( %file ) )
   {
      echo("Select a directory");
      return;
   }
   else if( !%this.FDS_BROWSEFOLDER && !isFile( %file ) )
   {
      echo("Select a file");
      return;
   }
   
   if( %this.FDS_MUSTEXIST )
   {
      if( !isFile( %file ) && !isDirectory( %file ) )
      {
         echo("Target must exist: " @ %file );
         return;
      }
   }  
   
   %this.data.finished = 1;
   %this.data.files[0] = %file;
   
   Canvas.popDialog(%this);
   
   %this-->fileNameEdit.setText( "" );
}

function PlatformFileDialog::clear( %this )
{
   %itemArray = %this-->itemArray;
   
   while( %itemArray.getCount() )
   {
      %item = %itemArray.getObject( 0 );
      %this.freeItem( %item );      
   }
}

function PlatformFileDialog::getNewItem( %this )
{
   if( %this.freeItemSet.getCount() )
      %item = %this.freeItemSet.getObject( 0 );
   
   if( isObject(%item) )
   {
      %this.freeItemSet.remove( %item );
   }
   else
   {
      //create new
      %item = new GuiIconButtonCtrl();
      %item.className = "FileDialogItem";
      %item.profile = "ToolsGuiIconButtonProfile";
      %item.textLocation = "left";
      %item.iconLocation = "left";
      %item.iconBitmap = "";
      %item.text = "";
   }  
   
   return %item;
}

function PlatformFileDialog::freeItem( %this, %item )
{
   %this-->itemArray.remove( %item );
   
   //clear
   %item.setText( "" );
   %item.iconBitmap = "";
   %item.textMargin = 0;
   %item.textLocation = "left";
   %item.iconLocation = "left";
   %item.resetState();
   
   PlatformFileDialog.freeItemSet.add( %item );
}

function PlatformFileDialog::addDir( %this, %dir )
{
   //echo( "Dir: " @ %dir );
   %item = %this.getNewItem();
   %item.setText( %dir );
   %item.isDir = true;
   %item.iconBitmap = "core/art/gui/images/folder";
   %item.textLocation = "left";
   %item.iconLocation = "left";
   %item.textMargin = 24;
   %this-->itemArray.add( %item );
}

function PlatformFileDialog::addFile( %this, %file )
{
   //echo( "File: " @ %file );
   %item = %this.getNewItem();
   %item.text = strreplace( %file, %this.searchDir, "" );
   %item.isDir = false;
   %this-->itemArray.add( %item );
}

function PlatformFileDialog::onWake( %this )
{
   %this.update();
}

function PlatformFileDialog::onSleep( %this )
{
   %this.data.finished = 1;
}

function PlatformFileDialog::update( %this )
{
   %this.clear();
   
   %this-->popUpMenu.text = %this.searchDir;

   // dirs
   %dirList = getDirectoryList( %this.searchDir, 0 );
   %wordCount = getFieldCount( %dirList );
   for( %i = 0; %i < %wordCount; %i++ )
   {
      %dirItem = GetField( %dirList, %i );
      %this.addDir( %dirItem );
   }
   
   //files
   %pattern = %this.filter[0];
   //echo( %pattern );
   %file = findFirstFileMultiExpr( %this.searchDir @ %pattern, false);
   
   while( %file !$= "" )
   {      
      %this.addFile( %file );
      %file = findNextFileMultiExpr( %pattern );
   }
}