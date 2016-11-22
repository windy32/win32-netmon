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

#ifndef GW_PIE_CHART_H
#define GW_PIE_CHART_H

#include "GdiWidget.h"

class GwPieChart : public GdiWidget
{
protected:
    enum { MAX_ITEM = 64 };

    int _count;
    const TCHAR *_descs[MAX_ITEM];
    COLORREF _colors[MAX_ITEM];
    __int64 _values[MAX_ITEM];

    HFONT _hFont;

    int _maxDescWidth;
    int _maxValueWidth;
    int _legendWidth;
    int _legendHeight; 
    int _pieWidth;
    int _pieHeight;
    int _pieRadius;
    int _marginLegend;
    int _marginDesc;
    int _marginValue;

protected:
    void CalcSize();

public:
    GwPieChart(HDC hdcTarget, int x, int y, int maxWidth, int maxHeight, 
        const TCHAR **descs, COLORREF *colors, __int64 *values, int count);
    virtual ~GwPieChart();

    virtual void Paint();
};

#endif