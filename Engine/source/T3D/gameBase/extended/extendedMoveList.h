#ifndef _EXTENDEDMOVELIST_H_
#define _EXTENDEDMOVELIST_H_

#ifndef _TVECTOR_H_
#include "core/util/tVector.h"
#endif
#ifndef _MOVELIST_H_
#include "T3D/gameBase/moveList.h"
#endif

#include "T3D/gameBase/extended/extendedMove.h"

class ExtendedMoveList : public MoveList
{
   typedef MoveList Parent;

public:

   ExtendedMoveList();

   void clientWriteMovePacket(BitStream *);
   void clientReadMovePacket(BitStream *);

   void serverWriteMovePacket(BitStream *);
   void serverReadMovePacket(BitStream *);

   void advanceMove();
   void onAdvanceObjects() {}
   U32 getMoves(Move** movePtr,U32* numMoves);
   U32 getExtMoves( ExtendedMove** movePtr, U32 *numMoves );

   void collectMove();
   void pushMove( const Move &mv );
   void pushExtMove( const ExtendedMove &mv );
   void clearMoves( U32 count );

   virtual void reset();

   bool isBacklogged();

   bool areMovesPending();

   void ackMoves( U32 count );

protected:

   U32 mMoveCredit;

   Vector<ExtendedMove> mExtMoveVec;

protected:
   bool getNextExtMove( ExtendedMove &curMove );
};

#endif   // _EXTENDEDMOVELIST_H_
