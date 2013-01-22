#ifndef GDI_WIDGET_H
#define GDI_WIDGET_H

// Netmon paints various kinds of graphs include histograms, pie charts, etc.
//
// One way to make the painting code reusable is creating user-defined window classes.
//
// GDI Widget is a simpler way that allows users create reusable graphs & charts 
// without implementing a complete window class.
//
// The GDI Widgets are basically static objects that does not process any 
// mouse or keyboard events.
//
// A typical usage is as shown below
//
//     else if( uMsg == WM_PAINT )
//     {
//         PAINTSTRUCT stPS;
//         BeginPaint(hWnd, &stPS);
//
//         // Prepare data for pie chart
//         descs = ...
//         colors = ....
//         values = ...
//
//         // Create GDI widget object
//     	   GwPieChart pieChart(
//             stPS.hdc,                  // Target HDC
//             20, 20, 180, 90,           // The left, top, width & height of the GDI widget
//             descs, colors, values, 5); // Other concrete-widget-specific data
//
//         // Paint
//         pieChart.Paint();
//     
//         // Paint others
//         ....
//     
//         EndPaint(hWnd, &stPS);
//     }

class GdiWidget
{
protected:
	HDC _hdcTarget;

	int _x;
	int _y;
	int _maxWidth;
	int _maxHeight;
	int _width;
	int _height;

protected:
	BOOL GwSetPixel(int x, int y, COLORREF color);
	BOOL GwMoveToEx(int x, int y, LPPOINT lppt);
	BOOL GwLineTo(int x, int y);
	BOOL GwRectangle(int left, int top, int right, int bottom);
	BOOL GwEllipse(int left, int top, int right, int bottom);
	BOOL GwPie(int left, int top, int right, int bottom, int xr1, int yr1, int xr2, int yr2);
	BOOL GwTextOut(int x, int y, LPCTSTR lpString, int c);
public:
	GdiWidget(HDC hdcTarget, int x, int y, int maxWidth, int maxHeight);
	virtual ~GdiWidget();
	int GetWidth();
	int GetHeight();

	virtual void Paint() = 0;
};

#endif