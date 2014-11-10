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
#include "console/engineAPI.h"

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

DefineConsoleMethod( SimResponseCurve, addPoint, void, ( F32 value, F32 time ), , "addPoint( F32 value, F32 time )" )
{
   object->addPoint( value, time );
}

DefineConsoleMethod( SimResponseCurve, getValue, F32, ( F32 time ), , "getValue( F32 time )" )
{
   return object->getValue( time );
}

DefineConsoleMethod( SimResponseCurve, clear, void, (), , "clear()" )
{
   object->clear();
}