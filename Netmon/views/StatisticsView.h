#ifndef STATISTICS_VIEW_H
#define STATISTICS_VIEW_H

#include "NetView.h"
#include "StatisticsModel.h"

class StatisticsView : public NetView
{
protected:
    // GDI Objects
    static HDC     _hdcTarget;
    static HDC     _hdcBuf;
    static HBITMAP _hbmpBuf;

    // Model Object
    static StatisticsModel *_model;

protected:
    static void DrawGraph();
    static void WINAPI TimerProc(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);

public:
    virtual void Init(StatisticsModel *model);
    virtual void End();
    virtual void SetProcessUid(int puid);

    virtual LRESULT DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif
