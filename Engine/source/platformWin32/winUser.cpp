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
#include "console/console.h"
#include "core/stringTable.h"
#include "core/strings/unicode.h"

#ifndef TORQUE_OS_WIN64
typedef long SHANDLE_PTR;
#endif

#include <shlobj.h>
#include <windows.h>
#include <lmcons.h>

#define CSIDL_PROFILE 0x0028

const char *Platform::getUserDataDirectory() 
{
   TCHAR szBuffer[ MAX_PATH + 1 ];

   if(! SHGetSpecialFolderPath( NULL, szBuffer, CSIDL_APPDATA, true ) )
      return "";

   TCHAR *ptr = szBuffer;
   while(*ptr)
   {
      if(*ptr == '\\')
         *ptr = '/';
      ++ptr;
   }

#ifdef UNICODE
   char path[ MAX_PATH * 3 + 1 ];
   convertUTF16toUTF8( szBuffer, path );
#else
   char* path = szBuffer;
#endif


   return StringTable->insert( path );
}

const char *Platform::getUserHomeDirectory() 
{
   TCHAR szBuffer[ MAX_PATH + 1 ];
   if(! SHGetSpecialFolderPath( NULL, szBuffer, CSIDL_PERSONAL, false ) )
      if(! SHGetSpecialFolderPath( NULL, szBuffer, CSIDL_COMMON_DOCUMENTS, false ) )
         return "";

   TCHAR *ptr = szBuffer;
   while(*ptr)
   {
      if(*ptr == '\\')
         *ptr = '/';
      ++ptr;
   }

#ifdef UNICODE
   char path[ MAX_PATH * 3 + 1 ];
   convertUTF16toUTF8( szBuffer, path );
#else
   char* path = szBuffer;
#endif

   return StringTable->insert( path );
}


bool Platform::getUserIsAdministrator()
{
   BOOL   fReturn         = FALSE;
   DWORD  dwStatus;
   DWORD  dwAccessMask;
   DWORD  dwAccessDesired;
   DWORD  dwACLSize;
   DWORD  dwStructureSize = sizeof(PRIVILEGE_SET);
   PACL   pACL            = NULL;
   PSID   psidAdmin       = NULL;

   HANDLE hToken              = NULL;
   HANDLE hImpersonationToken = NULL;

   PRIVILEGE_SET   ps;
   GENERIC_MAPPING GenericMapping;

   PSECURITY_DESCRIPTOR     psdAdmin           = NULL;
   SID_IDENTIFIER_AUTHORITY SystemSidAuthority = SECURITY_NT_AUTHORITY;


   /*
   Determine if the current thread is running as a user that is a member of
   the local admins group.  To do this, create a security descriptor that
   has a DACL which has an ACE that allows only local aministrators access.
   Then, call AccessCheck with the current thread's token and the security
   descriptor.  It will say whether the user could access an object if it
   had that security descriptor.  Note: you do not need to actually create
   the object.  Just checking access against the security descriptor alone
   will be sufficient.
   */
   const DWORD ACCESS_READ  = 1;
   const DWORD ACCESS_WRITE = 2;


   __try
   {

      /*
      AccessCheck() requires an impersonation token.  We first get a primary
      token and then create a duplicate impersonation token.  The
      impersonation token is not actually assigned to the thread, but is
      used in the call to AccessCheck.  Thus, this function itself never
      impersonates, but does use the identity of the thread.  If the thread
      was impersonating already, this function uses that impersonation context.
      */
      if (!OpenThreadToken(GetCurrentThread(), TOKEN_DUPLICATE|TOKEN_QUERY, TRUE, &hToken))
      {
         if (GetLastError() != ERROR_NO_TOKEN)
            __leave;

         if (!OpenProcessToken(GetCurrentProcess(), TOKEN_DUPLICATE|TOKEN_QUERY, &hToken))
            __leave;
      }

      if (!DuplicateToken (hToken, SecurityImpersonation, &hImpersonationToken))
         __leave;


      /*
      Create the binary representation of the well-known SID that
      represents the local administrators group.  Then create the security
      descriptor and DACL with an ACE that allows only local admins access.
      After that, perform the access check.  This will determine whether
      the current user is a local admin.
      */
      if (!AllocateAndInitializeSid(&SystemSidAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
         DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &psidAdmin))
         __leave;

      psdAdmin = LocalAlloc(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH);
      if (psdAdmin == NULL)
         __leave;

      if (!InitializeSecurityDescriptor(psdAdmin, SECURITY_DESCRIPTOR_REVISION))
         __leave;

      // Compute size needed for the ACL.
      dwACLSize = sizeof(ACL) + sizeof(ACCESS_ALLOWED_ACE) + GetLengthSid(psidAdmin) - sizeof(DWORD);

      pACL = (PACL)LocalAlloc(LPTR, dwACLSize);
      if (pACL == NULL)
         __leave;

      if (!InitializeAcl(pACL, dwACLSize, ACL_REVISION2))
         __leave;

      dwAccessMask= ACCESS_READ | ACCESS_WRITE;

      if (!AddAccessAllowedAce(pACL, ACL_REVISION2, dwAccessMask, psidAdmin))
         __leave;

      if (!SetSecurityDescriptorDacl(psdAdmin, TRUE, pACL, FALSE))
         __leave;

      /*
      AccessCheck validates a security descriptor somewhat; set the group
      and owner so that enough of the security descriptor is filled out to
      make AccessCheck happy.
      */
      SetSecurityDescriptorGroup(psdAdmin, psidAdmin, FALSE);
      SetSecurityDescriptorOwner(psdAdmin, psidAdmin, FALSE);

      if (!IsValidSecurityDescriptor(psdAdmin))
         __leave;

      dwAccessDesired = ACCESS_READ;

      /*
      Initialize GenericMapping structure even though you
      do not use generic rights.
      */
      GenericMapping.GenericRead    = ACCESS_READ;
      GenericMapping.GenericWrite   = ACCESS_WRITE;
      GenericMapping.GenericExecute = 0;
      GenericMapping.GenericAll     = ACCESS_READ | ACCESS_WRITE;

      if (!AccessCheck(psdAdmin, hImpersonationToken, dwAccessDesired,
         &GenericMapping, &ps, &dwStructureSize, &dwStatus,
         &fReturn))
      {
         fReturn = FALSE;
         __leave;
      }
   }
   __finally
   {

      // Clean up.
      if (pACL) LocalFree(pACL);
      if (psdAdmin) LocalFree(psdAdmin);
      if (psidAdmin) FreeSid(psidAdmin);
      if (hImpersonationToken) CloseHandle (hImpersonationToken);
      if (hToken) CloseHandle (hToken);
   }

   return fReturn;

}