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

#include "tResponseCurve.h"

IMPLEMENT_CONOBJECT( SimResponseCurve );

ConsoleDocClass( SimResponseCurve,
				"@brief A ResponseCurve<F32> wrapped as a SimObject.\n\n"
				"Currently no applied use, not network ready, not intended "
				"for game development, for editors or internal use only.\n\n "
				"@internal");

SimResponseCurve::SimResponseCurve()
{

}

bool SimResponseCurve::onAdd()
{
   if ( !Parent::onAdd() )
      return false;

   return true;
}

void SimResponseCurve::onRemove()
{
   Parent::onRemove();
}

void SimResponseCurve::addPoint(F32 value, F32 time)
{
   mCurve.addPoint( value, time );
}

F32 SimResponseCurve::getValue(F32 time)
{
   return mCurve.getVal( time );
}

void SimResponseCurve::clear()
{
   mCurve.clear();
}

ConsoleMethod( SimResponseCurve, addPoint, void, 4, 4, "addPoint( F32 value, F32 time )" )
{
   object->addPoint( dAtof(argv[2]), dAtof(argv[3]) );
}

ConsoleMethod( SimResponseCurve, getValue, F32, 3, 3, "getValue( F32 time )" )
{
   return object->getValue( dAtof(argv[2]) );
}

ConsoleMethod( SimResponseCurve, clear, void, 2, 2, "clear()" )
{
   object->clear();
}