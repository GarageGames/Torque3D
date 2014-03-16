///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains preprocessor stuff. This should be the first included header.
 *	\file		IcePreprocessor.h
 *	\author		Pierre Terdiman
 *	\date		April, 4, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef __ICEPREPROCESSOR_H__
#define __ICEPREPROCESSOR_H__
	// Check platform
   #if _XBOX_VER >= 200 
      #pragma message("Compiling on Xbox 360...")
      #define PLATFORM_XENON
   #elif defined( _WIN64 ) || defined( WIN64 ) || \
         defined( _WIN32 ) || defined( WIN32 )
		#pragma message("Compiling on Windows...")
		#define PLATFORM_WINDOWS
   #elif defined(SN_TARGET_PS3)
      #pragma message("Compiling on PS3...")
      #define PLATFORM_PS3
	#else
		#pragma message("Compiling on unknown platform...")
	#endif

	// Check compiler
	#if defined(_MSC_VER)
		#pragma message("Compiling with VC++...")
		#define COMPILER_VISUAL_CPP
	#else
		#pragma message("Compiling with unknown compiler...")
	#endif

	// Check compiler options. If this file is included in user-apps, this
	// shouldn't be needed, so that they can use what they like best.
	#ifndef ICE_DONT_CHECK_COMPILER_OPTIONS
		#ifdef COMPILER_VISUAL_CPP
			#if defined(_CHAR_UNSIGNED)
			#endif

			#if defined(_CPPRTTI)
				#error Please disable RTTI...
			#endif

			#if defined(_CPPUNWIND)
				#error Please disable exceptions...
			#endif

			#if defined(_MT)
				// Multithreading
			#endif
		#endif
	#endif

	// Check debug mode
	#ifdef	DEBUG			// May be defined instead of _DEBUG. Let's fix it.
		#ifndef	_DEBUG
			#define _DEBUG
		#endif
	#endif

	#ifdef  _DEBUG
	// Here you may define items for debug builds
	#endif

	#ifndef THIS_FILE
		#define THIS_FILE			__FILE__
	#endif

	#ifndef ICE_NO_DLL
		#ifdef ICECORE_EXPORTS
			#define ICECORE_API			__declspec(dllexport)
		#else
			#define ICECORE_API			__declspec(dllimport)
		#endif
	#else
			#define ICECORE_API
	#endif

	// Don't override new/delete
//	#define DEFAULT_NEWDELETE
	#define DONT_TRACK_MEMORY_LEAKS

	#define FUNCTION				extern "C"

	// Cosmetic stuff [mainly useful with multiple inheritance]
	#define	override(base_class)	virtual

	// Our own inline keyword, so that:
	// - we can switch to __forceinline to check it's really better or not
	// - we can remove __forceinline if the compiler doesn't support it
//	#define inline_				__forceinline
//	#define inline_				inline

	// Contributed by Bruce Mitchener
		#if defined(COMPILER_VISUAL_CPP)
//			#define inline_			__forceinline
			#define inline_			inline
		#elif defined(__GNUC__) && __GNUC__ < 3
			#define inline_ inline
		#elif defined(__GNUC__)
			#define inline_ inline __attribute__ ((always_inline))
		#else
			#define inline_ inline
		#endif

	// Down the hatch
	#pragma inline_depth( 255 )

	#ifdef COMPILER_VISUAL_CPP
		#pragma intrinsic(memcmp)
		#pragma intrinsic(memcpy)
		#pragma intrinsic(memset)
		#pragma intrinsic(strcat)
		#pragma intrinsic(strcmp)
		#pragma intrinsic(strcpy)
		#pragma intrinsic(strlen)
		#pragma intrinsic(abs)
		#pragma intrinsic(labs)
	#endif

	// ANSI compliance
/*	#ifdef  _DEBUG
		// Remove painful warning in debug
		inline_ bool __False__(){ return false; }
		#define for if(__False__()){}	else for
	#else
		#define for if(0){}	else for
	#endif */

#endif // __ICEPREPROCESSOR_H__
