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
#include "GwGroupbox.h"

#include "../utils/Utils.h"

GwGroupbox::GwGroupbox(
    HDC hdcTarget, int x, int y, int maxWidth, int maxHeight, 
    const TCHAR *caption, COLORREF color) : GdiWidget(hdcTarget, x, y, maxWidth, maxHeight)
{
    // Save parameters
    _caption = caption;
    _color = color;

    // Create font
    _hFont = Utils::MyCreateFont(TEXT("MS Shell Dlg 2"), 14, 0, true);

    // Calculate size
    CalcSize();
}

void GwGroupbox::CalcSize()
{
    _width = _maxWidth;
    _height = _maxHeight;

    _contentX = 8;
    _contentY = 20;
    _contentWidth = _width - 8 - 8;
    _contentHeight = _height - 20 - 8;
}

GwGroupbox::~GwGroupbox()
{
    DeleteObject(_hFont);
}

void GwGroupbox::Paint()
{
    // Select font
    HFONT hOldFont = (HFONT) SelectObject(_hdcTarget, _hFont);

    // Box
    SetDCPenColor(_hdcTarget, RGB(0x80, 0x80, 0x80));
    GwRectangle(0, 0, _width, _height);

    // Caption
    SetTextAlign(_hdcTarget, TA_LEFT);
    SetTextColor(_hdcTarget, _color);
    GwTextOut(-7, 1, _caption, _tcslen(_caption));

    // Restore
    SelectObject(_hdcTarget, hOldFont);
    SetDCPenColor(_hdcTarget, RGB(0, 0, 0));
}

void GwGroupbox::GetContentArea(int *pX, int *pY, int *pWidth, int *pHeight)
{
    *pX = _contentX + _x;
    *pY = _contentY + _y;
    *pWidth = _contentWidth;
    *pHeight = _contentHeight;
}
