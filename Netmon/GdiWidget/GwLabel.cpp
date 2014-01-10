#include "stdafx.h"
#include "GwLabel.h"

#include "../utils/Utils.h"

GwLabel::GwLabel(
    HDC hdcTarget, int x, int y, int maxWidth, int maxHeight, 
    const TCHAR *text) : GdiWidget(hdcTarget, x, y, maxWidth, maxHeight)
{
    // Save parameters
    _tcscpy_s(_text, 1024, text);

    // Create font
    _hFont = Utils::MyCreateFont(TEXT("MS Shell Dlg 2"), 14, 0, false);

    // Set Color
    SetTextColor(_hdcTarget, RGB(0, 0, 0));
}

GwLabel::~GwLabel()
{
    DeleteObject(_hFont);
}

void GwLabel::Paint()
{
    // Select font
    HFONT hOldFont = (HFONT) SelectObject(_hdcTarget, _hFont);

    // Caption
    SetTextAlign(_hdcTarget, TA_LEFT);
    GwTextOut(0, 0, _text, _tcslen(_text));

    // Restore
    SelectObject(_hdcTarget, hOldFont);
}
