#ifndef GW_LABEL_H
#define GW_LABEL_H

#include "GdiWidget.h"

class GwLabel : public GdiWidget
{
protected:
    TCHAR _text[1024];
    HFONT _hFont;

public:
    GwLabel(HDC hdcTarget, int x, int y, int maxWidth, int maxHeight, const TCHAR *caption);
    virtual ~GwLabel();

    virtual void Paint();
};

#endif