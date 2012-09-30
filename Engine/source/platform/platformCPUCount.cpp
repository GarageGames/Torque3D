// Original code is:
// Copyright (c) 2005 Intel Corporation 
// All Rights Reserved
//
// CPUCount.cpp : Detects three forms of hardware multi-threading support across IA-32 platform
//					The three forms of HW multithreading are: Multi-processor, Multi-core, and 
//					HyperThreading Technology.
//					This application enumerates all the logical processors enabled by OS and BIOS,
//					determine the HW topology of these enabled logical processors in the system 
//					using information provided by CPUID instruction.
//					A multi-processing system can support any combination of the three forms of HW
//					multi-threading support. The relevant topology can be identified using a 
//					three level decomposition of the "initial APIC ID" into 
//					Package_id, core_id, and SMT_id. Such decomposition provides a three-level map of 
//					the topology of hardware resources and
//					allow multi-threaded software to manage shared hardware resources in 
//					the platform to reduce resource contention

//					Multicore detection algorithm for processor and cache topology requires
//					all leaf functions of CPUID instructions be available. System administrator
//					must ensure BIOS settings is not configured to restrict CPUID functionalities.
//-------------------------------------------------------------------------------------------------

#include "platform/platform.h"
#include "platform/platformCPUCount.h"

// Consoles don't need this
#if defined(TORQUE_OS_XENON) || defined(TORQUE_OS_PS3)
namespace CPUInfo 
{

EConfig CPUCount(U32& TotAvailLogical, U32& TotAvailCore, U32& PhysicalNum)
{
   TotAvailLogical = 6;
   TotAvailCore = 6;
   PhysicalNum = 3;

   return CONFIG_MultiCoreAndHTEnabled;
}

}; // namespace
#else

#ifdef TORQUE_OS_LINUX
// 	The Linux source code listing can be compiled using Linux kernel verison 2.6 
//	or higher (e.g. RH 4AS-2.8 using GCC 3.4.4). 
//	Due to syntax variances of Linux affinity APIs with earlier kernel versions 
//	and dependence on glibc library versions, compilation on Linux environment 
//	with older kernels and compilers may require kernel patches or compiler upgrades.

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sched.h>
#define DWORD unsigned long
#elif defined( TORQUE_OS_WIN32 )
#include <windows.h>
#elif defined( TORQUE_OS_MAC )
#  include <sys/types.h>
#  include <sys/sysctl.h>
#else
#error Not implemented on platform.
#endif
#include <stdio.h>
#include <assert.h>

namespace CPUInfo {

#define HWD_MT_BIT         0x10000000     // EDX[28]  Bit 28 is set if HT or multi-core is supported
#define NUM_LOGICAL_BITS   0x00FF0000     // EBX[23:16] Bit 16-23 in ebx contains the number of logical
      // processors per physical processor when execute cpuid with 
      // eax set to 1
#define NUM_CORE_BITS      0xFC000000     // EAX[31:26] Bit 26-31 in eax contains the number of cores minus one
      // per physical processor when execute cpuid with 
      // eax set to 4. 


#define INITIAL_APIC_ID_BITS  0xFF000000  // EBX[31:24] Bits 24-31 (8 bits) return the 8-bit unique 
      // initial APIC ID for the processor this code is running on.


      #ifndef TORQUE_OS_MAC
      static unsigned int  CpuIDSupported(void);      
      static unsigned int  find_maskwidth(unsigned int);
      static unsigned int  HWD_MTSupported(void);
      static unsigned int  MaxLogicalProcPerPhysicalProc(void);
      static unsigned int  MaxCorePerPhysicalProc(void);
      static unsigned char GetAPIC_ID(void);
      static unsigned char GetNzbSubID(unsigned char, unsigned char, unsigned char);
      #endif

      static char g_s3Levels[2048];

#ifndef TORQUE_OS_MAC

      //
      // CpuIDSupported will return 0 if CPUID instruction is unavailable. Otherwise, it will return 
      // the maximum supported standard function.
      //
      static unsigned int CpuIDSupported(void)
      {
         unsigned int MaxInputValue;
         // If CPUID instruction is supported
#ifdef TORQUE_COMPILER_GCC
         try    
         {		
            MaxInputValue = 0;
            // call cpuid with eax = 0
            asm
               (
               "pushl %%ebx\n\t"
               "xorl %%eax,%%eax\n\t"
               "cpuid\n\t"
               "popl %%ebx\n\t"
               : "=a" (MaxInputValue)
               : 
               : "%ecx", "%edx"
               );		
         }
         catch (...)
         {
            return(0);                   // cpuid instruction is unavailable
         }
#elif defined( TORQUE_COMPILER_VISUALC )
         try
         {
            MaxInputValue = 0;
            // call cpuid with eax = 0
            __asm
            {
               xor eax, eax
                  cpuid
                  mov MaxInputValue, eax
            }
         }
         catch (...)
         {
            return(0);                   // cpuid instruction is unavailable
         }
#else
#  error Not implemented.
#endif

         return MaxInputValue;

      }



      //
      // Function returns the maximum cores per physical package. Note that the number of 
      // AVAILABLE cores per physical to be used by an application might be less than this
      // maximum value.
      //

      static unsigned int MaxCorePerPhysicalProc(void)
      {

         unsigned int Regeax        = 0;

         if (!HWD_MTSupported()) return (unsigned int) 1;  // Single core
#ifdef TORQUE_COMPILER_GCC
         {
            asm
               (
               "pushl %ebx\n\t"
               "xorl %eax, %eax\n\t"
               "cpuid\n\t"
               "cmpl $4, %eax\n\t"			// check if cpuid supports leaf 4
               "jl .single_core\n\t"		// Single core
               "movl $4, %eax\n\t"		
               "movl $0, %ecx\n\t"			// start with index = 0; Leaf 4 reports
               "popl %ebx\n\t"
               );								// at least one valid cache level
            asm
               (
               "cpuid"
               : "=a" (Regeax)
               :
               : "%ecx", "%edx"
               );		
            asm
               (
               "jmp .multi_core\n"
               ".single_core:\n\t"
               "xor %eax, %eax\n"
               ".multi_core:"
               );		
         }
#elif defined( TORQUE_COMPILER_VISUALC )
         __asm
         {
            xor eax, eax
               cpuid
               cmp eax, 4			// check if cpuid supports leaf 4
               jl single_core		// Single core
               mov eax, 4			
               mov ecx, 0			// start with index = 0; Leaf 4 reports
               cpuid				// at least one valid cache level
               mov Regeax, eax
               jmp multi_core

single_core:
            xor eax, eax		

multi_core:

         }
#else
#  error Not implemented.
#endif
         return (unsigned int)((Regeax & NUM_CORE_BITS) >> 26)+1;

      }



      //
      // The function returns 0 when the hardware multi-threaded bit is not set.
      //
      static unsigned int HWD_MTSupported(void)
      {


         unsigned int Regedx      = 0;


         if ((CpuIDSupported() >= 1))
         {
#ifdef TORQUE_COMPILER_GCC
            asm 
               (
               "pushl %%ebx\n\t"
               "movl $1,%%eax\n\t"
               "cpuid\n\t"
               "popl %%ebx\n\t"
               : "=d" (Regedx)
               :
               : "%eax","%ecx"
               );
#elif defined( TORQUE_COMPILER_VISUALC )
            __asm
            {
               mov eax, 1
                  cpuid
                  mov Regedx, edx
            }		
#else
#  error Not implemented.
#endif
         }

         return (Regedx & HWD_MT_BIT);  


      }



      //
      // Function returns the maximum logical processors per physical package. Note that the number of 
      // AVAILABLE logical processors per physical to be used by an application might be less than this
      // maximum value.
      //
      static unsigned int MaxLogicalProcPerPhysicalProc(void)
      {

         unsigned int Regebx = 0;

         if (!HWD_MTSupported()) return (unsigned int) 1;
#ifdef TORQUE_COMPILER_GCC
         asm 
            (
            "movl $1,%%eax\n\t"
            "cpuid"
            : "=b" (Regebx)
            :
            : "%eax","%ecx","%edx"
            );
#elif defined( TORQUE_COMPILER_VISUALC )
         __asm
         {
            mov eax, 1
               cpuid
               mov Regebx, ebx
         }
#else
#  error Not implemented.
#endif
         return (unsigned int) ((Regebx & NUM_LOGICAL_BITS) >> 16);

      }


      static unsigned char GetAPIC_ID(void)
      {

         unsigned int Regebx = 0;
#ifdef TORQUE_COMPILER_GCC
         asm
            (
            "movl $1, %%eax\n\t"	
            "cpuid"
            : "=b" (Regebx) 
            :
            : "%eax","%ecx","%edx" 
            );

#elif defined( TORQUE_COMPILER_VISUALC )
         __asm
         {
            mov eax, 1
               cpuid
               mov Regebx, ebx
         }
#else
#  error Not implemented.
#endif                                

         return (unsigned char) ((Regebx & INITIAL_APIC_ID_BITS) >> 24);

      }

      //
      // Determine the width of the bit field that can represent the value count_item. 
      //
      unsigned int find_maskwidth(unsigned int CountItem)
      {
         unsigned int MaskWidth,
            count = CountItem;
#ifdef TORQUE_COMPILER_GCC
         asm
            (
#ifdef __x86_64__		// define constant to compile  
            "push %%rcx\n\t"		// under 64-bit Linux
            "push %%rax\n\t"
#else
            "pushl %%ecx\n\t"
            "pushl %%eax\n\t"
#endif
            //		"movl $count, %%eax\n\t" //done by Assembler below
            "xorl %%ecx, %%ecx"
            //		"movl %%ecx, MaskWidth\n\t" //done by Assembler below
            : "=c" (MaskWidth)
            : "a" (count)
            //		: "%ecx", "%eax" We don't list these as clobbered because we don't want the assembler
            //to put them back when we are done
            );
         asm
            (
            "decl %%eax\n\t"
            "bsrw %%ax,%%cx\n\t"
            "jz next\n\t"
            "incw %%cx\n\t"
            //		"movl %%ecx, MaskWidth\n" //done by Assembler below
            : "=c" (MaskWidth)
            :
         );
         asm
            (
            "next:\n\t"
#ifdef __x86_64__
            "pop %rax\n\t"
            "pop %rcx"		
#else
            "popl %eax\n\t"
            "popl %ecx"		
#endif
            );

#elif defined( TORQUE_COMPILER_VISUALC )
         __asm
         {
            mov eax, count
               mov ecx, 0
               mov MaskWidth, ecx
               dec eax
               bsr cx, ax
               jz next
               inc cx
               mov MaskWidth, ecx
next:

         }
#else
#  error Not implemented.
#endif
         return MaskWidth;
      }


      //
      // Extract the subset of bit field from the 8-bit value FullID.  It returns the 8-bit sub ID value
      //
      static unsigned char GetNzbSubID(unsigned char FullID,
         unsigned char MaxSubIDValue,
         unsigned char ShiftCount)
      {
         unsigned int MaskWidth;
         unsigned char MaskBits;

         MaskWidth = find_maskwidth((unsigned int) MaxSubIDValue);
         MaskBits  = (0xff << ShiftCount) ^ 
            ((unsigned char) (0xff << (ShiftCount + MaskWidth)));

         return (FullID & MaskBits);
      }

#endif


      //
      //
      //
      EConfig CPUCount(U32& TotAvailLogical, U32& TotAvailCore, U32& PhysicalNum)
      {
         EConfig StatusFlag = CONFIG_UserConfigIssue;

         g_s3Levels[0] = 0;
         TotAvailCore = 1;
         PhysicalNum  = 1;
         
         unsigned int numLPEnabled = 0;
         int MaxLPPerCore = 1;

#ifdef TORQUE_OS_MAC

         //FIXME: This isn't a proper port but more or less just some sneaky cheating
         //  to get around having to mess with yet another crap UNIX-style API.  Seems
         //  like there isn't a way to do this that's working across all OSX incarnations
         //  and machine configurations anyway.

         int numCPUs;
         int numPackages;

         // Get the number of CPUs.

         size_t len = sizeof( numCPUs );
         if( sysctlbyname( "hw.ncpu", &numCPUs, &len, 0, 0 ) == -1 )
            return CONFIG_UserConfigIssue;

         // Get the number of packages.
         len = sizeof( numPackages );
         if( sysctlbyname( "hw.packages", &numPackages, &len, 0, 0 ) == -1 )
            return CONFIG_UserConfigIssue;

         TotAvailCore = numCPUs;
         TotAvailLogical = numCPUs;
         PhysicalNum = numPackages;
#else

         U32 dwAffinityMask;
         int j = 0;
         unsigned char apicID, PackageIDMask;
         unsigned char tblPkgID[256], tblCoreID[256], tblSMTID[256];
         char	tmp[256];

#ifdef TORQUE_OS_LINUX
         //we need to make sure that this process is allowed to run on 
         //all of the logical processors that the OS itself can run on.
         //A process could acquire/inherit affinity settings that restricts the 
         // current process to run on a subset of all logical processor visible to OS.

         // Linux doesn't easily allow us to look at the Affinity Bitmask directly,
         // but it does provide an API to test affinity maskbits of the current process 
         // against each logical processor visible under OS.
         int sysNumProcs = sysconf(_SC_NPROCESSORS_CONF); //This will tell us how many 
         //CPUs are currently enabled.

         //this will tell us which processors this process can run on. 
         cpu_set_t allowedCPUs;	 
         sched_getaffinity(0, sizeof(allowedCPUs), &allowedCPUs);

         for (int i = 0; i < sysNumProcs; i++ )
         {
            if ( CPU_ISSET(i, &allowedCPUs) == 0 )
               return CONFIG_UserConfigIssue;
         }
#elif defined( TORQUE_OS_WIN32 )
         DWORD dwProcessAffinity, dwSystemAffinity;
         GetProcessAffinityMask(GetCurrentProcess(), 
            &dwProcessAffinity,
            &dwSystemAffinity);
         if (dwProcessAffinity != dwSystemAffinity)  // not all CPUs are enabled
            return CONFIG_UserConfigIssue;
#else
#  error Not implemented.
#endif

         // Assume that cores within a package have the SAME number of 
         // logical processors.  Also, values returned by
         // MaxLogicalProcPerPhysicalProc and MaxCorePerPhysicalProc do not have
         // to be power of 2.

         MaxLPPerCore = MaxLogicalProcPerPhysicalProc() / MaxCorePerPhysicalProc();
         dwAffinityMask = 1;

#ifdef TORQUE_OS_LINUX
         cpu_set_t currentCPU;
         while ( j < sysNumProcs )
         {
            CPU_ZERO(&currentCPU);
            CPU_SET(j, &currentCPU);
            if ( sched_setaffinity (0, sizeof(currentCPU), &currentCPU) == 0 )
            {
               sleep(0);  // Ensure system to switch to the right CPU
#elif defined( TORQUE_OS_WIN32 )
         while (dwAffinityMask && dwAffinityMask <= dwSystemAffinity)
         {
            if (SetThreadAffinityMask(GetCurrentThread(), dwAffinityMask))
            {
               Sleep(0);  // Ensure system to switch to the right CPU
#else
#  error Not implemented.
#endif
               apicID = GetAPIC_ID();


               // Store SMT ID and core ID of each logical processor
               // Shift vlaue for SMT ID is 0
               // Shift value for core ID is the mask width for maximum logical
               // processors per core

               tblSMTID[j]  = GetNzbSubID(apicID, MaxLPPerCore, 0);
               unsigned char maxCorePPP = MaxCorePerPhysicalProc();
               unsigned char maskWidth = find_maskwidth(MaxLPPerCore);
               tblCoreID[j] = GetNzbSubID(apicID, maxCorePPP, maskWidth);

               // Extract package ID, assume single cluster.
               // Shift value is the mask width for max Logical per package

               PackageIDMask = (unsigned char) (0xff << 
                  find_maskwidth(MaxLogicalProcPerPhysicalProc()));

               tblPkgID[j] = apicID & PackageIDMask;
               sprintf(tmp,"  AffinityMask = %d; Initial APIC = %d; Physical ID = %d, Core ID = %d,  SMT ID = %d\n",
                  dwAffinityMask, apicID, tblPkgID[j], tblCoreID[j], tblSMTID[j]);
               strcat(g_s3Levels, tmp);

               numLPEnabled ++;   // Number of available logical processors in the system.

            } // if

            j++;  
            dwAffinityMask = 1 << j;
         } // while

         // restore the affinity setting to its original state
#ifdef TORQUE_OS_LINUX
         sched_setaffinity (0, sizeof(allowedCPUs), &allowedCPUs);
         sleep(0);
#elif defined( TORQUE_OS_WIN32 )
         SetThreadAffinityMask(GetCurrentThread(), dwProcessAffinity);
         Sleep(0);
#else
#  error Not implemented.
#endif
         TotAvailLogical = numLPEnabled;

         //
         // Count available cores (TotAvailCore) in the system
         //
         unsigned char CoreIDBucket[256];
         DWORD ProcessorMask, pCoreMask[256];
         unsigned int i, ProcessorNum;

         CoreIDBucket[0] = tblPkgID[0] | tblCoreID[0];
         ProcessorMask = 1;
         pCoreMask[0] = ProcessorMask;

         for (ProcessorNum = 1; ProcessorNum < numLPEnabled; ProcessorNum++)
         {
            ProcessorMask <<= 1;
            for (i = 0; i < TotAvailCore; i++)
            {
               // Comparing bit-fields of logical processors residing in different packages
               // Assuming the bit-masks are the same on all processors in the system.
               if ((tblPkgID[ProcessorNum] | tblCoreID[ProcessorNum]) == CoreIDBucket[i])
               {
                  pCoreMask[i] |= ProcessorMask;
                  break;
               }

            }  // for i

            if (i == TotAvailCore)   // did not match any bucket.  Start a new one.
            {
               CoreIDBucket[i] = tblPkgID[ProcessorNum] | tblCoreID[ProcessorNum];
               pCoreMask[i] = ProcessorMask;

               TotAvailCore++;	// Number of available cores in the system

            }

         }  // for ProcessorNum


         //
         // Count physical processor (PhysicalNum) in the system
         //
         unsigned char PackageIDBucket[256];
         DWORD pPackageMask[256];

         PackageIDBucket[0] = tblPkgID[0];
         ProcessorMask = 1;
         pPackageMask[0] = ProcessorMask;

         for (ProcessorNum = 1; ProcessorNum < numLPEnabled; ProcessorNum++)
         {
            ProcessorMask <<= 1;
            for (i = 0; i < PhysicalNum; i++)
            {
               // Comparing bit-fields of logical processors residing in different packages
               // Assuming the bit-masks are the same on all processors in the system.
               if (tblPkgID[ProcessorNum]== PackageIDBucket[i])
               {
                  pPackageMask[i] |= ProcessorMask;
                  break;
               }

            }  // for i

            if (i == PhysicalNum)   // did not match any bucket.  Start a new one.
            {
               PackageIDBucket[i] = tblPkgID[ProcessorNum];
               pPackageMask[i] = ProcessorMask;

               PhysicalNum++;	// Total number of physical processors in the system

            }

         }  // for ProcessorNum
#endif

         //
         // Check to see if the system is multi-core 
         // Check if the system is hyper-threading
         //
         if (TotAvailCore > PhysicalNum) 
         {
            // Multi-core
            if (MaxLPPerCore == 1)
               StatusFlag = CONFIG_MultiCoreAndHTNotCapable;
            else if (numLPEnabled > TotAvailCore)
               StatusFlag = CONFIG_MultiCoreAndHTEnabled;
            else StatusFlag = CONFIG_MultiCoreAndHTDisabled;

         }
         else
         {
            // Single-core
            if (MaxLPPerCore == 1)
               StatusFlag = CONFIG_SingleCoreAndHTNotCapable;
            else if (numLPEnabled > TotAvailCore)
               StatusFlag = CONFIG_SingleCoreHTEnabled;
            else StatusFlag = CONFIG_SingleCoreHTDisabled;


         }



         return StatusFlag;
      }

} // namespace CPUInfo
#endif