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

// RSS Feed integration structures
// I apologize in advance if this RSS reader is too restrictive with regard
// to tags/enclosures. I may revisit it at some point to add support

//------------------------------------------------------------------------------
// RSS Headline Item
//------------------------------------------------------------------------------
function constructRSSHeadline( %headline, %link )
{
   %ret = new ScriptObject()
   {
      class = "RSSHeadline";
      _headline = %headline;
      _link = %link;
   };
   
   return %ret;
}

function RSSHeadline::getHeadline( %this )
{
   return %this._headline;
}

function RSSHeadline::getLink( %this )
{
   return %this._link;
}

function RSSHeadline::sameAs( %this, %headline )
{
   return ( strcmp( %this.toString(), %headline.toString() ) == 0 );
}

function RSSHeadline::toString( %this )
{
   return %this.getHeadline() @ " ( " @ %this.getLink() @ " ) ";
}

//------------------------------------------------------------------------------

function constructRSSHeadlineCollection()
{
   %ret = new ScriptObject()
   {
      class = "RSSHeadlineCollection";
   };
   
   // Create sim group for it
   %ret._simGroup = new SimGroup();
   
   return %ret;
}

function RSSHeadlineCollection::getObject( %this, %index )
{
   %ret = %this._simGroup.getObject( %index );
   
   if( !isObject( %ret ) )
   {
      warn( "No such index in headline collection." );
      return -1;
   }
   
   return %ret;
}

function RSSHeadlineCollection::getCount( %this )
{
   return %this._simGroup.getCount();
}

function RSSHeadlineCollection::addHeadline( %this, %headline, %skipReorder )
{
   for( %i = 0; %i < %this.getCount(); %i++ )
   {
      %obj = %this.getObject( %i );
         
      if( %obj.sameAs( %headline ) )
      {
         //echo( "cache hit headline: " @ %headline.toString() );
         return false;
      }
   }
   
   %this._simGroup.add( %headline );
   
   if( !%skipReorder )
      %this._simGroup.bringToFront( %headline );
      
   //echo( "adding headline: " @ %headline.toString() );

   return true;
}

function RSSHeadlineCollection::writeToFile( %this, %file )
{
   $rssHeadlineCollection::count = %this.getCount();
   
   for( %i = 0; %i < %this.getCount(); %i++ )
   {
      %hdl = %this.getObject( %i );
      $rssHeadlineCollection::headline[%i] = %hdl.getHeadline();
      $rssHeadlineCollection::link[%i] = %hdl.getLink();
   }
   
   export( "$rssHeadlineCollection::*", %file, false );
}

function RSSHeadlineCollection::loadFromFile( %this, %file )
{
   %this._simGroup.clear();
   
   $rssHeadlineCollection::count = 0;
   
   %file = getPrefsPath(%file);
   if (isFile(%file) || isFile(%file @ ".dso"))
      exec( %file );

   for( %i = 0; %i < $rssHeadlineCollection::count; %i++ )
   {
      //echo( "[LD: " @ %i @ "] Headline: " @ $rssHeadlineCollection::headline[%i] );
      //echo( "[LD: " @ %i @ "] Link: " @ $rssHeadlineCollection::link[%i] );
      
      %hdl = constructRSSHeadline( $rssHeadlineCollection::headline[%i],
                                   $rssHeadlineCollection::link[%i] );
                                   
      // This does negate the cache check, but that is ok -pw                             
      %this.addHeadline( %hdl, true );
   }
}
