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

#include <sys/types.h>
#include <sys/sysctl.h>
#include <mach/machine.h>
#include <math.h>
#include <CoreServices/CoreServices.h>

#include "platformMac/platformMacCarb.h"
#include "platform/platformAssert.h"
#include "console/console.h"
#include "core/stringTable.h"


// Original code by Sean O'Brien (http://www.garagegames.com/community/forums/viewthread/81815).


// Reads sysctl() string value into buffer at DEST with maximum length MAXLEN
// Return: 0 on success, non-zero is error in accordance with stdlib and <errno.h>
int _getSysCTLstring(const char key[], char * dest, size_t maxlen) {
	size_t len = 0;
	int err;
	// Call with NULL for 'dest' to have the required size stored in 'len'. If the 'key'
	// doesn't exist, 'err' will be -1 and if all goes well, it will be 0.
	err = sysctlbyname(key, NULL, &len, NULL, 0);
	if (err == 0) {
		AssertWarn((len <= maxlen), ("Insufficient buffer length for SYSCTL() read. Truncating.\n"));
		if (len > maxlen)
			len = maxlen;
		// Call with actual pointers to 'dest' and clamped 'len' fields to perform the read.
		err = sysctlbyname(key, dest, &len, NULL, 0);
	}
	return err;
}

// TEMPLATED Reads sysctl() integer value into variable DEST of type T
// The two predominant types used are unsigned longs and unsiged long longs
// and the size of the argument is on a case-by-case value. As a "guide" the
// resources at Apple claim that any "byte count" or "frequency" values will
// be returned as ULL's and most everything else will be UL's.
// Return: 0 on success, non-zero is error in accordance with stdlib and <errno.h>
template <typename T>
int _getSysCTLvalue(const char key[], T * dest) {
	size_t len = 0;
	int err;
	// Call with NULL for 'dest' to get the size. If the 'key' doesn't exist, the
	// 'err' returned will be -1, so 0 indicates success.
	err = sysctlbyname(key, NULL, &len, NULL, 0);
	if (err == 0) {
		AssertFatal((len == sizeof(T)), "Mis-matched destination type for SYSCTL() read.\n");
		// We're just double-checking that we're being called with the correct type of
		// pointer for 'dest' so we don't clobber anything nearby when writing back.
		err = sysctlbyname(key, dest, &len, NULL, 0);
	}
	return err;
}

Platform::SystemInfo_struct Platform::SystemInfo;

#define BASE_MHZ_SPEED      0

void Processor::init()
{
	U32 procflags;
	int err, cpufam, cputype, cpusub;
	char buf[20];
	unsigned long lraw;
	unsigned long long llraw;
	
	Con::printf( "System & Processor Information:" );
   
   SInt32 MacVersion;
   if( Gestalt( gestaltSystemVersion, &MacVersion ) == noErr )
   {
      U32 revision = MacVersion & 0xf;
      U32 minorVersion = ( MacVersion & 0xf0 ) >> 4;
      U32 majorVersion = ( MacVersion & 0xff00 ) >> 8;
      
      Con::printf( "   OSX Version: %x.%x.%x", majorVersion, minorVersion, revision );
   }
	
	err = _getSysCTLstring("kern.ostype", buf, sizeof(buf));	
	if (err)
		Con::printf( "   Unable to determine OS type\n" );
	else
		Con::printf( "   Mac OS Kernel name: %s", buf);
	
	err = _getSysCTLstring("kern.osrelease", buf, sizeof(buf));	
	if (err)
		Con::printf( "   Unable to determine OS release number\n" );
	else
		Con::printf( "   Mac OS Kernel version: %s", buf );
	
	err = _getSysCTLvalue<unsigned long long>("hw.memsize", &llraw);	
	if (err)
		Con::printf( "   Unable to determine amount of physical RAM\n" );
	else
		Con::printf( "   Physical memory installed: %d MB", (llraw >> 20));
	
	err = _getSysCTLvalue<unsigned long>("hw.usermem", &lraw);	
	if (err)
		Con::printf( "   Unable to determine available user address space\n");
	else
		Con::printf( "   Addressable user memory: %d MB", (lraw >> 20));
	
	////////////////////////////////
	// Values for the Family Type, CPU Type and CPU Subtype are defined in the
	// SDK files for the Mach Kernel ==>  mach/machine.h
	////////////////////////////////
	
	// CPU Family, Type, and Subtype
	cpufam = 0;
	cputype = 0;
	cpusub = 0;
	err = _getSysCTLvalue<unsigned long>("hw.cpufamily", &lraw);	
	if (err)
		Con::printf( "   Unable to determine 'family' of CPU\n");
	else {
		cpufam = (int) lraw;
		err = _getSysCTLvalue<unsigned long>("hw.cputype", &lraw);	
		if (err)
			Con::printf( "   Unable to determine CPU type\n");
		else {
			cputype = (int) lraw;
			err = _getSysCTLvalue<unsigned long>("hw.cpusubtype", &lraw);	
			if (err)
				Con::printf( "   Unable to determine CPU subtype\n");
			else
				cpusub = (int) lraw;
			// If we've made it this far, 
			Con::printf( "   Installed processor ID: Family 0x%08x  Type %d  Subtype %d",cpufam, cputype,cpusub);
		}
	}
	
	// The Gestalt version was known to have issues with some Processor Upgrade cards
	// but it is uncertain whether this version has similar issues.
	err = _getSysCTLvalue<unsigned long long>("hw.cpufrequency", &llraw);
	if (err) {
		llraw = BASE_MHZ_SPEED;
		Con::printf( "   Unable to determine CPU Frequency. Defaulting to %d MHz\n", llraw);
	} else {
		llraw /= 1000000;
		Con::printf( "   Installed processor clock frequency: %d MHz", llraw);
	}
	Platform::SystemInfo.processor.mhz = (unsigned int)llraw;
	
	// Here's one that the original version of this routine couldn't do -- number
	// of processors (cores)
   U32 ncpu = 1;
	err = _getSysCTLvalue<unsigned long>("hw.ncpu", &lraw);
	if (err)
		Con::printf( "   Unable to determine number of processor cores\n");
	else
   {
      ncpu = lraw;
		Con::printf( "   Installed/available processor cores: %d", lraw);
   }
	
	// Now use CPUFAM to determine and then store the processor type
	// and 'friendly name' in GG-accessible structure. Note that since
	// we have access to the Family code, the Type and Subtypes are useless.
	//
	// NOTE: Even this level of detail is almost assuredly not needed anymore
	// and the Optional Capability flags (further down) should be more than enough.
	switch(cpufam)
	{
		case CPUFAMILY_POWERPC_G3:
			Platform::SystemInfo.processor.type = CPU_PowerPC_G3;
			Platform::SystemInfo.processor.name = StringTable->insert("PowerPC G3");
			break;
		case CPUFAMILY_POWERPC_G4:
			Platform::SystemInfo.processor.type = CPU_PowerPC_G3;
			Platform::SystemInfo.processor.name = StringTable->insert("PowerPC G4");
			break;
		case CPUFAMILY_POWERPC_G5:
			Platform::SystemInfo.processor.type = CPU_PowerPC_G3;
			Platform::SystemInfo.processor.name = StringTable->insert("PowerPC G5");
			break;
		case CPUFAMILY_INTEL_6_14:
			Platform::SystemInfo.processor.type = CPU_Intel_Core;
         if( ncpu == 2 )
            Platform::SystemInfo.processor.name = StringTable->insert("Intel Core Duo");
         else
            Platform::SystemInfo.processor.name = StringTable->insert("Intel Core");
			break;
      #ifdef CPUFAMILY_INTEL_6_23
      case CPUFAMILY_INTEL_6_23:
      #endif
		case CPUFAMILY_INTEL_6_15:
			Platform::SystemInfo.processor.type = CPU_Intel_Core2;
         if( ncpu == 4 )
            Platform::SystemInfo.processor.name = StringTable->insert("Intel Core 2 Quad");
         else
            Platform::SystemInfo.processor.name = StringTable->insert("Intel Core 2 Duo");
			break;
         
      #ifdef CPUFAMILY_INTEL_6_26
      case CPUFAMILY_INTEL_6_26:
         Platform::SystemInfo.processor.type = CPU_Intel_Core2;
         Platform::SystemInfo.processor.name = StringTable->insert( "Intel 'Nehalem' Core Processor" );
         break;
      #endif
      
		default:
			// explain why we can't get the processor type.
			Con::warnf( "   Unknown Processor (family, type, subtype): 0x%x\t%d  %d", cpufam, cputype, cpusub);
			// for now, identify it as an x86 processor, because Apple is moving to Intel chips...
			Platform::SystemInfo.processor.type = CPU_X86Compatible;
			Platform::SystemInfo.processor.name = StringTable->insert("Unknown Processor, assuming x86 Compatible");
			break;
	}
   // Now we can directly query the system about a litany of "Optional" processor capabilities
	// and determine the status by using BOTH the 'err' value and the 'lraw' value. If we request
	// a non-existant feature from SYSCTL(), the 'err' result will be -1; 0 denotes it exists 
	// >>>> BUT <<<<<
	// it may not be supported, only defined. Thus we need to check 'lraw' to determine if it's 
	// actually supported/implemented by the processor: 0 = no, 1 = yes, others are undefined.
	procflags = 0;
	// Seriously this one should be an Assert()
	err = _getSysCTLvalue<unsigned long>("hw.optional.floatingpoint", &lraw);	
	if ((err==0)&&(lraw==1)) procflags |= CPU_PROP_FPU;
	// List of chip-specific features
	err = _getSysCTLvalue<unsigned long>("hw.optional.mmx", &lraw);	
	if ((err==0)&&(lraw==1)) procflags |= CPU_PROP_MMX;
	err = _getSysCTLvalue<unsigned long>("hw.optional.sse", &lraw);	
	if ((err==0)&&(lraw==1)) procflags |= CPU_PROP_SSE;
	err = _getSysCTLvalue<unsigned long>("hw.optional.sse2", &lraw);	
	if ((err==0)&&(lraw==1)) procflags |= CPU_PROP_SSE2;
	err = _getSysCTLvalue<unsigned long>("hw.optional.sse3", &lraw);	
	if ((err==0)&&(lraw==1)) procflags |= CPU_PROP_SSE3;
	err = _getSysCTLvalue<unsigned long>("hw.optional.supplementalsse3", &lraw);	
	if ((err==0)&&(lraw==1)) procflags |= CPU_PROP_SSE3xt;
	err = _getSysCTLvalue<unsigned long>("hw.optional.sse4_1", &lraw);	
	if ((err==0)&&(lraw==1)) procflags |= CPU_PROP_SSE4_1;
	err = _getSysCTLvalue<unsigned long>("hw.optional.sse4_2", &lraw);	
	if ((err==0)&&(lraw==1)) procflags |= CPU_PROP_SSE4_2;
	err = _getSysCTLvalue<unsigned long>("hw.optional.altivec", &lraw);	
	if ((err==0)&&(lraw==1)) procflags |= CPU_PROP_ALTIVEC;
	// Finally some architecture-wide settings
	err = _getSysCTLvalue<unsigned long>("hw.ncpu", &lraw);	
	if ((err==0)&&(lraw>1)) procflags |= CPU_PROP_MP;
	err = _getSysCTLvalue<unsigned long>("hw.cpu64bit_capable", &lraw);	
	if ((err==0)&&(lraw==1)) procflags |= CPU_PROP_64bit;
	err = _getSysCTLvalue<unsigned long>("hw.byteorder", &lraw);	
	if ((err==0)&&(lraw==1234)) procflags |= CPU_PROP_LE;

	Platform::SystemInfo.processor.properties = procflags;
	
	Con::printf( "%s, %2.2f GHz", Platform::SystemInfo.processor.name, F32( Platform::SystemInfo.processor.mhz ) / 1000.0 );
	if (Platform::SystemInfo.processor.properties & CPU_PROP_MMX)
		Con::printf( "   MMX detected");
	if (Platform::SystemInfo.processor.properties & CPU_PROP_SSE)
		Con::printf( "   SSE detected");
	if (Platform::SystemInfo.processor.properties & CPU_PROP_SSE2)
		Con::printf( "   SSE2 detected");
	if (Platform::SystemInfo.processor.properties & CPU_PROP_SSE3)
		Con::printf( "   SSE3 detected");
	if (Platform::SystemInfo.processor.properties & CPU_PROP_ALTIVEC)
		Con::printf( "   AltiVec detected");
	
	Con::printf( "" );
   
   // Trigger the signal
   Platform::SystemInfoReady.trigger();
}
