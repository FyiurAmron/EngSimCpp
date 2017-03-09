#include <math.h>
#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>

#ifdef _WIN32
#include "dos2win.h"
#endif

#define   SQRT2      1.4142    // sqrt(2)
#define   SQRT3      1.7321    // sqrt(3)
#define   SQRT2DIV3  SQRT2 / SQRT3 // sqrt( 2/3 )
#define   SQRT6      2.4495    // sqrt(6)

/*------------------------------------------------------------------------*/
/* Parametry silnika o mocy 1.5 kW typ 2Sg90L4                            */
/*------------------------------------------------------------------------*/
double Rs = 0.0584, Rr = 0.0559, Lm = 2.171, Ls = 2.246, Lr = 2.246, JJ = 38.4 * 2.5;

//double ia, ib, ic, ua, ub, uc;
double i3ph[3], u3ph[3];

//double ht = 0.005, h; /* Krok calkowania ht w milisekundach */
double tImp; /* Okres impulsowania do generowania napiecia */
double timeCnt;
//bool sterowanie = true;

int PWM_FAULT_INIT = 42, PWM_FAULT = 42;

double ud = 1.52; //napiecie obwodu posredniczacego
//double /*uS, rhoU, phiU*/; // modul i predkosc wirowania wektroa napiecia do pwm
//double /*rhoI*/;
//double usx, usy; //napiecia zasilania silnika
double /*isx, isy,*/ frx1, fry1, frx3, fry3; //rownania modelu silnika
double omegaR;
double tau; //czas wzgledny
double a1, a2, a3, a4, a5, a6, w;
double m0;

// model multiskalarny
double x11, x12, x21, x22;
double inv_x21, usx_zad, usy_zad, u1, u2;
double v1, v2;

// regulator
double e11, e12, e21, e22;
double x11z, x12z, x21z, x22z;
double kp11, ki11, kp12, ki12, kp21, ki21, kp22, ki22;

double usx1, usy1, usx3, usy3;
double isx1, isy1, isx3, isy3;
double is1, is3, rhoU3;

double usx, usy;

//double timeCnt;

//#define   TWZG   0.1*M_PI  // przelicznik z czasu rzeczywistego na wzgledny
#define   TWZG   1  // przelicznik z czasu rzeczywistego na wzgledny

//deklaracje zmiennych do PWM
double rhoU1, uS1, uS3;
double uDC;
double omegaU1 = 1, omegaU3 = 0.5, deltaRho;

//wygenerowane skladowe w PWM
double usx1wyg, usy1wyg, usx3wyg, usy3wyg;

//obciozenie
double R3 = 1, L3 = 1; //R3=0.31622776601683793319988935444327*1.2360,L3=10*0.31622776601683793319988935444327*3.2360;
//double R1 = 1, L1 = 1; //R1=0.31622776601683793319988935444327*1.2360,L1=10*0.31622776601683793319988935444327*3.2360;//1

//zmienne dla ukladu sterowania
bool interrupt = true;

double h, ht; /* krok calkowania rzeczywisty i wzgledny */

double getRho( const double x, const double y ) {
    double a = x * x + y * y;
    if ( a == 0 ) {
        return 0;
    }

    a = sqrt( a );
    double arc = acos( x / a );
    return ( y / a >= 0 ) ? arc : 2 * M_PI - arc;
}

inline double limit( const double v, const double limit ) {
    if ( v > limit ) {
        return limit;
    } else if ( v < -limit ) {
        return -limit;
    }
    return v;
}

inline double wrapAngle( const double angleRad ) {
    double mod = fmod( angleRad, 2 * M_PI );
    return mod >= 0 ? mod : mod + 2 * M_PI;
}

//#define LIMIT  2.5
#define LIMIT  2.0

void TrapDery( double dx[4], double x[4] ) {
    //regulator x11
    //x11z = 1;
    e11 = x11z - x11;
    x[3] = limit( x[3], LIMIT );
    x12z = limit( x[3] + kp11 * e11, LIMIT );
    dx[3] = e11 * ki11;

    //regulator x12
    e12 = x12z - x12;
    x[2] = limit( x[2], LIMIT );
    v1 = limit( x[2] + kp12 * e12, LIMIT );
    dx[2] = e12 * ki12;

    //regulator x21
    //x21z = 1;
    e21 = x21z - x21;
    x[1] = limit( x[1], LIMIT );
    x22z = limit( x[1] + kp21 * e21, LIMIT );
    dx[1] = e21 * ki21;

    //regulator x22
    e22 = x22z - x22;
    x[0] = limit( x[0], LIMIT );
    v2 = limit( x[0] + kp22 * e22, LIMIT );
    dx[0] = e22 * ki22;
}

void Trapez( int varCount, double h, double z[32] ) {
    static double dz[32], dx[32], x[32];

    TrapDery( dz, z );
    for( int i = 0; i < varCount; i++ ) {
        x[i] = z[i] + h * dz[i];
    }
    TrapDery( dx, x );
    for( int i = 0; i < varCount; i++ ) {
        z[i] += 0.5 * h * ( dz[i] + dx[i] );
    }
}

//#define VAR_COUNT  5
#define VAR_COUNT  10

inline void pLR( double p_dLoad[VAR_COUNT], double pLoad[VAR_COUNT], double usx1m, double usy1m, double usx3m, double usy3m ) {
    double /*ptau,*/ isx1, isy1, isx3, isy3, frx1, fry1, frx3, fry3;

    //ptau = pLoad[0]; /* zmienne calkowane */
    isx1 = pLoad[1];
    isy1 = pLoad[2];
    isx3 = pLoad[3];
    isy3 = pLoad[4];
    frx1 = pLoad[5];
    fry1 = pLoad[6];
    frx3 = pLoad[7];
    fry3 = pLoad[8];
    omegaR = pLoad[9];

    p_dLoad[0] = 1; /* Czas */
    /* rownania modelu silnika */
    /*
    p_dLoad[1] = ( usx1m - pisx1 * R1 ) / L1;
    p_dLoad[2] = ( usy1m - pisy1 * R1 ) / L1;
    p_dLoad[3] = ( usx3m - pisx3 * R3 ) / L3;
    p_dLoad[4] = ( usy3m - pisy3 * R3 ) / L3;
     */
    p_dLoad[1] = a1 * isx1 + a2 * frx1 + omegaR * a3 * fry1 + a4 * usx1;
    p_dLoad[2] = a1 * isy1 + a2 * fry1 - omegaR * a3 * frx1 + a4 * usy1;
    p_dLoad[3] = a1 * isx3 + a2 * frx3 + omegaR * a3 * fry1 + a4 * usx1;
    p_dLoad[4] = a1 * isy3 + a2 * fry3 - omegaR * a3 * frx1 + a4 * usy1;
    p_dLoad[5] = a5 * frx1 + a6 * isx1 - omegaR * fry1;
    p_dLoad[6] = a5 * fry1 + a6 * isy1 + omegaR * frx1;
    p_dLoad[7] = a5 * frx3 + a6 * isx3 - omegaR * fry1;
    p_dLoad[8] = a5 * fry3 + a6 * isy3 + omegaR * frx1;
    p_dLoad[9] = ( ( frx1 * isy1 - fry1 * isx1 ) * Lm / Lr - m0 ) / JJ;
}

inline void LR( double h, double usx1m, double usy1m, double usx3m, double usy3m,
        double *isx1m, double *isy1m, double *isx3m, double *isy3m,
        double *frx1m, double *fry1m, double *frx3m, double *fry3m ) {
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
    *frx1m = load[5];
    *fry1m = load[6];
    *frx3m = load[7];
    *fry3m = load[8];
    omegaR = load[9];
}

//////////

// warunki początkowe
double x[32] = { 0 };
double y[32] = { 0 };

#define PWM_FAULT_TIME  500

#include "PWM5f.c"

double usx1_zad, usy1_zad;

int dos_main( ) {
    //wartosci zadane
    x11z = 1;
    x21z = 1;
    m0 = .1;

    /*-------------------------------------------------*/
    /* Obliczanie wspolczynnikow rownan modelu silnika */
    /*-------------------------------------------------*/
    w = Ls * Lr - Lm * Lm;
    a1 = -( Rs * Lr * Lr + Rr * Lm * Lm ) / ( Lr * w );
    a2 = Rr * Lm / ( Lr * w );
    a3 = Lm / w;
    a4 = Lr / w;
    a5 = -Rr / Lr;
    a6 = Rr * Lm / Lr;
    //a7=w/Lr;

    /*
    h = 0.1 * M_PI * ht; // ht w milisekundach
    tImp = 0.1 * M_PI * 0.15;
    uS = 0.91;
     */

    ht = 1E-4; /* krok calkowania w milisekundach */
    h = ht * TWZG; /* krok calkowania przeliczony na czas wzgledny */
    tImp = .150 * TWZG; // okres impulsowania falownika
    deltaRho = 2 * M_PI * ( 0.001 * tImp / TWZG ) / ( 20E-3 ); //przyrost kata na przerwanie przy predkosci 2PI/20ms, czas w sekundach)

    uDC = 2;
    uS1 = 1.0;
    uS3 = 0 * 0.2;

    //wkfi=.1; wkf1=.01; wkf2=.01; prog=.1; progD=.05;
    //kfi=10; kf1=1; kf2=10; kD=.8;  /*Nastawy dla Timp=.2ms */
    //wkfi=.1; wkf1=.01; wkf2=.01; prog=.12; progD=.05;
    //kfi=8; kf1=1; kf2=6; kD=.8;  /*Nastawy dla Timp=.2ms */

    //kp11=20; ki11=1; kp21=20; ki21=1;
    //kp11=20; ki11=.3; kp21=20; ki21=.1;
    //kp12=10; ki12=2;  kp22=10; ki22=2;

    //kp11=20,ki11=2,kp21=10,ki21=1;          // do ideal
    //kp11=5,ki11=.05,kp21=.31,ki21=.2;          // do ideal
    //kp12=10,ki12=2,kp22=10,ki22=2;
    //kp12=5,ki12=2,kp22=5,ki22=2;
    kp11 = 10;
    ki11 = .01;
    kp21 = 20;
    ki21 = .1;
    kp22 = 20;
    ki22 = 2;
    kp12 = 20;
    ki12 = 2;

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
    /* format:
     # przykladowe dane w formacie np.
     # time x11z m0
     @ 300 0.5 0.9
     @ 800 0.5 0.1
     @ 1500 1.5 0.9
     */

    bool detected = false;

    while( true ) {
        timeCnt += ht;

        if ( nextTimeTrigger != -1 && timeCnt > nextTimeTrigger ) {
            fscanf( fpSetup, "%lf %lf", &x11z, &m0 );

            inputChar = fgetc( fpSetup );
            while( inputChar == '#' ) {
                inputChar = fgetc( fpSetup );
                while( inputChar != '\n' && inputChar != EOF ) {
                    inputChar = fgetc( fpSetup );
                }
            }
            if ( inputChar == EOF ) {
                nextTimeTrigger = -1;
            } else {
                fscanf( fpSetup, "%d", &nextTimeTrigger );
                //fprintf( fpLog, "%d", nextTimeTrigger );
            }
        }

        if ( interrupt ) {
            rhoU1 += omegaU1 * deltaRho;
            rhoU1 = wrapAngle( rhoU1 );

            rhoU3 += omegaU3 * -deltaRho;
            rhoU3 = wrapAngle( rhoU3 );

            // zmienne multiskalarne
            x11 = omegaR;
            x12 = frx1 * isy1 - fry1 * isx1;
            x21 = frx1 * frx1 + fry1 * fry1;
            x22 = frx1 * isx1 + fry1 * isy1;
            fprintf( fpOut, "%f %f %f %f\n", x11, x12, x21, x22 );

            if ( timeCnt > 10 ) {
                Trapez( 4, tImp, x );

                // odsprzezenie
                u1 = ( -v1 * ( a1 + a5 ) + x11 * ( x22 + a3 * x21 ) ) / a4;
                u2 = ( -v2 * ( a1 + a5 ) - x11 * x12 - a2 * x21 - a6 * ( isx1 * isx1 + isy1 * isy1 ) ) / a4;
                inv_x21 = 1.0 / x21;
                if ( inv_x21 < 0.001 ) {
                    inv_x21 = 0.001;
                }
                usx1_zad = ( u2 * frx1 - u1 * fry1 ) * inv_x21;
                usy1_zad = ( u2 * fry1 + u1 * frx1 ) * inv_x21;
                usx1_zad = limit( usx1_zad, 1.5 );
                usy1_zad = limit( usy1_zad, 1.5 );
                fprintf( fpOut, "%f %f\n", usx1_zad, usy1_zad );

                uS1 = sqrt( usx1_zad * usx1_zad + usy1_zad * usy1_zad );
                rhoU1 = getRho( usx1_zad, usy1_zad );
            }
            interrupt = false;
        }

        if ( timeCnt > PWM_FAULT_TIME ) {
            PWM_FAULT = PWM_FAULT_INIT;
        }

        PWM5f( tImp, uS1, rhoU1, uS3, rhoU3, uDC, h, &usx1, &usy1, &usx3, &usy3, &usx1wyg, &usy1wyg, &usx3wyg, &usy3wyg, &interrupt );
        LR( h, usx1, usy1, usx3, usy3,
                &isx1, &isy1, &isx3, &isy3,
                &frx1, &fry1, &frx3, &fry3 );

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

        //if ( !detected && x21 > 2.0 * x21z ) {
#if 0
        rhoI = getRho( isx, isy );
#define FAULT_FACTOR  2.0
        if ( !detected ) {
            if ( x12 > FAULT_FACTOR * LIMIT || x21 > FAULT_FACTOR * LIMIT || x22 > FAULT_FACTOR * LIMIT ) {
                fprintf( fpOut, "awaria wykryta w momencie %f\n", timeCnt );
                fclose( fpOut );
                detected = true;
            }
        }
#endif

        //transform_XY_to_3ph( i3ph, isx, isy );
        //transform_XY_to_3ph( u3ph, usx, usy );
        //fprintf( fpOut, "." );
        main_mon( );
    } // while(true);
}