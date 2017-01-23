#include "stdafx.h"
#include "SimConfig.h"

#include "CChartWindow.h"
#include "CPropWindow.h"
//#include "CChart.h"
//#include "CSignal.h"
#include "CSimulation.h"
#include "CFile.h"
#include "MDosSim.h"

/*#define ARMA_USE_LAPACK
#define ARMA_USE_BLAS
#include <armadillo>*/

#include "dos2win.h"
#include "dos/dos_MAIN.h"
//#include "dos/main.h"

//double m0=1;

CChartWindow wChartW;


//***** PROSZĘ NIE EDYTOWAć TEGO KODU ***********************
//***** edycji należy dokonywać w funkcji Simulations() *****

class CSimulationOK : public CSimulation {
public:
    MDosSim dos;

    CSimulationOK( ) : dos( this, main, &TIME ) {
        ;
    }
    void PreCalc( );
};
CSimulationOK sim;

double OnKeyChart( WPARAM keyCode, UINT message, UINT specialKeys, void *parameter ) {
    if ( message == WM_KEYDOWN ) {
        if ( keyCode == VK_SPACE ) {
            wChartW.SetXPos( sim.t );
            sim.ResumeSimulation( );
        }
    }

    return 0;
}

void CSimulationOK::PreCalc( ) {
    double speed = 1;
    int ptOutsideWnd;


    dos.TriggerCalcOnce( );

    ptOutsideWnd = wChartW.PointOutsideWindowX( t );
    if ( ptOutsideWnd == 0 ) {
        wChartW.OrderRedraw( );
    }
    else if ( ptOutsideWnd == 1 ) {
        PauseSimulation( );
    }
}

void Interface( ) {
    ;
}

//***** KONIEC ZAKAZU EDYCJI ***********************
//**************************************************

void Simulations( ) {
    static CPropWindow wProps;
    static CPropertiesGroup propsGroup;

    CPropWindow::sim = &sim;
    CChart::sim = &sim;

    dos_model = &sim.dos;

    propsGroup.groupName = _T( "Zmiana parametrow" );
    propsGroup.AddProperty( &i2zad, _T( "i2zad" ), NULL, -2, 2 );
    //propsGroup.AddProperty(&i2zad,_T("i2zad"),NULL,-2,2);

    wProps.Create( _T( "Parametry" ), 400, 550, &propsGroup );
    wProps.SetParent( &wChartW );

    wChartW.AddKeyProc( OnKeyChart, NULL );
    wChartW.Create( _T( "symulacja" ), 1280, 700 );





    //dodawanie obszarów wykresów do okna
    wChartW.AddCharts( 5 );

    //definiowanie rejestrowanych i wyświetlanych sygnałów
    wChartW.charts[0]->AddSignal( &gamma, _T( "gamma" ), RGB_( 240, 0, 220 ) );
    wChartW.charts[1]->AddSignal( &i1, _T( "i1" ), RGB_( 44, 134, 167 ) );
    wChartW.charts[2]->AddSignal( &i2, _T( "i2" ), RGB_( 0, 255, 255 ) ); //TIME - czas symulacji dosowej _s ->/1000, bez => w sekundach
    wChartW.charts[3]->AddSignal( &uzas, _T( "uzas" ), RGB_( 44, 134, 167 ) );
    wChartW.charts[4]->AddSignal( &u1, _T( "u1" ), RGB_( 44, 134, 167 ) );
    //wChartW.charts[3]->AddSignal(&TIME_s,&x21,_T("opis"),RGB_(44,134,167));






    //definiowanie poczatkowego polozenia wykresow
    //polozenie na osi X ustawia sie dla wszystkich okien jednoczesnie
    //polozenia na osi Y okresla sie dla kazdego wykresu oddzielnie
    wChartW.SetXPos( 0 );
    wChartW.charts[0]->SetYPos( -1 );
    wChartW.charts[1]->SetYPos( 0 );
    //wChartW.charts[2]->SetYPos(-1);

    //definiowanie poczatkowej skali wykresow
    //skale na osi X ustawia sie dla wszystkich okien jednoczesnie, podajac szerokosc w jednostkach wykresu (w sekundach w tym przypadku)
    //skale na osi Y okresla sie dla kazdego wykresu oddzielnie
    wChartW.SetWidth( 1 );
    wChartW.charts[0]->SetHeight( 2 );
    wChartW.charts[1]->SetHeight( 2 );
    //	wChartW.charts[2]->SetHeight(2);

    //definiowanie kolorow okna (tla,siatki,etykiet)
    wChartW.SetColors( RGBA( 0, 0, 0, 128 ), RGBA( 255, 255, 255, 100 ), RGB_( 255, 255, 0 ) );


    sim.SetStep( 1e-5, 5e-5, 2.5 ); //kroki oraz czas symulacji
    sim.StartSimulation( ); //rozpoczecie symulacji
}

void FinishSimulations( ) {
    sim.EndSimulation( );
}