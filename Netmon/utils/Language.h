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
		STRING_TABLE_MAX  = 40087,
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
	// For example, if return value is 2, 2 languages plugins are available, with language id 0 and 1.
	static int Load();

	// Get the english name & native name for the language
	// The first parameter is the language id.
	static BOOL GetName(int langId, TCHAR *szEnglish, int cchEnglish, TCHAR *szNative, int cchNative);

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
