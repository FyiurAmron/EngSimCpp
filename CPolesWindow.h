#ifndef POLESWINDOW_H
#define POLESWINDOW_H

#include "CChartWindow.h"
#include "CChart.h"
#include "CGraphics.h"
#include "CGDI.h"

#include <map>
#include <vector>

class CPolesWindow : public CChartWindow
{
	static bool	bWndClassRegistered;
	static void RegisterWndClass();
	LRESULT ChartWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK StaticChartWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	static std::map<HWND,CPolesWindow*> allWindows;

	CGraphics				*graph;
	double					scale;
	
	
public:
	int						prevPosX;
	int						prevPosY;
	std::vector<CChart *>	charts;

	CPolesWindow(){;}

	HWND Create(LPCTSTR szTitle, int nWidth, int nHeight);
	void ChangeXScale(double scale, int forceRecalc = true);
	void AddChart();
	void SetXPos(double x);
	
};

#endif