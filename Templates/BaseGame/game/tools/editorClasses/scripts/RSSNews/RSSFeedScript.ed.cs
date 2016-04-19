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

// RSS ticker configuration:
$RSSFeed::serverName = "feeds.garagegames.com";
$RSSFeed::serverPort = 80;
$RSSFeed::serverURL  = "/product/tgea";
$RSSFeed::userAgent = "TorqueGameEngineAdvances/1.1";
$RSSFeed::maxNewHeadlines = 10;

// Load up the helper objects
exec( "./RSSStructs.ed.cs" );

function RSSFeedObject::onConnected(%this)
{
   //echo("RSS Feed");
   //echo("   - Requesting RSS data at URL: " @ $RSSFeed::serverURL );
   
   // Reset some useful state information.
   $RSSFeed::lineCount = 0;
   $RSSFeed::requestResults = "";
   
   // Load the cache here...
   $RSSFeed::rssCache = 0;
   
   // Request our RSS.
   %this.send("GET " @ $RSSFeed::serverURL @ " HTTP/1.0\nHost: " @ $RSSFeed::serverName @ "\nUser-Agent: " @ $RSSFeed::userAgent @ "\n\r\n\r\n");
}

function RSSFeedObject::onLine(%this, %line)
{
   // Collate info.
   $RSSFeed::lineCount++;
   $RSSFeed::requestResults = $RSSFeed::requestResults @ %line;
}

function RSSFeedObject::getTagContents(%this, %string, %tag, %startChar)
{
   // This function occasionally makes Torque hard crash. It doesn't
   // seem to do it anymore but be careful!
   
   // Ok, get thing between <%tag> and </%tag> after char #
   // %startChar in the passed string.
   
   %startTag = "<" @ %tag @ ">";
   %endTag   = "</" @ %tag @ ">";
   
   %startTagOffset = strpos(%string, %startTag, %startChar);
   
   // Compensate for presence of start tag.
   %startOffset = %startTagOffset + strlen(%startTag);
   
   // Ok, now look for end tag.
   %endTagOffset = strpos(%string, %endTag, %startOffset - 1);
   
   // If we didn't find it, bail. 
   if(%endTagOffset < 0)
   return "";
   
   // Evil hack - store last found item in a global.
   %this.lastOffset = %endTagOffset;
   
   // And get & return the substring between the tags.
   %result = getSubStr(%string, %startOffset, %endTagOffset - %startOffset);
   
   // Do a little mojo to deal with &quot; and some other htmlentities.
   %result = strreplace(%result, "&quot;", "\"");
   %result = strreplace(%result, "&amp;",  "&");
   
   return %result;
}

function RSSFeedObject::onDisconnect(%this)
{
   // Create collection and load cache.
   %ret = constructRSSHeadlineCollection();
   %ret.loadFromFile( "RSSCache.cs" );
   
   // Ok, we have a full buffer now, hopefully. Let's process it.
   //echo("   - Got " @ $RSSFeed::lineCount @ " lines.");
   
   // We want the feed title and the first three headlines + links.
   
   // Feed title - get the first <title> occurence in the string.
   %title = %this.getTagContents($RSSFeed::requestResults, "title", 0);
   %titleLink = %this.getTagContents($RSSFeed::requestResults, "link", 0);
   
   //echo("   - Feed title: '" @ %title @ "'");
   //echo("   - Feed link:  '" @ %titleLink @ "'");
   
   %newItems = "";
   
   // Ok, get the first headlines, if any...
   for( %i = 0; %i < $RSSFeed::maxNewHeadlines; %i++ )
   {
      %headline[%i]     = %this.getTagContents($RSSFeed::requestResults, "title", %this.lastOffset);
      %headlineLink[%i] = %this.getTagContents($RSSFeed::requestResults, "link", %this.lastOffset);
      
      // Skip the content - it's not going to do anything but confuse us.
      %garbage = %this.getTagContents($RSSFeed::requestResults, "content:encoded", %this.lastOffset);
      %isNew = %ret.addHeadline( constructRSSHeadline( %headline[%i], %headlineLink[%i] ) );
      
      if( %isNew )
      {
         %newItems = true;
         //echo("   - Headline      #" @ %i @ " : '" @ %headline[%i] @ "'");
         //echo("   - Headline Link #" @ %i @ " : '" @ %headlineLink[%i] @ "'");
      }
   }
   
   if( %this._callback !$= "" )
   {
      %params = %ret;
      
      if( %newItems )
         %params = %params @ ", \"" @ %newItems @ "\"";
         
      eval( %this._callback @ "(" @ %params @ ");" );
   }
   
   %ret.writeToFile( "RSSCache.cs", false );
}

function RSSUpdate::initialize( %callback )
{
   new TCPObject(RSSFeedObject);
   RSSFeedObject._callback = %callback;
   
   RSSFeedObject.connect( $RSSFeed::serverName @ ":" @ $RSSFeed::serverPort );
}

function RSSUpdate::destroy()
{
   if(isObject(RSSFeedObject))
      RSSFeedObject.delete();
}
