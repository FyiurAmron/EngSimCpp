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
	int						orderedRedraw;	//określa czy zamówiono odświeżenie okna
	
	
public:
	int						prevPosX;	//poprzednie położenie kursora myszki w pixelach
	int						prevPosY;	//poprzednie położenie kursora myszki w pixelach
	std::vector<CChart *>	charts;		//wektor obszarów wykresów
	//std::wstring			customText,customText2;

	

	CChartWindow(){;}

	/// <summary>Tworzy okno z wykresami</summary>
	///	<param name="szTitle">ciąg znaków opisujący tytuł okna na pasku okna</param>
	///	<param name="nWidth">Szerokość okna w pixelach.</param>
	///	<param name="nHeight">Wysokość okna w pixelach. Wysokość uwzględnia także pasek tytułowy, zatem obszar roboczy jest mniejszy niż nHeight</param>
	/// <returns>zwraca: uchwyt okna</returns>
	HWND Create(LPCTSTR szTitle, int nWidth, int nHeight);

	/// <summary>Zmienia skale na osi X</summary>
	///	<param name="scale">liczba pixeli przypadająca na jednostkę wykresu</param>
	///	<param name="forceRecalc">ustala czy zapisane w pamięci przebiegi zostale ponownie przeskalowane tak, by wyświetlać mniej punktów; możliwe wartości: true/false; domyślna wartość: true</param>
	void ChangeXScale(double scale, int forceRecalc = true);
	
	/// <summary>Zmienia skale na osi Y</summary>
	///	<param name="scale">liczba pixeli przypadająca na jednostkę wykresu</param>
	void ChangeYScale(double scale);
	
	/// <summary>Dodaje jeden obszar wykresów do okna</summary>
	void AddChart();

	/// <summary>Dodaje obszary wykresów do okna</summary>
	///	<param name="num">liczba dodawanych obszarów</param>
	void AddCharts(int num);
	
	/// <summary>Ustawia wartość X na początku okna (po lewej stronie)</summary>
	///	<param name="x">wartość wyrażona w jednostkach wykresu</param>
	void SetXPos(double x);

	/// <summary>Wymusza odświeżenie okna. Ewentualne odświeżenie występuje nie częściej niż co 50ms.</summary>
	void OrderRedraw();

	/// <summary>Ustawia kolory obszaru wykresów</summary>
	///	<param name="cBackground">kolor tła, można użyć makra RGBA lub RGB</param>
	///	<param name="cGrid">kolor siatki, można użyć makra RGBA lub RGB</param>
	///	<param name="cLabels">kolor napisów, można użyć makra RGBA lub RGB</param>
	void SetColors(DWORD cBackground, DWORD cGrid, DWORD cLabels);

	/// <summary>Ustawia szerokość obszaru wszystkich wykresów okna w jednostkach wykresu</summary>
	///	<param name="width">szerokość obszaru wykresu w jednostkach wykresu</param>
	void SetWidth(double width);

	/// <summary>Sprawdza czy współrzędna znajduje się w aktualnie wyświetlanym obszarze. Pod uwagę brany jest obszar pierwszego wykresu okna.</summary>
	///	<param name="x">Współrzędna X wyrażona w jednostkach wykresu (nie w pixelach)</param>
	/// <returns>zwraca: 0 - gdy punkt znajduje się w obszarze okna, 1 - gdy punkt znajduje się poza obszarem po prawej stronie, -1 - gdy punkt jest poza obszarem po lewej stronie, 2 - gdy okno nie ma zdefiniowanego żadnego wykresu</returns>
	int PointOutsideWindowX(double x);

	void Autoscale();
};

#endif