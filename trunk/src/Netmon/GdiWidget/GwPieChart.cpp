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
#include "GwPieChart.h"

#include "../utils/Utils.h"

GwPieChart::GwPieChart(
    HDC hdcTarget, int x, int y, int maxWidth, int maxHeight, 
    const TCHAR **descs, COLORREF *colors, 
    __int64 *values, int count) : GdiWidget(hdcTarget, x, y, maxWidth, maxHeight)
{
    assert(count > 0 && count < MAX_ITEM);

    // Save parameters
    _count = count;

    for(int i = 0; i < count; i++)
    {
        _descs[i] = descs[i];
        _colors[i] = colors[i];
        _values[i] = values[i];
    }

    // Select font for painting
    _hFont = Utils::MyCreateFont(TEXT("MS Shell Dlg 2"), 14, 0, false);
    HFONT hOldFont = (HFONT) SelectObject(hdcTarget, _hFont);

    // Description text length
    SIZE stSize;
    _maxDescWidth = 0;

    for(int i = 0; i < count; i++)
    {
        GetTextExtentPoint32(hdcTarget, _descs[i], _tcslen(_descs[i]), &stSize);
        _maxDescWidth = ( stSize.cx > _maxDescWidth ) ? stSize.cx : _maxDescWidth;
    }

    // Value text length
    GetTextExtentPoint32(hdcTarget, TEXT("100.0%"), 6, &stSize);
    _maxValueWidth = stSize.cx;

    // Restore font
    SelectObject(hdcTarget, hOldFont);

    // Calculate size
    CalcSize();
}

GwPieChart::~GwPieChart()
{
    DeleteObject(_hFont);
}

void GwPieChart::CalcSize()
{
    //                    8   3      3
    //  ------------------------------------
    // |                 ||L1||Desc1||Value1|
    // |                 ||L2||Desc2||Value2|
    // |                 ||L3||Desc3||Value3|
    // |       Pie       ||L4||Desc4||Value4|
    // |                 ||  ||     ||      |
    // |                 ||  ||     ||      |
    // |                 ||  ||     ||      |
    //  ------------------------------------
    _legendWidth = 12;  // 12 x 11 pixel, border 1 pixel
    _legendHeight = 11; 

    _marginLegend = 8;
    _marginDesc = 4;
    _marginValue = 3;

    _pieWidth = _maxWidth - _maxValueWidth - _maxDescWidth - 
        _legendWidth - _marginLegend - _marginDesc - _marginValue;
    _pieHeight = _maxHeight;

    if (_pieWidth > _pieHeight )
    {
        _pieRadius = _pieHeight / 2;

        _width = _maxWidth - (_pieWidth - _pieHeight);
        _height = _maxHeight;
    }
    else
    {
        _pieRadius = _pieWidth / 2;

        _width = _maxWidth;
        _height = _maxHeight;
    }
}

void GwPieChart::Paint()
{
    // Circle (background)
    SetDCBrushColor(_hdcTarget, RGB(255, 255, 255));
    GwEllipse(0, 0, _pieRadius * 2, _pieRadius * 2);

    // Calc sum of values
    __int64 sum = 0;
    for(int i = 0; i < _count; i++)
    {
        sum += _values[i];
    }

    // Select font
    HFONT hOldFont = (HFONT) SelectObject(_hdcTarget, _hFont);
    SetTextColor(_hdcTarget, RGB(0x00, 0x00, 0x00));

    // Pie chart
    const double PI = 3.1415926535897932384626;
    double angleStart = 0;
    double angleEnd = 0;

    SetTextAlign(_hdcTarget, TA_LEFT);
    SetDCPenColor(_hdcTarget, RGB(0x80, 0x80, 0x80));

    for(int i = 0; i < _count; i++)
    {
        SetDCBrushColor(_hdcTarget, _colors[i]);

        // Pie
        double percentage = (sum == 0) ? 0 : (double)_values[i] / sum;
        angleEnd += percentage * (2 * PI);

        if (angleStart != angleEnd )
        {
            int xr1 = (int)(_pieRadius - _pieRadius * sin(angleStart));
            int yr1 = (int)(_pieRadius - _pieRadius * cos(angleStart));
            int xr2 = (int)(_pieRadius - _pieRadius * sin(angleEnd));
            int yr2 = (int)(_pieRadius - _pieRadius * cos(angleEnd));

            if (xr1 != xr2 || yr1 != yr2 || (angleEnd - angleStart > PI) )
            {
                GwPie(0, 0, _pieRadius * 2, _pieRadius * 2, xr1, yr1, xr2, yr2);
            }
        }
        angleStart += percentage * (2 * PI);

        // Box
        GwRectangle( 
            _pieWidth + _marginLegend, 
            (int)(_legendHeight * (i * 1.5)), 
            _pieWidth + _marginLegend + _legendWidth, 
            (int)(_legendHeight * (i * 1.5 + 1)));

        // Description
        GwTextOut( 
            _pieWidth + _marginLegend + _legendWidth + _marginDesc, 
            (int)(_legendHeight * (i * 1.5)) - 2,
            _descs[i], 
            _tcslen(_descs[i]));

        // Value
        TCHAR szPercentage[32];
        _stprintf_s(szPercentage, _countof(szPercentage), TEXT("%.1lf%%"), 100 * percentage);
        GwTextOut( 
            _pieWidth +_marginLegend + _legendWidth + _marginDesc + _maxDescWidth + _marginValue,
            (int)(_legendHeight * (i * 1.5)) - 2,
            szPercentage, 
            _tcslen(szPercentage));
    }

    SelectObject(_hdcTarget, hOldFont);
    SetDCPenColor(_hdcTarget, RGB(0, 0, 0));
    SetDCBrushColor(_hdcTarget, RGB(255, 255, 255));
}