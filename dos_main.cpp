#define IN_MAIN
#include "dos_main.h"
#include "dos2win.h"
//#include <process.h>
#include <math.h>
//#include <stdio.h>
//#include <io.h>
//#include <conio.h>
/*-------------------------------------------------------------------------*/
/* program CHOPPER.C - silnik obcowzbudny pradu stalego zasilany z        */
/* przerywacza przez filtr dolnoprzepustowy				   */
/* wielkosci w jednostkach wzglednych 			                   */
/* nieliniowy regulator ze sprzezeniami					   */
/*-------------------------------------------------------------------------*/

#include <math.h>
#include <stdio.h>
#include <io.h>

//#define p2 1.414213562  /* pierwiastek z 2 */
//#define p3 1.732050808  /* pierwiastek z 3 */
//#define p6 2.449489743  /* pierwiastek z 6 */

int Np = 1, Nk = 5;
double Lf = 0.01, Cf = 0.00055, Rf = 0.1, Rt = 0.7, Lt = 0.0117, Et = 0.01, Ld = 0.01;
/*---------------------------------------------*/
/* Parametry ukladu przerywacza                */
/*---------------------------------------------*/

double h, ht; /* czas wzgledny, krok calkowania */
double Timp, impuls;
double YY[32];
double zero; /*zero do wykresow*/
double DD[8][4];
float tab[7];

/*---------------------------------------------*/
/* LLL - co ile krokow calkowania grafika      */
/* L_signal - ile wykresow na ekranie (max. 6) */
/*---------------------------------------------*/
int L_signal = 6, LLL = 400; /* LLL-co ile krokow grafika */

/*------------------------------------------*/
/* deklaracje funkcji dolaczonych w PROJECT */
/*------------------------------------------*/

double limit( double wejscie, double ograniczenie );
double nzero( double x, double ogr );

double limit( double wejscie, double ograniczenie ) {
    double wyjscie;

    if ( wejscie >= 0 ) {
        if ( wejscie > ograniczenie ) wyjscie = ograniczenie;
        else wyjscie = wejscie;
    } else {
        if ( wejscie<-ograniczenie ) wyjscie = -ograniczenie;
        else wyjscie = wejscie;
    }
    return (wyjscie );
}


/*-------------------------------------------------------------------------*/
/*                   Uklad rownan rozniczkowych                            */

/*-------------------------------------------------------------------------*/
void F2DERY( double DV[32], double V[32] ) {

    kp1 = 100; /* nastawy regulatora PI */
    ki1 = 1;

    tau = V[1];
    izas = V[2];
    u1 = V[3];
    i2 = V[4];

    /* regulator PI pradu */
    Ei2 = i2zad - i2; /* uchyb pradu i2 */
    V[5] = limit( V[5], 1 );
    DV[5] = ki1*Ei2;
    gamma = limit( kp1 * Ei2 + V[5], 1 ); /* wsp. wypelnienia */



    if ( gamma < 0 ) gamma = 0; /* ograniczenie   0 =< gamma =< 1 */
    if ( gamma > 1 ) gamma = 1;

    u2 = gamma*u1; /* przerywacz idealny */
    if ( u2 < 0.0 ) u2 = 0.0; /* napiecie u2 >= 0 */
    i1 = gamma*i2;

    DV[1] = 1; /* Czas */
    /* uklad przerywacz z filtrem */
    DV[2] = ( uzas - Rf * izas - u1 ) / ( Lf / 100 );
    DV[3] = ( izas - gamma * i2 ) / Cf;
    DV[4] = ( gamma * u1 - Rt * i2 - Et ) / Lo;
}

/*-------------------------------------------------------------------------*/

/* n,N-pierwsze i ostatnie rownanie do calkowania,H-krok,Y-zmienne */

void F2( int naa, int NAA, double HAA, double YAA[32] ) {
    double DYAA[32], DXAA[32], XAA[32];
    int jaa;
    /* ----------------------------------------------------------------------- */
    F2DERY( DYAA, YAA );
    for( jaa = naa; jaa <= NAA; jaa++ ) XAA[jaa] = YAA[jaa] + HAA * DYAA[jaa];
    F2DERY( DXAA, XAA );
    for( jaa = naa; jaa <= NAA; jaa++ ) YAA[jaa] = YAA[jaa] + HAA * ( .4 * DYAA[jaa] + .6 * DXAA[jaa] );
}
//#include "PAR.C"

/*-------------------------------------------------------------------------*/
/*--------------P R O G R A M   G L O W N Y--------------------------------*/

/*-------------------------------------------------------------------------*/
int dos_main( ) {
    double Y[32]; /*zmienne calkowane */
    int i, J; /* liczniki petli */
    int LL = 0;
    //Lf=0.01,Cf=0.00055,Rf=0.1,Rt=0.7,Lt=0.0117,Et=0.01,Ld=0.01;
    /*-------------------------------------------------------------------------*/
    /*                    Wczytanie wartosci poczatkowych                      */
    /*-------------------------------------------------------------------------*/
    ht = 0.01; /* ht krok calkowania w milisekundach */
    h = ht / 1000; /* czas rzeczywisty */
    uzas = 0.8; /* napiecie zasilajace */
    /*fi=0.5;*/ /* i2 zadane dla reg. nieliniowego */
    a1 = 100; /* wsp. nieliniowego regulatora */
    a0 = 1;
    k = 0.8;
    i2zad = 0.55; /* i2 zadane dla reg. PI */

    /*-------------------------------------------------------------------------*/
    /*                  Obliczanie wspolczynnikow                              */
    /*-------------------------------------------------------------------------*/
    Lo = Ld + Lt;

    /*-------------------------------------------------------------------------*/
    /*         Obliczanie warunkow poczatkowych do rownan rozniczkowych        */
    /*-------------------------------------------------------------------------*/
    for( J = 0; J < ( Nk + 1 ); J++ ) Y[J] = 0;
    Y[2] = 0; /* izas */
    Y[3] = 1; /* u1 */
    Y[4] = 0; /* i2 */

    /*-------------------------------------------------------------------------*/
    /*                           Petla glowna                                  */
    /*-------------------------------------------------------------------------*/
    do {
        //for (J=0;J<(Nk+1);J++) YY[J]=Y[J];
        TIME += ht;

        if ( TIME > 400 ) i2zad = 1.0;
        if ( TIME > 800 ) i2zad = 0.1;
        if ( TIME > 1500 ) i2zad = 0.55;
        if ( TIME > 2000 ) i2zad = 0.1;
        if ( TIME > 2200 ) i2zad = 2;
        if ( TIME > 2400 ) i2zad = 0.1;



        /*-------------------------------------------------------------------------*/
        /*             Rozwiazanie ukladu rownan rozniczkowych                     */
        /*-------------------------------------------------------------------------*/
        F2( Np, Nk, h, Y );

        /*-------------------------------------------------------------------------*/
        /*                              Grafika                                    */
        /*-------------------------------------------------------------------------*/
        if ( LL == 0 ) {
            LL = LLL;
        }
        LL -= 1;
        main_mon( );
    } while( 1 );
}


