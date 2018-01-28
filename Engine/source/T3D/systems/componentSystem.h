#pragma once
#include "console/engineAPI.h"

template<typename T>
class SystemInterface
{
public:
   bool mIsEnabled;
   bool mIsServer;

   static Vector<T*> all;

   SystemInterface()
   {
      all.push_back((T*)this);
   }

   virtual ~SystemInterface()
   {
      for (U32 i = 0; i < all.size(); i++)
      {
         if (all[i] == (T*)this)
         {
            all.erase(i);
            return;
         }
      }
   }
};
template<typename T> Vector<T*> SystemInterface<T>::all(0);