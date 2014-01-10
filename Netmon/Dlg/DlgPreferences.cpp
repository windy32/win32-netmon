#include "stdafx.h"
#include "DlgPreferences.h"

#include "../res/resource.h"

#include "../utils/Utils.h"
#include "../utils/Language.h"
#include "../utils/Profile.h"

#include "../views/DetailView.h"

///----------------------------------------------------------------------------------------------//
///                                    Global Variables                                          //
///----------------------------------------------------------------------------------------------//
extern HINSTANCE     g_hInstance;
extern NetmonProfile g_profile;
extern int           g_nAdapters;
extern TCHAR         g_szAdapterNames[16][256];

extern bool          g_bCapture;
static DetailView   *g_pDetailView;

///----------------------------------------------------------------------------------------------// 
///                                    Called by Message Handlers                                //
///----------------------------------------------------------------------------------------------//
static void InitProfile(HWND hWnd)
{
    TCHAR szAdapter[256];
    TCHAR szAutoStart[MAX_PATH];
    BOOL bAutoCapture;
    BOOL bDtViewEnable;
    int  iDtViewMaxSpace;

    g_profile.GetAdapter(szAdapter, 256);
    g_profile.GetAutoStart(szAutoStart, MAX_PATH);
    g_profile.GetAutoCapture(&bAutoCapture);
    g_profile.GetDtViewEnable(&bDtViewEnable);
    g_profile.GetDtViewMaxSpace(&iDtViewMaxSpace);

    // - Adapter
    ComboBox_SelectString(GetDlgItem(hWnd, IDC_PREF_DEFAULT_ADAPTER), -1, szAdapter);

    // - AutoStart
    if( szAutoStart[0] != 0 ) // Not empty string
    {
        Button_SetCheck(GetDlgItem(hWnd, IDC_PREF_AUTO_START), BST_CHECKED);

        // Get full path name of Netmon.exe
        TCHAR szNetmon[MAX_PATH];
        GetModuleFileName(0, szNetmon, MAX_PATH);

        // Sync registry autorun item
        if( _tcscmp(szNetmon, szAutoStart) != 0 )
        {
            int iResult = MessageBox(hWnd, 
                TEXT("Netmon has detected that the autorun item in registry\n")
                TEXT("doesn't match the running executable file.\n")
                TEXT("\n")
                TEXT("Will you update the registry now?"), 
                TEXT("Netmon"), MB_YESNO | MB_ICONQUESTION);

            if( iResult == IDYES )
            {
                // Build command line
                TCHAR szRegUpdater[MAX_PATH];
                Utils::GetFilePathInCurrentDir(szRegUpdater, MAX_PATH, TEXT("RegUpdater.exe"));

                // Start process and update registry
                int iExitCode;
                if( Utils::StartProcessAndWait(szRegUpdater, szNetmon, &iExitCode, TRUE))
                {
                    if( iExitCode == 0 )
                    {
                        g_profile.SetAutoStart(szNetmon); // Update profile
                    }
                    else
                    {
                        MessageBox(hWnd, TEXT("An error occurred updating registry!"), 
                            TEXT("Netmon"), MB_OK | MB_ICONWARNING);
                    }
                }
                else
                {
                    MessageBox(hWnd, TEXT("Cannot execute RegUpdater!"), 
                        TEXT("Netmon"), MB_OK | MB_ICONWARNING);
                }
            }
        }
    }

    // - AutoCapture
    if( bAutoCapture )
    {
        Button_SetCheck(GetDlgItem(hWnd, IDC_PREF_AUTO_CAPTURE), BST_CHECKED);
    }

    // - DtViewEnable
    if( bDtViewEnable )
    {
        Button_SetCheck(GetDlgItem(hWnd, IDC_PREF_ENABLE), BST_CHECKED);
    }

    // - DtViewMaxSpace
    if( iDtViewMaxSpace == 0 )
    {
        EnableWindow(GetDlgItem(hWnd, IDE_PREF_MAX_DTVIEW), FALSE);
    }
    else
    {
        Button_SetCheck(GetDlgItem(hWnd, IDC_PREF_MAX_DTVIEW), BST_CHECKED);
        SetDlgItemInt(hWnd, IDE_PREF_MAX_DTVIEW, iDtViewMaxSpace, FALSE);
    }
}

static void InitLanguage(HWND hWnd)
{
    SetDlgItemText(hWnd, IDL_PREF_DEFAULT_ADAPTER, Language::GetString(IDS_PREF_DEFAULT_ADAPTER));
    SetDlgItemText(hWnd, IDC_PREF_AUTO_START,      Language::GetString(IDS_PREF_AUTO_START));
    SetDlgItemText(hWnd, IDC_PREF_AUTO_CAPTURE,    Language::GetString(IDS_PREF_AUTO_CAPTURE));
    SetDlgItemText(hWnd, IDG_PREF_DETAIL_VIEW,     Language::GetString(IDS_PREF_DTVIEW));
    SetDlgItemText(hWnd, IDC_PREF_ENABLE,          Language::GetString(IDS_PREF_DTVIEW_ENABLE));
    SetDlgItemText(hWnd, IDC_PREF_MAX_DTVIEW,      Language::GetString(IDS_PREF_DTVIEW_MAX_SPACE));
    SetDlgItemText(hWnd, IDB_PREF_COMPACT,         Language::GetString(IDS_PREF_DTVIEW_COMPACT));
    SetDlgItemText(hWnd, IDB_PREF_DELETE_ALL,      Language::GetString(IDS_PREF_DTVIEW_DELETE_ALL));
    SetDlgItemText(hWnd, IDB_PREF_OK,              Language::GetString(IDS_PREF_OK));
    SetDlgItemText(hWnd, IDB_PREF_CANCEL,          Language::GetString(IDS_PREF_CANCEL));
}

///----------------------------------------------------------------------------------------------// 
///                                    L2 Message Handlers                                       //
///----------------------------------------------------------------------------------------------//
static void OnCheckDtViewEnable(HWND hWnd)
{
    if( Button_GetCheck(GetDlgItem(hWnd, IDC_PREF_ENABLE)) == BST_CHECKED )
    {
        EnableWindow(GetDlgItem(hWnd, IDC_PREF_MAX_DTVIEW), TRUE);
        if( Button_GetCheck(GetDlgItem(hWnd, IDC_PREF_MAX_DTVIEW)) == BST_CHECKED )
        {
            EnableWindow(GetDlgItem(hWnd, IDE_PREF_MAX_DTVIEW), TRUE);
        }
    }
    else
    {
        SetDlgItemText(hWnd, IDE_PREF_MAX_DTVIEW, TEXT(""));
        EnableWindow(GetDlgItem(hWnd, IDC_PREF_MAX_DTVIEW), FALSE);
        EnableWindow(GetDlgItem(hWnd, IDE_PREF_MAX_DTVIEW), FALSE);
    }
}

static void OnCheckDtViewMaxSpace(HWND hWnd)
{
    if( Button_GetCheck(GetDlgItem(hWnd, IDC_PREF_MAX_DTVIEW)) == BST_CHECKED )
    {
        EnableWindow(GetDlgItem(hWnd, IDE_PREF_MAX_DTVIEW), TRUE);
    }
    else
    {
        SetDlgItemText(hWnd, IDE_PREF_MAX_DTVIEW, TEXT(""));
        EnableWindow(GetDlgItem(hWnd, IDE_PREF_MAX_DTVIEW), FALSE);
    }
}

static void OnOk(HWND hWnd)
{
    BOOL bParamOk = TRUE;
    TCHAR szAdapter[256];

    // Get setting from UI and save profile
    ComboBox_GetText(GetDlgItem(hWnd, IDC_PREF_DEFAULT_ADAPTER), szAdapter, 256);

    // - Detail view max space
    if( Button_GetCheck(GetDlgItem(hWnd, IDC_PREF_ENABLE)) == BST_CHECKED )
    {
        if( Button_GetCheck(GetDlgItem(hWnd, IDC_PREF_MAX_DTVIEW)) == BST_CHECKED )
        {
            BOOL bTranslated = FALSE;
            int iDtViewMaxSpace = GetDlgItemInt(hWnd, IDE_PREF_MAX_DTVIEW, &bTranslated, FALSE);

            if( !(bTranslated && iDtViewMaxSpace >= 10 && iDtViewMaxSpace <= 1024 ))
            {
                bParamOk = FALSE;
                MessageBox(hWnd, TEXT("Max disk space should be between 10 MB and 1024 MB!"), 
                    TEXT("Error"), MB_OK | MB_ICONWARNING);
            }

            if( bParamOk )
            {
                g_profile.SetDtViewMaxSpace(iDtViewMaxSpace);
            }
        }
        else
        {
            g_profile.SetDtViewMaxSpace(0);
        }

        if( bParamOk )
        {
            g_profile.SetDtViewEnable(TRUE);
        }
    }
    else // - Detail view enable
    {
        g_profile.SetDtViewEnable(FALSE);
        g_profile.SetDtViewMaxSpace(0);
    }

    if( bParamOk )
    {
        // - Adapter
        g_profile.SetAdapter(szAdapter);

        // - Auto start
        if( Button_GetCheck(GetDlgItem(hWnd, IDC_PREF_AUTO_START)) == BST_CHECKED )
        {
            // Get Netmon's auto start item in registry
            TCHAR szAutoStart[MAX_PATH];
            g_profile.GetAutoStart(szAutoStart, MAX_PATH);

            // Get Netmon's actual path
            TCHAR szNetmon[MAX_PATH];
            GetModuleFileName(0, szNetmon, MAX_PATH);

            // Update registry if necessary
            if( _tcscmp(szAutoStart, szNetmon) != 0 )
            {
                // Build command line
                TCHAR szRegUpdater[MAX_PATH];
                Utils::GetFilePathInCurrentDir(szRegUpdater, MAX_PATH, TEXT("RegUpdater.exe"));

                // Start process and update registry
                int iExitCode;
                if( Utils::StartProcessAndWait(szRegUpdater, szNetmon, &iExitCode, TRUE))
                {
                    if (iExitCode != 0 )
                    {
                        MessageBox(hWnd, TEXT("An error occurred updating registry!"), 
                            TEXT("Netmon"), MB_OK | MB_ICONWARNING);
                    }
                    else
                    {
                        g_profile.SetAutoStart(szNetmon); // Update profile
                    }
                }
                else
                {
                    MessageBox(hWnd, TEXT("Cannot execute RegUpdater!"), 
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
            if( szAutoStart[0] != 0 )
            {
                int iExitCode;
                TCHAR szRegUpdater[MAX_PATH];
                Utils::GetFilePathInCurrentDir(szRegUpdater, MAX_PATH, TEXT("RegUpdater.exe"));

                if( Utils::StartProcessAndWait(szRegUpdater, TEXT("-c"), &iExitCode, TRUE))
                {
                    if (iExitCode != 0 )
                    {
                        MessageBox(hWnd, TEXT("An error occurred updating registry!"), 
                            TEXT("Netmon"), MB_OK | MB_ICONWARNING);
                    }
                    else
                    {
                        g_profile.SetAutoStart(TEXT("")); // Update profile
                    }
                }
                else
                {
                    MessageBox(hWnd, TEXT("Cannot execute RegUpdater!"), 
                        TEXT("Netmon"), MB_OK | MB_ICONWARNING);
                }
            }
        }

        // - Auto capture
        g_profile.SetAutoCapture(
            Button_GetCheck(GetDlgItem(hWnd, IDC_PREF_AUTO_CAPTURE)) == BST_CHECKED);
    }

    // Close dialog
    if( bParamOk )
    {
        EndDialog(hWnd, 0);
    }
}

static void OnCancel(HWND hWnd)
{
    EndDialog(hWnd, 0);
}

static void OnDeleteAll(HWND hWnd)
{
    if( g_bCapture )
    {
        MessageBox(hWnd, TEXT("Please stop capture first."), 
            TEXT("Netmon"), MB_OK | MB_ICONINFORMATION);
    }
    else
    {
        Utils::DeleteAllPackets();
        g_pDetailView->OnAllPacketsDeleted();
        MessageBox(hWnd, TEXT("All packets have been deleted."), 
            TEXT("Netmon"), MB_OK | MB_ICONINFORMATION);
    }
}

static void OnCompact(HWND hWnd)
{
    if( g_bCapture )
    {
        MessageBox(hWnd, TEXT("Please stop capture first."), 
            TEXT("Netmon"), MB_OK | MB_ICONINFORMATION);
    }
    else
    {
        SQLite::Flush();
        SQLite::Exec(TEXT("Vacuum;"), false);
        MessageBox(hWnd, TEXT("Compact finished."), 
            TEXT("Netmon"), MB_OK | MB_ICONINFORMATION);
    }
}

///----------------------------------------------------------------------------------------------// 
///                                    L1 Message Handlers                                       //
///----------------------------------------------------------------------------------------------//
static void OnInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    // Save pointer to detail view
    g_pDetailView = (DetailView *)lParam;

    // Load Icon
    HICON hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(ICO_MAIN));
    SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);

    // Init language
    InitLanguage(hWnd);

    // Size Dialog
    SetWindowPos(hWnd, HWND_TOP, 140, 140, 415, 275, 0);

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
    MoveWindow(GetDlgItem(hWnd, IDG_PREF_DETAIL_VIEW),     15,  90,  380, 110, FALSE);
    MoveWindow(GetDlgItem(hWnd, IDC_PREF_ENABLE),          25,  110, 300, 20, FALSE);
    MoveWindow(GetDlgItem(hWnd, IDC_PREF_MAX_DTVIEW),      25,  135, 280, 20, FALSE);
    MoveWindow(GetDlgItem(hWnd, IDE_PREF_MAX_DTVIEW),      315, 135, 65,  20, FALSE);
    MoveWindow(GetDlgItem(hWnd, IDB_PREF_DELETE_ALL),      25,  160, 100, 25, FALSE);
    MoveWindow(GetDlgItem(hWnd, IDB_PREF_COMPACT),         135, 160, 100, 25, FALSE);
    MoveWindow(GetDlgItem(hWnd, IDB_PREF_OK),              230, 210, 80,  25, FALSE);
    MoveWindow(GetDlgItem(hWnd, IDB_PREF_CANCEL),          315, 210, 80,  25, FALSE);

    // Get Device Names
    for(int i = 0; i < g_nAdapters; i++)
    {
        if( i < _countof(g_szAdapterNames))
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
    if( wParam == IDC_PREF_ENABLE )
    {
        OnCheckDtViewEnable(hWnd);
    }
    else if( wParam == IDC_PREF_MAX_DTVIEW )
    {
        OnCheckDtViewMaxSpace(hWnd);
    }
    else if( wParam == IDB_PREF_OK )
    {
        OnOk(hWnd);
    }
    else if( wParam == IDB_PREF_CANCEL )
    {
        OnCancel(hWnd);
    }
    else if( wParam == IDB_PREF_DELETE_ALL )
    {
        OnDeleteAll(hWnd);
    }
    else if( wParam == IDB_PREF_COMPACT )
    {
        OnCompact(hWnd);
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
