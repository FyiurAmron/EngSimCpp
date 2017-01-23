#ifndef CHART_H
#define CHART_H

#include <string>
#include <vector>
#include <math.h>

#include "CSimulation.h"

extern class CSignal;
extern class CGraphics;

class CChart {
    std::string xLabel;
    std::string yLabel;
    int x1;
    int x2;
    int y1;
    int y2;

    int frameX1, frameY1, frameX2, frameY2;

    RECT clientArea;

    int gridResX;
    int gridResY;

    int marginLeft, marginRight;
    int marginTop, marginBottom;

    bool resapleRequired;


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

    CGraphics *graph;
    int transforming; //czy bedzie skalowany i przesuwany (czy kliknieto mysza na tym wykresie)
    double markerX;
    double x0, y0;
    double xWidth;
    double yHeight;
    double scaleX;
    double scaleY;

    double minX, maxX, minY, maxY;


    CChart( );

    ~CChart( ) {
        MessageBox( NULL, _T( "sdf" ), _T( "sdf" ), MB_OK );
    }
    //CChart(const CChart& obj);
    CChart( CGraphics *graph );

    /// <summary>Rysuje wykres</summary>
    void DrawChart( );

    /// <summary>Przemieszcza wykres</summary>
    ///	<param name="x">przemieszczenie na osi x w jednostkach wykresu</param>
    ///	<param name="y">przemieszczenie na osi y w jednostkach wykresu</param>
    void SetPlotOffset( double x, double y );

    /// <summary>Zmienia skale wykresu.</summary>
    ///	<param name="x">gdy incremetal==false, nowa skala na osi X; gdy incremental==true, skala jest zmieniana wykladniczo wraz ze wzrostem wartosci x</param>
    ///	<param name="y">gdy incremetal==false, nowa skala na osi Y; gdy incremental==true, skala jest zmieniana wykladniczo wraz ze wzrostem wartosci y</param>
    ///	<param name="increment">określa czy nowa skala będzie równa wartościom parametrów x,y (wartosć false), czy będzie zmieniana przyrostowo (wartość true)</param>
    ///	<param name="forceRecalc">gdy true: tablicami z wyświetlanymi wartościami będzie zaktualizowana tak by nie obciążać procesora; gdy false: obliczenia nie jest przeprowadzane</param>
    void ChangeScale( double x, double y, int increment = false, int forceRecalc = true );


    void SetArea( int x1, int y1, int x2, int y2 );

    /// <summary>Definiuje nowy sygnał do rejestracji oraz dodaje go do wykresu.</summary>
    ///	<param name="x">wskaźnik do rejestrowanej zmiennej (oś X)</param>
    ///	<param name="y">wskaźnik do rejestrowanej zmiennej (oś Y)</param>
    ///	<param name="name">nazwa sygnału/przebiegu</param>
    /// <param name="offset">pionowe przesuniecie wykresu</param>
    /// <param name="scale">skalowanie przebiegu, wykonywane po przesunieciu</param>
    ///	<param name="color">kolor przebiegu, można użyć makr: RGB, RGBA</param>
    void AddSignal( const double *x, const double *y, std::wstring name, DWORD color, double offset = 0, double scale = 1 );

    /// <summary>Definiuje nowy sygnał do rejestracji oraz dodaje go do wykresu. Automatycznie przyjmuje czas symulacji jako wartość na osi X.</summary>
    ///	<param name="y">wskaźnik do rejestrowanej zmiennej</param>
    ///	<param name="name">nazwa sygnału/przebiegu</param>
    /// <param name="offset">pionowe przesuniecie wykresu</param>
    /// <param name="scale">skalowanie przebiegu, wykonywane po przesunieciu</param>
    ///	<param name="color">kolor przebiegu, można użyć makr: RGB, RGBA</param>
    void AddSignal( const double *y, std::wstring name, DWORD color, double offset = 0, double scale = 1 );

    /// <summary>Dodaje istniejący sygnał do wykresu</summary>
    ///	<param name="signal">wskaźnik do sygnału</param>
    ///	<param name="color">kolor przebiegu, można użyć makr: RGB, RGBA</param>
    void AddSignal( CSignal *signal, DWORD color );

    /// <summary>Dodaje istniejący sygnał do wykresu</summary>
    ///	<param name="signalVect">wskaźnik do wektora zawierającego sygnał</param>
    ///	<param name="signalElement">index vectora z dodawanym sygnałem</param>
    ///	<param name="color">kolor przebiegu, można użyć makr: RGB, RGBA</param>
    void AddSignal( std::vector<CSignal> *signalVect, int signalElement, DWORD color );

    /// <summary>Aktualizuje wskaźniki do sygnałów. Należy wywołać funkcję po każdej zmianie rozmiaru wektorów zawierających wykresy.</summary>
    void UpdateSignalsPointers( );

    /// <summary>Sprawdza czy kursor myszy znajduje się nad obszarem wykresu</summary>
    ///	<param name="mouseX">położenie myszy na osi X wyrażone w pixelach</param>
    ///	<param name="mouseY">położenie myszy na osi Y wyrażone w pixelach</param>
    /// <returns>zwraca: 1 - gdy kursor znajduje się nad wykresem, 0 - kursor jest poza wykresem</returns>
    int Contains( int mouseX, int mouseY );

    /// <summary>Aktualizuje pozycję znacznika prezentującego dokładne wartości przebiegów</summary>
    ///	<param name="mouseX">położenie myszy na osi X wyrażone w pixelach</param>
    ///	<param name="mouseY">położenie myszy na osi Y wyrażone w pixelach</param>
    void UpdateMarker( int mouseX, int mouseY );

    /// <summary>Przeskalowuje wszystkie przebiegi przypisane do wykresu tak by ograniczyć liczbę rysowanych punktów</summary>
    void RewriteSignalsAfterScale( );

    /// <summary>Rysuje legendę w obszarze wykresu</summary>
    ///	<param name="transparency">przeźroczystość legendy: 0-255</param>
    void DrawLegend( int transparency );

    /// <summary>Ustawia kolory obszaru wykresów</summary>
    ///	<param name="cBackground">kolor tła, można użyć makra RGBA lub RGB</param>
    ///	<param name="cGrid">kolor siatki, można użyć makra RGBA lub RGB</param>
    ///	<param name="cLabels">kolor napisów, można użyć makra RGBA lub RGB</param>
    void SetColors( DWORD cBackground, DWORD cGrid, DWORD cLabels );

    /// <summary>Ustawia wysokość obszaru wykresu w jednostkach wykresu</summary>
    ///	<param name="height">wysokość obszaru wykresu w jednostkach wykresu</param>
    void SetHeight( double height );

    /// <summary>Ustawia szerokość obszaru wykresu w jednostkach wykresu</summary>
    ///	<param name="width">szerokość obszaru wykresu w jednostkach wykresu</param>
    void SetWidth( double width );

    /// <summary>Ustawia wartość na osi Y na dole obszaru wykresu</summary>
    ///	<param name="height">wartość Y na dole obszaru wykresu</param>
    void SetYPos( double y );

    void FindMargins( );
    void SetMargins( );
    void SetMarkerFontSize( double fontSize );
};


#endif