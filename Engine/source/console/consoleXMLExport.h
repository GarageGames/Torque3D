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

#ifndef _CONSOLEXMLEXPORT_H_
#define _CONSOLEXMLEXPORT_H_

#ifndef _TORQUE_STRING_H_
#include "core/util/str.h"
#endif

class SimXMLDocument;

/// @defgroup console_reflection Reflection
/// @ingroup console_system Console System
///
/// Exports console information to XML representation
/// @{

namespace Con {

   class XMLExport
   {

   public:

      XMLExport();
      ~XMLExport();

      // writes console information in XML format to  the file specified
      void exportXML(String& str);

   private:

      void exportBaseTypes();
      void exportEntryTypes();
      void exportNamespaces();

      SimXMLDocument *mXML;

   };

};

/// @}

#endif //_CONSOLEXMLEXPORT_H_
