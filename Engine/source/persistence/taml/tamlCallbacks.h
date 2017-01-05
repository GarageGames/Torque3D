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

#ifndef _TAML_CALLBACKS_H_
#define _TAML_CALLBACKS_H_

//-----------------------------------------------------------------------------

class TamlCustomNodes;
class SimObject;

//-----------------------------------------------------------------------------

class TamlCallbacks
{
    friend class Taml;

private:
    /// Called prior to Taml writing the object.
    virtual void onTamlPreWrite( void ) = 0;

    /// Called after Taml has finished writing the object.
    virtual void onTamlPostWrite( void ) = 0;

    /// Called prior to Taml reading the object.
    virtual void onTamlPreRead( void ) = 0;

    /// Called after Taml has finished reading the object.
    /// The custom properties is additionally passed here for object who want to process it at the end of reading.
    virtual void onTamlPostRead( const TamlCustomNodes& customNodes ) = 0;

    /// Called after Taml has finished reading the object and has added the object to any parent.
    virtual void onTamlAddParent( SimObject* pParentObject ) = 0;

    /// Called during the writing of the object to allow custom properties to be written.
    virtual void onTamlCustomWrite( TamlCustomNodes& customNodes ) = 0;

    /// Called during the reading of the object to allow custom properties to be read.
    virtual void onTamlCustomRead( const TamlCustomNodes& customNodes ) = 0;
};

#endif // _TAML_CALLBACKS_H_