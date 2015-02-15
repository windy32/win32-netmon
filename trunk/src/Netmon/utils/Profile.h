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

#ifndef PROFILE_H
#define PROFILE_H

// The Profile class is used to operate an ini file with only one section.
//
// Here's an example.
//
// [Netmon Preference v2]
// Adapter=Intel(R) PRO/
// AutoStart=
// AutoCapture=0
// RtViewEnabled=1
// MtViewEnabled=1
// StViewEnabled=0
// DtViewEnabled=0
// HiddenProcess=1 3 6 7 9 16
// ShowHidden=0
// Language=English
//
// Related Windows API --------------------------------------------------------
//
// WritePrivateProfileString
//     WritePrivateProfileString("Netmon", "Adapter", "Intel(R) PRO/", "D:\\Netmon.ini")
//
// GetPrivateProfileString
//     GetPrivateProfileString("Netmon", "AutoStart", "", szValue, 256, "D:\\Netmon.ini")
//
class Profile
{
protected:
    TCHAR _szFileName[MAX_PATH];
    TCHAR _szSectionName[MAX_PATH];

public:
    // Input some information of the profile
    void Init(const TCHAR *szFileName, const TCHAR *szSectionName);

    // Get string key
    // If key doesn't exist, an empty string "" is written to buf, and return value is FALSE.
    BOOL GetString(const TCHAR *szOption, TCHAR *buf, int cchLen);

    // Get int key
    // If key doesn't exit or is empty or is not numeric, 
    // nothing is written to pValue, and return value is FALSE.
    BOOL GetInt(const TCHAR *szOption, int *pValue);

    // Set string key
    // If the ini file / section / key doesn't exist, it's created.
    // If the function still fails somehow, return value is FALSE.
    BOOL SetString(const TCHAR *szOption, const TCHAR *szValue);

    // Set int key
    // If the ini file / section / key doesn't exist, it's created.
    // If the function still fails somehow, return value is FALSE.
    BOOL SetInt(const TCHAR *szOption, int iValue);
};

class NetmonProfile
{
protected:
    Profile _pf;

    TCHAR _szAdapter[256];
    TCHAR _szAutoStart[MAX_PATH]; // Full path of Netmon, or empty string
    BOOL _bAutoCapture;

    BOOL _bRtViewEnabled;
    BOOL _bMtViewEnabled;
    BOOL _bStViewEnabled;
    BOOL _bDtViewEnabled;

    std::vector<int> _hiddenProcesses;
    BOOL _bShowHidden;

    TCHAR _szLanguage[64];

public:
    // Netmon calls Load() at startup to get preferences from Netmon.ini.
    // Items in the ini file will be saved to NetmonProfile's member variables.
    // If the ini file or a certain key doesn't exist, 
    // default value will be written to the ini file.
    BOOL Load(const TCHAR *szDefaultAdapter);

    // Netmon may then call GetXXX to get settings and apply settings while running.
    // It will also call GetXXX to get settings when "Preferences" dialog shows up.
    //
    // Netmon will call SetXXX to save settings when user clicks the "OK" button
    // in the "Preferences" dialog, some of which may be applied immediately, while some are not.
    BOOL GetAdapter(TCHAR *szAdapter, int cchLen);
    BOOL SetAdapter(const TCHAR *szAdapter);

    BOOL GetAutoStart(TCHAR *szAutoStart, int cchLen);
    BOOL SetAutoStart(const TCHAR *szAutoStart);

    BOOL GetAutoCapture(BOOL *pAutoCapture);
    BOOL SetAutoCapture(BOOL bAutoCapture);

    BOOL GetRtViewEnabled(BOOL *pEnable);
    BOOL SetRtViewEnabled(BOOL bEnable);

    BOOL GetMtViewEnabled(BOOL *pEnable);
    BOOL SetMtViewEnabled(BOOL bEnable);

    BOOL GetStViewEnabled(BOOL *pEnable);
    BOOL SetStViewEnabled(BOOL bEnable);

    BOOL GetDtViewEnabled(BOOL *pEnable);
    BOOL SetDtViewEnabled(BOOL bEnable);

    // Settings below are not configured in the preference dialog
    BOOL GetHiddenProcesses(std::vector<int> &processes);
    BOOL SetHiddenProcesses(const std::vector<int> &processes);

    BOOL GetLanguage(TCHAR *szLanguage, int cchLen);
    BOOL SetLanguage(const TCHAR *szLanguage);

    BOOL GetShowHidden(BOOL *pShowHidden);
    BOOL SetShowHidden(BOOL bShowHidden);
};

#endif
