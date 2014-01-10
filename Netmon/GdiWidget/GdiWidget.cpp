#include "stdafx.h"
#include "GdiWidget.h"

BOOL GdiWidget::GwSetPixel(int x, int y, COLORREF color)
{
    return SetPixel(_hdcTarget, x + _x, y + _y, color);
}

BOOL GdiWidget::GwMoveToEx(int x, int y, LPPOINT lppt)
{
    return MoveToEx(_hdcTarget, x + _x, y + _y, lppt);
}

BOOL GdiWidget::GwLineTo(int x, int y)
{
    return LineTo(_hdcTarget, x  + _x, y + _y);
}

BOOL GdiWidget::GwRectangle(int left, int top, int right, int bottom)
{
    return Rectangle(_hdcTarget, left + _x, top + _y, right + _x, bottom + _y);
}

BOOL GdiWidget::GwEllipse(int left, int top, int right, int bottom)
{
    return Ellipse(_hdcTarget, left + _x, top + _y, right + _x, bottom + _y);
}

BOOL GdiWidget::GwPie(int left, int top, int right, int bottom, int xr1, int yr1, int xr2, int yr2)
{
    return Pie(_hdcTarget, left + _x, top + _y, right + _x, bottom + _y, xr1 + _x, yr1 + _y, xr2 + _x, yr2 + _y);
}

BOOL GdiWidget::GwTextOut(int x, int y, LPCTSTR lpString, int c)
{
    return TextOut(_hdcTarget, x + _x, y + _y, lpString, c);
}

GdiWidget::GdiWidget(HDC hdcTarget, int x, int y, int maxWidth, int maxHeight)
{
    _hdcTarget = hdcTarget;
    _x = x;
    _y = y;
    _maxWidth = maxWidth;
    _maxHeight = maxHeight;
}

GdiWidget::~GdiWidget()
{
}

int GdiWidget::GetWidth()
{
    return _width;
}

int GdiWidget::GetHeight()
{
    return _height;
}
