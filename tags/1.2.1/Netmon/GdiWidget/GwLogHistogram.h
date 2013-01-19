#ifndef GW_LOG_HISTOGRAM_H
#define GW_LOG_HISTOGRAM_H

#include "GwHistogram.h"

class GwLogHistogram : public GwHistogram
{
protected:
	int _logBase;
	int _logSegments;

public:
	GwLogHistogram(HDC hdcTarget, int x, int y, int maxWidth, int maxHeight, __int64 *values, int cValue, int *scales, int cScale, const TCHAR *caption, COLORREF color, int logBase, int logSegments);

	virtual void Paint();
};

#endif