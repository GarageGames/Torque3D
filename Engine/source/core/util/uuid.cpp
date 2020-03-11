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
// Original code:
/*
** Copyright (C) 1998-1999 Greg Stein. All Rights Reserved.
**
** By using this file, you agree to the terms and conditions set forth in
** the LICENSE.html file which can be found at the top level of the mod_dav
** distribution or at http://www.webdav.org/mod_dav/license-1.html.
**
** Contact information:
**   Greg Stein, PO Box 3151, Redmond, WA, 98073
**   gstein@lyra.org, http://www.webdav.org/mod_dav/
*/

/*
** DAV opaquelocktoken scheme implementation
**
** Written 5/99 by Keith Wannamaker, wannamak@us.ibm.com
** Adapted from ISO/DCE RPC spec and a former Internet Draft
** by Leach and Salz:
** http://www.ics.uci.edu/pub/ietf/webdav/uuid-guid/draft-leach-uuids-guids-01
**
** Portions of the code are covered by the following license:
**
** Copyright (c) 1990- 1993, 1996 Open Software Foundation, Inc.
** Copyright (c) 1989 by Hewlett-Packard Company, Palo Alto, Ca. &
** Digital Equipment Corporation, Maynard, Mass.
** Copyright (c) 1998 Microsoft.
** To anyone who acknowledges that this file is provided "AS IS"
** without any express or implied warranty: permission to use, copy,
** modify, and distribute this file for any purpose is hereby
** granted without fee, provided that the above copyright notices and
** this notice appears in all source code copies, and that none of
** the names of Open Software Foundation, Inc., Hewlett-Packard
** Company, or Digital Equipment Corporation be used in advertising
** or publicity pertaining to distribution of the software without
** specific, written prior permission.  Neither Open Software
** Foundation, Inc., Hewlett-Packard Company, Microsoft, nor Digital Equipment
** Corporation makes any representations about the suitability of
** this software for any purpose.
*/

#include "platform/platform.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>

#include "core/util/md5.h"

#if defined (TORQUE_OS_MAC) && defined(TORQUE_CPU_X64)
typedef unsigned int    unsigned32;
#else
typedef unsigned long   unsigned32;
#endif
typedef unsigned short  unsigned16;
typedef unsigned char   unsigned8;

typedef struct {
    char nodeID[6];
} uuid_node_t;

#undef xuuid_t

typedef struct _uuid_t
{
    unsigned32	time_low;
    unsigned16	time_mid;
    unsigned16	time_hi_and_version;
    unsigned8	clock_seq_hi_and_reserved;
    unsigned8	clock_seq_low;
    unsigned8	node[6];
} xuuid_t;

/* data type for UUID generator persistent state */
	
typedef struct {
    uuid_node_t node;     /* saved node ID */
    unsigned16 cs;        /* saved clock sequence */
} uuid_state;

#if defined(_WIN32)
#include <windows.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#endif

/* set the following to the number of 100ns ticks of the actual resolution of
   your system's clock */
#define UUIDS_PER_TICK 1024

/* Set this to what your compiler uses for 64 bit data type */
#ifdef _WIN32
#define unsigned64_t unsigned __int64
#define I64(C) C
#else
#define unsigned64_t unsigned long long
#define I64(C) C##LL
#endif

typedef unsigned64_t uuid_time_t;

static void format_uuid_v1(xuuid_t * uuid, unsigned16 clockseq, uuid_time_t timestamp, uuid_node_t node);
static void get_current_time(uuid_time_t * timestamp);
static unsigned16 true_random(void);
static void get_pseudo_node_identifier(uuid_node_t *node);
static void get_system_time(uuid_time_t *uuid_time);
static void get_random_info(unsigned char seed[16]);


/* dav_create_opaquelocktoken - generates a UUID version 1 token.
 *   Clock_sequence and node_address set to pseudo-random
 *   numbers during init.  
 *
 *   Should postpend pid to account for non-seralized creation?
 */
static int create_token(uuid_state *st, xuuid_t *u)
{
    uuid_time_t timestamp;
    
    get_current_time(&timestamp);
    format_uuid_v1(u, st->cs, timestamp, st->node);
    
    return 1;
}

/*
 * dav_create_uuid_state - seed UUID state with pseudorandom data
 */
static void create_uuid_state(uuid_state *st)
{
    st->cs = true_random();
    get_pseudo_node_identifier(&st->node);
}

/*
 * dav_format_opaquelocktoken - generates a text representation
 *    of an opaquelocktoken
 */
static void format_token(char *target, const xuuid_t *u)
{
  sprintf(target, "%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
	  u->time_low, u->time_mid, u->time_hi_and_version,
	  u->clock_seq_hi_and_reserved, u->clock_seq_low,
	  u->node[0], u->node[1], u->node[2],
	  u->node[3], u->node[4], u->node[5]);
}

/* convert a pair of hex digits to an integer value [0,255] */
static int dav_parse_hexpair(const char *s)
{
    int result;
    int temp;

    result = s[0] - '0';
    if (result > 48)
	result = (result - 39) << 4;
    else if (result > 16)
	result = (result - 7) << 4;
    else
	result = result << 4;

    temp = s[1] - '0';
    if (temp > 48)
	result |= temp - 39;
    else if (temp > 16)
	result |= temp - 7;
    else
	result |= temp;

    return result;
}

/* dav_parse_locktoken:  Parses string produced from
 *    dav_format_opaquelocktoken back into a xuuid_t
 *    structure.  On failure, return DAV_IF_ERROR_PARSE,
 *    else DAV_IF_ERROR_NONE.
 */
static int parse_token(const char *char_token, xuuid_t *bin_token)
{
    int i;

    for (i = 0; i < 36; ++i) {
	char c = char_token[i];
	if (!isxdigit(c) &&
	    !(c == '-' && (i == 8 || i == 13 || i == 18 || i == 23)))
	    return -1;
    }
    if (char_token[36] != '\0')
	return -1;

    bin_token->time_low =
	(dav_parse_hexpair(&char_token[0]) << 24) |
	(dav_parse_hexpair(&char_token[2]) << 16) |
	(dav_parse_hexpair(&char_token[4]) << 8) |
	dav_parse_hexpair(&char_token[6]);

    bin_token->time_mid =
	(dav_parse_hexpair(&char_token[9]) << 8) |
	dav_parse_hexpair(&char_token[11]);

    bin_token->time_hi_and_version =
	(dav_parse_hexpair(&char_token[14]) << 8) |
	dav_parse_hexpair(&char_token[16]);

    bin_token->clock_seq_hi_and_reserved = dav_parse_hexpair(&char_token[19]);
    bin_token->clock_seq_low = dav_parse_hexpair(&char_token[21]);

    for (i = 6; i--;)
	bin_token->node[i] = dav_parse_hexpair(&char_token[i*2+24]);

    return 0;
}

/* format_uuid_v1 -- make a UUID from the timestamp, clockseq, and node ID */
static void format_uuid_v1(xuuid_t * uuid, unsigned16 clock_seq,
			   uuid_time_t timestamp, uuid_node_t node)
{
    /* Construct a version 1 uuid with the information we've gathered
     * plus a few constants. */
    uuid->time_low = (unsigned long)(timestamp & 0xFFFFFFFF);
    uuid->time_mid = (unsigned short)((timestamp >> 32) & 0xFFFF);
    uuid->time_hi_and_version = (unsigned short)((timestamp >> 48) & 0x0FFF);
    uuid->time_hi_and_version |= (1 << 12);
    uuid->clock_seq_low = clock_seq & 0xFF;
    uuid->clock_seq_hi_and_reserved = (clock_seq & 0x3F00) >> 8;
    uuid->clock_seq_hi_and_reserved |= 0x80;
    memcpy(&uuid->node, &node, sizeof uuid->node);
}

/* get-current_time -- get time as 60 bit 100ns ticks since whenever.
   Compensate for the fact that real clock resolution is less than 100ns. */
static void get_current_time(uuid_time_t * timestamp)
{
    uuid_time_t         time_now;
    static uuid_time_t  time_last;
    static unsigned16   uuids_this_tick;
    static int          inited = 0;
    
    if (!inited) {
        get_system_time(&time_now);
        uuids_this_tick = UUIDS_PER_TICK;
        inited = 1;
    };
	
    while (1) {
	get_system_time(&time_now);
        
	/* if clock reading changed since last UUID generated... */
	if (time_last != time_now) {
	    /* reset count of uuids gen'd with this clock reading */
	    uuids_this_tick = 0;
	    break;
	};
	if (uuids_this_tick < UUIDS_PER_TICK) {
	    uuids_this_tick++;
	    break;
	};        /* going too fast for our clock; spin */
    };    /* add the count of uuids to low order bits of the clock reading */

    *timestamp = time_now + uuids_this_tick;
    time_last = time_now;
}

/* true_random -- generate a crypto-quality random number.
   This sample doesn't do that. */
static unsigned16 true_random(void)
{
    uuid_time_t time_now;

    get_system_time(&time_now);
    time_now = time_now/UUIDS_PER_TICK;
    srand((unsigned int)(((time_now >> 32) ^ time_now)&0xffffffff));

    return rand();
}

/* This sample implementation generates a random node ID   *
 * in lieu of a system dependent call to get IEEE node ID. */
static void get_pseudo_node_identifier(uuid_node_t *node)
{
    unsigned char seed[16];

    get_random_info(seed);
    seed[0] |= 0x80;
    memcpy(node, seed, sizeof(uuid_node_t));
}

/* system dependent call to get the current system time.
   Returned as 100ns ticks since Oct 15, 1582, but resolution may be
   less than 100ns.  */

#ifdef _WIN32

static void get_system_time(uuid_time_t *uuid_time)
{
    ULARGE_INTEGER time;

    GetSystemTimeAsFileTime((FILETIME *)&time);

    /* NT keeps time in FILETIME format which is 100ns ticks since
       Jan 1, 1601.  UUIDs use time in 100ns ticks since Oct 15, 1582.
       The difference is 17 Days in Oct + 30 (Nov) + 31 (Dec)
       + 18 years and 5 leap days.    */
	
    time.QuadPart +=
	(unsigned __int64) (1000*1000*10)
	* (unsigned __int64) (60 * 60 * 24)
	* (unsigned __int64) (17+30+31+365*18+5);
    *uuid_time = time.QuadPart;
}

#if defined(_XBOX)
#include "platform/platformAssert.h"
#endif

static void get_random_info(unsigned char seed[16])
{
#if defined(_XBOX)
   AssertFatal(false, "get_random_info not implemented on Xbox360");
#else
    MD5_CTX c;
    struct {
        MEMORYSTATUS m;
        SYSTEM_INFO s;
        FILETIME t;
        LARGE_INTEGER pc;
        DWORD tc;
        DWORD l;
        TCHAR hostname[MAX_COMPUTERNAME_LENGTH + 1];

    } r;
    
    MD5Init(&c);    /* memory usage stats */
    GlobalMemoryStatus(&r.m);    /* random system stats */
    GetSystemInfo(&r.s);    /* 100ns resolution (nominally) time of day */
    GetSystemTimeAsFileTime(&r.t);    /* high resolution performance counter */
    QueryPerformanceCounter(&r.pc);    /* milliseconds since last boot */
    r.tc = GetTickCount();
    r.l = MAX_COMPUTERNAME_LENGTH + 1;

    GetComputerName(r.hostname, &r.l );
    MD5Update(&c, (unsigned char *) &r, sizeof(r));
    MD5Final(seed, &c);
#endif
}

#else /* WIN32 */

static void get_system_time(uuid_time_t *uuid_time)
{
    struct timeval tp;

    gettimeofday(&tp, (struct timezone *)0);

    /* Offset between UUID formatted times and Unix formatted times.
       UUID UTC base time is October 15, 1582.
       Unix base time is January 1, 1970.      */
    *uuid_time = (tp.tv_sec * 10000000) + (tp.tv_usec * 10) +
        I64(0x01B21DD213814000);
}
 
static void get_random_info(unsigned char seed[16])
{
    MD5_CTX c;
    /* Leech & Salz use Linux-specific struct sysinfo;
     * replace with pid/tid for portability (in the spirit of mod_unique_id) */
    struct {
	/* Add thread id here, if applicable, when we get to pthread or apr */
	pid_t pid;
        struct timeval t;
        char hostname[257];

    } r;

    MD5Init(&c);
    r.pid = getpid();
    gettimeofday(&r.t, (struct timezone *)0);
    gethostname(r.hostname, 256);
    MD5Update(&c, (unsigned char *)&r, sizeof(r));
    MD5Final(seed, &c);
}

#endif /* WIN32 */

#include "core/util/uuid.h"

namespace {
   bool gUUIDStateInitialized;
   uuid_state gUUIDState;
}

namespace Torque
{
   UUID UUID::smNull;
   
   void UUID::generate()
   {
      if( !gUUIDStateInitialized )
      {
         create_uuid_state( &gUUIDState );
         gUUIDStateInitialized = true;
      }
      
      create_token( &gUUIDState, ( xuuid_t* ) this );
   }
   
   String UUID::toString() const
   {
      char buffer[ 1024 ];
      format_token( buffer, ( xuuid_t* ) this );
      return buffer;
   }
   
   bool UUID::fromString( const char* str )
   {
      if( parse_token( str, ( xuuid_t* ) this ) != 0 )
      {
         dMemset( this, 0, sizeof( UUID ) );
         return false;
      }
      
      return true;
   }
   
   U32 UUID::getHash() const
   {
      return ( a + b + c + d + e + f[ 0 ] + f[ 1 ] + f[ 2 ] + f[ 3 ] + f[ 4 ] + f[ 5 ] );
   }
}
