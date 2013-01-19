#ifndef GW_GROUPBOX_H
#define GW_GROUPBOX_H

#include "GdiWidget.h"

class GwGroupbox : public GdiWidget
{
protected:
	const TCHAR *_caption;
	COLORREF _color;

	HFONT _hFont;

	int _contentX;
	int _contentY;
	int _contentWidth;
	int _contentHeight;

protected:
	void CalcSize();

public:
	GwGroupbox(HDC hdcTarget, int x, int y, int maxWidth, int maxHeight, const TCHAR *caption, COLORREF color);
	virtual ~GwGroupbox();

	virtual void Paint();

	virtual void GetContentArea(int *pX, int *pY, int *pWidth, int *pHeight);
};

#endif