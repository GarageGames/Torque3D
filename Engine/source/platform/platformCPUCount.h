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

#ifndef _TORQUE_PLATFORM_PLATFORMCPUCOUNT_H_
#define _TORQUE_PLATFORM_PLATFORMCPUCOUNT_H_

#include "platform/platform.h"

namespace CPUInfo
{
   enum EConfig
   {
      CONFIG_UserConfigIssue,
      CONFIG_SingleCoreHTEnabled,
      CONFIG_SingleCoreHTDisabled,
      CONFIG_SingleCoreAndHTNotCapable,
      CONFIG_MultiCoreAndHTNotCapable,
      CONFIG_MultiCoreAndHTEnabled,
      CONFIG_MultiCoreAndHTDisabled,
   };

   inline bool isMultiCore( EConfig config )
   {
      switch( config )
      {
      case CONFIG_MultiCoreAndHTNotCapable:
      case CONFIG_MultiCoreAndHTEnabled:
      case CONFIG_MultiCoreAndHTDisabled:
         return true;

      default:
         return false;
      }
   }

   inline bool isHyperThreaded( EConfig config )
   {
      switch( config )
      {
      case CONFIG_SingleCoreHTEnabled:
      case CONFIG_MultiCoreAndHTEnabled:
         return true;

      default:
         return false;
      }
   }

   EConfig CPUCount( U32& totalAvailableLogical,
      U32& totalAvailableCores,
      U32& numPhysical );

} // namespace CPUInfo

#endif // _TORQUE_PLATFORM_PLATFORMCOUNT_H_

