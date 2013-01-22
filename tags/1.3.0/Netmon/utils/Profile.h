#ifndef PROFILE_H
#define PROFILE_H

// The Profile class is used to operate an ini file with only one section.
//
// Here's an example.
//
// [Netmon Preference v1]
// AdapterName=Intel(R) PRO/
// AutoStart=1
// AutoCapture=1
// DtViewEnable=1
// DtViewMaxSpace=0
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
	// If key doesn't exit or is empty or is not numeric, nothing is written to pValue, and return value is FALSE.
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
	BOOL _bDtViewEnable;
	int  _iDtViewMaxSpace;

public:
	// Netmon calls Load() at startup to get preferences from Netmon.ini.
	// Items in the ini file will be saved to NetmonProfile's member variables.
	// If the ini file or a certain key doesn't exist, default value will be written to the ini file.
	BOOL Load(const TCHAR *szDefaultAdapter);

	// Netmon may then call GetXXX to get settings and apply settings while running.
	// It will also call GetXXX to get settings when "Preferences" dialog shows up.
	//
	// Netmon will call SetXXX to save settings when user clicks the "OK" in the "Preferences" dialog, 
	// some of which may be applied immediately, while some are not.
	BOOL GetAdapter(TCHAR *szAdapter, int cchLen);
	BOOL SetAdapter(const TCHAR *szAdapter);

	BOOL GetAutoStart(TCHAR *szAutoStart, int cchLen);
	BOOL SetAutoStart(const TCHAR *szAutoStart);

	BOOL GetAutoCapture(BOOL *pAutoCapture);
	BOOL SetAutoCapture(BOOL bAutoCapture);

	BOOL GetDtViewEnable(BOOL *pEnable);
	BOOL SetDtViewEnable(BOOL bEnable);

	BOOL GetDtViewMaxSpace(int *pMaxSpace);
	BOOL SetDtViewMaxSpace(int iMaxSpace);
};

#endif
