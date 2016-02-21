//-----------------------------------------------------------------------------
// Copyright (c) 2013 GarageGames, LLC
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

#include "fsTinyXml.h"
#include "console/console.h"

bool fsTiXmlDocument::LoadFile( const char * pFilename, TiXmlEncoding encoding )
{
   // Expand the file-path.
   char filenameBuffer[1024];
   Con::expandToolScriptFilename( filenameBuffer, sizeof(filenameBuffer), pFilename );

   FileStream stream;

#ifdef TORQUE_OS_ANDROID
   if (strlen(pFilename) > strlen(filenameBuffer)) {
      strcpy(filenameBuffer, pFilename);
   }
#endif

   // File open for read?
   if ( !stream.open( filenameBuffer, Torque::FS::File::Read ) )
   {
      // No, so warn.
      Con::warnf("TamlXmlParser::parse() - Could not open filename '%s' for parse.", filenameBuffer );
      return false;
   }

   // Load document from stream.
   if ( !LoadFile( stream ) )
   {
      // Warn!
      Con::warnf("TamlXmlParser: Could not load Taml XML file from stream.");
      return false;
   }

   // Close the stream.
   stream.close();
   return true;
}

bool fsTiXmlDocument::SaveFile( const char * pFilename ) const
{
   // Expand the file-name into the file-path buffer.
   char filenameBuffer[1024];
   Con::expandToolScriptFilename( filenameBuffer, sizeof(filenameBuffer), pFilename );

   FileStream stream;

   // File opened?
   if ( !stream.open( filenameBuffer, Torque::FS::File::Write ) )
   {
      // No, so warn.
      Con::warnf("Taml::writeFile() - Could not open filename '%s' for write.", filenameBuffer );
      return false;
   }

   bool ret = SaveFile(stream);

   stream.close();
   return ret;
}

bool fsTiXmlDocument::LoadFile( FileStream &stream, TiXmlEncoding encoding )
{
   // Delete the existing data:
   Clear();
   //TODO: Can't clear location, investigate if this gives issues.
   //doc.location.Clear();

   // Get the file size, so we can pre-allocate the string. HUGE speed impact.
   long length = stream.getStreamSize();

   // Strange case, but good to handle up front.
   if ( length <= 0 )
   {
      SetError( TiXmlDocument::TIXML_ERROR_DOCUMENT_EMPTY, 0, 0, TIXML_ENCODING_UNKNOWN );
      return false;
   }

   // Subtle bug here. TinyXml did use fgets. But from the XML spec:
   // 2.11 End-of-Line Handling
   // <snip>
   // <quote>
   // ...the XML processor MUST behave as if it normalized all line breaks in external 
   // parsed entities (including the document entity) on input, before parsing, by translating 
   // both the two-character sequence #xD #xA and any #xD that is not followed by #xA to 
   // a single #xA character.
   // </quote>
   //
   // It is not clear fgets does that, and certainly isn't clear it works cross platform. 
   // Generally, you expect fgets to translate from the convention of the OS to the c/unix
   // convention, and not work generally.

   /*
   while( fgets( buf, sizeof(buf), file ) )
   {
   data += buf;
   }
   */

   char* buf = new char[ length+1 ];
   buf[0] = 0;

   if ( !stream.read( length, buf ) ) {
      delete [] buf;
      SetError( TiXmlDocument::TIXML_ERROR_OPENING_FILE, 0, 0, TIXML_ENCODING_UNKNOWN );
      return false;
   }

   // Process the buffer in place to normalize new lines. (See comment above.)
   // Copies from the 'p' to 'q' pointer, where p can advance faster if
   // a newline-carriage return is hit.
   //
   // Wikipedia:
   // Systems based on ASCII or a compatible character set use either LF  (Line feed, '\n', 0x0A, 10 in decimal) or 
   // CR (Carriage return, '\r', 0x0D, 13 in decimal) individually, or CR followed by LF (CR+LF, 0x0D 0x0A)...
   //                * LF:    Multics, Unix and Unix-like systems (GNU/Linux, AIX, Xenix, Mac OS X, FreeBSD, etc.), BeOS, Amiga, RISC OS, and others
   //                * CR+LF: DEC RT-11 and most other early non-Unix, non-IBM OSes, CP/M, MP/M, DOS, OS/2, Microsoft Windows, Symbian OS
   //                * CR:    Commodore 8-bit machines, Apple II family, Mac OS up to version 9 and OS-9

   const char* p = buf;        // the read head
   char* q = buf;                        // the write head
   const char CR = 0x0d;
   const char LF = 0x0a;

   buf[length] = 0;
   while( *p ) {
      assert( p < (buf+length) );
      assert( q <= (buf+length) );
      assert( q <= p );

      if ( *p == CR ) {
         *q++ = LF;
         p++;
         if ( *p == LF ) {                // check for CR+LF (and skip LF)
            p++;
         }
      }
      else {
         *q++ = *p++;
      }
   }
   assert( q <= (buf+length) );
   *q = 0;

   Parse( buf, 0, encoding );

   delete [] buf;
   return !Error();
}

bool fsTiXmlDocument::SaveFile( FileStream &stream ) const
{
   if ( useMicrosoftBOM ) 
   {
      const unsigned char TIXML_UTF_LEAD_0 = 0xefU;
      const unsigned char TIXML_UTF_LEAD_1 = 0xbbU;
      const unsigned char TIXML_UTF_LEAD_2 = 0xbfU;

      stream.write( TIXML_UTF_LEAD_0 );
      stream.write( TIXML_UTF_LEAD_1 );
      stream.write( TIXML_UTF_LEAD_2 );
   }
   Print( stream, 0 );
   return true;
}

void fsTiXmlDocument::Print( FileStream& stream, int depth ) const
{
   for ( const TiXmlNode* node=FirstChild(); node; node=node->NextSibling() )
   {
      //AttemptPrintTiNode(const_cast<TiXmlNode*>(node), stream, depth);
      dynamic_cast<const fsTiXmlNode*>(node)->Print( stream, depth );
      stream.writeText( "\n" );
   }
}

void fsTiXmlAttribute::Print( FileStream& stream, int depth, TIXML_STRING* str ) const
{
   TIXML_STRING n, v;

   TiXmlString value = TiXmlString(Value());

   EncodeString( NameTStr(), &n );
   EncodeString( value, &v );

   for ( int i=0; i< depth; i++ ) {
      stream.writeText( "    " );
   }

   if (value.find ('\"') == TIXML_STRING::npos) {
      const char* pValue = v.c_str();
      char buffer[4096];
      const S32 length = dSprintf(buffer, sizeof(buffer), "%s=\"%s\"", n.c_str(), pValue);
      stream.write(length, buffer);
      if ( str ) {
         (*str) += n; (*str) += "=\""; (*str) += v; (*str) += "\"";
      }
   }
   else {
      char buffer[4096];
      const S32 length = dSprintf(buffer, sizeof(buffer), "%s='%s'", n.c_str(), v.c_str());
      stream.write(length, buffer);
      if ( str ) {
         (*str) += n; (*str) += "='"; (*str) += v; (*str) += "'";
      }
   }
}

void fsTiXmlDeclaration::Print(FileStream& stream, int depth, TiXmlString* str) const
{
   stream.writeStringBuffer( "<?xml " );
   if ( str )	 (*str) += "<?xml ";

   if ( !version.empty() ) {
      stream.writeFormattedBuffer( "version=\"%s\" ", version.c_str ());
      if ( str ) { (*str) += "version=\""; (*str) += version; (*str) += "\" "; }
   }
   if ( !encoding.empty() ) {
      stream.writeFormattedBuffer( "encoding=\"%s\" ", encoding.c_str ());
      if ( str ) { (*str) += "encoding=\""; (*str) += encoding; (*str) += "\" "; }
   }
   if ( !standalone.empty() ) {
      stream.writeFormattedBuffer( "standalone=\"%s\" ", standalone.c_str ());
      if ( str ) { (*str) += "standalone=\""; (*str) += standalone; (*str) += "\" "; }
   }
   stream.writeStringBuffer( "?>" );
   if ( str )	 (*str) += "?>";
}

void fsTiXmlElement::Print(FileStream& stream, int depth) const
{
   int i;
   for ( i=0; i<depth; i++ ) {
      stream.writeStringBuffer( "    " );
   }

   stream.writeFormattedBuffer( "<%s", value.c_str() );

   const TiXmlAttribute* attrib;
   for ( attrib = attributeSet.First(); attrib; attrib = attrib->Next() )
   {
      stream.writeStringBuffer( "\n" );
      dynamic_cast<const fsTiXmlAttribute*>(attrib)->Print( stream, depth+1 );
   }

   // There are 3 different formatting approaches:
   // 1) An element without children is printed as a <foo /> node
   // 2) An element with only a text child is printed as <foo> text </foo>
   // 3) An element with children is printed on multiple lines.
   TiXmlNode* node;
   if ( !firstChild )
   {
      stream.writeStringBuffer( " />" );
   }
   else if ( firstChild == lastChild && firstChild->ToText() )
   {
      stream.writeStringBuffer( ">" );
      dynamic_cast<const fsTiXmlNode*>(firstChild)->Print( stream, depth + 1 );
      stream.writeFormattedBuffer( "</%s>", value.c_str() );
   }
   else
   {
      stream.writeStringBuffer( ">" );

      for ( node = firstChild; node; node=node->NextSibling() )
      {
         if ( !node->ToText() )
         {
            stream.writeStringBuffer( "\n" );
         }
         dynamic_cast<const fsTiXmlNode*>(node)->Print( stream, depth+1 );
      }
      stream.writeStringBuffer( "\n" );
      for( i=0; i<depth; ++i ) {
         stream.writeStringBuffer( "    " );
      }
      stream.writeFormattedBuffer( "</%s>", value.c_str() );
   }
}

void fsTiXmlComment::Print(FileStream& stream, int depth) const
{
   for ( int i=0; i<depth; i++ )
   {
      stream.writeStringBuffer( "    " );
   }
   stream.writeFormattedBuffer( "<!--%s-->", value.c_str() );
}

void fsTiXmlText::Print(FileStream& stream, int depth) const
{ 
   if ( cdata )
   {
      int i;
      stream.writeStringBuffer( "\n" );
      for ( i=0; i<depth; i++ ) {
         stream.writeStringBuffer( "    " );
      }
      stream.writeFormattedBuffer( "<![CDATA[%s]]>\n", value.c_str() );	// unformatted output
   }
   else
   {
      TIXML_STRING buffer;
      EncodeString( value, &buffer );
      stream.writeFormattedBuffer( "%s", buffer.c_str() );
   }
}

void fsTiXmlUnknown::Print(FileStream& stream, int depth) const
{
   for ( int i=0; i<depth; i++ )
      stream.writeStringBuffer( "    " );

   stream.writeFormattedBuffer( "<%s>", value.c_str() );
}

static TiXmlNode* TiNodeIdentify( TiXmlNode* parent, const char* p, TiXmlEncoding encoding )
{
   	TiXmlNode* returnNode = 0;

	p = TiXmlNode::SkipWhiteSpace( p, encoding );
	if( !p || !*p || *p != '<' )
	{
		return 0;
	}

	p = TiXmlNode::SkipWhiteSpace( p, encoding );

	if ( !p || !*p )
	{
		return 0;
	}

	// What is this thing? 
	// - Elements start with a letter or underscore, but xml is reserved.
	// - Comments: <!--
	// - Decleration: <?xml
	// - Everthing else is unknown to tinyxml.
	//

	const char* xmlHeader = { "<?xml" };
	const char* commentHeader = { "<!--" };
	const char* dtdHeader = { "<!" };
	const char* cdataHeader = { "<![CDATA[" };

	if ( TiXmlNode::StringEqual( p, xmlHeader, true, encoding ) )
	{
		#ifdef DEBUG_PARSER
			TIXML_LOG( "XML parsing Declaration\n" );
		#endif
		returnNode = new fsTiXmlDeclaration();
	}
	else if ( TiXmlNode::StringEqual( p, commentHeader, false, encoding ) )
	{
		#ifdef DEBUG_PARSER
			TIXML_LOG( "XML parsing Comment\n" );
		#endif
		returnNode = new fsTiXmlComment();
	}
	else if ( TiXmlNode::StringEqual( p, cdataHeader, false, encoding ) )
	{
		#ifdef DEBUG_PARSER
			TIXML_LOG( "XML parsing CDATA\n" );
		#endif
		TiXmlText* text = new fsTiXmlText( "" );
		text->SetCDATA( true );
		returnNode = text;
	}
	else if ( TiXmlNode::StringEqual( p, dtdHeader, false, encoding ) )
	{
		#ifdef DEBUG_PARSER
			TIXML_LOG( "XML parsing Unknown(1)\n" );
		#endif
		returnNode = new fsTiXmlUnknown();
	}
	else if (    TiXmlNode::IsAlpha( *(p+1), encoding )
			  || *(p+1) == '_' )
	{
		#ifdef DEBUG_PARSER
			TIXML_LOG( "XML parsing Element\n" );
		#endif
		returnNode = new fsTiXmlElement( "" );
	}
	else
	{
		#ifdef DEBUG_PARSER
			TIXML_LOG( "XML parsing Unknown(2)\n" );
		#endif
		returnNode = new fsTiXmlUnknown();
	}

	if ( returnNode )
	{
		// Set the parent, so it can report errors
		returnNode->parent = parent;
	}
	return returnNode;
}

TiXmlNode* fsTiXmlDocument::Identify(  const char* p, TiXmlEncoding encoding )
{
   return TiNodeIdentify(this, p, encoding);
}

TiXmlNode* fsTiXmlElement::Identify(  const char* p, TiXmlEncoding encoding )
{
   return TiNodeIdentify(this, p, encoding);
}

const char* fsTiXmlElement::Parse( const char* p, TiXmlParsingData* data, TiXmlEncoding encoding )
{
	p = SkipWhiteSpace( p, encoding );
	TiXmlDocument* document = GetDocument();

	if ( !p || !*p )
	{
		if ( document ) document->SetError( TIXML_ERROR_PARSING_ELEMENT, 0, 0, encoding );
		return 0;
	}

	if ( data )
	{
		data->Stamp( p, encoding );
		location = data->Cursor();
	}

	if ( *p != '<' )
	{
		if ( document ) document->SetError( TIXML_ERROR_PARSING_ELEMENT, p, data, encoding );
		return 0;
	}

	p = SkipWhiteSpace( p+1, encoding );

	// Read the name.
	const char* pErr = p;

    p = ReadName( p, &value, encoding );
	if ( !p || !*p )
	{
		if ( document )	document->SetError( TIXML_ERROR_FAILED_TO_READ_ELEMENT_NAME, pErr, data, encoding );
		return 0;
	}

    TIXML_STRING endTag ("</");
	endTag += value;

	// Check for and read attributes. Also look for an empty
	// tag or an end tag.
	while ( p && *p )
	{
		pErr = p;
		p = SkipWhiteSpace( p, encoding );
		if ( !p || !*p )
		{
			if ( document ) document->SetError( TIXML_ERROR_READING_ATTRIBUTES, pErr, data, encoding );
			return 0;
		}
		if ( *p == '/' )
		{
			++p;
			// Empty tag.
			if ( *p  != '>' )
			{
				if ( document ) document->SetError( TIXML_ERROR_PARSING_EMPTY, p, data, encoding );		
				return 0;
			}
			return (p+1);
		}
		else if ( *p == '>' )
		{
			// Done with attributes (if there were any.)
			// Read the value -- which can include other
			// elements -- read the end tag, and return.
			++p;
			p = ReadValue( p, data, encoding );		// Note this is an Element method, and will set the error if one happens.
			if ( !p || !*p ) {
				// We were looking for the end tag, but found nothing.
				// Fix for [ 1663758 ] Failure to report error on bad XML
				if ( document ) document->SetError( TIXML_ERROR_READING_END_TAG, p, data, encoding );
				return 0;
			}

			// We should find the end tag now
			// note that:
			// </foo > and
			// </foo> 
			// are both valid end tags.
			if ( StringEqual( p, endTag.c_str(), false, encoding ) )
			{
				p += endTag.length();
				p = SkipWhiteSpace( p, encoding );
				if ( p && *p && *p == '>' ) {
					++p;
					return p;
				}
				if ( document ) document->SetError( TIXML_ERROR_READING_END_TAG, p, data, encoding );
				return 0;
			}
			else
			{
				if ( document ) document->SetError( TIXML_ERROR_READING_END_TAG, p, data, encoding );
				return 0;
			}
		}
		else
		{
			// Try to read an attribute:
			TiXmlAttribute* attrib = new fsTiXmlAttribute();
			if ( !attrib )
			{
				return 0;
			}

			attrib->SetDocument( document );
			pErr = p;
			p = attrib->Parse( p, data, encoding );

			if ( !p || !*p )
			{
				if ( document ) document->SetError( TIXML_ERROR_PARSING_ELEMENT, pErr, data, encoding );
				delete attrib;
				return 0;
			}

			// Handle the strange case of double attributes:
			#ifdef TIXML_USE_STL
			TiXmlAttribute* node = attributeSet.Find( attrib->NameTStr() );
			#else
			TiXmlAttribute* node = attributeSet.Find( attrib->Name() );
			#endif
			if ( node )
			{
				if ( document ) document->SetError( TIXML_ERROR_PARSING_ELEMENT, pErr, data, encoding );
				delete attrib;
				return 0;
			}

			attributeSet.Add( attrib );
		}
	}
	return p;
}

/*
TiXmlNode* fsTiXmlNode::Identify(char const* p, TiXmlEncoding encoding)
{	
TiXmlNode* returnNode = 0;

p = TiXmlBase::SkipWhiteSpace( p, encoding );
if( !p || !*p || *p != '<' )
{
return 0;
}

p = TiXmlBase::SkipWhiteSpace( p, encoding );

if ( !p || !*p )
{
return 0;
}

// What is this thing? 
// - Elements start with a letter or underscore, but xml is reserved.
// - Comments: <!--
// - Decleration: <?xml
// - Everthing else is unknown to tinyxml.
//

const char* xmlHeader = { "<?xml" };
const char* commentHeader = { "<!--" };
const char* dtdHeader = { "<!" };
const char* cdataHeader = { "<![CDATA[" };

if ( TiXmlBase::StringEqual( p, xmlHeader, true, encoding ) )
{
#ifdef DEBUG_PARSER
TIXML_LOG( "XML parsing Declaration\n" );
#endif
returnNode = new fsTiXmlDeclaration();
}
else if ( TiXmlBase::StringEqual( p, commentHeader, false, encoding ) )
{
#ifdef DEBUG_PARSER
TIXML_LOG( "XML parsing Comment\n" );
#endif
returnNode = new fsTiXmlComment();
}
else if ( TiXmlBase::StringEqual( p, cdataHeader, false, encoding ) )
{
#ifdef DEBUG_PARSER
TIXML_LOG( "XML parsing CDATA\n" );
#endif
fsTiXmlText* text = new fsTiXmlText( "" );
text->SetCDATA( true );
returnNode = text;
}
else if ( TiXmlBase::StringEqual( p, dtdHeader, false, encoding ) )
{
#ifdef DEBUG_PARSER
TIXML_LOG( "XML parsing Unknown(1)\n" );
#endif
returnNode = new fsTiXmlUnknown();
}
else if (    TiXmlBase::IsAlpha( *(p+1), encoding )
|| *(p+1) == '_' )
{
#ifdef DEBUG_PARSER
TIXML_LOG( "XML parsing Element\n" );
#endif
returnNode = new fsTiXmlElement( "" );
}
else
{
#ifdef DEBUG_PARSER
TIXML_LOG( "XML parsing Unknown(2)\n" );
#endif
returnNode = new fsTiXmlUnknown();
}

if ( returnNode )
{
// Set the parent, so it can report errors
returnNode->parent = this;
}
return returnNode;
}

const unsigned char TIXML_UTF_LEAD_0 = 0xefU;
const unsigned char TIXML_UTF_LEAD_1 = 0xbbU;
const unsigned char TIXML_UTF_LEAD_2 = 0xbfU;

char const* fsTiXmlDocument::Parse(char const* p, TiXmlParsingData* prevData, TiXmlEncoding encoding)
{
ClearError();

// Parse away, at the document level. Since a document
// contains nothing but other tags, most of what happens
// here is skipping white space.
if ( !p || !*p )
{
SetError( TiXmlBase::TIXML_ERROR_DOCUMENT_EMPTY, 0, 0, TIXML_ENCODING_UNKNOWN );
return 0;
}

// Note that, for a document, this needs to come
// before the while space skip, so that parsing
// starts from the pointer we are given.
location.Clear();
if ( prevData )
{
location.row = prevData->Cursor().row;
location.col = prevData->Cursor().col;
}
else
{
location.row = 0;
location.col = 0;
}
TiXmlParsingData data( p, TabSize(), location.row, location.col );
location = data.Cursor();

if ( encoding == TIXML_ENCODING_UNKNOWN )
{
// Check for the Microsoft UTF-8 lead bytes.
const unsigned char* pU = (const unsigned char*)p;
if (	*(pU+0) && *(pU+0) == TIXML_UTF_LEAD_0
&& *(pU+1) && *(pU+1) == TIXML_UTF_LEAD_1
&& *(pU+2) && *(pU+2) == TIXML_UTF_LEAD_2 )
{
encoding = TIXML_ENCODING_UTF8;
useMicrosoftBOM = true;
}
}

p = TiXmlBase::SkipWhiteSpace( p, encoding );
if ( !p )
{
SetError( TiXmlBase::TIXML_ERROR_DOCUMENT_EMPTY, 0, 0, TIXML_ENCODING_UNKNOWN );
return 0;
}

while ( p && *p )
{
TiXmlNode* node = fsTiXmlNode::Identify( p, encoding );
if ( node )
{
p = node->Parse( p, &data, encoding );
LinkEndChild( node );
}
else
{
break;
}

// Did we get encoding info?
if (    encoding == TIXML_ENCODING_UNKNOWN
&& node->ToDeclaration() )
{
TiXmlDeclaration* dec = node->ToDeclaration();
const char* enc = dec->Encoding();
assert( enc );

if ( *enc == 0 )
encoding = TIXML_ENCODING_UTF8;
else if ( TiXmlBase::StringEqual( enc, "UTF-8", true, TIXML_ENCODING_UNKNOWN ) )
encoding = TIXML_ENCODING_UTF8;
else if ( TiXmlBase::StringEqual( enc, "UTF8", true, TIXML_ENCODING_UNKNOWN ) )
encoding = TIXML_ENCODING_UTF8;	// incorrect, but be nice
else 
encoding = TIXML_ENCODING_LEGACY;
}

p = TiXmlBase::SkipWhiteSpace( p, encoding );
}

// Was this empty?
if ( !firstChild ) {
SetError( TiXmlBase::TIXML_ERROR_DOCUMENT_EMPTY, 0, 0, encoding );
return 0;
}

// All is well.
return p;
}
*/
