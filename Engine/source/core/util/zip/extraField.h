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

#ifndef _EXTRAFIELD_H_
#define _EXTRAFIELD_H_

class Stream;

namespace Zip
{

/// @addtogroup zipint_group
/// @ingroup zip_group
// @{


// Forward Refs
class ExtraField;

// Creation Helpers
typedef ExtraField *(*ExtraFieldCreateFn)();

template<class T> ExtraField * createExtraField()
{
   return new T;
}

// ExtraField base class
class ExtraField
{
   ExtraField *mNext;

protected:
   U16 mID;
   ExtraFieldCreateFn mCreateFn;

public:
   ExtraField()
   {
      mID = 0;
      mCreateFn = NULL;
   }
   ExtraField(U16 id, ExtraFieldCreateFn fnCreate);
   
   virtual ~ExtraField() {}
   
   inline U16 getID()                  { return mID; }

   virtual bool read(Stream *stream) = 0;

   // Run time creation methods
   static ExtraField *create(U16 id);
};

#define  DeclareExtraField(name) \
   name(U16 id, ExtraFieldCreateFn fnCreate) : Parent(id, fnCreate) {}

#define ImplementExtraField(name, id)       \
   name gExtraField##name##instance(id, &createExtraField<name>);

// @}

} // end namespace Zip

#endif // _EXTRAFIELD_H_
