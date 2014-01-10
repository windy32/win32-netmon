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
    GwPieChart(HDC hdcTarget, int x, int y, int maxWidth, int maxHeight, const TCHAR **descs, COLORREF *colors, __int64 *values, int count);
    virtual ~GwPieChart();

    virtual void Paint();
};

#endif