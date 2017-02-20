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

/*------------------------------------------------------------------------*/
/* Parametry silnika o mocy 1.5 kW typ 2Sg90L4                            */
/*------------------------------------------------------------------------*/
double Rs = 0.0584, Rr = 0.0559, Lm = 2.171, Ls = 2.246, Lr = 2.246, JJ = 38.4 * 2.5;

//double ia, ib, ic, ua, ub, uc;
double i3ph[3], u3ph[3];

double ht = 0.005, h; /* Krok calkowania ht w milisekundach */
double tImp; /* Okres impulsowania do generowania napiecia */
double timeCnt;
bool sterowanie = true;

int integrationArgCount = 8; /* liczba calkowanych zmiennych - model silnika */

double ud = 1.52; //napiecie obwodu posredniczacego
double uS, rhoU; // modul i predkosc wirowania wektroa napiecia do pwm
double usx, usy; //napiecia zasilania silnika
double isx, isy, frx, fry; //rownania modelu silnika
double omegaR;
double tau; //czas wzgledny
double a1, a2, a3, a4, a5, a6, w;
double m0;

// model multiskalarny
double x11, x12, x21, x22;
double odwr_x21, usx_zad, usy_zad, u1, u2;
double v1, v2;

// regulator
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

void F4Dery( double dv[32], double v[32] ) {
    tau = v[1]; /* zmienne calkowane */
    isx = v[2];
    isy = v[3];
    frx = v[4];
    fry = v[5];
    omegaR = v[6];

    dv[1] = 1; /* Czas */
    /* rownania modelu silnika */
    dv[2] = a1 * isx + a2 * frx + omegaR * a3 * fry + a4 * usx;
    dv[3] = a1 * isy + a2 * fry - omegaR * a3 * frx + a4 * usy;
    dv[4] = a5 * frx + a6 * isx - omegaR * fry;
    dv[5] = a5 * fry + a6 * isy + omegaR * frx;
    dv[6] = ( ( frx * isy - fry * isx ) * Lm / Lr - m0 ) / JJ;
}

void F4( int varCount, double h, double z[32] ) {
    static double dz[32];
    static double q[5][32];

    F4Dery( dz, z );
    for( int i = 0; i < varCount; i++ ) {
        q[0][i] = z[i];
        q[1][i] = h * dz[i];
        z[i] = q[0][i] + 0.5 * q[1][i];
    }
    F4Dery( dz, z );
    for( int i = 0; i < varCount; i++ ) {
        q[2][i] = h * dz[i];
        z[i] = q[0][i] + 0.5 * q[2][i];
    }
    F4Dery( dz, z );
    for( int i = 0; i < varCount; i++ ) {
        q[3][i] = h * dz[i];
        z[i] = q[0][i] + q[3][i];
    }
    F4Dery( dz, z );
    for( int i = 0; i < varCount; i++ ) {
        q[4][i] = h * dz[i];
        z[i] = 1.0 / 6 * ( q[1][i] + q[4][i] ) + 1.0 / 3 * ( q[2][i] + q[3][i] ) + q[0][i];
    }
}

void TrapDery( double dx[4], double x[4] ) {
    //regulator x11
    //x11z = 1;
    e11 = x11z - x11;
    x[3] = limit( x[3], 2.0 );
    x12z = limit( x[3] + kp11 * e11, 2.0 );
    dx[3] = e11 * ki11;

    //regulator x12
    e12 = x12z - x12;
    x[2] = limit( x[2], 2.0 );
    v1 = limit( x[2] + kp12 * e12, 2.0 );
    dx[2] = e12 * ki12;

    //regulator x21
    //x21z = 1;
    e21 = x21z - x21;
    x[1] = limit( x[1], 2.0 );
    x22z = limit( x[1] + kp21 * e21, 2.0 );
    dx[1] = e21 * ki21;

    //regulator x22
    e22 = x22z - x22;
    x[0] = limit( x[0], 2.0 );
    v2 = limit( x[0] + kp22 * e22, 2.0 );
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

void PWM( double tImp, double uS, double rhoU, double ud, double h ) {
    static double t;
    static int rhoUN;
    static bool cykl = true;
    static double t1, t2, t0;

    int stan_pocz;
    double tA, tB, tC;

    const double td = 0;
    const double sgn_ia_komp = 0, sgn_ib_komp = 0, sgn_ic_komp = 0;
    const double sgn_ia = 0, sgn_ib = 0, sgn_ic = 0;

    t += h;

    if ( t > tImp ) { // co okres impulsowania
        rhoU = wrapAngle( rhoU );
        rhoUN = floor( rhoU / ( ( 2 * M_PI ) / 6 ) ); // ktory sektor?
        int idu1 = rhoUN + 1;
        int idu2 = idu1 % 6 + 1;

        double uSX = uS * cos( rhoU );
        double uSY = uS * sin( rhoU ); // to opoznienie symuluje czas na obl sterowania
        double wt = ux[idu1] * uy[idu2] - uy[idu1] * ux[idu2];
        double k = tImp / ( ud * wt );
        t1 = k * ( uSX * uy[idu2] - uSY * ux[idu2] );
        t2 = -k * ( uSX * uy[idu1] - uSY * ux[idu1] );

        t0 = ( tImp - t1 - t2 ) / 2;
        cykl = !cykl;
        sterowanie = true; //pozwala na uruchomienie sterowania
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

    int bitA, bitB, bitC;

    //	PWM_ALTERA();
    if ( t <= tA ) {
        bitA = stan_pocz;
    } else if ( t <= tA + td ) {
        bitA = ( sgn_ia > 0 ) ? 0 : 1;
    } else if ( t <= tImp ) {
        bitA = !stan_pocz;
    }

    if ( t <= tB ) {
        bitB = stan_pocz;
    } else if ( t <= tB + td ) {
        bitB = ( sgn_ib > 0 ) ? 0 : 1;
    } else if ( t <= tImp ) {
        bitB = !stan_pocz;
    }

    if ( t <= tC ) {
        bitC = stan_pocz;
    } else if ( t <= tC + td ) {
        bitC = ( sgn_ic > 0 ) ? 0 : 1;
    } else if ( t <= tImp ) {
        bitC = !stan_pocz;
    }

    int bitsAll = bitC << 2 | bitB << 1 | bitA;

    usx = ux_out[bitsAll] * ud;
    usy = uy_out[bitsAll] * ud;

    if ( t > tImp ) {
        t = 0; //czyli co okres impulsowania
    }
}

// warunki początkowe
double x[32] = { 0 };
double y[32] = { 0 };

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

    h = 0.1 * M_PI * ht; /* ht w milisekundach */
    //tmin=.006*.1*M_PI;
    tImp = .15 * 0.1 * M_PI;
    uS = .91;

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
            rhoU += uS * tImp;
            rhoU = wrapAngle( rhoU );

            // zmienne multiskalarne
            x11 = omegaR;
            x12 = frx * isy - fry * isx;
            x21 = frx * frx + fry * fry;
            x22 = frx * isx + fry * isy;

            if ( timeCnt > 10 ) {
                Trapez( integrationArgCount, tImp, x );
                //REGULATORY();

                // odsprzezenie
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

                uS = sqrt( usx_zad * usx_zad + usy_zad * usy_zad );
                rhoU = getRho( usx_zad, usy_zad );
            }
            sterowanie = false;
        }

        PWM( tImp, uS, rhoU, ud, h );

        F4( integrationArgCount, h, y ); // rownania rozniczkowe silnika - rozwiazanie metoda RK4

        transform_XY_to_3ph( i3ph, isx, isy );
        transform_XY_to_3ph( u3ph, usx, usy );

        main_mon( );
    } // while(true);
}