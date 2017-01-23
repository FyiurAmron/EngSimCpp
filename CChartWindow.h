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
	int						orderedRedraw;	//okreœla czy zamówiono odœwie¿enie okna
	
	
public:
	int						prevPosX;	//poprzednie po³o¿enie kursora myszki w pixelach
	int						prevPosY;	//poprzednie po³o¿enie kursora myszki w pixelach
	std::vector<CChart *>	charts;		//wektor obszarów wykresów
	//std::wstring			customText,customText2;

	

	CChartWindow(){;}

	/// <summary>Tworzy okno z wykresami</summary>
	///	<param name="szTitle">ci¹g znaków opisuj¹cy tytu³ okna na pasku okna</param>
	///	<param name="nWidth">Szerokoœæ okna w pixelach.</param>
	///	<param name="nHeight">Wysokoœæ okna w pixelach. Wysokoœæ uwzglêdnia tak¿e pasek tytu³owy, zatem obszar roboczy jest mniejszy ni¿ nHeight</param>
	/// <returns>zwraca: uchwyt okna</returns>
	HWND Create(LPCTSTR szTitle, int nWidth, int nHeight);

	/// <summary>Zmienia skale na osi X</summary>
	///	<param name="scale">liczba pixeli przypadaj¹ca na jednostkê wykresu</param>
	///	<param name="forceRecalc">ustala czy zapisane w pamiêci przebiegi zostale ponownie przeskalowane tak, by wyœwietlaæ mniej punktów; mo¿liwe wartoœci: true/false; domyœlna wartoœæ: true</param>
	void ChangeXScale(double scale, int forceRecalc = true);
	
	/// <summary>Zmienia skale na osi Y</summary>
	///	<param name="scale">liczba pixeli przypadaj¹ca na jednostkê wykresu</param>
	void ChangeYScale(double scale);
	
	/// <summary>Dodaje jeden obszar wykresów do okna</summary>
	void AddChart();

	/// <summary>Dodaje obszary wykresów do okna</summary>
	///	<param name="num">liczba dodawanych obszarów</param>
	void AddCharts(int num);
	
	/// <summary>Ustawia wartoœæ X na pocz¹tku okna (po lewej stronie)</summary>
	///	<param name="x">wartoœæ wyra¿ona w jednostkach wykresu</param>
	void SetXPos(double x);

	/// <summary>Wymusza odœwie¿enie okna. Ewentualne odœwie¿enie wystêpuje nie czêœciej ni¿ co 50ms.</summary>
	void OrderRedraw();

	/// <summary>Ustawia kolory obszaru wykresów</summary>
	///	<param name="cBackground">kolor t³a, mo¿na u¿yæ makra RGBA lub RGB</param>
	///	<param name="cGrid">kolor siatki, mo¿na u¿yæ makra RGBA lub RGB</param>
	///	<param name="cLabels">kolor napisów, mo¿na u¿yæ makra RGBA lub RGB</param>
	void SetColors(DWORD cBackground, DWORD cGrid, DWORD cLabels);

	/// <summary>Ustawia szerokoœæ obszaru wszystkich wykresów okna w jednostkach wykresu</summary>
	///	<param name="width">szerokoœæ obszaru wykresu w jednostkach wykresu</param>
	void SetWidth(double width);

	/// <summary>Sprawdza czy wspó³rzêdna znajduje siê w aktualnie wyœwietlanym obszarze. Pod uwagê brany jest obszar pierwszego wykresu okna.</summary>
	///	<param name="x">Wspó³rzêdna X wyra¿ona w jednostkach wykresu (nie w pixelach)</param>
	/// <returns>zwraca: 0 - gdy punkt znajduje siê w obszarze okna, 1 - gdy punkt znajduje siê poza obszarem po prawej stronie, -1 - gdy punkt jest poza obszarem po lewej stronie, 2 - gdy okno nie ma zdefiniowanego ¿adnego wykresu</returns>
	int PointOutsideWindowX(double x);

	void Autoscale();
};

#endif