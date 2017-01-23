#ifndef CHARTWINDOW_H
#define CHARTWINDOW_H

#include "CWindow.h"
#include "CChart.h"
#include "CGraphics.h"
#include "CGDI.h"

#include <map>
#include <vector>
#include <string>

class CChartWindow : public CWindow
{
	static bool	bWndClassRegistered;
	static void RegisterWndClass();
	LRESULT ChartWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK StaticChartWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	static std::map<HWND,CChartWindow*> allWindows;

	CGraphics				*graph;
	double					scale;
	int						orderedRedraw;	//okre�la czy zam�wiono od�wie�enie okna
	
	
public:
	int						prevPosX;	//poprzednie po�o�enie kursora myszki w pixelach
	int						prevPosY;	//poprzednie po�o�enie kursora myszki w pixelach
	std::vector<CChart *>	charts;		//wektor obszar�w wykres�w
	//std::wstring			customText,customText2;

	

	CChartWindow(){;}

	/// <summary>Tworzy okno z wykresami</summary>
	///	<param name="szTitle">ci�g znak�w opisuj�cy tytu� okna na pasku okna</param>
	///	<param name="nWidth">Szeroko�� okna w pixelach.</param>
	///	<param name="nHeight">Wysoko�� okna w pixelach. Wysoko�� uwzgl�dnia tak�e pasek tytu�owy, zatem obszar roboczy jest mniejszy ni� nHeight</param>
	/// <returns>zwraca: uchwyt okna</returns>
	HWND Create(LPCTSTR szTitle, int nWidth, int nHeight);

	/// <summary>Zmienia skale na osi X</summary>
	///	<param name="scale">liczba pixeli przypadaj�ca na jednostk� wykresu</param>
	///	<param name="forceRecalc">ustala czy zapisane w pami�ci przebiegi zostale ponownie przeskalowane tak, by wy�wietla� mniej punkt�w; mo�liwe warto�ci: true/false; domy�lna warto��: true</param>
	void ChangeXScale(double scale, int forceRecalc = true);
	
	/// <summary>Zmienia skale na osi Y</summary>
	///	<param name="scale">liczba pixeli przypadaj�ca na jednostk� wykresu</param>
	void ChangeYScale(double scale);
	
	/// <summary>Dodaje jeden obszar wykres�w do okna</summary>
	void AddChart();

	/// <summary>Dodaje obszary wykres�w do okna</summary>
	///	<param name="num">liczba dodawanych obszar�w</param>
	void AddCharts(int num);
	
	/// <summary>Ustawia warto�� X na pocz�tku okna (po lewej stronie)</summary>
	///	<param name="x">warto�� wyra�ona w jednostkach wykresu</param>
	void SetXPos(double x);

	/// <summary>Wymusza od�wie�enie okna. Ewentualne od�wie�enie wyst�puje nie cz�ciej ni� co 50ms.</summary>
	void OrderRedraw();

	/// <summary>Ustawia kolory obszaru wykres�w</summary>
	///	<param name="cBackground">kolor t�a, mo�na u�y� makra RGBA lub RGB</param>
	///	<param name="cGrid">kolor siatki, mo�na u�y� makra RGBA lub RGB</param>
	///	<param name="cLabels">kolor napis�w, mo�na u�y� makra RGBA lub RGB</param>
	void SetColors(DWORD cBackground, DWORD cGrid, DWORD cLabels);

	/// <summary>Ustawia szeroko�� obszaru wszystkich wykres�w okna w jednostkach wykresu</summary>
	///	<param name="width">szeroko�� obszaru wykresu w jednostkach wykresu</param>
	void SetWidth(double width);

	/// <summary>Sprawdza czy wsp�rz�dna znajduje si� w aktualnie wy�wietlanym obszarze. Pod uwag� brany jest obszar pierwszego wykresu okna.</summary>
	///	<param name="x">Wsp�rz�dna X wyra�ona w jednostkach wykresu (nie w pixelach)</param>
	/// <returns>zwraca: 0 - gdy punkt znajduje si� w obszarze okna, 1 - gdy punkt znajduje si� poza obszarem po prawej stronie, -1 - gdy punkt jest poza obszarem po lewej stronie, 2 - gdy okno nie ma zdefiniowanego �adnego wykresu</returns>
	int PointOutsideWindowX(double x);

	void Autoscale();
};

#endif