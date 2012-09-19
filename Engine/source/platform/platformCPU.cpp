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

#include "platform/platform.h"
#include "platform/platformCPUCount.h"
#include "core/strings/stringFunctions.h"
#include "core/stringTable.h"
#include "core/util/tSignal.h"

Signal<void(void)> Platform::SystemInfoReady;

enum CPUFlags
{
   BIT_FPU     = BIT(0),
   BIT_RDTSC   = BIT(4),
   BIT_MMX     = BIT(23),
   BIT_SSE     = BIT(25),
   BIT_SSE2    = BIT(26),
   BIT_3DNOW   = BIT(31),

   // These use a different value for comparison than the above flags
   BIT_SSE3    = BIT(0),
   BIT_SSE3xt  = BIT(9),
   BIT_SSE4_1  = BIT(19),
   BIT_SSE4_2  = BIT(20),
};

// fill the specified structure with information obtained from asm code
void SetProcessorInfo(Platform::SystemInfo_struct::Processor& pInfo,
   char* vendor, U32 processor, U32 properties, U32 properties2)
{
   Platform::SystemInfo.processor.properties |= (properties & BIT_FPU)   ? CPU_PROP_FPU : 0;
   Platform::SystemInfo.processor.properties |= (properties & BIT_RDTSC) ? CPU_PROP_RDTSC : 0;
   Platform::SystemInfo.processor.properties |= (properties & BIT_MMX)   ? CPU_PROP_MMX : 0;

   if (dStricmp(vendor, "GenuineIntel") == 0)
   {
      pInfo.properties |= (properties & BIT_SSE) ? CPU_PROP_SSE : 0;
      pInfo.properties |= (properties & BIT_SSE2) ? CPU_PROP_SSE2 : 0;
      pInfo.properties |= (properties2 & BIT_SSE3) ? CPU_PROP_SSE3 : 0;
      pInfo.properties |= (properties2 & BIT_SSE3xt) ? CPU_PROP_SSE3xt : 0;
      pInfo.properties |= (properties2 & BIT_SSE4_1) ? CPU_PROP_SSE4_1 : 0;
      pInfo.properties |= (properties2 & BIT_SSE4_2) ? CPU_PROP_SSE4_2 : 0;

      pInfo.type = CPU_Intel_Unknown;
      // switch on processor family code
      switch ((processor >> 8) & 0x0f)
      {
         case 4:
            pInfo.type = CPU_Intel_486;
            pInfo.name = StringTable->insert("Intel 486 class");
            break;

            // Pentium Family
         case 5:
            // switch on processor model code
            switch ((processor >> 4) & 0xf)
            {
               case 1:
               case 2:
               case 3:
                  pInfo.type = CPU_Intel_Pentium;
                  pInfo.name = StringTable->insert("Intel Pentium");
                  break;
               case 4:
                  pInfo.type = CPU_Intel_PentiumMMX;
                  pInfo.name = StringTable->insert("Intel Pentium MMX");
                  break;
               default:
                  pInfo.type = CPU_Intel_Pentium;
                  pInfo.name = StringTable->insert( "Intel (unknown)" );
                  break;
            }
            break;

            // Pentium Pro/II/II family
         case 6:
         {
            U32 extendedModel = ( processor & 0xf0000 ) >> 16;
            // switch on processor model code
            switch ((processor >> 4) & 0xf)
            {
               case 1:
                  pInfo.type = CPU_Intel_PentiumPro;
                  pInfo.name = StringTable->insert("Intel Pentium Pro");
                  break;
               case 3:
               case 5:
                  pInfo.type = CPU_Intel_PentiumII;
                  pInfo.name = StringTable->insert("Intel Pentium II");
                  break;
               case 6:
                  pInfo.type = CPU_Intel_PentiumCeleron;
                  pInfo.name = StringTable->insert("Intel Pentium Celeron");
                  break;
               case 7:
               case 8:
               case 11:
                  pInfo.type = CPU_Intel_PentiumIII;
                  pInfo.name = StringTable->insert("Intel Pentium III");
                  break;
               case 0xA:
                  if( extendedModel == 1)
                  {
                     pInfo.type = CPU_Intel_Corei7Xeon;
                     pInfo.name = StringTable->insert( "Intel Core i7 / Xeon" );
                  }
                  else
                  {
                     pInfo.type = CPU_Intel_PentiumIII;
                     pInfo.name = StringTable->insert( "Intel Pentium III Xeon" );
                  }
                  break;
               case 0xD:
                  if( extendedModel == 1 )
                  {
                     pInfo.type = CPU_Intel_Corei7Xeon;
                     pInfo.name = StringTable->insert( "Intel Core i7 / Xeon" );
                  }
                  else
                  {
                     pInfo.type = CPU_Intel_PentiumM;
                     pInfo.name = StringTable->insert( "Intel Pentium/Celeron M" );
                  }
                  break;
               case 0xE:
                  pInfo.type = CPU_Intel_Core;
                  pInfo.name = StringTable->insert( "Intel Core" );
                  break;
               case 0xF:
                  pInfo.type = CPU_Intel_Core2;
                  pInfo.name = StringTable->insert( "Intel Core 2" );
                  break;
               default:
                  pInfo.type = CPU_Intel_PentiumPro;
                  pInfo.name = StringTable->insert( "Intel (unknown)" );
                  break;
            }
            break;
         }

            // Pentium4 Family
         case 0xf:
            pInfo.type = CPU_Intel_Pentium4;
            pInfo.name = StringTable->insert( "Intel Pentium 4" );
            break;

         default:
            pInfo.type = CPU_Intel_Unknown;
            pInfo.name = StringTable->insert( "Intel (unknown)" );
            break;
      }
   }
   //--------------------------------------
   else
      if (dStricmp(vendor, "AuthenticAMD") == 0)
      {
         // AthlonXP processors support SSE
         pInfo.properties |= (properties & BIT_SSE) ? CPU_PROP_SSE : 0;
         pInfo.properties |= ( properties & BIT_SSE2 ) ? CPU_PROP_SSE2 : 0;
         pInfo.properties |= (properties & BIT_3DNOW) ? CPU_PROP_3DNOW : 0;
         // switch on processor family code
         switch ((processor >> 8) & 0xf)
         {
            // K6 Family
            case 5:
               // switch on processor model code
               switch ((processor >> 4) & 0xf)
               {
                  case 0:
                  case 1:
                  case 2:
                  case 3:
                     pInfo.type = CPU_AMD_K6_3;
                     pInfo.name = StringTable->insert("AMD K5");
                     break;
                  case 4:
                  case 5:
                  case 6:
                  case 7:
                     pInfo.type = CPU_AMD_K6;
                     pInfo.name = StringTable->insert("AMD K6");
                     break;
                  case 8:
                     pInfo.type = CPU_AMD_K6_2;
                     pInfo.name = StringTable->insert("AMD K6-2");
                     break;
                  case 9:
                  case 10:
                  case 11:
                  case 12:
                  case 13:
                  case 14:
                  case 15:
                     pInfo.type = CPU_AMD_K6_3;
                     pInfo.name = StringTable->insert("AMD K6-3");
                     break;
               }
               break;

               // Athlon Family
            case 6:
               pInfo.type = CPU_AMD_Athlon;
               pInfo.name = StringTable->insert("AMD Athlon");
               break;

            default:
               pInfo.type = CPU_AMD_Unknown;
               pInfo.name = StringTable->insert("AMD (unknown)");
               break;
         }
      }
   //--------------------------------------
      else
         if (dStricmp(vendor, "CyrixInstead") == 0)
         {
            switch (processor)
            {
               case 0x520:
                  pInfo.type = CPU_Cyrix_6x86;
                  pInfo.name = StringTable->insert("Cyrix 6x86");
                  break;
               case 0x440:
                  pInfo.type = CPU_Cyrix_MediaGX;
                  pInfo.name = StringTable->insert("Cyrix Media GX");
                  break;
               case 0x600:
                  pInfo.type = CPU_Cyrix_6x86MX;
                  pInfo.name = StringTable->insert("Cyrix 6x86mx/MII");
                  break;
               case 0x540:
                  pInfo.type = CPU_Cyrix_GXm;
                  pInfo.name = StringTable->insert("Cyrix GXm");
                  break;
               default:
                  pInfo.type = CPU_Cyrix_Unknown;
                  pInfo.name = StringTable->insert("Cyrix (unknown)");
                  break;
            }
         }

   // Get multithreading caps.

   CPUInfo::EConfig config = CPUInfo::CPUCount( pInfo.numLogicalProcessors, pInfo.numAvailableCores, pInfo.numPhysicalProcessors );
   pInfo.isHyperThreaded = CPUInfo::isHyperThreaded( config );
   pInfo.isMultiCore = CPUInfo::isMultiCore( config );

   // Trigger the signal
   Platform::SystemInfoReady.trigger();
}
