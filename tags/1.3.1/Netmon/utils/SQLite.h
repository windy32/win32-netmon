#ifndef SQLITE_H
#define SQLITE_H

#include "../sqlite/sqlite3.h"

class SQLiteRow;

typedef signed short   int16_t;
typedef signed int     int32_t;
typedef signed __int64 int64_t;

typedef void (* SQLiteCallback)(SQLiteRow *);

class SQLiteRow
{
public:
	typedef enum enumSQLiteColumnType
	{
		TYPE_INT16,
		TYPE_INT32,
		TYPE_INT64,
		TYPE_STRING
	} ColumnType;

protected:
	// In this way, the variant item can be a 
	//  - signed int16
	//  - signed int32
	//  - signed int64
	//  - Null terminated string not longer than 1023 bytes
	typedef union unionColumnItem
	{
		int16_t vInt16;
		int32_t vInt32;
		int64_t vInt64;
		TCHAR   vStr[1024];
	} ColumnItem;

	std::vector<ColumnType> _columnType;
	std::vector<ColumnItem> _columnData;

public:
	void       InsertType(ColumnType type);
	ColumnType GetType(int i);

	void       SetData(int i, void *value);
	int32_t    GetDataInt32(int i);
	int64_t    GetDataInt64(int i);
	TCHAR     *GetDataStr(int i);
};

class SQLite
{
protected:
	// SQLite Database Object
	static sqlite3 *db;

	// Transaction Counter
	static int counter;

	// Critical Section
	static CRITICAL_SECTION stCS;

protected:
	// Execute a SQL command that has no return value and is one-step-done
	static bool Exec(const TCHAR *command);

public:
	// Open Database
	static bool Open(const TCHAR *fileName);

	// Close Database
	static bool Close();

	// Return if a table exist
	static bool TableExist(const TCHAR *tableName);

	// Execute a SQL command that has no return value and is one-step-done
	static bool Exec(const TCHAR *command, bool cached);

	// Execute cached commands (commit transaction)
	static bool Flush();

	// Select
	//  - command [In]
	//      The select command.
	//  - row [Out]
	//      When a full row has been read, data will be written to row,
	//  - callback [In]
	//      and then "callback" is called so that caller can process the coming data of the row
	static bool Select(const TCHAR *command, SQLiteRow *row, SQLiteCallback callback);

	// Overrided Version (Not more than one row will be selected)
	static bool Select(const TCHAR *command, SQLiteRow *row);

	static __int64 GetLastInsertRowId();
};

#endif
