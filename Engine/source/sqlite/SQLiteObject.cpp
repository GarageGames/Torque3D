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
//-----------------------------------------------------------------------------

#include "SQLiteObject.h"

#include "console/simBase.h"
#include "console/engineAPI.h"

#include "console/consoleInternal.h"
#include <cstdlib>

IMPLEMENT_CONOBJECT(SQLiteObject);


SQLiteObject::SQLiteObject()
{
   m_pDatabase = NULL;
   m_szErrorString = NULL;
   m_iLastResultSet = 0;
   m_iNextResultSet = 1;
}

SQLiteObject::~SQLiteObject()
{
	S32 index;
	// if we still have a database open, close it
	CloseDatabase();
	// Clear out any error string we may have left
	ClearErrorString();
	// Clean up result sets
	//
	// Boy oh boy this is such a crazy hack!
	// I can't seem to iterate through a vector and clean it up without screwing the vector.
	// So (HACK HACK HACK) what i'm doing for now is making a temporary vector that
	// contains a list of the result sets that the user hasn't cleaned up.
	// Clean up all those result sets, then delete the temp vector.
	Vector<S32> vTemp;
	Vector<S32>::iterator iTemp;

	VectorPtr<sqlite_resultset*>::iterator i;
	for (i = m_vResultSets.begin(); i != m_vResultSets.end(); i++)
	{
		vTemp.push_back((*i)->iResultSet);
	}
	index = 0;
	for (iTemp = vTemp.begin(); iTemp != vTemp.end(); iTemp++)
	{
		Con::warnf("SQLiteObject Warning: Result set #%i was not cleared by script.  Clearing it now.", vTemp[index]);
		ClearResultSet(vTemp[index]);
		index++;
	}

	m_vResultSets.clear();

}

bool SQLiteObject::processArguments(S32 argc, const char **argv)
{
	if (argc == 0)
		return true;
	else
		return true;
}

bool SQLiteObject::onAdd()
{
	if (!Parent::onAdd())
		return false;

	const char *name = getName();
	if (name && name[0] && getClassRep())
	{
		Namespace *parent = getClassRep()->getNameSpace();
		Con::linkNamespaces(parent->mName, name);
		mNameSpace = Con::lookupNamespace(name);
	}

	return true;
}

// This is the function that gets called when an instance
// of your object is being removed from the system and being
// destroyed.  Use this to do your clean up and what not.
void SQLiteObject::onRemove()
{
	CloseDatabase();
	Parent::onRemove();
}

// To be honest i'm not 100% sure on when this is called yet.
// Basically its used to set the values of any persistant fields
// the object has.  Similiar to the way datablocks work.  I'm
// just not sure how and when this gets called.
void SQLiteObject::initPersistFields()
{
	Parent::initPersistFields();
}

//-----------------------------------------------------------------------
// These functions below are our custom functions that we will tie into
// script.

S32 Callback(void *pArg, S32 argc, char **argv, char **columnNames)
{
	// basically this callback is called for each row in the SQL query result.
	// for each row, argc indicates how many columns are returned.
	// columnNames[i] is the name of the column
	// argv[i] is the value of the column

	sqlite_resultrow* pRow;
	sqlite_resultset* pResultSet;
	char* name;
	char* value;
	S32 i;

	if (argc == 0)
		return 0;

	pResultSet = (sqlite_resultset*)pArg;
	if (!pResultSet)
		return -1;

	// create a new result row
	pRow = new sqlite_resultrow;
	pResultSet->iNumCols = argc;
	// loop through all the columns and stuff them into our row
	for (i = 0; i < argc; i++)
	{
		// DBEUG CODE
  //      Con::printf("%s = %s\n", columnNames[i], argv[i] ? argv[i] : "NULL");
		dsize_t columnNameLen = dStrlen(columnNames[i]) + 1;
		name = new char[columnNameLen];
		dStrcpy(name, columnNames[i], columnNameLen);
		pRow->vColumnNames.push_back(name);
		if (argv[i])
		{
			dsize_t valueLen = dStrlen(argv[i]) + 1;
			value = new char[valueLen];
			dStrcpy(value, argv[i], valueLen);
			pRow->vColumnValues.push_back(value);
		}
		else
		{
			value = new char[10];
			dStrcpy(value, "NULL", 10);
			pRow->vColumnValues.push_back(value);
		}
	}
	pResultSet->iNumRows++;
	pResultSet->vRows.push_back(pRow);

	// return 0 or else the sqlexec will be aborted.
	return 0;
}

bool SQLiteObject::OpenDatabase(const char* filename)
{
	// check to see if we already have an open database, and
	// if so, close it.
	CloseDatabase();

	Con::printf("CALLING THE OLD OPEN DATABASE FUNCTION!!!!!!!!!!!!!!!!!!");

	// We persist the error string so that the script may make a  
	 // GetLastError() call at any time.  However when we get  
	 // ready to make a call which could result in a new error,   
	 // we need to clear what we have to avoid a memory leak.  
	ClearErrorString();

	S32 isOpen = sqlite3_open(filename, &m_pDatabase);
	if (isOpen == SQLITE_ERROR)
	{
		// there was an error and the database could not  
		// be opened.  
		m_szErrorString = (char *)sqlite3_errmsg(m_pDatabase);
		Con::executef(this, "2", "onOpenFailed()", m_szErrorString);
		return false;
	}
	else
	{
		// database was opened without error
		Con::executef(this, "1", "onOpened()");

		//Now, for OpenSimEarth, load spatialite dll, so we can have GIS functions.
		//S32 canLoadExt = sqlite3_load_extension(m_pDatabase,"mod_spatialite.dll",0,0);
		//Con::printf("opened spatialite extension: %d",canLoadExt);
		//Sigh, no luck yet. Cannot find function GeomFromText().
	}
	return true;
}

S32 SQLiteObject::ExecuteSQL(const char* sql)
{
	S32 iResult;
	sqlite_resultset* pResultSet;

	// create a new resultset
	pResultSet = new sqlite_resultset;

	if (pResultSet)
	{
		pResultSet->bValid = false;
		pResultSet->iCurrentColumn = 0;
		pResultSet->iCurrentRow = 0;
		pResultSet->iNumCols = 0;
		pResultSet->iNumRows = 0;
		pResultSet->iResultSet = m_iNextResultSet;
		pResultSet->vRows.clear();
		m_iLastResultSet = m_iNextResultSet;
		m_iNextResultSet++;
	}
	else
		return 0;

	iResult = sqlite3_exec(m_pDatabase, sql, Callback, (void*)pResultSet, &m_szErrorString);
	if (iResult == 0)
	{
		//SQLITE_OK
		SaveResultSet(pResultSet);
		Con::executef(this, "1", "onQueryFinished()");
		return pResultSet->iResultSet;
	}
	else
	{
		// error occured
		Con::executef(this, "2", "onQueryFailed", m_szErrorString);
		Con::errorf("SQLite failed to execute query, error %s", m_szErrorString);
		delete pResultSet;
		return 0;
	}

	return 0;
}

void SQLiteObject::CloseDatabase()
{
	if (m_pDatabase)
		sqlite3_close(m_pDatabase);

	m_pDatabase = NULL;
}

//(The following function is courtesy of sqlite.org, minus changes to use m_pDatabase instead of pInMemory.)
/*
** This function is used to load the contents of a database file on disk
** into the "main" database of open database connection pInMemory, or
** to save the current contents of the database opened by pInMemory into
** a database file on disk. pInMemory is probably an in-memory database,
** but this function will also work fine if it is not.
**
** Parameter zFilename points to a null-terminated string containing the
** name of the database file on disk to load from or save to. If parameter
** isSave is non-zero, then the contents of the file zFilename are
** overwritten with the contents of the database opened by pInMemory. If
** parameter isSave is zero, then the contents of the database opened by
** pInMemory are replaced by data loaded from the file zFilename.
**
** If the operation is successful, SQLITE_OK is returned. Otherwise, if
** an error occurs, an SQLite error code is returned.
*/
S32 SQLiteObject::loadOrSaveDb(const char *zFilename, bool isSave) 
{
	S32 rc;                   /* Function return code */
	sqlite3 *pFile;           /* Database connection opened on zFilename */
	sqlite3_backup *pBackup;  /* Backup object used to copy data */
	sqlite3 *pTo;             /* Database to copy to (pFile or pInMemory) */
	sqlite3 *pFrom;           /* Database to copy from (pFile or pInMemory) */

							  /* Open the database file identified by zFilename. Exit early if this fails
							  ** for any reason. */

	Con::printf("calling loadOrSaveDb, isSave = %d", isSave);

	if (isSave == false)
	{//If we're loading, have to create the memory database.
		if (!(SQLITE_OK == sqlite3_open(":memory:", &m_pDatabase)))
		{
			Con::printf("Unable to open a memory database!");
			return 0;
		}
	}
	rc = sqlite3_open(zFilename, &pFile);
	if (rc == SQLITE_OK) {

		/* If this is a 'load' operation (isSave==0), then data is copied
		** from the database file just opened to database pInMemory.
		** Otherwise, if this is a 'save' operation (isSave==1), then data
		** is copied from pInMemory to pFile.  Set the variables pFrom and
		** pTo accordingly. */
		pFrom = (isSave ? m_pDatabase : pFile);
		pTo = (isSave ? pFile : m_pDatabase);

		/* Set up the backup procedure to copy from the "main" database of
		** connection pFile to the main database of connection pInMemory.
		** If something goes wrong, pBackup will be set to NULL and an error
		** code and message left in connection pTo.
		**
		** If the backup object is successfully created, call backup_step()
		** to copy data from pFile to pInMemory. Then call backup_finish()
		** to release resources associated with the pBackup object.  If an
		** error occurred, then an error code and message will be left in
		** connection pTo. If no error occurred, then the error code belonging
		** to pTo is set to SQLITE_OK.
		*/
		pBackup = sqlite3_backup_init(pTo, "main", pFrom, "main");
		if (pBackup) {
			(void)sqlite3_backup_step(pBackup, -1);
			(void)sqlite3_backup_finish(pBackup);
		}
		rc = sqlite3_errcode(pTo);
	}

	/* Close the database connection opened on database file zFilename
	** and return the result of this function. */
	(void)sqlite3_close(pFile);

	//if (isSave ==  true)  // Actually, cancel this, I'm sure it will happen automatically and if we don't do it here, we can also use
	//{                     // this function for periodic saves, such as after saving mission.
	//	sqlite3_close(m_pDatabase);
	//}

	Con::printf("finished loadOrSaveDb, rc = %d", rc);

	if (rc == 0)
		return true;
	else
		return false;
}

void SQLiteObject::NextRow(S32 resultSet)
{
	sqlite_resultset* pResultSet;

	pResultSet = GetResultSet(resultSet);
	if (!pResultSet)
		return;

	pResultSet->iCurrentRow++;
}

bool SQLiteObject::EndOfResult(S32 resultSet)
{
	sqlite_resultset* pResultSet;

	pResultSet = GetResultSet(resultSet);
	if (!pResultSet)
		return true;

	if (pResultSet->iCurrentRow >= pResultSet->iNumRows)
		return true;

	return false;
}

void SQLiteObject::ClearErrorString()
{
	if (m_szErrorString)
		sqlite3_free(m_szErrorString);

	m_szErrorString = NULL;
}

void SQLiteObject::ClearResultSet(S32 index)
{
	if (index <= 0)
		return;

	sqlite_resultset* resultSet;
	S32 iResultSet;

	// Get the result set specified by index
	resultSet = GetResultSet(index);
	iResultSet = GetResultSetIndex(index);
	if ((!resultSet) || (!resultSet->bValid))
	{
		Con::warnf("Warning SQLiteObject::ClearResultSet(%i) failed to retrieve specified result set.  Result set was NOT cleared.", index);
		return;
	}
	// Now we have the specific result set to be cleared.
	// What we need to do now is iterate through each "Column" in each "Row"
	// and free the strings, then delete the entries.
	VectorPtr<sqlite_resultrow*>::iterator iRow;
	VectorPtr<char*>::iterator iColumnName;
	VectorPtr<char*>::iterator iColumnValue;

	for (iRow = resultSet->vRows.begin(); iRow != resultSet->vRows.end(); iRow++)
	{
		// Iterate through rows
		// for each row iterate through all the column values and names
		for (iColumnName = (*iRow)->vColumnNames.begin(); iColumnName != (*iRow)->vColumnNames.end(); iColumnName++)
		{
			// Iterate through column names.  Free the memory.
			delete[](*iColumnName);
		}
		for (iColumnValue = (*iRow)->vColumnValues.begin(); iColumnValue != (*iRow)->vColumnValues.end(); iColumnValue++)
		{
			// Iterate through column values.  Free the memory.
			delete[](*iColumnValue);
		}
		// free memory used by the row
		delete (*iRow);
	}
	// empty the resultset
	resultSet->vRows.clear();
	resultSet->bValid = false;
	delete resultSet;
	m_vResultSets.erase_fast(iResultSet);
}

sqlite_resultset* SQLiteObject::GetResultSet(S32 iResultSet)
{
	// Get the result set specified by iResultSet
	VectorPtr<sqlite_resultset*>::iterator i;
	for (i = m_vResultSets.begin(); i != m_vResultSets.end(); i++)
	{
		if ((*i)->iResultSet == iResultSet)
			break;
	}

	return *i;
}

S32 SQLiteObject::GetResultSetIndex(S32 iResultSet)
{
	S32 iIndex;
	// Get the result set specified by iResultSet
	VectorPtr<sqlite_resultset*>::iterator i;
	iIndex = 0;
	for (i = m_vResultSets.begin(); i != m_vResultSets.end(); i++)
	{
		if ((*i)->iResultSet == iResultSet)
			break;
		iIndex++;
	}

	return iIndex;
}

bool SQLiteObject::SaveResultSet(sqlite_resultset* pResultSet)
{
	// Basically just add this to our vector.  It should already be filled up.
	pResultSet->bValid = true;
	m_vResultSets.push_back(pResultSet);

	return true;
}

S32 SQLiteObject::GetColumnIndex(S32 iResult, const char* columnName)
{
	S32 iIndex;
	VectorPtr<char*>::iterator i;
	sqlite_resultset* pResultSet;
	sqlite_resultrow* pRow;

	pResultSet = GetResultSet(iResult);
	if (!pResultSet)
		return 0;

	pRow = pResultSet->vRows[0];
	if (!pRow)
		return 0;

	iIndex = 0;
	for (i = pRow->vColumnNames.begin(); i != pRow->vColumnNames.end(); i++)
	{
		if (dStricmp((*i), columnName) == 0)
			return iIndex + 1;
		iIndex++;
	}

	return 0;
}

S32 SQLiteObject::numResultSets()
{
	return m_vResultSets.size();
}

void SQLiteObject::escapeSingleQuotes(const char* source, char *dest)
{
	//To Do: This function needs to step through the source string and insert another single quote  
	//immediately after every single quote it finds.

}


//-----------------------------------------------------------------------
// These functions are the code that actually tie our object into the scripting
// language.  As you can see each one of these is called by script and in turn
// calls the C++ class function.
// FIX: change all these to DefineEngineMethod!

ConsoleMethod(SQLiteObject, openDatabase, bool, 3, 3, "(const char* filename) Opens the database specifed by filename.  Returns true or false.")
{
	return object->OpenDatabase(argv[2]);
}

DefineEngineMethod(SQLiteObject, loadOrSaveDb, bool, (const char* filename, bool isSave), ,
	"Loads or saves a cached database from the disk db specifed by filename. Second argument determines loading (false) or saving (true). Returns true or false.")
{
	return object->loadOrSaveDb(filename, isSave);
}

ConsoleMethod(SQLiteObject, closeDatabase, void, 2, 2, "Closes the active database.")
{
	object->CloseDatabase();
}

ConsoleMethod(SQLiteObject, query, S32, 4, 0, "(const char* sql, S32 mode) Performs an SQL query on the open database and returns an identifier to a valid result set. mode is currently unused, and is reserved for future use.")
{
	S32 iCount;
	S32 iIndex, iLen, iNewIndex, iArg, iArgLen, i;
	char* szNew;

	if (argc == 4)
		return object->ExecuteSQL(argv[2]);
	else if (argc > 4)
	{
		// Support for printf type querys, as per Ben Garney's suggestion
		// Basically what this does is allow the user to insert question marks into their query that will
		// be replaced with actual data.  For example:
		// "SELECT * FROM data WHERE id=? AND age<7 AND name LIKE ?"

		// scan the query and count the question marks
		iCount = 0;
		iLen = dStrlen(argv[2]);
		for (iIndex = 0; iIndex < iLen; iIndex++)
		{
			if (argv[2][iIndex] == '?')
				iCount++;
		}

		// now that we know how many replacements we have, we need to make sure we
		// have enough arguments to replace them all.  All arguments above 4 should be our data
		if (argc - 4 == iCount)
		{
			// ok we have the correct number of arguments
			// so now we need to calc the length of the new query string.  This is easily achieved.
			// We simply take our base string length, subtract the question marks, then add in
			// the number of total characters used by our arguments.
			iLen = dStrlen(argv[2]) - iCount;
			for (iIndex = 1; iIndex <= iCount; iIndex++)
			{
				iLen = iLen + dStrlen(argv[iIndex + 3]);
			}
			// iLen should now be the length of our new string
			szNew = new char[iLen];

			// now we need to replace all the question marks with the actual arguments
			iLen = dStrlen(argv[2]);
			iNewIndex = 0;
			iArg = 1;
			for (iIndex = 0; iIndex <= iLen; iIndex++)
			{
				if (argv[2][iIndex] == '?')
				{
					// ok we need to replace this question mark with the actual argument
					// and iterate our pointers and everything as needed.  This is no doubt
					// not the best way to do this, but it works for me for now.
					// My god this is really a mess.
					iArgLen = dStrlen(argv[iArg + 3]);
					// copy first character
					szNew[iNewIndex] = argv[iArg + 3][0];
					// copy rest of characters, and increment iNewIndex
					for (i = 1; i < iArgLen; i++)
					{
						iNewIndex++;
						szNew[iNewIndex] = argv[iArg + 3][i];
					}
					iArg++;

				}
				else
					szNew[iNewIndex] = argv[2][iIndex];

				iNewIndex++;
			}
		}
		else
			return 0; // incorrect number of question marks vs arguments
		Con::printf("Old SQL: %s\nNew SQL: %s", argv[2].getStringValue(), szNew);
		return object->ExecuteSQL(szNew);
	}

	return 0;
}

ConsoleMethod(SQLiteObject, clearResult, void, 3, 3, "(S32 resultSet) Clears memory used by the specified result set, and deletes the result set.")
{
	object->ClearResultSet(dAtoi(argv[2]));
}

ConsoleMethod(SQLiteObject, nextRow, void, 3, 3, "(S32 resultSet) Moves the result set's row pointer to the next row.")
{
	sqlite_resultset* pResultSet;
	pResultSet = object->GetResultSet(dAtoi(argv[2]));
	if (pResultSet)
	{
		pResultSet->iCurrentRow++;
	}
}

ConsoleMethod(SQLiteObject, previousRow, void, 3, 3, "(S32 resultSet) Moves the result set's row pointer to the previous row")
{
	sqlite_resultset* pResultSet;
	pResultSet = object->GetResultSet(dAtoi(argv[2]));
	if (pResultSet)
	{
		pResultSet->iCurrentRow--;
	}
}

ConsoleMethod(SQLiteObject, firstRow, void, 3, 3, "(S32 resultSet) Moves the result set's row pointer to the very first row in the result set.")
{
	sqlite_resultset* pResultSet;
	pResultSet = object->GetResultSet(dAtoi(argv[2]));
	if (pResultSet)
	{
		pResultSet->iCurrentRow = 0;
	}
}

ConsoleMethod(SQLiteObject, lastRow, void, 3, 3, "(S32 resultSet) Moves the result set's row pointer to the very last row in the result set.")
{
	sqlite_resultset* pResultSet;
	pResultSet = object->GetResultSet(dAtoi(argv[2]));
	if (pResultSet)
	{
		pResultSet->iCurrentRow = pResultSet->iNumRows - 1;
	}
}

ConsoleMethod(SQLiteObject, setRow, void, 4, 4, "(S32 resultSet S32 row) Moves the result set's row pointer to the row specified.  Row indices start at 1 not 0.")
{
	sqlite_resultset* pResultSet;
	pResultSet = object->GetResultSet(dAtoi(argv[2]));
	if (pResultSet)
	{
		pResultSet->iCurrentRow = dAtoi(argv[3]) - 1;
	}
}

ConsoleMethod(SQLiteObject, getRow, S32, 3, 3, "(S32 resultSet) Returns what row the result set's row pointer is currently on.")
{
	sqlite_resultset* pResultSet;
	pResultSet = object->GetResultSet(dAtoi(argv[2]));
	if (pResultSet)
	{
		return pResultSet->iCurrentRow + 1;
	}
	else
		return 0;
}

ConsoleMethod(SQLiteObject, numRows, S32, 3, 3, "(S32 resultSet) Returns the number of rows in the result set.")
{
	sqlite_resultset* pResultSet;
	pResultSet = object->GetResultSet(dAtoi(argv[2]));
	if (pResultSet)
	{
		return pResultSet->iNumRows;
	}
	else
		return 0;
}

ConsoleMethod(SQLiteObject, numColumns, S32, 3, 3, "(S32 resultSet) Returns the number of columns in the result set.")
{
	sqlite_resultset* pResultSet;
	pResultSet = object->GetResultSet(dAtoi(argv[2]));
	if (pResultSet)
	{
		return pResultSet->iNumCols;
	}
	else
		return 0;
}

ConsoleMethod(SQLiteObject, endOfResult, bool, 3, 3, "(S32 resultSet) Checks to see if the internal pointer for the specified result set is at the end, indicating there are no more rows left to read.")
{
	return object->EndOfResult(dAtoi(argv[2]));
}

ConsoleMethod(SQLiteObject, EOR, bool, 3, 3, "(S32 resultSet) Same as endOfResult().")
{
	return object->EndOfResult(dAtoi(argv[2]));
}

ConsoleMethod(SQLiteObject, EOF, bool, 3, 3, "(S32 resultSet) Same as endOfResult().")
{
	return object->EndOfResult(dAtoi(argv[2]));
}

ConsoleMethod(SQLiteObject, getColumnIndex, S32, 4, 4, "(resultSet columnName) Looks up the specified column name in the specified result set, and returns the columns index number.  A return value of 0 indicates the lookup failed for some reason (usually this indicates you specified a column name that doesn't exist or is spelled wrong).")
{
	return object->GetColumnIndex(dAtoi(argv[2]), argv[3]);
}

ConsoleMethod(SQLiteObject, getColumnName, const char *, 4, 4, "(resultSet columnIndex) Looks up the specified column index in the specified result set, and returns the column's name.  A return value of an empty string indicates the lookup failed for some reason (usually this indicates you specified a column index that is invalid or exceeds the number of columns in the result set). Columns are index starting with 1 not 0")
{
	sqlite_resultset* pResultSet;
	sqlite_resultrow* pRow;
	S32 iColumn;

	pResultSet = object->GetResultSet(dAtoi(argv[2]));
	if (pResultSet)
	{
		pRow = pResultSet->vRows[pResultSet->iCurrentRow];
		if (!pRow)
			return "";

		// We assume they specified column by index.  If they know the column name they wouldn't be calling this function :)
		iColumn = dAtoi(argv[3]);
		if (iColumn == 0)
			return "";  // column indices start at 1, not 0

		 // now we should have an index for our column name
		if (pRow->vColumnNames[iColumn])
			return pRow->vColumnNames[iColumn];
		else
			return "";
	}
	else
		return "";
}

ConsoleMethod(SQLiteObject, getColumn, const char *, 4, 4, "(resultSet column) Returns the value of the specified column (Column can be specified by name or index) in the current row of the specified result set. If the call fails, the returned string will indicate the error.")
{
	sqlite_resultset* pResultSet;
	sqlite_resultrow* pRow;
	S32 iColumn;

	pResultSet = object->GetResultSet(dAtoi(argv[2]));
	if (pResultSet)
	{
		if (pResultSet->vRows.size() == 0)
			return "NULL";

		pRow = pResultSet->vRows[pResultSet->iCurrentRow];
		if (!pRow)
			return "invalid_row";

		// Is column specified by a name or an index?
		iColumn = dAtoi(argv[3]);
		if (iColumn == 0)
		{
			// column was specified by a name
			iColumn = object->GetColumnIndex(dAtoi(argv[2]), argv[3]);
			// if this is still 0 then we have some error
			if (iColumn == 0)
				return "invalid_column";
		}

		// We temporarily padded the index in GetColumnIndex() so we could return a 
		// 0 for error.  So now we need to drop it back down.
		iColumn--;

		// now we should have an index for our column data
		if (pRow->vColumnValues[iColumn])
			return pRow->vColumnValues[iColumn];
		else
			return "NULL";
	}
	else
		return "invalid_result_set";
}

ConsoleMethod(SQLiteObject, getColumnNumeric, F32, 4, 4, "(resultSet column) Returns the value of the specified column (Column can be specified by name or index) in the current row of the specified result set. If the call fails, the returned string will indicate the error.")
{
	sqlite_resultset* pResultSet;
	sqlite_resultrow* pRow;
	S32 iColumn;

	pResultSet = object->GetResultSet(dAtoi(argv[2]));
	if (pResultSet)
	{

		if (pResultSet->vRows.size() == 0)
			return -1;

		pRow = pResultSet->vRows[pResultSet->iCurrentRow];
		if (!pRow)
			return -1;//"invalid_row";

		 // Is column specified by a name or an index?
		iColumn = dAtoi(argv[3]);
		if (iColumn == 0)
		{
			// column was specified by a name
			iColumn = object->GetColumnIndex(dAtoi(argv[2]), argv[3]);
			// if this is still 0 then we have some error
			if (iColumn == 0)
				return -1;//"invalid_column";
		}

		// We temporarily padded the index in GetColumnIndex() so we could return a 
		// 0 for error.  So now we need to drop it back down.
		iColumn--;

		// now we should have an index for our column data
		if (pRow->vColumnValues[iColumn])
			return dAtof(pRow->vColumnValues[iColumn]);
		else
			return 0;
	}
	else
		return -1;//"invalid_result_set";
}

ConsoleMethod(SQLiteObject, escapeString, const char *, 3, 3, "(string) Escapes the given string, making it safer to pass into a query.")
{
	// essentially what we need to do here is scan the string for any occurrences of: ', ", and \
    // and prepend them with a slash: \', \", \\

	// to do this we first need to know how many characters we are replacing so we can calculate
	// the size of the new string
	S32 iCount;
	S32 iIndex, iLen, iNewIndex;
	char* szNew;

	iCount = 0;
	iLen = dStrlen(argv[2]);
	for (iIndex = 0; iIndex < iLen; iIndex++)
	{
		if (argv[2][iIndex] == '\'')
			iCount++;
		else if (argv[2][iIndex] == '\"')
			iCount++;
		else if (argv[2][iIndex] == '\\')
			iCount++;

	}
	//   Con::printf("escapeString counts %i instances of characters to be escaped.  New string will be %i characters longer for a total of %i characters.", iCount, iCount, iLen+iCount);
	szNew = new char[iLen + iCount];
	iNewIndex = 0;
	for (iIndex = 0; iIndex <= iLen; iIndex++)
	{
		if (argv[2][iIndex] == '\'')
		{
			szNew[iNewIndex] = '\\';
			iNewIndex++;
			szNew[iNewIndex] = '\'';
		}
		else if (argv[2][iIndex] == '\"')
		{
			szNew[iNewIndex] = '\\';
			iNewIndex++;
			szNew[iNewIndex] = '\"';
		}
		else if (argv[2][iIndex] == '\\')
		{
			szNew[iNewIndex] = '\\';
			iNewIndex++;
			szNew[iNewIndex] = '\\';
		}
		else
			szNew[iNewIndex] = argv[2][iIndex];

		iNewIndex++;
	}
	//   Con::printf("Last characters of each string (new, old): %s, %s", argv[2][iIndex-1], szNew[iNewIndex-1]);
	//   Con::printf("Old String: %s\nNew String: %s", argv[2], szNew);

	return szNew;
}


ConsoleMethod(SQLiteObject, numResultSets, S32, 2, 2, "numResultSets()")
{
	return object->numResultSets();
}

ConsoleMethod(SQLiteObject, getLastRowId, S32, 2, 2, "getLastRowId()")
{
	return object->getLastRowId();
}