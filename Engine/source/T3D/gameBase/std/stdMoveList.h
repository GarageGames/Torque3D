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

#ifndef _MOVELIST_STD_H_
#define _MOVELIST_STD_H_

#ifndef _MOVELIST_H_
#include "T3D/gameBase/moveList.h"
#endif

class StdMoveList : public MoveList
{
   typedef MoveList Parent;

public:

   StdMoveList();

   void clientWriteMovePacket(BitStream *);
   void clientReadMovePacket(BitStream *);

   void serverWriteMovePacket(BitStream *);
   void serverReadMovePacket(BitStream *);

   U32 getMoves(Move**,U32* numMoves);
   void clearMoves(U32 count);

   void advanceMove();
   void onAdvanceObjects() {}

protected:

   U32 mMoveCredit;
};

#endif // _MOVELIST_STD_H_
