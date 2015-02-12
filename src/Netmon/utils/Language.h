// Copyright (C) 2012-2014 F32 (feng32tc@gmail.com)
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 3 as
// published by the Free Software Foundation;
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 

#ifndef LANGUAGE_H
#define LANGUAGE_H

#include "../../Lang/resource.h"

class Language
{
protected:
    enum 
    { 
        // Length of a string table (see "Netmon/Lang/resource.h")
        STRING_TABLE_MIN  = 40000,
        STRING_TABLE_MAX  = 40095,
        STRING_TABLE_SIZE = STRING_TABLE_MAX - STRING_TABLE_MIN + 1
    };

    class StringTable
    {
    protected:
        enum
        {
            MAX_LEN = 256 // Max length of a string
        };
        TCHAR _table[STRING_TABLE_SIZE][MAX_LEN];

    public:
        StringTable()
        {
            RtlZeroMemory(_table, sizeof(_table));
        }

        const TCHAR *Get(int strId)
        {
            return _table[strId - STRING_TABLE_MIN];
        }

        void Set(int strId, const TCHAR *str)
        {
            _tcscpy_s(_table[strId - STRING_TABLE_MIN], MAX_LEN, str);
        }
    };

protected:
    static int _curLang;
    static std::vector<StringTable> _tables;

public:
    // Search for language plugins, and fill string table
    //
    // Return value is the number of language plugins successfully loaded
    //
    // The number of plugins available is also associated with the language id.
    // For example, if return value is 2, 
    // two languages plugins are available, with language id 0 and 1.
    static int Load();

    // Get the english name & native name for the language
    // The first parameter is the language id.
    static BOOL GetName(
        int langId, TCHAR *szEnglish, int cchEnglish, TCHAR *szNative, int cchNative);

    // Select a language as current
    static BOOL Select(int langId);

    // Get current language id
    static int GetLangId();

    // Get a string in current language
    static const TCHAR *GetString(int strId);

    // Get date string
    static void GetYearMonthString(TCHAR *buf, int cchLen, int year, int month);
    static void GetDateTimeString(TCHAR *buf, int cchLen, int time, int usec);
};

#endif
