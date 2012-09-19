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

#ifndef _MSGBOX_H_
#define _MSGBOX_H_


// [tom, 10/17/2006] Note: If you change either of these enums, make sure you
// update the relevant code in the all the platform layers.

// [pauls, 3/20/2007] Reduced the available types of dialog boxes in order to
// maintain a consistent but platform - appropriate look and feel in Torque.

enum MBButtons
{
   MBOk,
   MBOkCancel,
   MBRetryCancel,
   MBSaveDontSave,
   MBSaveDontSaveCancel,
};

enum MBIcons
{
   MIWarning,
   MIInformation,
   MIQuestion,
   MIStop,
};

enum MBReturnVal
{
   MROk = 1,   // Start from 1 to allow use of 0 for errors
   MRCancel,
   MRRetry,
   MRDontSave,
};



extern void initMessageBoxVars();

#endif // _MSGBOX_H_
