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

#include "stdafx.h"
#include "Language.h"

int Language::_curLang;
std::vector<Language::StringTable> Language::_tables;

int Language::Load()
{
    WIN32_FIND_DATA stFindFile;
    HANDLE hFindFile;
    HMODULE hPlugin;

    TCHAR szCurrentExe[MAX_PATH];
    TCHAR szCurrentDir[MAX_PATH];
    TCHAR szPluginDir[MAX_PATH];
    TCHAR szPluginTemplate[MAX_PATH];
    TCHAR szFullName[MAX_PATH];
    TCHAR *pFilePart;

    TCHAR szResString[MAX_PATH];

    // Get full path name of Netmon.exe
    GetModuleFileName(0, szCurrentExe, MAX_PATH);

    // Get base directory
    GetFullPathName(szCurrentExe, MAX_PATH, szCurrentDir, &pFilePart);
    *pFilePart = TEXT('\0');

    // Get plugin directory
    _tcscpy_s(szPluginDir, MAX_PATH, szCurrentDir);
    _tcscat_s(szPluginDir, MAX_PATH, TEXT("Lang\\"));

    // Get plugin template
    _tcscpy_s(szPluginTemplate, MAX_PATH, szPluginDir);
    _tcscat_s(szPluginTemplate, MAX_PATH, TEXT("*.dll"));

    // Find files
    hFindFile = FindFirstFile(szPluginTemplate, &stFindFile); // Fix when release

    if (hFindFile != INVALID_HANDLE_VALUE )
    {
        do
        {
            if (!(stFindFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ) // If it's not a folder
            {
                _tcscpy_s(szFullName, MAX_PATH, szPluginDir);
                _tcscat_s(szFullName, MAX_PATH, stFindFile.cFileName);

                if ((hPlugin = LoadLibrary(szFullName)) != NULL )
                {
                    BOOL bValid = TRUE;
                    StringTable st;

                    // Read string table
                    for(int i = STRING_TABLE_MIN; i <= STRING_TABLE_MAX; i++)
                    {
                        if (LoadString(hPlugin, i, szResString, MAX_PATH) == 0 ) // String not exist
                        {
                            bValid = FALSE;
                            break;
                        }
                        else
                        {
                            st.Set(i, szResString);
                        }
                    }

                    if (bValid ) // All strings for this language are successfully loaded
                    {
                        _tables.push_back(st);
                    }

                    FreeLibrary(hPlugin);
                }
            }
        } while( FindNextFile(hFindFile, &stFindFile) != FALSE );

        FindClose(hFindFile);
    }

    return _tables.size();
}

BOOL Language::GetName(int langId, TCHAR *szEnglish, int cchEnglish, TCHAR *szNative, int cchNative)
{
    if (langId >= 0 && langId < (int)_tables.size())
    {
        const TCHAR *szEnglishName = _tables[langId].Get(IDS_LANG_NAME_ENG);
        const TCHAR *szNativeName = _tables[langId].Get(IDS_LANG_NAME_NATIVE);

        _tcscpy_s(szEnglish, cchEnglish, szEnglishName);
        _tcscpy_s(szNative, cchNative, szNativeName);

        return TRUE;
    }

    return FALSE;
}

BOOL Language::Select(int langId)
{
    if (langId >= 0 && langId < (int)_tables.size())
    {
        _curLang =langId;
        return TRUE;
    }

    return FALSE;
}

int Language::GetLangId()
{
    return _curLang;
}

const TCHAR *Language::GetString(int strId)
{
    if (_tables.size() > 0 )
    {
        return _tables[_curLang].Get(strId);
    }
    else
    {
        return NULL;
    }
}

void Language::GetYearMonthString(TCHAR *buf, int cchLen, int year, int month)
{
    const TCHAR szEnglishMonths[12][16] = 
    {
        TEXT("January"),
        TEXT("February"),
        TEXT("March"),
        TEXT("April"),
        TEXT("May"),
        TEXT("June"),
        TEXT("July"),
        TEXT("August"),
        TEXT("September"),
        TEXT("October"),
        TEXT("November"),
        TEXT("December")
    };
    const TCHAR *szEnglishName = _tables[_curLang].Get(IDS_LANG_NAME_ENG);

    if (_tcscmp(szEnglishName, TEXT("English")) == 0 ) // English
    {
        _stprintf_s(buf, cchLen, TEXT("%s %d"), szEnglishMonths[month], year);
    }
    else if (_tcscmp(szEnglishName, TEXT("Chinese Simplified")) == 0 ) // Chinese Simplified
    {
        _stprintf_s(buf, cchLen, TEXT("%d年%d月"), year, month + 1);
    }
    else // Default: English
    {
        _stprintf_s(buf, cchLen, TEXT("%s %d"), szEnglishMonths[month], year);
    }
}

void Language::GetDateTimeString(TCHAR *buf, int cchLen, int time, int usec)
{
    const TCHAR szEnglishMonths[12][16] = 
    {
        TEXT("January"),
        TEXT("February"),
        TEXT("March"),
        TEXT("April"),
        TEXT("May"),
        TEXT("June"),
        TEXT("July"),
        TEXT("August"),
        TEXT("September"),
        TEXT("October"),
        TEXT("November"),
        TEXT("December")
    };
    const TCHAR *szEnglishName = _tables[_curLang].Get(IDS_LANG_NAME_ENG);

    struct tm tmTime;
    _localtime32_s(&tmTime, (__time32_t *)&time);

    if (_tcscmp(szEnglishName, TEXT("English")) == 0 ) // English
    {
        _stprintf_s(buf, cchLen, TEXT("%s %d, %d - %02d:%02d:%02d.%04d"), 
            szEnglishMonths[tmTime.tm_mon], 
            (int)tmTime.tm_mday, 
            (int)tmTime.tm_year + 1900,
            (int)tmTime.tm_hour, 
            (int)tmTime.tm_min, 
            (int)tmTime.tm_sec, 
            usec / 100);
    }
    else if (_tcscmp(szEnglishName, TEXT("Chinese Simplified")) == 0 ) // Chinese Simplified
    {
        _stprintf_s(buf, cchLen, TEXT("%d年%d月%d日 - %02d:%02d:%02d.%04d"), 
            (int)tmTime.tm_year + 1900,
            (int)tmTime.tm_mon, 
            (int)tmTime.tm_mday, 
            (int)tmTime.tm_hour, 
            (int)tmTime.tm_min, 
            (int)tmTime.tm_sec, 
            usec / 100);
    }
    else // Default: English
    {
        _stprintf_s(buf, cchLen, TEXT("%s %d, %d - %02d:%02d:%02d.%04d"), 
            szEnglishMonths[tmTime.tm_mon], 
            tmTime.tm_mday, 
            tmTime.tm_year + 1900,
            tmTime.tm_hour, 
            tmTime.tm_min, 
            tmTime.tm_sec, 
            usec / 100);
    }
}
