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
//
// Additional Copyrights
//		Copyright 2004 John Vanderbeck  
//		Copyright 2016 Chris Calef
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// This code implements support for SQLite into Torque and TorqueScript
//
// Essentially this creates a scriptable object that interfaces with SQLite.
//
// The supported SQL subset of SQLite can be found here:
// http://www.sqlite.org/lang.html
//-----------------------------------------------------------------------------

#ifndef _SQLITEOBJECT_H_
#define _SQLITEOBJECT_H_

#ifndef _SIMBASE_H_
#include "console/simBase.h"
#endif

#include "sqlite3.h"
#include "core/util/tVector.h"

struct sqlite_resultrow
{
   VectorPtr<char*> vColumnNames;
   VectorPtr<char*> vColumnValues;
};

struct sqlite_resultset
{
   S32                           iResultSet;
   S32                           iCurrentRow;
   S32                           iCurrentColumn;
   S32                           iNumRows;
   S32                           iNumCols;
   bool                          bValid;
   VectorPtr<sqlite_resultrow*>  vRows;
};


class SQLiteObject : public SimObject
{
   // This typedef is required for tie ins with the script language.
   //--------------------------------------------------------------------------
	protected:
      typedef SimObject Parent;
   //--------------------------------------------------------------------------

   public:
      SQLiteObject();
      ~SQLiteObject();

      // These are overloaded functions from SimObject that we handle for
      // tie in to the script language.  The .cc file has more in depth
      // comments on these.
      //-----------------------------------------------------------------------
      bool processArguments(S32 argc, const char **argv);
      bool onAdd();
      void onRemove();
      static void initPersistFields();
      //-----------------------------------------------------------------------

      //-----------------------------------------------------------------------
      // Called to open a database using the sqlite_open() function.
      // If the open fails, the function returns false, and sets the
      // global error string.  The script interface will automatically
      // call the onOpenFailed() script callback and pass this string
      // in if it fails.  If it succeeds the script interface will call
      // the onOpened() script callback.
      bool OpenDatabase(const char* filename);
      void CloseDatabase();
	  S32 loadOrSaveDb(const char *zFilename, bool isSave);//This code courtesy of sqlite.org.
      S32 ExecuteSQL(const char* sql);
      void NextRow(S32 resultSet);
      bool EndOfResult(S32 resultSet);
	  void escapeSingleQuotes(const char* source, char *dest);

      // support functions
      void ClearErrorString();
      void ClearResultSet(S32 index);
      sqlite_resultset* GetResultSet(S32 iResultSet);
      bool SaveResultSet(sqlite_resultset* pResultSet);
      S32 GetResultSetIndex(S32 iResultSet);
      S32 GetColumnIndex(S32 iResult, const char* columnName);
	  S32 numResultSets();

	  //Prepared Statements! We need a way to make them and extend them to script.
	  //void prepareStatement(sqlite3_stmt*,);
	  //void finalizeStatement();
	  //void bindInteger();
	  //...

	  sqlite3*                       m_pDatabase;
   private:
      char*                         m_szErrorString;
      VectorPtr<sqlite_resultset*>  m_vResultSets;
      S32                           m_iLastResultSet;
      S32                           m_iNextResultSet;
	  

   // This macro ties us into the script engine, and MUST MUST MUST be declared
   // in a public section of the class definition.  If it isn't you WILL get
   // errors that will confuse you.
   //--------------------------------------------------------------------------
   public:
   DECLARE_CONOBJECT(SQLiteObject);
	S32 getLastRowId() { return sqlite3_last_insert_rowid(m_pDatabase); };
   //--------------------------------------------------------------------------
};

#endif // _SQLITEOBJECT_H_