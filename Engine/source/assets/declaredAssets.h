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

#ifndef _DECLARED_ASSETS_H_
#define _DECLARED_ASSETS_H_

#ifndef _SIM_H_
#include "console/sim.h"
#endif

#ifndef _SIMOBJECT_H_
#include "console/simObject.h"
#endif

#ifndef _CONSOLEOBJECT_H_
#include "console/consoleObject.h"
#endif

//-----------------------------------------------------------------------------

class DeclaredAssets : public SimObject
{
    friend class AssetManager;

private:
    typedef SimObject Parent;

    StringTableEntry    mPath;
    StringTableEntry    mExtension;
    bool                mRecurse;

public:
    DeclaredAssets() :
        mPath( StringTable->EmptyString() ),
        mExtension(StringTable->EmptyString()),
        mRecurse( false )
        {}
    virtual ~DeclaredAssets() {}

    static void initPersistFields();

    inline void setPath( const char* pPath )            { mPath = StringTable->insert( pPath ); }
    inline StringTableEntry getPath( void ) const       { return mPath; }
    inline void setExtension( const char* pPath )       { mExtension = StringTable->insert( pPath ); }
    inline StringTableEntry getExtension( void ) const  { return mExtension; }
    inline void setRecurse( const bool recurse )        { mRecurse = recurse; }
    inline bool getRecurse( void ) const                { return mRecurse; }

    /// Declare Console Object.
    DECLARE_CONOBJECT( DeclaredAssets );
};

#endif // _DECLARED_ASSETS_H_

