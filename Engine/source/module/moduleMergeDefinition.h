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

#ifndef _MODULE_MERGE_DEFINITION_H
#define _MODULE_MERGE_DEFINITION_H

#ifndef _SIMBASE_H_
#include "console/simBase.h"
#endif

//-----------------------------------------------------------------------------

class ModuleMergeDefinition : public SimObject
{
private:
    typedef SimObject Parent;

    /// Module update
    StringTableEntry        mModuleMergePath;

public:
    ModuleMergeDefinition();
    virtual ~ModuleMergeDefinition() {}

    /// Engine.
    static void             initPersistFields();

    /// Module merge.
    inline void             setModuleMergePath( const char* pModuleMergePath )          { mModuleMergePath = StringTable->insert(pModuleMergePath); }
    inline StringTableEntry getModuleMergePath( void ) const                            { return mModuleMergePath; }

    /// Declare Console Object.
    DECLARE_CONOBJECT( ModuleMergeDefinition );
};

#endif // _MODULE_MERGE_DEFINITION_H

