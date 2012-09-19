/*

wavefront.cpp : A very small code snippet to read a Wavefront OBJ file into memory.

*/


/*!
**
** Copyright (c) 2009 by John W. Ratcliff mailto:jratcliffscarab@gmail.com
**
** Portions of this source has been released with the PhysXViewer application, as well as
** Rocket, CreateDynamics, ODF, and as a number of sample code snippets.
**
** If you find this code useful or you are feeling particularily generous I would
** ask that you please go to http://www.amillionpixels.us and make a donation
** to Troy DeMolay.
**
** DeMolay is a youth group for young men between the ages of 12 and 21.
** It teaches strong moral principles, as well as leadership skills and
** public speaking.  The donations page uses the 'pay for pixels' paradigm
** where, in this case, a pixel is only a single penny.  Donations can be
** made for as small as $4 or as high as a $100 block.  Each person who donates
** will get a link to their own site as well as acknowledgement on the
** donations blog located here http://www.amillionpixels.blogspot.com/
**
** If you wish to contact me you can use the following methods:
**
** Skype ID: jratcliff63367
** Yahoo: jratcliff63367
** AOL: jratcliff1961
** email: jratcliffscarab@gmail.com
**
**
** The MIT license:
**
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to deal
** in the Software without restriction, including without limitation the rights
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
** copies of the Software, and to permit persons to whom the Software is furnished
** to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in all
** copies or substantial portions of the Software.

** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
** WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
** CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/
#ifndef __PPCGEKKO__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

#include "wavefront.h"

#include <vector>

typedef std::vector< NxI32 > IntVector;
typedef std::vector< NxF32 > FloatVector;

#pragma warning(disable:4996)

namespace WAVEFRONT
{


/*******************************************************************/
/******************** InParser.h  ********************************/
/*******************************************************************/
class InPlaceParserInterface
{
public:
	virtual NxI32 ParseLine(NxI32 lineno,NxI32 argc,const char **argv) =0;  // return TRUE to continue parsing, return FALSE to abort parsing process
};

enum SeparatorType
{
	ST_DATA,        // is data
	ST_HARD,        // is a hard separator
	ST_SOFT,        // is a soft separator
	ST_EOS          // is a comment symbol, and everything past this character should be ignored
};

class InPlaceParser
{
public:
	InPlaceParser(void)
	{
		Init();
	}

	InPlaceParser(char *data,NxI32 len)
	{
		Init();
		SetSourceData(data,len);
	}

	InPlaceParser(const char *fname)
	{
		Init();
		SetFile(fname);
	}

	~InPlaceParser(void);

	void Init(void)
	{
		mQuoteChar = 34;
		mData = 0;
		mLen  = 0;
		mMyAlloc = false;
		for (NxI32 i=0; i<256; i++)
		{
			mHard[i] = ST_DATA;
			mHardString[i*2] = i;
			mHardString[i*2+1] = 0;
		}
		mHard[0]  = ST_EOS;
		mHard[32] = ST_SOFT;
		mHard[9]  = ST_SOFT;
		mHard[13] = ST_SOFT;
		mHard[10] = ST_SOFT;
	}

	void SetFile(const char *fname); // use this file as source data to parse.

	void SetSourceData(char *data,NxI32 len)
	{
		mData = data;
		mLen  = len;
		mMyAlloc = false;
	};

	NxI32  Parse(InPlaceParserInterface *callback); // returns true if entire file was parsed, false if it aborted for some reason

	NxI32 ProcessLine(NxI32 lineno,char *line,InPlaceParserInterface *callback);

	const char ** GetArglist(char *source,NxI32 &count); // convert source string into an arg list, this is a destructive parse.

	void SetHardSeparator(char c) // add a hard separator
	{
		mHard[c] = ST_HARD;
	}

	void SetHard(char c) // add a hard separator
	{
		mHard[c] = ST_HARD;
	}


	void SetCommentSymbol(char c) // comment character, treated as 'end of string'
	{
		mHard[c] = ST_EOS;
	}

	void ClearHardSeparator(char c)
	{
		mHard[c] = ST_DATA;
	}


	void DefaultSymbols(void); // set up default symbols for hard seperator and comment symbol of the '#' character.

	bool EOS(char c)
	{
		if ( mHard[c] == ST_EOS )
		{
			return true;
		}
		return false;
	}

	void SetQuoteChar(char c)
	{
		mQuoteChar = c;
	}

private:


	inline char * AddHard(NxI32 &argc,const char **argv,char *foo);
	inline bool   IsHard(char c);
	inline char * SkipSpaces(char *foo);
	inline bool   IsWhiteSpace(char c);
	inline bool   IsNonSeparator(char c); // non seperator,neither hard nor soft

	bool   mMyAlloc; // whether or not *I* allocated the buffer and am responsible for deleting it.
	char  *mData;  // ascii data to parse.
	NxI32    mLen;   // length of data
	SeparatorType  mHard[256];
	char   mHardString[256*2];
	char           mQuoteChar;
};

/*******************************************************************/
/******************** InParser.cpp  ********************************/
/*******************************************************************/
void InPlaceParser::SetFile(const char *fname)
{
	if ( mMyAlloc )
	{
		free(mData);
	}
	mData = 0;
	mLen  = 0;
	mMyAlloc = false;

	FILE *fph = fopen(fname,"rb");
	if ( fph )
	{
		fseek(fph,0L,SEEK_END);
		mLen = ftell(fph);
		fseek(fph,0L,SEEK_SET);
		if ( mLen )
		{
			mData = (char *) malloc(sizeof(char)*(mLen+1));
			size_t ok = fread(mData, mLen, 1, fph);
			if ( !ok )
			{
				free(mData);
				mData = 0;
			}
			else
			{
				mData[mLen] = 0; // zero byte terminate end of file marker.
				mMyAlloc = true;
			}
		}
		fclose(fph);
	}

}

InPlaceParser::~InPlaceParser(void)
{
	if ( mMyAlloc )
	{
		free(mData);
	}
}

#define MAXARGS 512

bool InPlaceParser::IsHard(char c)
{
	return mHard[c] == ST_HARD;
}

char * InPlaceParser::AddHard(NxI32 &argc,const char **argv,char *foo)
{
	while ( IsHard(*foo) )
	{
		const char *hard = &mHardString[*foo*2];
		if ( argc < MAXARGS )
		{
			argv[argc++] = hard;
		}
		foo++;
	}
	return foo;
}

bool   InPlaceParser::IsWhiteSpace(char c)
{
	return mHard[c] == ST_SOFT;
}

char * InPlaceParser::SkipSpaces(char *foo)
{
	while ( !EOS(*foo) && IsWhiteSpace(*foo) ) foo++;
	return foo;
}

bool InPlaceParser::IsNonSeparator(char c)
{
	if ( !IsHard(c) && !IsWhiteSpace(c) && c != 0 ) return true;
	return false;
}


NxI32 InPlaceParser::ProcessLine(NxI32 lineno,char *line,InPlaceParserInterface *callback)
{
	NxI32 ret = 0;

	const char *argv[MAXARGS];
	NxI32 argc = 0;

	char *foo = line;

	while ( !EOS(*foo) && argc < MAXARGS )
	{

		foo = SkipSpaces(foo); // skip any leading spaces

		if ( EOS(*foo) ) break;

		if ( *foo == mQuoteChar ) // if it is an open quote
		{
			foo++;
			if ( argc < MAXARGS )
			{
				argv[argc++] = foo;
			}
			while ( !EOS(*foo) && *foo != mQuoteChar ) foo++;
			if ( !EOS(*foo) )
			{
				*foo = 0; // replace close quote with zero byte EOS
				foo++;
			}
		}
		else
		{

			foo = AddHard(argc,argv,foo); // add any hard separators, skip any spaces

			if ( IsNonSeparator(*foo) )  // add non-hard argument.
			{
				bool quote  = false;
				if ( *foo == mQuoteChar )
				{
					foo++;
					quote = true;
				}

				if ( argc < MAXARGS )
				{
					argv[argc++] = foo;
				}

				if ( quote )
				{
					while (*foo && *foo != mQuoteChar ) foo++;
					if ( *foo ) *foo = 32;
				}

				// continue..until we hit an eos ..
				while ( !EOS(*foo) ) // until we hit EOS
				{
					if ( IsWhiteSpace(*foo) ) // if we hit a space, stomp a zero byte, and exit
					{
						*foo = 0;
						foo++;
						break;
					}
					else if ( IsHard(*foo) ) // if we hit a hard separator, stomp a zero byte and store the hard separator argument
					{
						const char *hard = &mHardString[*foo*2];
						*foo = 0;
						if ( argc < MAXARGS )
						{
							argv[argc++] = hard;
						}
						foo++;
						break;
					}
					foo++;
				} // end of while loop...
			}
		}
	}

	if ( argc )
	{
		ret = callback->ParseLine(lineno, argc, argv );
	}

	return ret;
}

NxI32  InPlaceParser::Parse(InPlaceParserInterface *callback) // returns true if entire file was parsed, false if it aborted for some reason
{
	assert( callback );
	if ( !mData ) return 0;

	NxI32 ret = 0;

	NxI32 lineno = 0;

	char *foo   = mData;
	char *begin = foo;


	while ( *foo )
	{
		if ( *foo == 10 || *foo == 13 )
		{
			lineno++;
			*foo = 0;

			if ( *begin ) // if there is any data to parse at all...
			{
				NxI32 v = ProcessLine(lineno,begin,callback);
				if ( v ) ret = v;
			}

			foo++;
			if ( *foo == 10 ) foo++; // skip line feed, if it is in the carraige-return line-feed format...
			begin = foo;
		}
		else
		{
			foo++;
		}
	}

	lineno++; // lasst line.

	NxI32 v = ProcessLine(lineno,begin,callback);
	if ( v ) ret = v;
	return ret;
}


void InPlaceParser::DefaultSymbols(void)
{
	SetHardSeparator(',');
	SetHardSeparator('(');
	SetHardSeparator(')');
	SetHardSeparator('=');
	SetHardSeparator('[');
	SetHardSeparator(']');
	SetHardSeparator('{');
	SetHardSeparator('}');
	SetCommentSymbol('#');
}


const char ** InPlaceParser::GetArglist(char *line,NxI32 &count) // convert source string into an arg list, this is a destructive parse.
{
	const char **ret = 0;

	static const char *argv[MAXARGS];
	NxI32 argc = 0;

	char *foo = line;

	while ( !EOS(*foo) && argc < MAXARGS )
	{

		foo = SkipSpaces(foo); // skip any leading spaces

		if ( EOS(*foo) ) break;

		if ( *foo == mQuoteChar ) // if it is an open quote
		{
			foo++;
			if ( argc < MAXARGS )
			{
				argv[argc++] = foo;
			}
			while ( !EOS(*foo) && *foo != mQuoteChar ) foo++;
			if ( !EOS(*foo) )
			{
				*foo = 0; // replace close quote with zero byte EOS
				foo++;
			}
		}
		else
		{

			foo = AddHard(argc,argv,foo); // add any hard separators, skip any spaces

			if ( IsNonSeparator(*foo) )  // add non-hard argument.
			{
				bool quote  = false;
				if ( *foo == mQuoteChar )
				{
					foo++;
					quote = true;
				}

				if ( argc < MAXARGS )
				{
					argv[argc++] = foo;
				}

				if ( quote )
				{
					while (*foo && *foo != mQuoteChar ) foo++;
					if ( *foo ) *foo = 32;
				}

				// continue..until we hit an eos ..
				while ( !EOS(*foo) ) // until we hit EOS
				{
					if ( IsWhiteSpace(*foo) ) // if we hit a space, stomp a zero byte, and exit
					{
						*foo = 0;
						foo++;
						break;
					}
					else if ( IsHard(*foo) ) // if we hit a hard separator, stomp a zero byte and store the hard separator argument
					{
						const char *hard = &mHardString[*foo*2];
						*foo = 0;
						if ( argc < MAXARGS )
						{
							argv[argc++] = hard;
						}
						foo++;
						break;
					}
					foo++;
				} // end of while loop...
			}
		}
	}

	count = argc;
	if ( argc )
	{
		ret = argv;
	}

	return ret;
}

/*******************************************************************/
/******************** Geometry.h  ********************************/
/*******************************************************************/

class GeometryVertex
{
public:
	NxF32        mPos[3];
	NxF32        mNormal[3];
	NxF32        mTexel[2];
};


class GeometryInterface
{
public:

	virtual void NodeTriangle(const GeometryVertex *v1,const GeometryVertex *v2,const GeometryVertex *v3, bool textured)
	{
	}

};


/*******************************************************************/
/******************** Obj.h  ********************************/
/*******************************************************************/


class OBJ : public InPlaceParserInterface
{
public:
  NxI32 LoadMesh(const char *fname,GeometryInterface *callback, bool textured);
  NxI32 ParseLine(NxI32 lineno,NxI32 argc,const char **argv);  // return TRUE to continue parsing, return FALSE to abort parsing process
private:

  void GetVertex(GeometryVertex &v,const char *face) const;

  FloatVector     mVerts;
  FloatVector     mTexels;
  FloatVector     mNormals;

  bool            mTextured;

  GeometryInterface *mCallback;
};


/*******************************************************************/
/******************** Obj.cpp  ********************************/
/*******************************************************************/

NxI32 OBJ::LoadMesh(const char *fname,GeometryInterface *iface, bool textured)
{
  mTextured = textured;
  NxI32 ret = 0;

  mVerts.clear();
  mTexels.clear();
  mNormals.clear();

  mCallback = iface;

  InPlaceParser ipp(fname);

  ipp.Parse(this);

return ret;
}

static const char * GetArg(const char **argv,NxI32 i,NxI32 argc)
{
  const char * ret = 0;
  if ( i < argc ) ret = argv[i];
  return ret;
}

void OBJ::GetVertex(GeometryVertex &v,const char *face) const
{
  v.mPos[0] = 0;
  v.mPos[1] = 0;
  v.mPos[2] = 0;

  v.mTexel[0] = 0;
  v.mTexel[1] = 0;

  v.mNormal[0] = 0;
  v.mNormal[1] = 1;
  v.mNormal[2] = 0;

  NxI32 index = atoi( face )-1;

  const char *texel = strstr(face,"/");

  if ( texel )
  {
    NxI32 tindex = atoi( texel+1) - 1;

    if ( tindex >=0 && tindex < (NxI32)(mTexels.size()/2) )
    {
    	const NxF32 *t = &mTexels[tindex*2];

      v.mTexel[0] = t[0];
      v.mTexel[1] = t[1];

    }

    const char *normal = strstr(texel+1,"/");
    if ( normal )
    {
      NxI32 nindex = atoi( normal+1 ) - 1;

      if (nindex >= 0 && nindex < (NxI32)(mNormals.size()/3) )
      {
      	const NxF32 *n = &mNormals[nindex*3];

        v.mNormal[0] = n[0];
        v.mNormal[1] = n[1];
        v.mNormal[2] = n[2];
      }
    }
  }

  if ( index >= 0 && index < (NxI32)(mVerts.size()/3) )
  {

    const NxF32 *p = &mVerts[index*3];

    v.mPos[0] = p[0];
    v.mPos[1] = p[1];
    v.mPos[2] = p[2];
  }

}

NxI32 OBJ::ParseLine(NxI32 lineno,NxI32 argc,const char **argv)  // return TRUE to continue parsing, return FALSE to abort parsing process
{
  NxI32 ret = 0;

  if ( argc >= 1 )
  {
    const char *foo = argv[0];
    if ( *foo != '#' )
    {
      if ( _stricmp(argv[0],"v") == 0 && argc == 4 )
      {
        NxF32 vx = (NxF32) atof( argv[1] );
        NxF32 vy = (NxF32) atof( argv[2] );
        NxF32 vz = (NxF32) atof( argv[3] );
        mVerts.push_back(vx);
        mVerts.push_back(vy);
        mVerts.push_back(vz);
      }
      else if ( _stricmp(argv[0],"vt") == 0 && (argc == 3 || argc == 4))
      {
        // ignore 4rd component if present
        NxF32 tx = (NxF32) atof( argv[1] );
        NxF32 ty = (NxF32) atof( argv[2] );
        mTexels.push_back(tx);
        mTexels.push_back(ty);
      }
      else if ( _stricmp(argv[0],"vn") == 0 && argc == 4 )
      {
        NxF32 normalx = (NxF32) atof(argv[1]);
        NxF32 normaly = (NxF32) atof(argv[2]);
        NxF32 normalz = (NxF32) atof(argv[3]);
        mNormals.push_back(normalx);
        mNormals.push_back(normaly);
        mNormals.push_back(normalz);
      }
      else if ( _stricmp(argv[0],"f") == 0 && argc >= 4 )
      {
        GeometryVertex v[32];

        NxI32 vcount = argc-1;

        for (NxI32 i=1; i<argc; i++)
        {
          GetVertex(v[i-1],argv[i] );
        }

		mCallback->NodeTriangle(&v[0],&v[1],&v[2], mTextured);

        if ( vcount >=3 ) // do the fan
        {
          for (NxI32 i=2; i<(vcount-1); i++)
          {
            mCallback->NodeTriangle(&v[0],&v[i],&v[i+1], mTextured);
          }
        }

      }
    }
  }

  return ret;
}




class BuildMesh : public GeometryInterface
{
public:

	NxI32 GetIndex(const NxF32 *p, const NxF32 *texCoord)
	{

		NxI32 vcount = (NxI32)mVertices.size()/3;

		if(vcount>0)
		{
			//New MS STL library checks indices in debug build, so zero causes an assert if it is empty.
			const NxF32 *v = &mVertices[0];
			const NxF32 *t = texCoord != NULL ? &mTexCoords[0] : NULL;

			for (NxI32 i=0; i<vcount; i++)
			{
				if ( v[0] == p[0] && v[1] == p[1] && v[2] == p[2] )
				{
					if (texCoord == NULL || (t[0] == texCoord[0] && t[1] == texCoord[1]))
					{
						return i;
					}
				}
				v+=3;
				if (t != NULL)
					t += 2;
			}
		}

		mVertices.push_back( p[0] );
		mVertices.push_back( p[1] );
		mVertices.push_back( p[2] );

		if (texCoord != NULL)
		{
			mTexCoords.push_back( texCoord[0] );
			mTexCoords.push_back( texCoord[1] );
		}

		return vcount;
	}

	virtual void NodeTriangle(const GeometryVertex *v1,const GeometryVertex *v2,const GeometryVertex *v3, bool textured)
	{
		mIndices.push_back( GetIndex(v1->mPos, textured ? v1->mTexel : NULL) );
		mIndices.push_back( GetIndex(v2->mPos, textured ? v2->mTexel : NULL) );
		mIndices.push_back( GetIndex(v3->mPos, textured ? v3->mTexel : NULL) );
	}

  const FloatVector& GetVertices(void) const { return mVertices; };
  const FloatVector& GetTexCoords(void) const { return mTexCoords; };
  const IntVector& GetIndices(void) const { return mIndices; };

private:
  FloatVector     mVertices;
  FloatVector     mTexCoords;
  IntVector       mIndices;
};

};

using namespace WAVEFRONT;

WavefrontObj::WavefrontObj(void)
{
	mVertexCount = 0;
	mTriCount    = 0;
	mIndices     = 0;
	mVertices    = NULL;
	mTexCoords   = NULL;
}

WavefrontObj::~WavefrontObj(void)
{
	delete mIndices;
	delete mVertices;
}

NxU32 WavefrontObj::loadObj(const char *fname, bool textured) // load a wavefront obj returns number of triangles that were loaded.  Data is persists until the class is destructed.
{

	NxU32 ret = 0;

	delete mVertices;
	mVertices = 0;
	delete mIndices;
	mIndices = 0;
	mVertexCount = 0;
	mTriCount = 0;


  BuildMesh bm;

  OBJ obj;

  obj.LoadMesh(fname,&bm, textured);


	const FloatVector &vlist = bm.GetVertices();
	const IntVector &indices = bm.GetIndices();
	if ( vlist.size() )
	{
		mVertexCount = (NxI32)vlist.size()/3;
		mVertices = new NxF32[mVertexCount*3];
		memcpy( mVertices, &vlist[0], sizeof(NxF32)*mVertexCount*3 );

		if (textured)
		{
			mTexCoords = new NxF32[mVertexCount * 2];
			const FloatVector& tList = bm.GetTexCoords();
			memcpy( mTexCoords, &tList[0], sizeof(NxF32) * mVertexCount * 2);
		}

		mTriCount = (NxI32)indices.size()/3;
		mIndices = new NxU32[mTriCount*3*sizeof(NxU32)];
		memcpy(mIndices, &indices[0], sizeof(NxU32)*mTriCount*3);
		ret = mTriCount;
	}


	return ret;
}

#endif
