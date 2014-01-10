#include "stdafx.h"
#include "SQLite.h"
#include "Utils.h"

void SQLiteRow::InsertType(SQLiteRow::ColumnType type)
{
    ColumnItem ci;

    memset(&ci, 0, sizeof(ci));

    _columnType.push_back(type);
    _columnData.push_back(ci);
}

SQLiteRow::ColumnType SQLiteRow::GetType(int i)
{
    return _columnType[i];
}

void SQLiteRow::SetData(int i, void *value)
{
    switch (_columnType[i])
    {
    case TYPE_INT16:
        _columnData[i].vInt16 = *(int16_t *)value;
        break;

    case TYPE_INT32:
        _columnData[i].vInt32 = *(int32_t *)value;
        break;

    case TYPE_INT64:
        _columnData[i].vInt64 = *(int64_t *)value;
        break;

    case TYPE_STRING:
        _tcscpy_s(_columnData[i].vStr, _countof(_columnData[i].vStr), (TCHAR *)value);
        break;

    default:
        break;
    }
}

int32_t SQLiteRow::GetDataInt32(int i)
{
    return _columnData[i].vInt32;
}

int64_t SQLiteRow::GetDataInt64(int i)
{
    return _columnData[i].vInt64;
}

TCHAR *SQLiteRow::GetDataStr(int i)
{
    return _columnData[i].vStr;
}

// SQLite Database Object
sqlite3         *SQLite::db = 0;
int              SQLite::counter = 0;
CRITICAL_SECTION SQLite::stCS;

// Open Database
bool SQLite::Open(const TCHAR *fileName)
{
    InitializeCriticalSection(&stCS);

    // Convert to UTF-8
    char szFileName[MAX_PATH];
    Utils::Utf16ToUtf8(fileName, szFileName, MAX_PATH);

    return sqlite3_open(szFileName, &db) == SQLITE_OK;
}

// Close Database
bool SQLite::Close()
{
    //DeleteCriticalSection(&stCS);

    if( counter > 0 )
    {
        Exec(TEXT("Commit Transaction;"));
    }

    return sqlite3_close(db) != SQLITE_BUSY;
}

bool SQLite::Flush()
{
    EnterCriticalSection(&stCS);

    bool retVel = true;

    if( counter > 0 )
    {
        retVel = Exec(TEXT("Commit Transaction;"));
        counter = 0;
    }

    LeaveCriticalSection(&stCS);
    return retVel;
}

// Return if a table exist
bool SQLite::TableExist(const TCHAR *tableName)
{
    EnterCriticalSection(&stCS);

    bool retValue = false;
    sqlite3_stmt *stmt = 0;

    // Build Command
    TCHAR command[512];
    _stprintf_s(command, _countof(command), TEXT("Select name From sqlite_master Where type In ('table','view') And name == \'%s\'"), tableName);

    // Convert to UTF-8
    char szCommand[512];
    Utils::Utf16ToUtf8(command, szCommand, 512);

    // Prepare
    if( sqlite3_prepare_v2(db, szCommand, -1, &stmt, 0) == SQLITE_OK)
    {
        if ( sqlite3_step(stmt) == SQLITE_ROW ) // Table exist
        {
            retValue = true;
        }
    }

    // Finalize
    sqlite3_finalize(stmt);

    LeaveCriticalSection(&stCS);
    return retValue;
}

// Execute a SQL command that has no return value and is one-step-done
bool SQLite::Exec(const TCHAR *command)
{
    bool retValue = false;
    sqlite3_stmt *stmt = 0;

    // Convert to UTF-8
    char szCommand[512];
    Utils::Utf16ToUtf8(command, szCommand, 512);

    // Prepare
    if( sqlite3_prepare_v2(db, szCommand, -1, &stmt, 0) == SQLITE_OK)
    {
        // Execute
        if( sqlite3_step(stmt) == SQLITE_DONE )
        {
            retValue = true;
        }
    }

    // Finalize
    sqlite3_finalize(stmt);

    return retValue;
}

// Execute a SQL command that has no return value and is one-step-done
bool SQLite::Exec(const TCHAR *command, bool cached)
{
    EnterCriticalSection(&stCS);
    bool retValue;

    if( !cached )
    {
        if( counter == 0 )
        {
            retValue = Exec(command);
        }
        else
        {
            retValue = Exec(command);
            retValue = Exec(TEXT("Commit Transaction;"));
            counter = 0;
        }
    }
    else
    {
        if( counter == 0 )
        {
            retValue = Exec(TEXT("Begin Transaction;"));
            retValue = Exec(command);
            counter += 1;
        }
        else
        {
            retValue = Exec(command);
            counter += 1;

            if( counter == 4000 )
            {
                retValue = Exec(TEXT("Commit Transaction;"));
                counter = 0;
            }
        }
    }

    LeaveCriticalSection(&stCS);
    return retValue;
}

// Select
bool SQLite::Select(const TCHAR *command, SQLiteRow *row, SQLiteCallback callback)
{
    EnterCriticalSection(&stCS);

    bool retValue = false;
    sqlite3_stmt *stmt = 0;

    // Convert to UTF-8
    char szCommand[512];
    Utils::Utf16ToUtf8(command, szCommand, 512);

    if( sqlite3_prepare_v2(db, szCommand, -1, &stmt, 0) == SQLITE_OK)
    {
        int result;

        do
        {
            result = sqlite3_step(stmt);

            if ( result == SQLITE_ROW ) // There's data returned
            {
                for(int i = 0; i < sqlite3_column_count(stmt); i++)
                {
                    int16_t vInt16;
                    int32_t vInt32;
                    int64_t vInt64;
                    char   *vStrUtf8;
                    TCHAR   vStrUtf16[1024];

                    // Save data for current column
                    switch (row->GetType(i))
                    {
                    case SQLiteRow::TYPE_INT16:
                        vInt16 = (int16_t)sqlite3_column_int(stmt, i);
                        row->SetData(i, (void *)&vInt16);
                        break;

                    case SQLiteRow::TYPE_INT32:
                        vInt32 = (int32_t)sqlite3_column_int(stmt, i);
                        row->SetData(i, (void *)&vInt32);
                        break;

                    case SQLiteRow::TYPE_INT64:
                        vInt64 = (int64_t)sqlite3_column_int64(stmt, i);
                        row->SetData(i, (void *)&vInt64);
                        break;

                    case SQLiteRow::TYPE_STRING:
                        vStrUtf8 = (char *)sqlite3_column_text(stmt, i);
                        
                        // Convert to UTF-16
                        Utils::Utf8ToUtf16(vStrUtf8, vStrUtf16, _countof(vStrUtf16));

                        row->SetData(i, (void *)vStrUtf16);
                        break;

                    default:
                        break;
                    }
                }

                // Call back
                callback(row);
            }
            else if( result == SQLITE_DONE ) // No more data
            {
                retValue = true;
            }
        } while( result == SQLITE_ROW );
    }

    // Finalize
    sqlite3_finalize(stmt);

    LeaveCriticalSection(&stCS);
    return retValue;
}

bool SQLite::Select(const TCHAR *command, SQLiteRow *row)
{
    EnterCriticalSection(&stCS);

    bool retValue = false;
    sqlite3_stmt *stmt = 0;

    // Convert to UTF-8
    char szCommand[512];
    Utils::Utf16ToUtf8(command, szCommand, 512);

    if( sqlite3_prepare_v2(db, szCommand, -1, &stmt, 0) == SQLITE_OK)
    {
        int result = sqlite3_step(stmt);

        if ( result == SQLITE_ROW ) // There's data returned
        {
            retValue = true;

            for(int i = 0; i < sqlite3_column_count(stmt); i++)
            {
                int16_t vInt16;
                int32_t vInt32;
                int64_t vInt64;
                char   *vStrUtf8;
                TCHAR   vStrUtf16[1024];

                // Save data for current column
                switch (row->GetType(i))
                {
                case SQLiteRow::TYPE_INT16:
                    vInt16 = (int16_t)sqlite3_column_int(stmt, i);
                    row->SetData(i, (void *)&vInt16);
                    break;

                case SQLiteRow::TYPE_INT32:
                    vInt32 = (int32_t)sqlite3_column_int(stmt, i);
                    row->SetData(i, (void *)&vInt32);
                    break;

                case SQLiteRow::TYPE_INT64:
                    vInt64 = (int64_t)sqlite3_column_int64(stmt, i);
                    row->SetData(i, (void *)&vInt64);
                    break;

                case SQLiteRow::TYPE_STRING:
                    vStrUtf8 = (char *)sqlite3_column_text(stmt, i);
                        
                    // Convert to UTF-16
                    Utils::Utf8ToUtf16(vStrUtf8, vStrUtf16, _countof(vStrUtf16));

                    row->SetData(i, (void *)vStrUtf16);
                    break;

                default:
                    break;
                }
            }
        }
    }

    // Finalize
    sqlite3_finalize(stmt);

    LeaveCriticalSection(&stCS);
    return retValue;
}

__int64 SQLite::GetLastInsertRowId()
{
    return sqlite3_last_insert_rowid(db);
}