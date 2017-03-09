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
#include "dos_main.h"
//#include "dos/main.h"

//double m0=1;

CChartWindow wChartW;

class CSimulationOK : public CSimulation {
public:
    MDosSim mDosSim;

    CSimulationOK( ) : mDosSim( this, dos_main, &timeCnt ) { }
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
    //double speed = 1;
    int ptOutsideWnd;


    mDosSim.TriggerCalcOnce( );

    ptOutsideWnd = wChartW.PointOutsideWindowX( t );
    if ( ptOutsideWnd == 0 ) {
        wChartW.OrderRedraw( );
    } else if ( ptOutsideWnd == 1 ) {
        PauseSimulation( );
    }
}

void Interface( ) { }

void Simulations( ) {
    static CPropWindow wProps;
    static CPropertiesGroup propsGroup;

    CPropWindow::sim = &sim;
    CChart::sim = &sim;

    dos_model = &sim.mDosSim;

    propsGroup.groupName = L"Zmiana parametrow";
    //propsGroup.AddProperty( &i2zad, L"i2zad", 0, -2, 2 );
    //propsGroup.AddProperty(&i2zad,_T("i2zad"),NULL,-2,2);

    wProps.Create( L"Parametry", 400, 550, &propsGroup );
    wProps.SetParent( &wChartW );

    wChartW.AddKeyProc( OnKeyChart, NULL );
    wChartW.Create( L"symulacja", 1280, 1000 );

    //dodawanie obszarów wykresów do okna
    wChartW.AddCharts( 8 );
    //wChartW.AddCharts( 4 );

    //definiowanie rejestrowanych i wyświetlanych sygnałów
#if 0
    wChartW.charts[0]->AddSignal( &gamma, L"gamma", RGB_( 240, 0, 220 ) );
    wChartW.charts[1]->AddSignal( &i1, L"i1", RGB_( 44, 134, 167 ) );
    wChartW.charts[2]->AddSignal( &i2, L"i2", RGB_( 0, 255, 255 ) );
    wChartW.charts[3]->AddSignal( &uzas, L"uzas", RGB_( 44, 134, 167 ) );
    wChartW.charts[4]->AddSignal( &u1, L"u1", RGB_( 44, 134, 167 ) );
    //TIME - czas symulacji dosowej _s ->/1000, bez => w sekundach
    //wChartW.charts[3]->AddSignal(&TIME_s,&x21,_T("opis"),RGB_(44,134,167));
#endif

    /*
     * dla multiskalarnego
     *
    wChartW.charts[7]->AddSignal( &x11, L"x11", RGB_( 240, 0, 220 ) );
    wChartW.charts[6]->AddSignal( &x11z, L"x11z", RGB_( 240, 0, 220 ) );
    wChartW.charts[5]->AddSignal( &x12, L"x12", RGB_( 44, 134, 167 ) );
    wChartW.charts[4]->AddSignal( &x12z, L"x12z", RGB_( 44, 134, 167 ) );
//    wChartW.charts[3]->AddSignal( &x21, L"x21", RGB_( 0, 255, 255 ) );
//    wChartW.charts[2]->AddSignal( &x21z, L"x21z", RGB_( 0, 255, 255 ) );
    //wChartW.charts[3]->AddSignal( &usx, L"usx", RGB_( 0, 255, 255 ) );
    //wChartW.charts[2]->AddSignal( &usy, L"usy", RGB_( 0, 255, 255 ) );
    wChartW.charts[3]->AddSignal( &isx, L"isx", RGB_( 0, 255, 255 ) );
    wChartW.charts[2]->AddSignal( &isy, L"isy", RGB_( 0, 255, 255 ) );
    //wChartW.charts[1]->AddSignal( &x22, L"x22", RGB_( 44, 134, 167 ) );
    //wChartW.charts[0]->AddSignal( &x22z, L"x22z", RGB_( 44, 134, 167 ) );
    wChartW.charts[1]->AddSignal( &rhoU, L"rhoU", RGB_( 0, 255, 255 ) );
    wChartW.charts[0]->AddSignal( &rhoU, L"rhoI", RGB_( 0, 255, 255 ) );
     /**/
    wChartW.charts[7]->AddSignal( &usx1, L"usx1", RGB_( 0, 255, 255 ) );
    wChartW.charts[6]->AddSignal( &usy1, L"usy1", RGB_( 0, 255, 255 ) );
    wChartW.charts[5]->AddSignal( &usx3, L"usx3", RGB_( 0, 255, 255 ) );
    wChartW.charts[4]->AddSignal( &usy3, L"usy3", RGB_( 0, 255, 255 ) );
    wChartW.charts[3]->AddSignal( &is1, L"is1", RGB_( 0, 255, 255 ) );
    wChartW.charts[2]->AddSignal( &is3, L"is3", RGB_( 0, 255, 255 ) );
    wChartW.charts[1]->AddSignal( &usx, L"usx", RGB_( 0, 255, 255 ) );
    wChartW.charts[0]->AddSignal( &usy, L"usy", RGB_( 0, 255, 255 ) );

    //definiowanie poczatkowego polozenia wykresow
    //polozenie na osi X ustawia sie dla wszystkich okien jednoczesnie
    //polozenia na osi Y okresla sie dla kazdego wykresu oddzielnie
    wChartW.SetXPos( 0 );
    /*
    wChartW.charts[5]->SetYPos( -2.2 );
    wChartW.charts[4]->SetYPos( -2.2 );
    wChartW.charts[3]->SetYPos( -1.25 );
    wChartW.charts[2]->SetYPos( -1.1 );
     */
    wChartW.charts[7]->SetYPos( -1.5 );
    wChartW.charts[6]->SetYPos( -1.5 );
    wChartW.charts[5]->SetYPos( -1.5 );
    wChartW.charts[4]->SetYPos( -1.5 );

    wChartW.charts[1]->SetYPos( -1.5 );
    wChartW.charts[0]->SetYPos( -1.5 );

    //definiowanie poczatkowej skali wykresow
    //skale na osi X ustawia sie dla wszystkich okien jednoczesnie, podajac szerokosc w jednostkach wykresu (w sekundach w tym przypadku)
    //skale na osi Y okresla sie dla kazdego wykresu oddzielnie
    wChartW.SetWidth( 1 );
    /*
    wChartW.charts[7]->SetHeight( 1.1 );
    wChartW.charts[6]->SetHeight( 1.1 );
    wChartW.charts[5]->SetHeight( 4.4 );
    wChartW.charts[4]->SetHeight( 4.4 );
    //wChartW.charts[3]->SetHeight( 1.1 );
    //wChartW.charts[2]->SetHeight( 1.1 );
    wChartW.charts[3]->SetHeight( 2.5 );
    wChartW.charts[2]->SetHeight( 2.2 );
    wChartW.charts[1]->SetHeight( 2 * M_PI );
    wChartW.charts[0]->SetHeight( ( 2 * M_PI ) / 6 );
     */
    wChartW.charts[7]->SetHeight( 3 );
    wChartW.charts[6]->SetHeight( 3 );
    wChartW.charts[5]->SetHeight( 3 );
    wChartW.charts[4]->SetHeight( 3 );
    wChartW.charts[3]->SetHeight( 1.5 );
    wChartW.charts[2]->SetHeight( 0.5 );
    wChartW.charts[1]->SetHeight( 3 );
    wChartW.charts[0]->SetHeight( 3 );

    //////////
    //definiowanie kolorow okna (tla,siatki,etykiet)
    wChartW.SetColors( RGBA( 0, 0, 0, 128 ), RGBA( 255, 255, 255, 100 ), RGB_( 255, 255, 0 ) );

    sim.SetStep( 1e-5, 5e-5, 2.5 ); //kroki oraz czas symulacji
    sim.StartSimulation( ); //rozpoczecie symulacji
}

void FinishSimulations( ) {
    sim.EndSimulation( );
}