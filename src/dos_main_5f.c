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
double omegaU1 = 1, omegaU3 = 1.0 / 3, deltaRho;

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

int PWM_FAULT = 42, PWM_FAULT_INIT = 1000000;

#include "PWM5f.c"

int dos_main( ) {
/*
        if ( timeCnt > 10 ) {
            return 0;
        }
*/

    ht = 1E-4; /* krok calkowania w milisekundach */
    h = ht * TWZG; /* krok calkowania przeliczony na czas wzgledny */
    tImp = .150 * TWZG; // okres impulsowania falownika
    deltaRho = 2 * M_PI * ( 0.001 * tImp / TWZG ) / ( 20E-3 ); //przyrost kata na przerwanie przy predkosci 2PI/20ms, czas w sekundach)

    /* PWM fal 5 fazowy */
    //uDC = 0.5;
    uDC = 2.0;

    uS1 = 1.0;
    //US1+=2e-6; if (US1>uDC) US1=uDC;

    uS3 = 0 * 0.2;
    //US3=0.50;

    //roU1=0.306;
    ///roU3=5.364;
    //roU1=M_PI/2;
    //roU3=0*M_PI/2;

    //temp[30]=floor(roU3/(M_PI/5));

    int nextTimeTrigger = -1;
    int inputChar;

    FILE* fpSetup = fopen( "setup.txt", "r" );
    //FILE* fpLog = fopen( "log.txt", "w" );
    FILE* fpOut = fopen( "out.txt", "w" );

    if ( fpSetup != NULL ) {
        inputChar = fgetc( fpSetup );
        if ( isdigit( inputChar ) ) {
            PWM_FAULT_INIT = inputChar - '0';
            inputChar = '#'; // ignore the rest of the line
        }
        while( inputChar == '#' ) {
            inputChar = fgetc( fpSetup );
            while( inputChar != '\n' && inputChar != EOF ) {
                inputChar = fgetc( fpSetup );
            }
            inputChar = fgetc( fpSetup );
        }
        if ( inputChar == EOF ) {
            nextTimeTrigger = -1;
        } else {
            fscanf( fpSetup, "%d", &nextTimeTrigger );
            //fprintf( fpLog, "%d", nextTimeTrigger );
        }
    }

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

#define PWM_FAULT_TIME  100

        if ( timeCnt > PWM_FAULT_TIME && PWM_FAULT != PWM_FAULT_INIT ) {
            PWM_FAULT = PWM_FAULT_INIT;
            baseVecNr = 1;
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

        double is[5];
        double io = 0;
        is[0] = 1.1708204 * io - 0.19543951 * isx1 + 0.94868330 * isx3 - 0.60150096 * isy1 + 0.97324899 * isy3;
        is[1] = 0.44721360 * io + 0.19543951 * isx1 - 0.51166727 * isx3 + 0.60150096 * isy1 + 0.37174803 * isy3;
        is[2] = -0.27639320 * io + 0.31622777 * isx1 - 0.12078826 * isx3 + 0.97324899 * isy1 - 1.5747499 * isy3;
        is[3] = 1.6180340 * io - 1.8512296 * isx1 + 0.70710678 * isx3 - 1.3449970 * isy1 + 2.1762509 * isy3;
        is[4] = -0.72360680 * io + 1.5350018 * isx1 - 1.0233345 * isx3 + 0.37174803 * isy1 - 1.9464980 * isy3;

        float is1_old = is1;
        is1 = sqrt( isx1 * isx1 + isy1 * isy1 );
        is3 = sqrt( isx3 * isx3 + isy3 * isy3 );

        //float dI = is1_old - is1;

        if ( timeCnt > 10.0 && is1 < 0.1 && baseVecNr != 2 ) {
            int nMin = -1;
            int min = 42;
            for ( int i = 0; i < 5; i++ ) {
                is[i] = fabs( is[i] );
                if ( is[i] < min ) {
                    nMin = i;
                    min = is[i];
                }
            }
            printf( "wykryto fault w fazie %d @ t == %f\n", nMin + 1, timeCnt );
            printf( "is %f %f %f %f %f\n", is[0], is[1], is[2], is[3], is[4] );
            // przykladowa kompensacja
            baseVecNr = 2;
            //uS1 = 0.33;
        }

        if ( baseVecNr == 2 ) {
            if ( is3 > 0.5 ) {
                //uS1 *= 0.999999;
                uS1 *= 0.99999;
            }
        }

#if 0

#define BUF_LEN  1000
//#define TRESH  0.5
#define TRESH  10
        static int cnt = 0;
        static float iArr[BUF_LEN * 2] = {0};
        static float iSum = 0;

        iSum -= iArr[cnt];
        int offset = (cnt + BUF_LEN / 2 ) % BUF_LEN;

        iSum += iArr[offset];
        iArr[cnt] = is1;
        cnt++;
        cnt %= BUF_LEN;
        //printf( "is1 = %.5f iAvg = %.5f @ %.5f\n", is1, iAvg, timeCnt );
        //printf( "%f\n", abs( is1 - iAvg ) );
      if ( timeCnt > 2.0 ) { // gdy minie czas na rozruch/wypelnienie bufora
        float iAvg = iSum / (BUF_LEN /2);
        if ( fabs( is1 - iAvg ) / iAvg > TRESH ) {
            printf( "is1 = %.5f iAvg = %.5f delta = %.5f @ %.5f\n", is1, iAvg, fabs( is1 - iAvg ) / iAvg, timeCnt );
            float angle = atan2( isy1, isx1 );
            float angleDeg = angle / M_PI * 180;
            if ( angleDeg < 0 ) {
                angleDeg += 360;
            }
            int faza = (angleDeg - 36) / 72; // TODO lekko poprawic lapanie
            printf( "wykryto fault w fazie %d (%f)\n", faza, angleDeg );
            // przykladowa kompensacja
            baseVecNr = 2;            
        }
      }
#endif
        /* skokowe zmieny wielkosci zadanych i zaklocajacych */
        //if(TIME>500)  m0=1;
        //if(TIME>1000) m0=0.1;
        //if(TIME>900) omegaRzad=1;
        //if(TIME>1500) omegaU=.8;

        main_mon( );
    }
}
