#ifndef CHART_H
#define CHART_H

#include <string>
#include <vector>
#include <math.h>

#include "CSimulation.h"

extern class CSignal;
extern class CGraphics;

class CChart
{
	
	std::string	xLabel;
	std::string	yLabel;
	int		x1;
	int		x2;
	int		y1;
	int		y2;

	int		frameX1,frameY1,frameX2,frameY2;

	RECT	clientArea;

	int	gridResX;
	int gridResY;

	int marginLeft,marginRight;
	int marginTop,marginBottom;

	bool	resapleRequired;

	
	std::vector<DWORD> plotColors;
	std::vector<int> prevXNum;
	std::vector<double> prevXVal;
	int nSignalNum;

	DWORD colorBackground;
	DWORD colorGrid;
	DWORD colorLabels;
	static int markerFontSize;
	
	//void Copy(const CChart& obj);
public:
	//CSignal		*signal;
	static CSimulation *sim;

	std::vector<CSignal *> signals;
	std::vector<std::vector<CSignal> *> signalsVect;
	std::vector<int> signalsElement;

	CGraphics	*graph;
	int transforming;	//czy bedzie skalowany i przesuwany (czy kliknieto mysza na tym wykresie)
	double	markerX;
	double	x0,y0;
	double	xWidth;
	double	yHeight;
	double	scaleX;
	double	scaleY;

	double minX,maxX,minY,maxY;
	

	CChart();
	~CChart(){MessageBox(NULL,_T("sdf"),_T("sdf"),MB_OK);}
	//CChart(const CChart& obj);
	CChart(CGraphics *graph);
	
	/// <summary>Rysuje wykres</summary>
	void DrawChart();

	/// <summary>Przemieszcza wykres</summary>
	///	<param name="x">przemieszczenie na osi x w jednostkach wykresu</param>
	///	<param name="y">przemieszczenie na osi y w jednostkach wykresu</param>
	void SetPlotOffset(double x, double y);

	/// <summary>Zmienia skale wykresu.</summary>
	///	<param name="x">gdy incremetal==false, nowa skala na osi X; gdy incremental==true, skala jest zmieniana wykladniczo wraz ze wzrostem wartosci x</param>
	///	<param name="y">gdy incremetal==false, nowa skala na osi Y; gdy incremental==true, skala jest zmieniana wykladniczo wraz ze wzrostem wartosci y</param>
	///	<param name="increment">okre�la czy nowa skala b�dzie r�wna warto�ciom parametr�w x,y (wartos� false), czy b�dzie zmieniana przyrostowo (warto�� true)</param>
	///	<param name="forceRecalc">gdy true: tablicami z wy�wietlanymi warto�ciami b�dzie zaktualizowana tak by nie obci��a� procesora; gdy false: obliczenia nie jest przeprowadzane</param>
	void ChangeScale(double x, double y, int increment = false, int forceRecalc = true);
	
	
	void SetArea(int x1, int y1, int x2, int y2);
	
	/// <summary>Definiuje nowy sygna� do rejestracji oraz dodaje go do wykresu.</summary>
	///	<param name="x">wska�nik do rejestrowanej zmiennej (o� X)</param>
	///	<param name="y">wska�nik do rejestrowanej zmiennej (o� Y)</param>
	///	<param name="name">nazwa sygna�u/przebiegu</param>
	/// <param name="offset">pionowe przesuniecie wykresu</param>
	/// <param name="scale">skalowanie przebiegu, wykonywane po przesunieciu</param>
	///	<param name="color">kolor przebiegu, mo�na u�y� makr: RGB, RGBA</param>
	void AddSignal(const double *x, const double *y, std::wstring name, DWORD color, double offset=0, double scale=1);

	/// <summary>Definiuje nowy sygna� do rejestracji oraz dodaje go do wykresu. Automatycznie przyjmuje czas symulacji jako warto�� na osi X.</summary>
	///	<param name="y">wska�nik do rejestrowanej zmiennej</param>
	///	<param name="name">nazwa sygna�u/przebiegu</param>
	/// <param name="offset">pionowe przesuniecie wykresu</param>
	/// <param name="scale">skalowanie przebiegu, wykonywane po przesunieciu</param>
	///	<param name="color">kolor przebiegu, mo�na u�y� makr: RGB, RGBA</param>
	void AddSignal(const double *y, std::wstring name, DWORD color, double offset=0, double scale=1);
	
	/// <summary>Dodaje istniej�cy sygna� do wykresu</summary>
	///	<param name="signal">wska�nik do sygna�u</param>
	///	<param name="color">kolor przebiegu, mo�na u�y� makr: RGB, RGBA</param>
	void AddSignal(CSignal *signal, DWORD color);

	/// <summary>Dodaje istniej�cy sygna� do wykresu</summary>
	///	<param name="signalVect">wska�nik do wektora zawieraj�cego sygna�</param>
	///	<param name="signalElement">index vectora z dodawanym sygna�em</param>
	///	<param name="color">kolor przebiegu, mo�na u�y� makr: RGB, RGBA</param>
	void AddSignal(std::vector<CSignal> *signalVect, int signalElement, DWORD color);
	
	/// <summary>Aktualizuje wska�niki do sygna��w. Nale�y wywo�a� funkcj� po ka�dej zmianie rozmiaru wektor�w zawieraj�cych wykresy.</summary>
	void UpdateSignalsPointers();

	/// <summary>Sprawdza czy kursor myszy znajduje si� nad obszarem wykresu</summary>
	///	<param name="mouseX">po�o�enie myszy na osi X wyra�one w pixelach</param>
	///	<param name="mouseY">po�o�enie myszy na osi Y wyra�one w pixelach</param>
	/// <returns>zwraca: 1 - gdy kursor znajduje si� nad wykresem, 0 - kursor jest poza wykresem</returns>
	int Contains(int mouseX, int mouseY);

	/// <summary>Aktualizuje pozycj� znacznika prezentuj�cego dok�adne warto�ci przebieg�w</summary>
	///	<param name="mouseX">po�o�enie myszy na osi X wyra�one w pixelach</param>
	///	<param name="mouseY">po�o�enie myszy na osi Y wyra�one w pixelach</param>
	void UpdateMarker(int mouseX, int mouseY);
	
	/// <summary>Przeskalowuje wszystkie przebiegi przypisane do wykresu tak by ograniczy� liczb� rysowanych punkt�w</summary>
	void RewriteSignalsAfterScale();

	/// <summary>Rysuje legend� w obszarze wykresu</summary>
	///	<param name="transparency">prze�roczysto�� legendy: 0-255</param>
	void DrawLegend(int transparency);

	/// <summary>Ustawia kolory obszaru wykres�w</summary>
	///	<param name="cBackground">kolor t�a, mo�na u�y� makra RGBA lub RGB</param>
	///	<param name="cGrid">kolor siatki, mo�na u�y� makra RGBA lub RGB</param>
	///	<param name="cLabels">kolor napis�w, mo�na u�y� makra RGBA lub RGB</param>
	void SetColors(DWORD cBackground, DWORD cGrid, DWORD cLabels);

	/// <summary>Ustawia wysoko�� obszaru wykresu w jednostkach wykresu</summary>
	///	<param name="height">wysoko�� obszaru wykresu w jednostkach wykresu</param>
	void SetHeight(double height); 

	/// <summary>Ustawia szeroko�� obszaru wykresu w jednostkach wykresu</summary>
	///	<param name="width">szeroko�� obszaru wykresu w jednostkach wykresu</param>
	void SetWidth(double width); 

	/// <summary>Ustawia warto�� na osi Y na dole obszaru wykresu</summary>
	///	<param name="height">warto�� Y na dole obszaru wykresu</param>
	void SetYPos(double y); 
	
	void FindMargins();
	void SetMargins();
	void SetMarkerFontSize(double fontSize);
};


#endif