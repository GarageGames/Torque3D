/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/* This file contains two versions of make_file_list(); one for Windows, and
 * one for other systems (POSIX).
 */
#ifdef WIN32
#include <io.h>
#include <ctype.h>
#include <string.h>
#include <malloc.h>

char**
make_file_list (const char* pattern, int* pnum_entries)
{
	int num_entries, length;
	char** StringTable;
	char** lpLastOffs;

	num_entries = 0;
	StringTable = 0;
	do
	{
		int slen;
		long handle;
		struct _finddata_t f;

		if (num_entries == 0)
			length = 0;
		else
		{
			slen = (num_entries + 1) * sizeof (char*);
			StringTable = (char**) malloc (slen + length);
			if (StringTable == 0)
				break;

			lpLastOffs = StringTable;
			*lpLastOffs = (char*)StringTable + slen;

			num_entries = 0;
			length = slen;
		}

		handle = _findfirst (pattern, &f);
		if (handle != -1)
		{
			do
			{
				if (f.attrib & (_A_HIDDEN | _A_SUBDIR | _A_SYSTEM))
					continue;

				slen = strlen (f.name) + 1;
				length += slen;

				if (StringTable)
				{
					char *lpStr;
					char **lpLo, **lpHi;

					lpLo = StringTable;
					lpHi = lpLastOffs - 1;
					while (lpLo <= lpHi)
					{
						char c1, c2;
						char *pStr;
						int LocLen;
						char **lpMid;

						lpMid = lpLo + ((lpHi - lpLo) >> 1);

						LocLen = lpMid[1] - lpMid[0];
						if (LocLen > slen)
							LocLen = slen;

						lpStr = lpMid[0];
						pStr = f.name;
						while (LocLen--
								&& (c1 = toupper (*lpStr++))
										== (c2 = toupper (*pStr++)))
							;

						if (c1 <= c2)
							lpLo = lpMid + 1;
						else
							lpHi = lpMid - 1;
					}

					lpStr = lpLo[0];
					memmove (lpStr + slen, lpStr, lpLastOffs[0] - lpLo[0]);
					strcpy (lpStr, f.name);

					for (lpHi = lpLastOffs++; lpHi >= lpLo; --lpHi)
						lpHi[1] = lpHi[0] + slen;
				}

				++num_entries;

			} while (_findnext (handle, &f) == 0);

			_findclose (handle);
		}
	} while (num_entries && StringTable == 0);

	if (StringTable == 0)
		*pnum_entries = 0;
	else
		*pnum_entries = num_entries;

	return StringTable;
}

#else  /* ! defined(WIN32) */

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <fnmatch.h>
#include <ctype.h>
#include <limits.h>
#include <string.h>
#include <malloc.h>

char**
make_file_list (const char* pattern, int* pnum_entries)
{
	size_t num_entries, length;
	char** StringTable;
	char** lpLastOffs;
	char* slash;            // Pointer inside pattern to the last /
	char path[PATH_MAX];  // buffer for a filename with path
	char* file;             // Pointer inside path to the filename
	size_t pathlen;        // length of path, excluding last / and filename

	slash = (char*) strrchr ((const char *) pattern, '/');
	if (slash == NULL)
	{
		pathlen = 1;
		path[0] = '.';
	}
	else
	{
		pathlen = slash - pattern;
		memcpy (path, pattern, pathlen);
		pattern = slash + 1;
	}
	file = path + pathlen + 1;
	
	num_entries = 0;
	StringTable = 0;
	do
	{
		int slen;
		DIR *handle;

		if (num_entries == 0)
			length = 0;
		else
		{
			slen = (num_entries + 1) * sizeof (char*);
			StringTable = (char**) malloc (slen + length);
			if (StringTable == 0)
				break;

			lpLastOffs = StringTable;
			*lpLastOffs = (char*)StringTable + slen;

			num_entries = 0;
			length = slen;
		}
		
		path[pathlen] = '\0';  // strip any file part
		handle = opendir((const char *) path);
		if (handle != NULL)
		{
			path[pathlen] = '/';
					// change the 0 char to a slash; a filename will be
					// attached here.
			while (1)
			{
				struct dirent *de;
				struct stat sb;

				de = readdir(handle);
				if (de == NULL)
					break;
				if (de->d_name[0] == '.')
					continue;
				strcpy (file, de->d_name);
						// attach the filename to path
				if (stat(path, &sb) == -1)
					continue;
				if (!S_ISREG(sb.st_mode))
					continue;
				if (fnmatch(pattern, de->d_name, 0) != 0)
					continue;

				slen = strlen (de->d_name) + 1;
				length += slen;

				if (StringTable)
				{
					char *lpStr;
					char **lpLo, **lpHi;

					lpLo = StringTable;
					lpHi = lpLastOffs - 1;
					while (lpLo <= lpHi)
					{
						char c1, c2;
						char *pStr;
						int LocLen;
						char **lpMid;

						lpMid = lpLo + ((lpHi - lpLo) >> 1);

						LocLen = lpMid[1] - lpMid[0];
						if (LocLen > slen)
							LocLen = slen;

						lpStr = lpMid[0];
						pStr = de->d_name;
						while (LocLen--
								&& (c1 = toupper (*lpStr++))
										== (c2 = toupper (*pStr++)))
							;

						if (c1 <= c2)
							lpLo = lpMid + 1;
						else
							lpHi = lpMid - 1;
					}

					lpStr = lpLo[0];
					memmove (lpStr + slen, lpStr, lpLastOffs[0] - lpLo[0]);
					strcpy (lpStr, de->d_name);

					for (lpHi = lpLastOffs++; lpHi >= lpLo; --lpHi)
						lpHi[1] = lpHi[0] + slen;
				}

				++num_entries;
			}
			closedir(handle);
		}
	} while (num_entries && StringTable == 0);

	if (StringTable == 0)
		*pnum_entries = 0;
	else
		*pnum_entries = num_entries;

	return StringTable;
}

#endif

void
free_file_list (char** list)
{
	if (list)
		free(list);
}
