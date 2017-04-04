#include <math.h>
#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>

#ifdef _WIN32
#include "dos2win.h"
#endif

double usx1, usy1, usx3, usy3;
double isx1, isy1, isx3, isy3;
double is1, is3, rhoU3;

double usx, usy;

double timeCnt;

/*------------------------------------------------------------------------*/
/* program NAPIECIE - silnik asynchroniczny zasilany z falownika napiecia */
/* modulacja PWM napieciowy, uklad otwarty zadawanie U i f stojana        */
/* Wielkosci w ukladzie prostokatnym x-y nieruchomym wzgledem stojana     */
/* Wprowadzona charakterystyka U/f=const                                  */
/*------------------------------------------------------------------------*/

//#define   TWZG   0.1*M_PI  // przelicznik z czasu rzeczywistego na wzgledny
#define   TWZG   1  // przelicznik z czasu rzeczywistego na wzgledny

//deklaracje zmiennych do PWM
double rhoU1, uS1, uS3;
double uDC;
double omegaU1 = 1, omegaU3 = 1.0/3, deltaRho;

//wygenerowane skladowe w PWM
double usx1wyg, usy1wyg, usx3wyg, usy3wyg;

//obciozenie
double R3 = 1, L3 = 1; //R3=0.31622776601683793319988935444327*1.2360,L3=10*0.31622776601683793319988935444327*3.2360;
double R1 = 1, L1 = 1; //R1=0.31622776601683793319988935444327*1.2360,L1=10*0.31622776601683793319988935444327*3.2360;//1

//zmienne dla ukladu sterowania
bool interrupt;

double tau; /* czas wzgledny */
double h, ht; /* krok calkowania rzeczywisty i wzgledny */

double tImp; /* okres impulsowania */

inline double wrapAngle( const double angleRad ) {
    double mod = fmod( angleRad, 2 * M_PI );
    return mod >= 0 ? mod : mod + 2 * M_PI;
}

#define VAR_COUNT  5

inline void pLR( double p_dLoad[VAR_COUNT], double pLoad[VAR_COUNT], double usx1m, double usy1m, double usx3m, double usy3m ) {
    double /*ptau,*/ pisx1, pisy1, pisx3, pisy3;

    //ptau = pLoad[0]; /* zmienne calkowane */
    pisx1 = pLoad[1];
    pisy1 = pLoad[2];
    pisx3 = pLoad[3];
    pisy3 = pLoad[4];

    p_dLoad[0] = 1; /* Czas */
    /* rownania modelu silnika */
    p_dLoad[1] = ( usx1m - pisx1 * R1 ) / L1;
    p_dLoad[2] = ( usy1m - pisy1 * R1 ) / L1;
    p_dLoad[3] = ( usx3m - pisx3 * R3 ) / L3;
    p_dLoad[4] = ( usy3m - pisy3 * R3 ) / L3;
}

inline void LR( double h, double usx1m, double usy1m, double usx3m, double usy3m, double *isx1m, double *isy1m, double *isx3m, double *isy3m ) {
    static double load[VAR_COUNT]; /* zmienne calkowane modelu silnika */
    static double dLoad[VAR_COUNT];
    static double q[VAR_COUNT][VAR_COUNT];

    pLR( dLoad, load, usx1m, usy1m, usx3m, usy3m );
    for( int i = 0; i < VAR_COUNT; i++ ) {
        q[0][i] = load[i];
        q[1][i] = h * dLoad[i];
        load[i] = q[0][i] + 0.5 * q[1][i];
    }


    pLR( dLoad, load, usx1m, usy1m, usx3m, usy3m );
    for( int i = 0; i < VAR_COUNT; i++ ) {
        q[2][i] = h * dLoad[i];
        load[i] = q[0][i] + 0.5 * q[2][i];
    }


    pLR( dLoad, load, usx1m, usy1m, usx3m, usy3m );
    for( int i = 0; i < VAR_COUNT; i++ ) {
        q[3][i] = h * dLoad[i];
        load[i] = q[0][i] + q[3][i];
    }


    pLR( dLoad, load, usx1m, usy1m, usx3m, usy3m );
    for( int i = 0; i < VAR_COUNT; i++ ) {
        q[4][i] = h * dLoad[i];
        load[i] = 1.0 / 6.0 * ( q[1][i] + q[4][i] ) + 1.0 / 3.0 * ( q[2][i] + q[3][i] ) + q[0][i];

    }
    tau = load[0];
    *isx1m = load[1];
    *isy1m = load[2];
    *isx3m = load[3];
    *isy3m = load[4];
}

int PWM_FAULT_INIT = 1, PWM_FAULT = 42;

#include "PWM5f.c"

int dos_main( ) {
    ht = 1E-4; /* krok calkowania w milisekundach */
    h = ht * TWZG; /* krok calkowania przeliczony na czas wzgledny */
    tImp = .150 * TWZG; // okres impulsowania falownika
    deltaRho = 2 * M_PI * ( 0.001 * tImp / TWZG ) / ( 20E-3 ); //przyrost kata na przerwanie przy predkosci 2PI/20ms, czas w sekundach)

    /* PWM fal 5 fazowy */
    uDC = 0.5;

    uS1 = 1.0;
    //US1+=2e-6; if (US1>uDC) US1=uDC;

    uS3 = 0 * 0.2;
    //US3=0.50;

    //roU1=0.306;
    ///roU3=5.364;
    //roU1=M_PI/2;
    //roU3=0*M_PI/2;

    //temp[30]=floor(roU3/(M_PI/5));

    while( true ) {
        timeCnt += ht; // uplyw czasu rzeczywistego

        if ( interrupt ) {

            /* kat polozenia wektora napiecia stojana */
            rhoU1 += omegaU1 * deltaRho;
            rhoU1 = wrapAngle( rhoU1 );

            rhoU3 -= omegaU3 * deltaRho;
            rhoU3 = wrapAngle( rhoU3 );

            //regulator napiec na kondensatorach filtru
            //			regulator_uC(omegaU_siec, uCd_zad, uCq_zad, uC0_zad, uCd, uCq, u0C, id_siec, iq_siec, i0_siec, &id_fal_zad, &iq_fal_zad, &i0_fal_zad);
            //uCd=Usiec; uCq=0; iq_fal_zad=0;

            //regulator pradu falownika sieciowego

            interrupt = false;
        }

#define PWM_FAULT_TIME  500

        if ( timeCnt > PWM_FAULT_TIME ) {
            PWM_FAULT = PWM_FAULT_INIT;
        }

        PWM5f( tImp, uS1, rhoU1, uS3, rhoU3, uDC, h, &usx1, &usy1, &usx3, &usy3, &usx1wyg, &usy1wyg, &usx3wyg, &usy3wyg, &interrupt );
        LR( h, usx1, usy1, usx3, usy3, &isx1, &isy1, &isx3, &isy3 );

        //obciazenie&usx1
        //ponizsze 4 linijki
        //zakomentowane -  napiecie wyj�ciowe jest szeregiem impulsow
        //aktywne			-	napiecie wyjsciowe wyliczane jest ze wzoru t1*u1+t2*u2+.... (t to czasy, u -skladowe wektorow
        /*
        usx1 = usx1wyg;
        usy1 = usy1wyg;
        usx3 = usx3wyg;
        usy3 = usy3wyg;
         */
        usx = usx1 + usx3;
        usy = usy1 + usy3;

        is1 = sqrt( isx1 * isx1 + isy1 * isy1 );
        is3 = sqrt( isx3 * isx3 + isy3 * isy3 );

        /* skokowe zmieny wielkosci zadanych i zaklocajacych */
        //if(TIME>500)  m0=1;
        //if(TIME>1000) m0=0.1;
        //if(TIME>900) omegaRzad=1;
        //if(TIME>1500) omegaU=.8;

        main_mon( );
    }
}
