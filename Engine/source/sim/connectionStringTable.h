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

#ifndef _H_CONNECTIONSTRINGTABLE
#define _H_CONNECTIONSTRINGTABLE

/// Maintain a table of strings which are shared across the network.
///
/// This allows us to reference strings in our network streams more efficiently.
class ConnectionStringTable
{
public:
   enum Constants {
      EntryCount = 32,
      EntryBitSize = 5,
      InvalidEntryId = 32,
   };
private:
   struct Entry {
      NetStringHandle string;   ///< Global string table entry of this string
                             ///  will be 0 if this string is unused.

      U32 index;             ///< index of this entry
      Entry *nextHash;       ///< the next hash entry for this id
      Entry *nextLink;       ///< the next in the LRU list
      Entry *prevLink;       ///< the prev entry in the LRU list
      bool receiveConfirmed; ///< The other side now has this string.
   };

   Entry mEntryTable[EntryCount];
   Entry *mHashTable[EntryCount];
   NetStringHandle mRemoteStringTable[EntryCount];
   Entry mLRUHead, mLRUTail;

   /// Connection over which we are maintaining this string table.
   NetConnection *mParent;

   inline void pushBack(Entry *entry) // pushes an entry to the back of the LRU list
   {
      entry->prevLink->nextLink = entry->nextLink;
      entry->nextLink->prevLink = entry->prevLink;
      entry->nextLink = &mLRUTail;
      entry->prevLink = mLRUTail.prevLink;
      entry->nextLink->prevLink = entry;
      entry->prevLink->nextLink = entry;
   }

public:
   /// Initialize the connection string table.
   ///
   /// @param  parent   Connection over which we are maintaining this string table.
   ConnectionStringTable(NetConnection *parent);

   /// Has the specified string been received on the other side?
   inline void confirmStringReceived(NetStringHandle &string, U32 index)
   {
      if(mEntryTable[index].string == string)
         mEntryTable[index].receiveConfirmed = true;
   }

   U32 checkString(NetStringHandle &stringTableId, bool *stringOnOtherSide = NULL);  ///< Checks if the global string ID is
                                                                                  ///  currently valid for this connection
                                                                                  ///  and returns the table ID.
                                                                                  ///  Sends a string event to the other side
                                                                                  ///  if it is not active.
                                                                                  ///  It will fill in stringOnOtherSide.

   U32 getNetSendId(NetStringHandle &stringTableId); ///< Same return value as checkString
                                                  ///  but will assert if the string is not
                                                  ///  valid.

   void mapString(U32 netId, NetStringHandle &string); ///< Maps a string that
                                                    ///  was just sent over the net
                                                    ///  to the corresponding net ID.

   inline NetStringHandle lookupString(U32 netId) ///< looks up the string ID and returns
   {                                           /// the global string table ID for that string.
      return mRemoteStringTable[netId];
   }

   /// @name Demo functionality
   /// @{

   void readDemoStartBlock(BitStream *stream);
   void writeDemoStartBlock(ResizeBitStream *stream);
   /// @}
};

#endif

