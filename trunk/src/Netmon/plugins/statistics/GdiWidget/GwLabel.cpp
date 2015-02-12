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
#include "GwLabel.h"

#include "../../../utils/Utils.h"

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
