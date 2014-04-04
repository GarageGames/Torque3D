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

#include "core/util/path.h"


namespace Torque
{

//-----------------------------------------------------------------------------

void Path::_split(String name)
{
   S32   pos = 0;
   S32   idx = 0;

   // Make sure we have platform separators
   name = PathToPlatform(name);

   // root:
   idx = name.find(':');
   if (idx >= 0)
   {
      mRoot = name.substr(0,idx);
      pos = idx + 1;
   }
   else if( name[ 0 ] == '/' )
   {
      mRoot = "/";
   }
   else
   {
      mRoot = "";
   }

   // Extract path and strip trailing '/'
   idx = name.find('/', 0, String::Right);
   if (idx >= pos)
   {
      S32 len = idx - pos;
      mPath = name.substr(pos,len? len: 1);
      mPath = Path::CleanSeparators(mPath);
      pos = idx + 1;
   }
   else
   {
      mPath = "";
   }

   // file.ext
   idx = name.find('.', 0, String::Right);
   if (idx >= pos)
   {
      mFile = name.substr(pos,idx - pos);
      mExt = name.substr(idx + 1,name.length() - idx - 1);
   }
   else
   {
      mFile = name.substr(pos,name.length() - pos);
      mExt = "";
   }
}

String Path::_join() const
{
   String name;
   if( mRoot != '/' )
      name = Path::Join(mRoot, ':', mPath);
   else
      name = mPath;
   name = Path::Join(name, '/', mFile);
   name = Path::Join(name, '.', mExt);
   return name;
}

String Path::CleanSeparators(String path)
{
   return path.replace( '\\', '/' );
}

String Path::CompressPath(String path)
{
   // Remove "./" and "../" references. A ".." will also result in the
   // removal of the proceeding directory.
   // Also cleans separators as in CleanSeparators().
   // Assumes there are no trailing "/"

   // Start by cleaning separators
   path = Path::CleanSeparators(path);

   U32   src = 0;
   U32   dst = 0;

   while (path[src])
   {
      if (path[src] == '/' && path[src + 1] == '/')
      {
         src += 1;
         continue;
      }
      else if (path[src] == '.')
      {
         if (path[src + 1] == 0)
         {
            if (dst && path[dst - 1] == '/')
               dst--;
            src++;
            break;
         }
         else if (path[src + 1] == '/')
         {
            src += 2;
            continue;
         }
         else if (path[src + 1] == '.')
         {
            if (path[src + 2] == 0)
            {
               if (dst && path[dst - 1] == '/')
                  dst = path.find('/', dst - 1, String::Right);
               src += 2;
               break;
            }
            if (dst && path[dst - 1] == '/')
               dst = path.find('/', dst - 1, String::Right) + 1;
            else
               dst += 3;

            src += 3;
            continue;
         }
      }

      if (dst != src)
      {
         String   end = path.substr(src, path.length() - src);
         if (dst > 0 && end[(String::SizeType)0] == '/' && path[dst-1] == '/')
            end = end.substr(1, end.length() - 1);
         path.replace(dst, path.length() - dst, end);
         src = dst;
      }
      else
      {
         src++;
         dst++;
      }
   }

   if (src - dst)
      path.erase(dst, src - dst);

   return path;
}

Torque::Path Path::MakeRelativePath( const Path &makeRelative, const Path &relativeTo, U32 mode )
{
   // We need to find the part of makeRelative that starts to diverge from
   // relativeTo. We only need to check up to the end of makeRelative or realtiveTo
   // (whichever comes first).
   U32 minDirCount = getMin(makeRelative.getDirectoryCount(), relativeTo.getDirectoryCount());

   // Store the index of the directory where we diverge. If we never diverge this
   // will end up being the same as the number of directories in makeRelative
   U32 divergeDirIdx = 0;

   for (divergeDirIdx = 0; divergeDirIdx < minDirCount; divergeDirIdx++)
   {
      // If our directories don't match then this is the diverge directory
      if (!makeRelative.getDirectory(divergeDirIdx).equal(relativeTo.getDirectory(divergeDirIdx), mode))
         break;
   }

   // Get the part of makeRelative's path after it diverged from relativeTo's path
   String uniquePath;

   // If we never diverged then divergeDirIdx will be equal to the number of
   // directories in makeRelative and this loop will immediately exit
   for (U32 i = divergeDirIdx; i < makeRelative.getDirectoryCount(); i++)
      uniquePath += makeRelative.getDirectory(i) + "/";

   // Go ahead and add the full file name
   uniquePath += makeRelative.getFullFileName();

   // Now calculate the relative offset
   String offsetPath;

   U32 numOffset = relativeTo.getDirectoryCount() - divergeDirIdx;

   // Push back an "up" for each directory we are offset
   for (U32 i = 0; i < numOffset; i++)
      offsetPath += "../";

   return offsetPath + uniquePath;
}

String Path::Join(const String& a,String::ValueType s,const String& b)
{
   switch (s)
   {
      case '/':
      {
         if (b.isEmpty() || (b.length() == 1 && (b.c_str()[0] == '/')))
            return a;

         if (a.isEmpty())
            return b;

         String::ValueType c = a[a.length()-1];

         if (c == ':' || ((c == '/') ^ (b.c_str()[0] == '/')))
            return a + b;

         if (c == '/' && b.c_str()[0] == '/')
            return a.substr(0,a.length() - 1) + b;
         break;
      }

      case ':':
      {
         if (a.isEmpty())
            return b;

         if (b.isEmpty())
            return a + ':';
         break;
      }

      case '.':
      {
         if (b.isEmpty())
            return a;

         if (a.isEmpty())
            return '.' + b;
         break;
      }

      default:
         break;
   }

   return a + s + b;
}

bool Path::appendPath( const Path &p )
{
   mPath = CompressPath(Join(mPath,'/',p.getPath()));
   mIsDirtyPath = true;
   return true;
}

const String &Path::getFullFileName() const
{
   if ( mIsDirtyFileName )
   {
      mFullFileName = mFile;

      if (mExt.isNotEmpty())
         mFullFileName += '.' + mExt;
      mIsDirtyFileName = false;
   }

   return mFullFileName;

}

const String& Path::getFullPath() const
{
   if ( mIsDirtyPath )
   {
      mFullPath = _join();
      mIsDirtyPath = false;
   }

   return mFullPath;
}

String Path::getFullPathWithoutRoot() const
{
   return Torque::Path::Join(getPath(), '/', getFullFileName());
}

String Path::getRootAndPath() const
{
   if( mRoot != '/' )
      return Path::Join(mRoot, ':', mPath);
   else
      return mPath;
}

const String& Path::setRoot(const String &s)
{
   if ( mRoot != s )
   {
      mIsDirtyPath = true;
      mRoot = s;
   }

   return mRoot;
}

const String& Path::setPath(const String &s)
{
   String   clean = CleanSeparators(s);
   
   if ( mPath != clean )
   {
      mIsDirtyPath = true;
      mPath = clean;
   }

   return mPath;
}

const String& Path::setFileName(const String &s)
{
   if ( mFile != s )
   {
      mIsDirtyPath = true;
      mIsDirtyFileName = true;
      mFile = s;
   }

   return mFile;
}

const String& Path::setExtension(const String &s)
{
   if ( mExt != s )
   {
      mIsDirtyPath = true;
      mIsDirtyFileName = true;
      mExt = s;
   }

   return mExt; 
}

bool Path::isDirectory() const
{
   return mFile.isEmpty() && mExt.isEmpty();
}

bool Path::isRelative() const
{
   return (mPath.isEmpty() || mPath.c_str()[0] != '/');
}

bool Path::isAbsolute() const
{
   return (!mPath.isEmpty() && mPath.c_str()[0] == '/');
}

U32 Path::getDirectoryCount() const
{
   if (mPath.isEmpty())
      return 0;

   U32   count = 0;
   U32   offset = 0;

   if (mPath.c_str()[0] == '/')
      offset++;

   while (offset < mPath.length())
   {
      if (mPath[offset++] == '/')
         count++;
   }

   return count + 1;
}

String Path::getDirectory(U32 count) const
{
   if (mPath.isEmpty())
      return String();

   U32 offset = 0;

   if (mPath.c_str()[0] == '/')
      offset++;

   while (count && offset < mPath.length())
   {
      if (mPath[offset++] == '/')
         count--;
   }

   U32 end = offset;

   while (end < mPath.length() && mPath[end] != '/')
      end++;

   return mPath.substr(offset,end - offset);
}

//-----------------------------------------------------------------------------

String PathToPlatform(String file)
{
   if (Path::OsSeparator != '/')
      file.replace( Path::OsSeparator, '/' );

   return file;
}

String PathToOS(String file)
{
   if (Path::OsSeparator != '/')
      file.replace( '/', Path::OsSeparator );

   return file;
}

} // Namespace

