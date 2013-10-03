#ifndef GW_HISTOGRAM_H
#define GW_HISTOGRAM_H

#include "GdiWidget.h"

class GwHistogram : public GdiWidget
{
protected:
	enum { MAX_VALUE = 2000, MAX_SCALE = 20 };

	__int64 _values[MAX_VALUE];
	int _cValue;
	int _scales[MAX_SCALE];
	int _cScale;
	const TCHAR *_caption;
	COLORREF _color;

	HFONT _hFont;
	HFONT _hEnglishFont;

	int _boxWidth;
	int _boxHeight;
	int _captionX;
	int _captionY;

protected:
	void CalcSize();

public:
	GwHistogram(HDC hdcTarget, int x, int y, int maxWidth, int maxHeight, __int64 *values, int cValue, int *scales, int cScale, const TCHAR *caption, COLORREF color);
	virtual ~GwHistogram();

	virtual void Paint();
};


#endif