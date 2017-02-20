#include <math.h>
#include <stdio.h>
#include <stdbool.h>
#ifdef _WIN32
#include "dos2win.h"
#endif

#define   SQRT2      1.4142    // sqrt(2)
#define   SQRT3      1.7321    // sqrt(3)
#define   SQRT2DIV3  SQRT2 / SQRT3 // sqrt( 2/3 )
#define   SQRT6      2.4495    // sqrt(6)

/* Parametry maszyny */
/*------------------------------------------------------------------------*/
/* Parametry silnika o mocy 1.5 kW typ 2Sg90L4                            */
/*------------------------------------------------------------------------*/
double Rs = 0.0584, Rr = 0.0559, Lm = 2.171, Ls = 2.246, Lr = 2.246, JJ = 38.4 * 2.5;

//double ia, ib, ic, ua, ub, uc;
double i3ph[3], u3ph[3];

double ht = 0.005, h; /*Krok calkowania ht w milisekundach */
double Timp; /*Okres impulsowania do generowania napiecia*/
double timeCnt;
bool sterowanie = true;
//double impuls;

int Np = 0, Nk = 7; //calkowanie regulatorow
int M = 8; /* liczba calkowanych zmiennych - model silnika */

double ud = 1.52; //napiecie obwodu posredniczacego
double US, rhoU; // modul i predkosc wirowania wektroa napiecia do pwm
double usx, usy; //napiecia zasilania silnika
double isx, isy, frx, fry; //rownania modelu silnika
double omegaR;
double tau; //czas wzgledny
double a1, a2, a3, a4, a5, a6, w;
double m0;

//model multiskalarny
double x11, x12, x21, x22;
double odwr_x21, usx_zad, usy_zad, u1, u2;
double v1, v2;

//regulator
double e11, e12, e21, e22;
double x11z, x12z, x21z, x22z;
double kp11, ki11, kp12, ki12, kp21, ki21, kp22, ki22;

double getRho( const double x, const double y ) {
    double a = x * x + y * y;
    if ( a == 0 ) {
        return 0;
    }

    a = sqrt( a );
    double arc = acos( x / a );
    return ( y / a >= 0 ) ? arc : 2 * M_PI - arc;
}

// alpha-beta transformation (Clark)

inline void transform_XY_to_3ph( double output[3], const double alpha, const double beta ) {
    output[0] = SQRT2DIV3 * alpha;
    output[1] = -alpha / SQRT6 + beta / SQRT2;
    output[2] = -alpha / SQRT6 - beta / SQRT2;
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

void F4Dery( double DV[32], double V[32] ) {
    tau = V[1]; /* zmienne calkowane */
    isx = V[2];
    isy = V[3];
    frx = V[4];
    fry = V[5];
    omegaR = V[6];

    DV[1] = 1; /* Czas */
    /* rownania modelu silnika */
    DV[2] = a1 * isx + a2 * frx + omegaR * a3 * fry + a4*usx;
    DV[3] = a1 * isy + a2 * fry - omegaR * a3 * frx + a4*usy;
    DV[4] = a5 * frx + a6 * isx - omegaR*fry;
    DV[5] = a5 * fry + a6 * isy + omegaR*frx;
    DV[6] = ( ( frx * isy - fry * isx ) * Lm / Lr - m0 ) / JJ;
}

void F4( int MM, double HF, double Z[32] ) {
    static double DZ[32];
    static double Q[5][32];
    int I;
    F4Dery( DZ, Z );
    for( I = 0; I < MM; I += 1 ) {
        Q[0][I] = Z[I];
        Q[1][I] = HF * DZ[I];
        Z[I] = Q[0][I] + 0.5 * Q[1][I];
    }
    F4Dery( DZ, Z );
    for( I = 0; I < MM; I += 1 ) {
        Q[2][I] = HF * DZ[I];
        Z[I] = Q[0][I] + 0.5 * Q[2][I];
    }
    F4Dery( DZ, Z );
    for( I = 0; I < MM; I += 1 ) {
        Q[3][I] = HF * DZ[I];
        Z[I] = Q[0][I] + Q[3][I];
    }
    F4Dery( DZ, Z );
    for( I = 0; I < MM; I += 1 ) {
        Q[4][I] = HF * DZ[I];
        Z[I] = ( Q[1][I] + Q[4][I] ) / 6 + ( Q[2][I] + Q[3][I] ) / 3 + Q[0][I];
    }
}

void TrapDery( double DX[10], double X[10] ) {
    //regulator x11
    //x11z = 1;
    e11 = x11z - x11;
    X[3] = limit( X[3], 2.0 );
    x12z = limit( X[3] + kp11*e11, 2.0 );
    DX[3] = e11*ki11;

    //regulator x12
    e12 = x12z - x12;
    X[2] = limit( X[2], 2.0 );
    v1 = limit( X[2] + kp12*e12, 2.0 );
    DX[2] = e12*ki12;

    //regulator x21
    //x21z = 1;
    e21 = x21z - x21;
    X[1] = limit( X[1], 2.0 );
    x22z = limit( X[1] + kp21*e21, 2.0 );
    DX[1] = e21*ki21;

    //regulator x22
    e22 = x22z - x22;
    X[0] = limit( X[0], 2.0 );
    v2 = limit( X[0] + kp22*e22, 2.0 );
    DX[0] = e22*ki22;
}

/* n,N-pierwsze i ostatnie rownanie do calkowania,H-krok,Y-zmienne */

void Trapez( int na, int NA, double HA, double ZA[32] ) {
    static double DZA[32], DXA[32], XA[32];
    int ja;

    TrapDery( DZA, ZA );
    for( ja = na; ja <= NA; ja++ ) {
        XA[ja] = ZA[ja] + HA * DZA[ja];
    }
    TrapDery( DXA, XA );
    for( ja = na; ja <= NA; ja++ ) {
        ZA[ja] = ZA[ja] + .5 * ( HA * DZA[ja] + HA * DXA[ja] );
    }
}

// 000, 001, 011, 010, 110, 100, 101, 111
const double ux[] = {
    0,
    SQRT2DIV3,
    SQRT2DIV3 / 2,
    -SQRT2DIV3 / 2,
    -SQRT2DIV3,
    -SQRT2DIV3 / 2,
    SQRT2DIV3 / 2,
    0
}, uy[] = {
    0,
    0,
    SQRT2 / 2,
    SQRT2 / 2,
    0,
    -SQRT2 / 2,
    -SQRT2 / 2,
    0
};

// 000, 001, 010, 011, 100, 101, 110, 111
const double ux_out[] = {
    0,
    SQRT2DIV3,
    -SQRT2DIV3 / 2,
    SQRT2DIV3 / 2,
    -SQRT2DIV3 / 2,
    SQRT2DIV3 / 2,
    -SQRT2DIV3,
    0
}, uy_out[] = {
    0,
    0,
    SQRT2 / 2,
    SQRT2 / 2,
    -SQRT2 / 2,
    -SQRT2 / 2,
    0,
    0
};

void PWM( double tImpMax, double uS, double rhoU, double ud, double h ) {
    /*-------------------------------------------------------------------------*/
    /*                Wyznaczenie czasow zalaczen wektorow                     */
    /*-------------------------------------------------------------------------*/
    static double tImp;

    static int rhoUN;
    static bool cykl = true;
    static int stan_pocz, stan_konc, stan_konc_a, stan_konc_b, stan_konc_c;
    static double t1, t2, t0;
    static double tA, tB, tC;

    double td = 0; // UWAGA: nie ma symulacji td
    double sgn_ia_komp = 0, sgn_ib_komp = 0, sgn_ic_komp = 0;
    double sgn_ia = 0, sgn_ib = 0, sgn_ic = 0;

    tImp += h;

    if ( tImp > tImpMax ) { // co okres impulsowania
        rhoU = wrapAngle( rhoU );
        rhoUN = floor( rhoU / ( ( 2 * M_PI ) / 6 ) ); // ktory sektor?
        int idu1 = rhoUN + 1;
        int idu2 = idu1 % 6 + 1;

        double uSX = uS * cos( rhoU );
        double uSY = uS * sin( rhoU ); // to opoznienie symuluje czas na obl sterowania
        double wt = ud * ( ux[idu1] * uy[idu2] - uy[idu1] * ux[idu2] );
        double k = tImpMax / ( ud * wt );
        t1 = k * ( uSX * uy[idu2] - uSY * ux[idu2] );
        t2 = -k * ( uSX * uy[idu1] - uSY * ux[idu1] );

        t0 = ( tImpMax - t1 - t2 ) / 2;
        cykl = !cykl;
        sterowanie = true; // uruchomia sterowanie
    }

    if ( cykl ) {
        stan_pocz = 0;

        switch( rhoUN ) {
            case 0:
                tA = t0;
                tB = t0 + t1;
                tC = t0 + t1 + t2;
                break;

            case 1:
                tA = t0 + t2;
                tB = t0;
                tC = t0 + t1 + t2;
                break;

            case 2:
                tA = t0 + t1 + t2;
                tB = t0;
                tC = t0 + t1;
                break;

            case 3:
                tA = t0 + t1 + t2;
                tB = t0 + t2;
                tC = t0;
                break;

            case 4:
                tA = t0 + t1;
                tB = t0 + t1 + t2;
                tC = t0;
                break;

            case 5:
                tA = t0;
                tB = t0 + t1 + t2;
                tC = t0 + t2;
                break;
        }

        if ( sgn_ia_komp > 0 ) {
            tA -= td;
        }
        if ( sgn_ib_komp > 0 ) {
            tB -= td;
        }
        if ( sgn_ic_komp > 0 ) {
            tC -= td;
        }
    } else {
        stan_pocz = 1;

        switch( rhoUN ) {
            case 0:
                tA = t0 + t1 + t2;
                tB = t0 + t2;
                tC = t0;
                break;

            case 1:
                tA = t0 + t1;
                tB = t0 + t1 + t2;
                tC = t0;
                break;

            case 2:
                tA = t0;
                tB = t0 + t1 + t2;
                tC = t0 + t2;
                break;

            case 3:
                tA = t0;
                tB = t0 + t1;
                tC = t0 + t1 + t2;
                break;

            case 4:
                tA = t0 + t2;
                tB = t0;
                tC = t0 + t1 + t2;
                break;

            case 5:
                tA = t0 + t1 + t2;
                tB = t0;
                tC = t0 + t1;
                break;
        }

        if ( sgn_ia_komp < 0 ) {
            tA -= td;
        }
        if ( sgn_ib_komp < 0 ) {
            tB -= td;
        }
        if ( sgn_ic_komp < 0 ) {
            tC -= td;
        }
    }

    //	PWM_ALTERA();
    if ( tImp <= tA ) {
        stan_konc_a = stan_pocz;
    } else if ( tImp <= tA + td ) {
        stan_konc_a = ( sgn_ia > 0 ) ? 0 : 1;
    } else if ( tImp <= tImpMax ) {
        stan_konc_a = !stan_pocz;
    }

    if ( tImp <= tB ) {
        stan_konc_b = stan_pocz;
    } else if ( tImp <= tB + td ) {
        stan_konc_b = ( sgn_ib > 0 ) ? 0 : 1;
    } else if ( tImp <= tImpMax ) {
        stan_konc_b = !stan_pocz;
    }

    if ( tImp <= tC ) {
        stan_konc_c = stan_pocz;
    } else if ( tImp <= tC + td ) {
        stan_konc_c = ( sgn_ic > 0 ) ? 0 : 1;
    } else if ( tImp <= tImpMax ) {
        stan_konc_c = !stan_pocz;
    }

    stan_konc = stan_konc_c * 4 + stan_konc_b * 2 + stan_konc_a;

    usx = ux_out[stan_konc] * ud;
    usy = uy_out[stan_konc] * ud;

    if ( tImp > tImpMax ) {
        tImp = 0; //czyli co okres impulsowania
    }
}

int dos_main( ) {
    double x[32]; /*zmienne calkowane */
    double y[32]; /*zmienne calkowane */
    //int LL=0;
    int i;
    /*------------------------------------*/
    /*  Obliczanie warunkow poczatkowych  */
    /*------------------------------------*/
    for( i = 0; i < M; i++ ) {
        x[i] = 0; // dla zmiennych ukladu sterowania - zerowe
    }
    for( i = 0; i < M; i++ ) {
        y[i] = 0; // zerowe warunki poczatkowe
    }

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

    h = 0.1 * M_PI * ht; /* ht w milisekundach */
    //tmin=.006*.1*M_PI;
    Timp = .15 * 0.1 * M_PI;
    US = .91;

    //LL = 0;
    //#include "warpocz.c"
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

    if ( fpSetup != NULL ) {
        inputChar = fgetc( fpSetup );
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

        if ( sterowanie ) {
            rhoU += US * Timp;
            rhoU = wrapAngle( rhoU );

            //zmienne multiskalarne
            x11 = omegaR;
            x12 = frx * isy - fry * isx;
            x21 = frx * frx + fry * fry;
            x22 = frx * isx + fry * isy;

            if ( timeCnt > 10 ) {
                Trapez( Np, Nk, Timp, x );
                //REGULATORY();

                //odsprzezenie
                u1 = ( -v1 * ( a1 + a5 ) + x11 * ( x22 + a3 * x21 ) ) / a4;
                u2 = ( -v2 * ( a1 + a5 ) - x11 * x12 - a2 * x21 - a6 * ( isx * isx + isy * isy ) ) / a4;
                odwr_x21 = x21;
                if ( odwr_x21 < 0.001 ) {
                    odwr_x21 = 0.001;
                }
                usx_zad = ( u2 * frx - u1 * fry ) / odwr_x21;
                usy_zad = ( u2 * fry + u1 * frx ) / odwr_x21;
                usx_zad = limit( usx_zad, 1.5 );
                usy_zad = limit( usy_zad, 1.5 );

                US = sqrt( usx_zad * usx_zad + usy_zad * usy_zad );
                rhoU = getRho( usx_zad, usy_zad );
            }
            sterowanie = false;
        }

        ///PWM
        //if (TIME<350)
        PWM( Timp, US, rhoU, ud, h );
        //if (TIME>350)
        //{usx=usx_zad; usy=usy_zad;}

        //usx=US*cos(roU); usy=US*sin(roU);
        //	PWMU_2WR(US,roU,ud,Timp,h);
        F4( M, h, y ); // SILNIK rownania rozniczkowe - rozwiazanie metoda RK4

        transform_XY_to_3ph( i3ph, isx, isy );
        transform_XY_to_3ph( u3ph, usx, usy );

        main_mon( );
    } // while(1))
}