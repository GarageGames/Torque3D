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

#ifndef _FIELDBRUSHOBJECT_H_
#define _FIELDBRUSHOBJECT_H_

#ifndef _SIM_H_
   #include "console/simObject.h"
#endif
#ifndef _SIMFIELDDICTIONARY_H_
   #include "console/simFieldDictionary.h"
#endif
#ifndef _CONSOLEINTERNAL_H_
   #include "console/consoleInternal.h"
#endif
#ifndef _TDICTIONARY_H_
   #include "core/util/tDictionary.h"
#endif


//-----------------------------------------------------------------------------
// Field Brush Object.
//-----------------------------------------------------------------------------

/// FieldBrushObject for static-field copying/pasting.
///
class FieldBrushObject : public SimObject
{
private:
    typedef SimObject Parent;

    // Destroy Fields.
    void destroyFields( void );

    StringTableEntry    mDescription;                   ///< Description.
    StringTableEntry    mSortName;                      ///< Sort Name.

public:
    FieldBrushObject();

    void copyFields( SimObject* pSimObject, const char* fieldList );
    void pasteFields( SimObject* pSimObject );
    
    static bool setDescription( void *object, const char *index, const char *data ) 
      { static_cast<FieldBrushObject*>(object)->setDescription(data); return false; };
    void setDescription( const char* description )  { mDescription = StringTable->insert(description); }
    StringTableEntry getDescription(void) const     { return mDescription; }

    static bool setSortName( void *object, const char *index, const char *data ) 
      { static_cast<FieldBrushObject*>(object)->setSortName(data); return false; };
    void setSortName( const char* sortName )  { mSortName = StringTable->insert(sortName); }
    StringTableEntry getSortName(void) const     { return mSortName; }

    static void initPersistFields();                    ///< Persist Fields.
    virtual void onRemove();                            ///< Called when the object is removed from the sim.

    DECLARE_CONOBJECT(FieldBrushObject);
};

#endif