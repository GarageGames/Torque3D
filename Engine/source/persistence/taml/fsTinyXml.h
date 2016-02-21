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

#ifndef _FSTINYXML_H_
#define _FSTINYXML_H_


#ifndef TINYXML_INCLUDED
#include "tinyxml/tinyxml.h"
#endif

#include "platform/platform.h"

#ifndef _FILESTREAM_H_
#include "core/stream/fileStream.h"
#endif

class fsTiXmlNode
{
public:
   virtual void Print( FileStream& stream, int depth ) const = 0;
};

class fsTiXmlDocument : public TiXmlDocument, public fsTiXmlNode
{
public:
   bool LoadFile( FileStream &stream, TiXmlEncoding encoding = TIXML_DEFAULT_ENCODING );
   bool SaveFile( FileStream &stream ) const;
	/// Load a file using the given filename. Returns true if successful.
	bool LoadFile( const char * filename, TiXmlEncoding encoding = TIXML_DEFAULT_ENCODING );
	/// Save a file using the given filename. Returns true if successful.
	bool SaveFile( const char * filename ) const;
   virtual void Print( FileStream& stream, int depth ) const;

   virtual TiXmlNode* Clone() const
   {
	   TiXmlDocument* clone = new fsTiXmlDocument();
	   if ( !clone )
		   return 0;

	   CopyTo( clone );
	   return clone;
   }
   TiXmlNode* Identify( const char* p, TiXmlEncoding encoding );
};

class fsTiXmlAttribute : public TiXmlAttribute, public fsTiXmlNode
{
public:
   virtual void Print( FileStream& stream, int depth, TIXML_STRING* str ) const;
   virtual void Print( FileStream& stream, int depth) const
   {
      Print(stream, depth, 0);
   }
};

class fsTiXmlDeclaration : public TiXmlDeclaration, public fsTiXmlNode
{
public:
   fsTiXmlDeclaration(){};
	fsTiXmlDeclaration(	const char* _version,
						const char* _encoding,
                  const char* _standalone ) : TiXmlDeclaration(_version, _encoding, _standalone) { }

   virtual void Print( FileStream& stream, int depth, TIXML_STRING* str ) const;
   virtual void Print( FileStream& stream, int depth) const
   {
      Print(stream, depth, 0);
   }

   virtual TiXmlNode* Clone() const
   {
	   fsTiXmlDeclaration* clone = new fsTiXmlDeclaration();

	   if ( !clone )
		   return 0;

	   CopyTo( clone );
	   return clone;
   }
};

class fsTiXmlElement : public TiXmlElement, public fsTiXmlNode
{
public:
   fsTiXmlElement(const char* in_value) : TiXmlElement(in_value) { }

   virtual void Print( FileStream& stream, int depth ) const;

   virtual TiXmlNode* Clone() const
   {
	   TiXmlElement* clone = new fsTiXmlElement( Value() );
	   if ( !clone )
		   return 0;

	   CopyTo( clone );
	   return clone;
   }

   void SetAttribute( const char* name, const char * _value )
   {
      TiXmlAttribute* attrib = attributeSet.Find( name );
      if(!attrib)
      {
         attrib = new fsTiXmlAttribute();
		   attributeSet.Add( attrib );
		   attrib->SetName( name );
      }
	   if ( attrib ) {
		   attrib->SetValue( _value );
	   }
   }
   void SetAttribute( const char * name, int value )
   {
	   TiXmlAttribute* attrib = attributeSet.Find( name );
      if(!attrib)
      {
         attrib = new fsTiXmlAttribute();
		   attributeSet.Add( attrib );
		   attrib->SetName( name );
      }
	   if ( attrib ) {
		   attrib->SetIntValue( value );
	   }
   }
   TiXmlNode* Identify( const char* p, TiXmlEncoding encoding );
   virtual const char* Parse( const char* p, TiXmlParsingData* data, TiXmlEncoding encoding );
};

class fsTiXmlComment : public TiXmlComment, public fsTiXmlNode
{
public:
   virtual void Print( FileStream& stream, int depth ) const;

   virtual TiXmlNode* Clone() const
   {
	   TiXmlComment* clone = new fsTiXmlComment();

	   if ( !clone )
		   return 0;

	   CopyTo( clone );
	   return clone;
   }
};

class fsTiXmlText : public TiXmlText, public fsTiXmlNode
{
public:
   fsTiXmlText (const char * initValue ) : TiXmlText(initValue) { }
   virtual void Print( FileStream& stream, int depth ) const;

   virtual TiXmlNode* Clone() const
   {
	   TiXmlText* clone = 0;
	   clone = new fsTiXmlText( "" );

	   if ( !clone )
		   return 0;

	   CopyTo( clone );
	   return clone;
   }
};

class fsTiXmlUnknown : public TiXmlUnknown, public fsTiXmlNode
{
public:
   virtual void Print( FileStream& stream, int depth ) const;

   virtual TiXmlNode* Clone() const
   {
	   TiXmlUnknown* clone = new fsTiXmlUnknown();

	   if ( !clone )
		   return 0;

	   CopyTo( clone );
	   return clone;
   }
};

static bool AttemptPrintTiNode(class fsTiXmlDocument* node, FileStream& stream, int depth)
{
   fsTiXmlDocument* fsDoc = dynamic_cast<fsTiXmlDocument*>(node);
   if(fsDoc != NULL)
   {
      fsDoc->Print(stream, depth);
      return true;
   }
   fsTiXmlUnknown* fsUnk = dynamic_cast<fsTiXmlUnknown*>(node);
   if(fsUnk != NULL)
   {
      fsUnk->Print(stream, depth);
      return true;
   }
   fsTiXmlText* fsTxt = dynamic_cast<fsTiXmlText*>(node);
   if(fsTxt != NULL)
   {
      fsTxt->Print(stream, depth);
      return true;
   }
   fsTiXmlComment* fsCom = dynamic_cast<fsTiXmlComment*>(node);
   if(fsCom != NULL)
   {
      fsCom->Print(stream, depth);
      return true;
   }
   fsTiXmlElement* fsElm = dynamic_cast<fsTiXmlElement*>(node);
   if(fsElm != NULL)
   {
      fsElm->Print(stream, depth);
      return true;
   }
   fsTiXmlDeclaration* fsDec = dynamic_cast<fsTiXmlDeclaration*>(node);
   if(fsDec != NULL)
   {
      fsDec->Print(stream, depth);
      return true;
   }
   fsTiXmlAttribute* fsAtt = dynamic_cast<fsTiXmlAttribute*>(node);
   if(fsAtt != NULL)
   {
      fsAtt->Print(stream, depth);
      return true;
   }
   return false;
}
#endif //_FSTINYXML_H_
