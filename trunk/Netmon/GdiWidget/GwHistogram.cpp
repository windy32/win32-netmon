#include "stdafx.h"
#include "GwHistogram.h"

#include "../utils/Utils.h"

GwHistogram::GwHistogram(
    HDC hdcTarget, int x, int y, int maxWidth, int maxHeight, 
    __int64 *values, int cValue, int *scales, int cScale, 
    const TCHAR *caption, COLORREF color) : GdiWidget(hdcTarget, x, y, maxWidth, maxHeight)
{
    assert(cValue > 0 && cValue < MAX_VALUE);
    assert(cScale > 0 && cScale < MAX_SCALE);

    // Save parameters
    _cValue = cValue;
    _cScale = cScale;

    for(int i = 0; i < cValue; i++)
    {
        _values[i] = values[i];
    }

    for(int i = 0; i < cScale; i++)
    {
        _scales[i] = scales[i];
    }

    _caption = caption;
    _color = color;

    // Create font
    _hEnglishFont = Utils::MyCreateFont(TEXT("Arial"), 12, 0, false);
    _hFont = Utils::MyCreateFont(TEXT("MS Shell Dlg 2"), 14, 0, false);

    // Calculate size
    CalcSize();
}

void GwHistogram::CalcSize()
{
    //  -------------------Caption--------
    // |                                 |
    // |                                 |
    // |                                 |
    // |            Histogram            |
    // |                                 |
    // |                                 |
    // |                                 |
    //  ---------------------------------
    // -x--x---------x------------------x-
    //  1  100       600             1500
    _boxWidth = _maxWidth;
    _boxHeight = _maxHeight - 20;
    _captionX = _maxWidth * 8 / 10;
    _captionY = 0;

    _width = _maxWidth;
    _height = _maxHeight;
}


GwHistogram::~GwHistogram()
{
    DeleteObject(_hFont);
    DeleteObject(_hEnglishFont);
}

void GwHistogram::Paint()
{
    // Select font
    HFONT hOldFont = (HFONT) SelectObject(_hdcTarget, _hEnglishFont);

    // Histogram box
    SetDCPenColor(_hdcTarget, RGB(0x80, 0x80, 0x80));
    GwRectangle(0, 0, _boxWidth, _boxHeight);

    // Caption
    SelectObject(_hdcTarget, _hEnglishFont);
    SetTextAlign(_hdcTarget, TA_RIGHT);
    SetTextColor(_hdcTarget, RGB(0, 0, 0));
    GwTextOut(_captionX, _captionY - 8, _caption, _tcslen(_caption));

    SelectObject(_hdcTarget, _hFont);

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
            GwLineTo(i + 1, _boxHeight - 2 - (int)(avr * (_boxHeight - 2) / maxAvr));
        }
    }

    // Restore
    SelectObject(_hdcTarget, hOldFont);
    SetDCPenColor(_hdcTarget, RGB(0, 0, 0));
}
