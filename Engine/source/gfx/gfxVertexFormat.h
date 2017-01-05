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

#ifndef _GFXVERTEXFORMAT_H_
#define _GFXVERTEXFORMAT_H_

#ifndef _TVECTOR_H_
#include "core/util/tVector.h"
#endif
#ifndef _GFXENUMS_H_
#include "gfx/gfxEnums.h"
#endif


/// The known Torque vertex element semantics.  You can use
/// other semantic strings, but they will be interpreted as
/// a TEXCOORD.
/// @see GFXVertexElement
/// @see GFXVertexFormat
namespace GFXSemantic
{
   extern const String POSITION;
   extern const String NORMAL;
   extern const String BINORMAL;
   extern const String TANGENT;
   extern const String TANGENTW;
   extern const String COLOR;
   extern const String TEXCOORD;
   extern const String BLENDWEIGHT;
   extern const String BLENDINDICES;
   extern const String PADDING;
}


/// This is a simple wrapper for the platform specific
/// vertex declaration data which is held by the vertex
/// format.  
///
/// If your using it... you probably shouldn't be.
///
/// @see GFXVertexFormat
class GFXVertexDecl
{
public:
   virtual ~GFXVertexDecl() {}
};


/// The element structure helps define the data layout 
/// for GFXVertexFormat.
///
/// @see GFXVertexFormat
///
class GFXVertexElement
{
   friend class GFXVertexFormat;

protected:

   /// The stream index when rendering from multiple
   /// vertex streams.  In most cases this is 0.
   U32 mStreamIndex;

   /// A valid Torque shader symantic.
   /// @see GFXSemantic   
   String mSemantic;

   /// The semantic index is used where there are
   /// multiple semantics of the same type.  For 
   /// instance with texcoords.
   U32 mSemanticIndex;

   /// The element type.
   GFXDeclType mType;

public:

   /// Default constructor.
   GFXVertexElement()
      :  mStreamIndex( 0 ),
         mSemanticIndex( 0 ),         
         mType( GFXDeclType_Float4 )
   {
   }

   /// Copy constructor.
   GFXVertexElement( const GFXVertexElement &elem )
      :  mStreamIndex( elem.mStreamIndex ),
         mSemantic( elem.mSemantic ),
         mSemanticIndex( elem.mSemanticIndex ),         
         mType( elem.mType )
   {
   }

   /// Returns the stream index.
   U32 getStreamIndex() const { return mStreamIndex; }

   /// Returns the semantic name which is usually a
   /// valid Torque semantic.
   /// @see GFXSemantic
   const String& getSemantic() const { return mSemantic; }

   /// Returns the semantic index which is used where there
   /// are multiple semantics of the same type.  For instance
   /// with texcoords.
   U32 getSemanticIndex() const { return mSemanticIndex; }

   /// Returns the type for the semantic.
   GFXDeclType getType() const { return mType; }

   /// Returns true of the semantic matches.
   bool isSemantic( const String& str ) const { return ( mSemantic == str ); }

   /// Returns the size in bytes of the semantic type.
   U32 getSizeInBytes() const;
};


/// The vertex format structure usually created via the declare and 
/// implement macros.
///
/// You can use this class directly to create a vertex format, but
/// note that it is expected to live as long as the VB that uses it 
/// exists.
///
/// @see GFXDeclareVertexFormat
/// @see GFXImplementVertexFormat
/// @see GFXVertexElement
///
class GFXVertexFormat
{
public:

   /// Default constructor for an empty format.
   GFXVertexFormat();

   /// The copy constructor.
   GFXVertexFormat( const GFXVertexFormat &format ) { copy( format ); }

   /// Tell this format it has instancing
   void enableInstancing();

   /// Copy the other vertex format.
   void copy( const GFXVertexFormat &format );

   /// Used to append a vertex format to the end of this one.
   void append( const GFXVertexFormat &format, U32 streamIndex = -1 );

   /// Returns a unique description string for this vertex format.
   const String& getDescription() const;

   /// Clears all the vertex elements.
   void clear();   

   /// Adds a vertex element to the format.
   ///
   /// @param semantic A valid Torque semantic string.
   /// @param type The element type.
   /// @param index The semantic index which is typically only used for texcoords.
   ///
   void addElement( const String& semantic, GFXDeclType type, U32 index = 0, U32 stream = 0 );
   
   /// Returns true if there is a NORMAL semantic in this vertex format.
   bool hasNormal() const;

   /// Returns true if there is a TANGENT semantic in this vertex format.
   bool hasTangent() const;

   /// Returns true if there is a COLOR semantic in this vertex format.
   bool hasColor() const;
   
   /// Returns true if there is a BLENDWEIGHT or BLENDINDICES semantic in this vertex format.
   bool hasBlendIndices() const;

   /// Return true if instancing is used with this vertex format.
   bool hasInstancing() const;

   /// Returns number of blend indices
   U32 getNumBlendIndices() const;

   /// Returns the texture coordinate count by 
   /// counting the number of TEXCOORD semantics.
   U32 getTexCoordCount() const;

   /// Returns true if these two formats are equal.
   inline bool isEqual( const GFXVertexFormat &format ) const;

   /// Returns the total elements in this format.
   U32 getElementCount() const { return mElements.size(); }

   /// Returns the vertex element by index.
   const GFXVertexElement& getElement( U32 index ) const { return mElements[index]; }

   /// Returns the size in bytes of the format as described.
   U32 getSizeInBytes() const;

   /// Returns the hardware specific vertex declaration for this format.
   GFXVertexDecl* getDecl() const;

protected:

   /// We disable the copy operator.
   GFXVertexFormat& operator =( const GFXVertexFormat& ) { return *this; }

   /// Recreates the description and state when 
   /// the format has been modified.
   void _updateDirty();

   /// Requests the vertex declaration from the GFX device.
   void _updateDecl();

   /// Set when the element list is changed.
   bool mDirty;

   /// Is set if there is a NORMAL semantic in this vertex format.   
   bool mHasNormal;

   /// Is true if there is a TANGENT semantic in this vertex format.
   bool mHasTangent;

   /// Is true if there is a COLOR semantic in this vertex format.
   bool mHasColor;
   
   /// Is true if there is a BLENDWEIGHT or BLENDINDICES semantic in this vertex format.
   bool mHasBlendIndices;

   /// Is instaning used with this vertex format.
   bool mHasInstancing;

   /// The texture coordinate count by counting the 
   /// number of "TEXCOORD" semantics.
   U32 mTexCoordCount;

   /// The size in bytes of the vertex format as described.
   U32 mSizeInBytes;

   /// An interned string which uniquely identifies the format.
   String mDescription;
   
   /// The elements of the vertex format.
   Vector<GFXVertexElement> mElements;

   /// The hardware specific vertex declaration.
   GFXVertexDecl *mDecl;
};


inline bool GFXVertexFormat::isEqual( const GFXVertexFormat &format ) const 
{
   // Comparing the strings works because we know both 
   // these are interned strings.  This saves one comparison
   // over the string equality operator.
   return getDescription().c_str() == format.getDescription().c_str(); 
}


/// This template class is usused to initialize the format in 
/// the GFXImplement/DeclareVertexFormat macros.  You shouldn't
/// need to use it directly in your code.
///
/// @see GFXVertexFormat
/// @see GFXImplementVertexFormat
///
template<class T>
class _GFXVertexFormatConstructor : public GFXVertexFormat
{
protected:

   void _construct();

public:

   _GFXVertexFormatConstructor() { _construct(); }
};


/// Helper template function which returns the correct 
/// GFXVertexFormat object for a vertex structure.
/// @see GFXVertexFormat
template<class T> inline const GFXVertexFormat* getGFXVertexFormat();

#ifdef TORQUE_OS_XENON

   /// On the Xbox360 we want we want to be sure that verts
   /// are on aligned boundariess.
   #define GFX_VERTEX_STRUCT __declspec(align(16)) struct

#else
   #define GFX_VERTEX_STRUCT struct
#endif


/// The vertex format declaration which is usally placed in your header 
/// file.  It should be used in conjunction with the implementation macro.
/// 
/// @param name The name for the vertex structure.
///
/// @code
///   
///   // A simple vertex format declaration.
///   GFXDeclareVertexFormat( GFXVertexPCT )
///   {
///      Point3F pos;
///      GFXVertexColor color;
///      Point2F texCoord;
///   }
///
/// @endcode
///
/// @see GFXImplementVertexFormat
///
#define GFXDeclareVertexFormat( name ) \
   GFX_VERTEX_STRUCT name; \
   extern const GFXVertexFormat _gfxVertexFormat##name; \
   template<> inline const GFXVertexFormat* getGFXVertexFormat<name>() { static _GFXVertexFormatConstructor<name> vertexFormat; return &vertexFormat; } \
   GFX_VERTEX_STRUCT name \


/// The vertex format implementation which is usally placed in your source 
/// file.  It should be used in conjunction with the declaration macro.
/// 
/// @param name The name of the vertex structure.
///
/// @code
///   
///   // A simple vertex format implementation.
///   GFXImplementVertexFormat( GFXVertexPCT )
///   {
///      addElement( "POSITION", GFXDeclType_Float3 );
///      addElement( "COLOR", GFXDeclType_Color );
///      addElement( "TEXCOORD", GFXDeclType_Float2, 0 );
///   }
///
/// @endcode
///
/// @see GFXDeclareVertexFormat
///
#define GFXImplementVertexFormat( name ) \
   template<>   void _GFXVertexFormatConstructor<name>::_construct() \

#endif // _GFXVERTEXFORMAT_H_
