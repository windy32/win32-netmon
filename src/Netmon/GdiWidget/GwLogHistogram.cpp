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
#include "GwLogHistogram.h"

#include "../Utils/utils.h"

GwLogHistogram::GwLogHistogram(
    HDC hdcTarget, int x, int y, int maxWidth, int maxHeight, 
    __int64 *values, int cValue, int *scales, int cScale, 
    const TCHAR *caption, COLORREF color, int logBase, int logSegments)
    : GwHistogram(
        hdcTarget, x, y, maxWidth, maxHeight, values, cValue, scales, cScale, caption, color)
{
    _logBase = logBase;
    _logSegments = logSegments;
}

void GwLogHistogram::Paint()
{
    // Select font
    HFONT hOldFont = (HFONT) SelectObject(_hdcTarget, _hEnglishFont);

    // Histogram box
    SetDCPenColor(_hdcTarget, RGB(0x80, 0x80, 0x80));
    GwRectangle(0, 0, _boxWidth, _boxHeight);

    // Segments
    SetDCPenColor(_hdcTarget, RGB(0xC0, 0xC0, 0xC0));
    for(int i = 1; i < _logSegments; i++)
    {
        GwMoveToEx(1, _boxHeight * i / _logSegments, 0);
        GwLineTo(_width - 1, _boxHeight * i / _logSegments);
    }

    // Caption
    SelectObject(_hdcTarget, _hFont);
    SetTextAlign(_hdcTarget, TA_RIGHT);
    SetTextColor(_hdcTarget, RGB(0, 0, 0));
    GwTextOut(_captionX, _captionY - 8, _caption, _tcslen(_caption));

    SelectObject(_hdcTarget, _hEnglishFont);

    // X axis
    SetDCPenColor(_hdcTarget, RGB(0xC0, 0xC0, 0xC0));

    GwMoveToEx(0, _boxHeight + 1, 0);
    GwLineTo(_width, _boxHeight + 1);

    GwMoveToEx(0, _boxHeight + 2, 0);
    GwLineTo(_width, _boxHeight + 2);

    // X axis scale
    for(int i = 0; i < _cScale; i++)
    {
        // Point
        GwSetPixel(1 + (_scales[i] - 1) * (_width - 2) / _cValue, _boxHeight + 1, RGB(0, 0, 0));
        GwSetPixel(1 + (_scales[i] - 1) * (_width - 2) / _cValue, _boxHeight + 2, RGB(0, 0, 0));

        // Text
        if( _scales[i] > _cValue / 2 )
        {
            SetTextAlign(_hdcTarget, TA_RIGHT);
        }
        else
        {
            SetTextAlign(_hdcTarget, TA_LEFT);
        }

        TCHAR szText[32];
        _stprintf_s(szText, _countof(szText), TEXT("%d"), _scales[i]);
        
        GwTextOut((_scales[i] - 1) * (_width - 2) / _cValue, _boxHeight + 3, 
            szText, _tcslen(szText));
    }

    // Histogram
    double maxAvr = 0;

    for(int i = 0; i < _width - 2; i++)
    {
        __int64 sum = 0;
        __int64 sample = 0;
        double avr = 0;

        for(int j = i * _cValue / (_width - 2); j < (i + 1) * _cValue / (_width - 2); j++)
        {
            sum += _values[j];
            sample += 1;
        }
        avr = (double)sum / sample;
        maxAvr = ( avr > maxAvr ) ? avr : maxAvr;
    }

    SetDCPenColor(_hdcTarget, _color);

    GwMoveToEx(1, _boxHeight - 2, 0);
    GwLineTo(_width - 1, _boxHeight - 2);

    if( maxAvr > 0 )
    {
        for(int i = 0; i < _width - 2; i++)
        {
            __int64 sum = 0;
            __int64 sample = 0;
            double avr = 0;

            for(int j = i * _cValue / (_width - 2); j < (i + 1) * _cValue / (_width - 2); j++)
            {
                sum += _values[j];
                sample += 1;
            }
            avr = (double)sum / sample;

            GwMoveToEx(i + 1, _boxHeight - 2, 0);

            // Calc y value of the histogram
            double logVal;
            if( avr / maxAvr < 1.0 / Utils::Exp(_logSegments - 1, _logBase))
            {
                logVal = (avr / maxAvr) * Utils::Exp(_logSegments - 1, _logBase) / _logSegments;
            }
            else 
            {
                logVal = 1 + Utils::Log(avr / maxAvr, _logBase) / _logSegments;
            }
            GwLineTo(i + 1, _boxHeight - 2 - (int)(logVal * (_boxHeight - 2)));
        }
    }

    // Restore
    SelectObject(_hdcTarget, hOldFont);
    SetDCPenColor(_hdcTarget, RGB(0, 0, 0));
}