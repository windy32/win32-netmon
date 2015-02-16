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
#include "DlgPreferences.h"

#include "res/resource.h"

#include "utils/Utils.h"
#include "utils/Profile.h"
#include "utils/Language.h"

///----------------------------------------------------------------------------------------------//
///                                    Global Variables                                          //
///----------------------------------------------------------------------------------------------//
extern HINSTANCE     g_hInstance;
extern NetmonProfile g_profile;
extern int           g_nAdapters;
extern TCHAR         g_szAdapterNames[16][256];

///----------------------------------------------------------------------------------------------// 
///                                    Called by Message Handlers                                //
///----------------------------------------------------------------------------------------------//
static void InitProfile(HWND hWnd)
{
    TCHAR szAdapter[256];
    TCHAR szAutoStart[MAX_PATH];
    BOOL bAutoCapture;

    BOOL bRtViewEnabled;
    BOOL bMtViewEnabled;
    BOOL bStViewEnabled;
    BOOL bDtViewEnabled;

    g_profile.GetAdapter(szAdapter, 256);
    g_profile.GetAutoStart(szAutoStart, MAX_PATH);
    g_profile.GetAutoCapture(&bAutoCapture);

    g_profile.GetRtViewEnabled(&bRtViewEnabled);
    g_profile.GetMtViewEnabled(&bMtViewEnabled);
    g_profile.GetStViewEnabled(&bStViewEnabled);
    g_profile.GetDtViewEnabled(&bDtViewEnabled);

    // - Adapter
    ComboBox_SelectString(GetDlgItem(hWnd, IDC_PREF_DEFAULT_ADAPTER), -1, szAdapter);

    // - AutoStart
    if (szAutoStart[0] != 0) // Not empty string
    {
        Button_SetCheck(GetDlgItem(hWnd, IDC_PREF_AUTO_START), BST_CHECKED);

        // Get full path name of Netmon.exe
        TCHAR szNetmon[MAX_PATH];
        GetModuleFileName(0, szNetmon, MAX_PATH);

        // Sync registry autorun item
        if (_tcscmp(szNetmon, szAutoStart) != 0)
        {
            int iResult = MessageBox(hWnd, Language::GetString(IDS_PREF_UPDATE_NOW),
                TEXT("Netmon"), MB_YESNO | MB_ICONQUESTION);

            if (iResult == IDYES)
            {
                // Build command line
                TCHAR szRegUpdater[MAX_PATH];
                Utils::GetFilePathInCurrentDir(szRegUpdater, MAX_PATH, TEXT("RegUpdater.exe"));

                // Start process and update registry
                int iExitCode;
                if (Utils::StartProcessAndWait(szRegUpdater, szNetmon, &iExitCode, TRUE))
                {
                    if (iExitCode == 0)
                    {
                        g_profile.SetAutoStart(szNetmon); // Update profile
                    }
                    else
                    {
                        MessageBox(hWnd, Language::GetString(IDS_PREF_UPDATE_ERROR), 
                            TEXT("Netmon"), MB_OK | MB_ICONWARNING);
                    }
                }
                else
                {
                    MessageBox(hWnd, Language::GetString(IDS_PREF_CANNOT_EXECUTE), 
                        TEXT("Netmon"), MB_OK | MB_ICONWARNING);
                }
            }
        }
    }

    // - AutoCapture
    if (bAutoCapture)
    {
        Button_SetCheck(GetDlgItem(hWnd, IDC_PREF_AUTO_CAPTURE), BST_CHECKED);
    }

    // - Enable Views
    if (bRtViewEnabled) Button_SetCheck(GetDlgItem(hWnd, IDC_PREF_RTVIEW), BST_CHECKED);
    if (bMtViewEnabled) Button_SetCheck(GetDlgItem(hWnd, IDC_PREF_MTVIEW), BST_CHECKED);
    if (bStViewEnabled) Button_SetCheck(GetDlgItem(hWnd, IDC_PREF_STVIEW), BST_CHECKED);
    if (bDtViewEnabled) Button_SetCheck(GetDlgItem(hWnd, IDC_PREF_DTVIEW), BST_CHECKED);
}

static void InitLanguage(HWND hWnd)
{
    SetDlgItemText(hWnd, IDL_PREF_DEFAULT_ADAPTER, Language::GetString(IDS_PREF_DEFAULT_ADAPTER));
    SetDlgItemText(hWnd, IDC_PREF_AUTO_START,      Language::GetString(IDS_PREF_AUTO_START));
    SetDlgItemText(hWnd, IDC_PREF_AUTO_CAPTURE,    Language::GetString(IDS_PREF_AUTO_CAPTURE));
    SetDlgItemText(hWnd, IDG_PREF_ENABLE_VIEWS,    Language::GetString(IDS_PREF_ENABLE_VIEWS));
    SetDlgItemText(hWnd, IDC_PREF_RTVIEW,          Language::GetString(IDS_PREF_RTVIEW));
    SetDlgItemText(hWnd, IDC_PREF_MTVIEW,          Language::GetString(IDS_PREF_MTVIEW));
    SetDlgItemText(hWnd, IDC_PREF_STVIEW,          Language::GetString(IDS_PREF_STVIEW));
    SetDlgItemText(hWnd, IDC_PREF_DTVIEW,          Language::GetString(IDS_PREF_DTVIEW));
    SetDlgItemText(hWnd, IDB_PREF_OK,              Language::GetString(IDS_PREF_OK));
    SetDlgItemText(hWnd, IDB_PREF_CANCEL,          Language::GetString(IDS_PREF_CANCEL));
}

///----------------------------------------------------------------------------------------------// 
///                                    L2 Message Handlers                                       //
///----------------------------------------------------------------------------------------------//
static void OnOk(HWND hWnd)
{
    // Get setting from UI and save profile

    // - Adapter
    TCHAR szAdapter[256];
    ComboBox_GetText(GetDlgItem(hWnd, IDC_PREF_DEFAULT_ADAPTER), szAdapter, 256);
    g_profile.SetAdapter(szAdapter);

    // - Auto start
    if (Button_GetCheck(GetDlgItem(hWnd, IDC_PREF_AUTO_START)) == BST_CHECKED )
    {
        // Get Netmon's auto start item in registry
        TCHAR szAutoStart[MAX_PATH];
        g_profile.GetAutoStart(szAutoStart, MAX_PATH);

        // Get Netmon's actual path
        TCHAR szNetmon[MAX_PATH];
        GetModuleFileName(0, szNetmon, MAX_PATH);

        // Update registry if necessary
        if (_tcscmp(szAutoStart, szNetmon) != 0 )
        {
            // Build command line
            TCHAR szRegUpdater[MAX_PATH];
            Utils::GetFilePathInCurrentDir(szRegUpdater, MAX_PATH, TEXT("RegUpdater.exe"));

            // Start process and update registry
            int iExitCode;
            if (Utils::StartProcessAndWait(szRegUpdater, szNetmon, &iExitCode, TRUE))
            {
                if (iExitCode != 0 )
                {
                    MessageBox(hWnd, Language::GetString(IDS_PREF_UPDATE_ERROR), 
                        TEXT("Netmon"), MB_OK | MB_ICONWARNING);
                }
                else
                {
                    g_profile.SetAutoStart(szNetmon); // Update profile
                }
            }
            else
            {
                MessageBox(hWnd, Language::GetString(IDS_PREF_CANNOT_EXECUTE), 
                    TEXT("Netmon"), MB_OK | MB_ICONWARNING);
            }
        }
    }
    else // "Auto start" not checked
    {
        // Get Netmon's auto start item in registry
        TCHAR szAutoStart[MAX_PATH];
        g_profile.GetAutoStart(szAutoStart, MAX_PATH);

        // Delete the auto start item in registry if necessary
        if (szAutoStart[0] != 0 )
        {
            int iExitCode;
            TCHAR szRegUpdater[MAX_PATH];
            Utils::GetFilePathInCurrentDir(szRegUpdater, MAX_PATH, TEXT("RegUpdater.exe"));

            if (Utils::StartProcessAndWait(szRegUpdater, TEXT("-c"), &iExitCode, TRUE))
            {
                if (iExitCode != 0 )
                {
                    MessageBox(hWnd, Language::GetString(IDS_PREF_UPDATE_ERROR), 
                        TEXT("Netmon"), MB_OK | MB_ICONWARNING);
                }
                else
                {
                    g_profile.SetAutoStart(TEXT("")); // Update profile
                }
            }
            else
            {
                MessageBox(hWnd, Language::GetString(IDS_PREF_CANNOT_EXECUTE), 
                    TEXT("Netmon"), MB_OK | MB_ICONWARNING);
            }
        }
    }

    // - Auto capture
    g_profile.SetAutoCapture(
        Button_GetCheck(GetDlgItem(hWnd, IDC_PREF_AUTO_CAPTURE)) == BST_CHECKED);

    // - Enable views
    BOOL bRtEnabled = FALSE;
    BOOL bMtEnabled = FALSE;
    BOOL bStEnabled = FALSE;
    BOOL bDtEnabled = FALSE;

    g_profile.GetRtViewEnabled(&bRtEnabled);
    g_profile.GetMtViewEnabled(&bMtEnabled);
    g_profile.GetStViewEnabled(&bStEnabled);
    g_profile.GetDtViewEnabled(&bDtEnabled);

    // At least one view should be enabled
    if (bRtEnabled == FALSE &&
        bMtEnabled == FALSE &&
        bStEnabled == FALSE &&
        bDtEnabled == FALSE)
    {
        MessageBox(hWnd, Language::GetString(IDS_PREF_AT_LEAST_ONE_VIEW), 
            TEXT("Netmon"), MB_OK | MB_ICONWARNING);
        return;
    }

    // If one of the views has been disabled, Netmon will prompt the user 
    // to decide whether related data in database should be deleted
    BOOL bNeedClearDatabase = FALSE;
    BOOL bNeedReboot = FALSE;

    BOOL bRtNowEnabled = Button_GetCheck(GetDlgItem(hWnd, IDC_PREF_RTVIEW)) == BST_CHECKED;
    BOOL bMtNowEnabled = Button_GetCheck(GetDlgItem(hWnd, IDC_PREF_MTVIEW)) == BST_CHECKED;
    BOOL bStNowEnabled = Button_GetCheck(GetDlgItem(hWnd, IDC_PREF_STVIEW)) == BST_CHECKED;
    BOOL bDtNowEnabled = Button_GetCheck(GetDlgItem(hWnd, IDC_PREF_DTVIEW)) == BST_CHECKED;

    if ((bRtEnabled != bRtNowEnabled) ||
        (bMtEnabled != bMtNowEnabled) ||
        (bStEnabled != bStNowEnabled) ||
        (bDtEnabled != bDtNowEnabled))
    {
        bNeedReboot = TRUE;
    }

    if ((bRtEnabled && !bRtNowEnabled) ||
        (bMtEnabled && !bMtNowEnabled) ||
        (bStEnabled && !bStNowEnabled) ||
        (bDtEnabled && !bDtNowEnabled))
    {
        bNeedClearDatabase = TRUE;
    }

    // Update Profile
    g_profile.SetRtViewEnabled(bRtNowEnabled);
    g_profile.SetMtViewEnabled(bMtNowEnabled);
    g_profile.SetStViewEnabled(bStNowEnabled);
    g_profile.SetDtViewEnabled(bDtNowEnabled);

    // Reboot
    if (bNeedReboot)
    {
        HWND hMainWindow = GetParent(hWnd);

        if (bNeedClearDatabase)
        {
            int iResult = MessageBox(hWnd, Language::GetString(IDS_PREF_WHETHER_DELETE_DATA),
                TEXT("Netmon"), MB_YESNO | MB_ICONQUESTION);

            MessageBox(hWnd, Language::GetString(IDS_NETMON_WILL_RESTART),
                TEXT("Netmon"), MB_OK | MB_ICONINFORMATION);

            PostMessage(hMainWindow, 
                (iResult == IDYES) ? WM_CLEAR_DB_AND_RESTART : WM_RESTART, NULL, NULL);
        }
        else
        {
            MessageBox(hWnd, Language::GetString(IDS_NETMON_WILL_RESTART),
                TEXT("Netmon"), MB_OK | MB_ICONINFORMATION);

            PostMessage(hMainWindow, WM_RESTART, NULL, NULL);
        }
    }

    // Close dialog
    EndDialog(hWnd, 0);
}

static void OnCancel(HWND hWnd)
{
    EndDialog(hWnd, 0);
}

///----------------------------------------------------------------------------------------------// 
///                                    L1 Message Handlers                                       //
///----------------------------------------------------------------------------------------------//
static void OnInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    // Load Icon
    HICON hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(ICO_MAIN));
    SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);

    // Init language
    InitLanguage(hWnd);

    // Size Dialog
    SetWindowPos(hWnd, HWND_TOP, 140, 140, 415, 290, 0);

    // Get Client Rectangle
    RECT stClientRect;
    GetClientRect(hWnd, &stClientRect);

    int x1 = stClientRect.left + 143;
    int y1 = stClientRect.top;
    int x2 = stClientRect.right;
    int y2 = stClientRect.bottom;

    // Move Controls
    MoveWindow(GetDlgItem(hWnd, IDL_PREF_DEFAULT_ADAPTER), 15,  15,  100, 20, FALSE);
    MoveWindow(GetDlgItem(hWnd, IDC_PREF_DEFAULT_ADAPTER), 120, 12,  275, 20, FALSE);
    MoveWindow(GetDlgItem(hWnd, IDC_PREF_AUTO_START),      15,  40,  380, 20, FALSE);
    MoveWindow(GetDlgItem(hWnd, IDC_PREF_AUTO_CAPTURE),    15,  65,  380, 20, FALSE);
    MoveWindow(GetDlgItem(hWnd, IDG_PREF_ENABLE_VIEWS),    15,  90,  380, 125, FALSE);
    MoveWindow(GetDlgItem(hWnd, IDC_PREF_RTVIEW),          25,  110, 300, 20, FALSE);
    MoveWindow(GetDlgItem(hWnd, IDC_PREF_MTVIEW),          25,  135, 300, 20, FALSE);
    MoveWindow(GetDlgItem(hWnd, IDC_PREF_STVIEW),          25,  160, 300, 20, FALSE);
    MoveWindow(GetDlgItem(hWnd, IDC_PREF_DTVIEW),          25,  185, 300, 20, FALSE);
    MoveWindow(GetDlgItem(hWnd, IDB_PREF_OK),              230, 225, 80,  25, FALSE);
    MoveWindow(GetDlgItem(hWnd, IDB_PREF_CANCEL),          315, 225, 80,  25, FALSE);

    // Get Device Names
    for(int i = 0; i < g_nAdapters; i++)
    {
        if (i < _countof(g_szAdapterNames))
        {
            ComboBox_AddString(GetDlgItem(hWnd, IDC_PREF_DEFAULT_ADAPTER), g_szAdapterNames[i]);
        }
    }

    // Read profile and update UI
    InitProfile(hWnd);
}

static void OnClose(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    EndDialog(hWnd, 0);
}

static void OnCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    if (wParam == IDB_PREF_OK )
    {
        OnOk(hWnd);
    }
    else if (wParam == IDB_PREF_CANCEL )
    {
        OnCancel(hWnd);
    }
}

///----------------------------------------------------------------------------------------------//
///                                    Preferences Dialog Proc                                   //
///----------------------------------------------------------------------------------------------//
INT_PTR CALLBACK ProcDlgPreferences(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
#define PROCESS_MSG(MSG, HANDLER) if(uMsg == MSG) { HANDLER(hWnd, wParam, lParam); return TRUE; }

    PROCESS_MSG(WM_INITDIALOG, OnInitDialog)
    PROCESS_MSG(WM_CLOSE,      OnClose)
    PROCESS_MSG(WM_COMMAND,    OnCommand)

#undef PROCESS_MSG

    return FALSE;
}
