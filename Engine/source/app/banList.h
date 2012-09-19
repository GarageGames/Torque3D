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

#ifndef _BANLIST_H_
#define _BANLIST_H_

#ifndef _ENGINEAPI_H_
   #include "console/engineAPI.h"
#endif
#ifndef _TVECTOR_H_
   #include "core/util/tVector.h"
#endif

/// Helper class to keep track of bans.
class BanList
{
   public:
   
      DECLARE_STATIC_CLASS( BanList );
   
      struct BanInfo
      {
         S32      uniqueId;
         char     transportAddress[128];
         S32      bannedUntil;
      };
      
   protected:

      Vector< BanInfo > list;
      
      static BanList* smInstance;

   public:

      BanList();
      
      static BanList* instance() { return smInstance; }

      void addBan(S32 uniqueId, const char *TA, S32 banTime);
      void addBanRelative(S32 uniqueId, const char *TA, S32 numSeconds);
      void removeBan(S32 uniqueId, const char *TA);
      bool isBanned(S32 uniqueId, const char *TA);
      bool isTAEq(const char *bannedTA, const char *TA);
      void exportToFile(const char *fileName);
};

#endif
