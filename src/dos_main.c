#include <math.h>
#ifdef _WIN32
#include "dos2win.h"
#endif

#define   SQRT2     1.4142    // pierwiastek z 2
#define   SQRT3     1.7321    // pierwiastek z 3
#define   SQRT6     2.4495    // pierwiastek z 6

/* Parametry maszyny */
/*------------------------------------------------------------------------*/
/* Parametry silnika o mocy 1.5 kW typ 2Sg90L4                            */
/*------------------------------------------------------------------------*/
double Rs = 0.0584, Rr = 0.0559, Lm = 2.171, Ls = 2.246, Lr = 2.246, JJ = 38.4 * 2.5;

double ia, ib, ic, ua, ub, uc;

double ht = 0.005, h; /*Krok calkowania ht w milisekundach */
double Timp; /*Okres impulsowania do generowania napiecia*/
double TIME;
int sterowanie = 1;
//double impuls;

int Np = 0, Nk = 7; //calkowanie regulatorow
int M = 8; /* liczba calkowanych zmiennych - model silnika */

double ud = 1.52; //napiecie obwodu posredniczacego
double US, roU; // modul i predkosc wirowania wektroa napiecia do pwm
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

/*-------------------------------------------------------------------------*/
/*          Transformacja z ukladu osi xy do ukladu trojfazowego           */

/*-------------------------------------------------------------------------*/
void tran2_3i( double Xt, double Yt ); // transformacja Clarka P=const
void tran2_3u( double Xt, double Yt ); // transformacja Clarka P=const

void tran2_3i( double Xt, double Yt ) {
    ia = ( SQRT2 / SQRT3 ) * Xt;
    ib = ( -1 / SQRT6 ) * Xt + ( 1 / SQRT2 ) * Yt;
    ic = ( -1 / SQRT6 ) * Xt - ( 1 / SQRT2 ) * Yt;
}

void tran2_3u( double Xt, double Yt ) {
    ua = ( SQRT2 / SQRT3 ) * Xt;
    ub = ( -1 / SQRT6 ) * Xt + ( 1 / SQRT2 ) * Yt;
    uc = ( -1 / SQRT6 ) * Xt - ( 1 / SQRT2 ) * Yt;
}

inline double limit( const double v, const double limit ) {
    if ( v > limit ) {
        return limit;
    } else if ( v < -limit ) {
        return -limit;
    }
    return v;
}

void F4DERY( double DV[32], double V[32] ) {
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
    F4DERY( DZ, Z );
    for( I = 0; I < MM; I += 1 ) {
        Q[0][I] = Z[I];
        Q[1][I] = HF * DZ[I];
        Z[I] = Q[0][I] + 0.5 * Q[1][I];
    }
    F4DERY( DZ, Z );
    for( I = 0; I < MM; I += 1 ) {
        Q[2][I] = HF * DZ[I];
        Z[I] = Q[0][I] + 0.5 * Q[2][I];
    }
    F4DERY( DZ, Z );
    for( I = 0; I < MM; I += 1 ) {
        Q[3][I] = HF * DZ[I];
        Z[I] = Q[0][I] + Q[3][I];
    }
    F4DERY( DZ, Z );
    for( I = 0; I < MM; I += 1 ) {
        Q[4][I] = HF * DZ[I];
        Z[I] = ( Q[1][I] + Q[4][I] ) / 6 + ( Q[2][I] + Q[3][I] ) / 3 + Q[0][I];
    }
}

void TRAPDERY( double DX[10], double X[10] ) {
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

void TRAPEZ( int na, int NA, double HA, double ZA[32] ) {
    static double DZA[32], DXA[32], XA[32];
    int ja;

    TRAPDERY( DZA, ZA );
    for( ja = na; ja <= NA; ja++ ) {
        XA[ja] = ZA[ja] + HA * DZA[ja];
    }
    TRAPDERY( DXA, XA );
    for( ja = na; ja <= NA; ja++ ) {
        ZA[ja] = ZA[ja] + .5 * ( HA * DZA[ja] + HA * DXA[ja] );
    }
}

void PWM( double Timp, double US, double roU, double ud, double h ) {
    /*-------------------------------------------------------------------------*/
    /*                Wyznaczenie czasow zalaczen wektorow                     */
    /*-------------------------------------------------------------------------*/
    extern double usx, usy;

    extern int sterowanie;
    static double impuls;
    static double wt;
    static double ux[8], uy[8], ux_out[8], uy_out[8];
    static int NroU;
    static int idu1, idu2;
    static int z1, z2, n1, n2, sektor, cykl = 1;
    static int stan_pocz, stan_konc, stan_konc_a, stan_konc_b, stan_konc_c;
    static double sgn_ia_komp, sgn_ib_komp, sgn_ic_komp;
    static double komp_ia, komp_ib, komp_ic;
    static double USX, USY;
    static double t1, t2, t0, td;
    static double czas_a, czas_b, czas_c;
    static double sgn_ia, sgn_ib, sgn_ic;
    //UWAGA - nie ma symulacji td
    td = 0;
    sgn_ia = 0;
    sgn_ib = 0;
    sgn_ic = 0;

    ux[1] = SQRT2 / SQRT3;
    ux[2] = SQRT2 / ( SQRT3 * 2 );
    ux[3] = -SQRT2 / ( SQRT3 * 2 );
    ux[4] = -SQRT2 / SQRT3;
    ux[5] = -SQRT2 / ( SQRT3 * 2 );
    ux[6] = SQRT2 / ( SQRT3 * 2 );

    uy[1] = 0;
    uy[2] = SQRT2 / 2;
    uy[3] = SQRT2 / 2;
    uy[4] = 0;
    uy[5] = -SQRT2 / 2;
    uy[6] = -SQRT2 / 2;

    impuls += h;

    if ( impuls > Timp ) { //czyli co okres impulsowania

        if ( roU > 2 * M_PI ) roU -= 2 * M_PI;
        if ( roU < 0 ) roU += 2 * M_PI;
        NroU = floor( roU / ( M_PI / 3 ) );

        switch( NroU ) {
                //	case 0: idu1=1; idu2=2; sektor=1; z1=8; n1=1; n2=2; z2=7; break;
                //	case 1: idu1=2; idu2=3; sektor=2; z1=7; n1=2; n2=3; z2=8; break;
                //	case 2: idu1=3; idu2=4; sektor=3; z1=8; n1=3; n2=4; z2=7; break;
                //	case 3: idu1=4; idu2=5; sektor=4; z1=7; n1=4; n2=5; z2=8; break;
                //	case 4: idu1=5; idu2=6; sektor=5; z1=8; n1=5; n2=6; z2=7; break;
                //	case 5: idu1=6; idu2=1; sektor=6; z1=7; n1=6; n2=1; z2=8; break;

            case 0: idu1 = 1;
                idu2 = 2;
                sektor = 1;
                z1 = 0;
                n1 = 1;
                n2 = 2;
                z2 = 0;
                break;
            case 1: idu1 = 2;
                idu2 = 3;
                sektor = 2;
                z1 = 0;
                n1 = 2;
                n2 = 3;
                z2 = 0;
                break;
            case 2: idu1 = 3;
                idu2 = 4;
                sektor = 3;
                z1 = 0;
                n1 = 3;
                n2 = 4;
                z2 = 0;
                break;
            case 3: idu1 = 4;
                idu2 = 5;
                sektor = 4;
                z1 = 0;
                n1 = 4;
                n2 = 5;
                z2 = 0;
                break;
            case 4: idu1 = 5;
                idu2 = 6;
                sektor = 5;
                z1 = 0;
                n1 = 5;
                n2 = 6;
                z2 = 0;
                break;
            case 5: idu1 = 6;
                idu2 = 1;
                sektor = 6;
                z1 = 0;
                n1 = 6;
                n2 = 1;
                z2 = 0;
                break;
        }
        USX = US * cos( roU );
        USY = US * sin( roU ); //to opoznienie symuluje czas na obl sterowania
        wt = ux[idu1] * uy[idu2] - uy[idu1] * ux[idu2];
        t1 = Timp * ( USX * uy[idu2] - USY * ux[idu2] ) / ( ud * wt );
        t2 = Timp * ( -USX * uy[idu1] + USY * ux[idu1] ) / ( ud * wt );

        t0 = ( Timp - t1 - t2 ) / 2;
        cykl = cykl * ( -1 );
        sterowanie = 1; //pozwala na uruchomienie sterowania
    }

    if ( cykl > 0 ) {
        stan_pocz = 0;

        komp_ia = ( sgn_ia_komp > 0 ) ? -1 : 0;
        komp_ib = ( sgn_ib_komp > 0 ) ? -1 : 0;
        komp_ic = ( sgn_ic_komp > 0 ) ? -1 : 0;

        switch( NroU ) {
            case 0:
                czas_a = t0 + komp_ia*td;
                czas_b = t0 + t1 + komp_ib*td;
                czas_c = t0 + t1 + t2 + komp_ic*td;
                break;

            case 1:
                czas_a = t0 + t2 + komp_ia*td;
                czas_b = t0 + komp_ib*td;
                czas_c = t0 + t1 + t2 + komp_ic*td;
                break;

            case 2:
                czas_a = t0 + t1 + t2 + komp_ia*td;
                czas_b = t0 + komp_ib*td;
                czas_c = t0 + t1 + komp_ic*td;
                break;

            case 3:
                czas_a = t0 + t1 + t2 + komp_ia*td;
                czas_b = t0 + t2 + komp_ib*td;
                czas_c = t0 + komp_ic*td;
                break;

            case 4:
                czas_a = t0 + t1 + komp_ia*td;
                czas_b = t0 + t1 + t2 + komp_ib*td;
                czas_c = t0 + komp_ic*td;
                break;

            case 5:
                czas_a = t0 + komp_ia*td;
                czas_b = t0 + t1 + t2 + komp_ib*td;
                czas_c = t0 + t2 + komp_ic*td;
                break;
        }

    } else {
        stan_pocz = 1;

        komp_ia = ( sgn_ia_komp < 0 ) ? -1 : 0;
        komp_ib = ( sgn_ib_komp < 0 ) ? -1 : 0;
        komp_ic = ( sgn_ic_komp < 0 ) ? -1 : 0;

        switch( NroU ) {
            case 0:
                czas_a = t0 + t1 + t2 + komp_ia*td;
                czas_b = t0 + t2 + komp_ib*td;
                czas_c = t0 + komp_ic*td;
                break;

            case 1:
                czas_a = t0 + t1 + komp_ia*td;
                czas_b = t0 + t1 + t2 + komp_ib*td;
                czas_c = t0 + komp_ic*td;
                break;

            case 2:
                czas_a = t0 + komp_ia*td;
                czas_b = t0 + t1 + t2 + komp_ib*td;
                czas_c = t0 + t2 + komp_ic*td;
                break;

            case 3:
                czas_a = t0 + komp_ia*td;
                czas_b = t0 + t1 + komp_ib*td;
                czas_c = t0 + t1 + t2 + komp_ic*td;
                break;

            case 4:
                czas_a = t0 + t2 + komp_ia*td;
                czas_b = t0 + komp_ib*td;
                czas_c = t0 + t1 + t2 + komp_ic*td;
                break;

            case 5:
                czas_a = t0 + t1 + t2 + komp_ia*td;
                czas_b = t0 + komp_ib*td;
                czas_c = t0 + t1 + komp_ic*td;
                break;
        }
    }

    //	PWM_ALTERA();
    if ( impuls <= czas_a ) {
        stan_konc_a = stan_pocz;
    } else if ( impuls <= czas_a + td ) {
        stan_konc_a = ( sgn_ia > 0 ) ? 0 : 1;
    } else if ( impuls <= Timp ) {
        stan_konc_a = !stan_pocz;
    }

    if ( impuls <= czas_b ) {
        stan_konc_b = stan_pocz;
    } else if ( impuls <= czas_b + td ) {
        stan_konc_b = ( sgn_ib > 0 ) ? 0 : 1;
    } else if ( impuls <= Timp ) {
        stan_konc_b = !stan_pocz;
    }

    if ( impuls <= czas_c ) {
        stan_konc_c = stan_pocz;
    } else if ( impuls <= czas_c + td ) {
        stan_konc_c = ( sgn_ic > 0 ) ? 0 : 1;
    } else if ( impuls <= Timp ) {
        stan_konc_c = !stan_pocz;
    }

    stan_konc = stan_konc_c * 4 + stan_konc_b * 2 + stan_konc_a * 1;

    ux_out[0] = 0;
    uy_out[0] = 0; //000
    ux_out[1] = SQRT2 / SQRT3;
    uy_out[1] = 0; //001
    ux_out[2] = -SQRT2 / ( SQRT3 * 2 );
    uy_out[2] = SQRT2 / 2; //010
    ux_out[3] = SQRT2 / ( SQRT3 * 2 );
    uy_out[3] = SQRT2 / 2; //011
    ux_out[4] = -SQRT2 / ( SQRT3 * 2 );
    uy_out[4] = -SQRT2 / 2; //100
    ux_out[5] = SQRT2 / ( SQRT3 * 2 );
    uy_out[5] = -SQRT2 / 2; //101
    ux_out[6] = -SQRT2 / SQRT3;
    uy_out[6] = 0; //110
    ux_out[7] = 0;
    uy_out[7] = 0; //111

    usx = ux_out[stan_konc] * ud;
    usy = uy_out[stan_konc] * ud;

    if ( impuls > Timp ) {
        impuls = 0; //czyli co okres impulsowania
    }
}

double kat( double x, double y );

double kat( double x, double y ) {
    double a = x * x + y * y;
    if ( a == 0 ) {
        return 0;
    }

    a = sqrt( a );
    double arc = acos( x / a );
    return ( y / a >= 0 ) ? arc : 2 * M_PI - arc;
}

int dos_main( ) {
    /*deklaracje zmiennych lokalnych */
    double Y[32]; /*zmienne calkowane */
    double X[32]; /*zmienne calkowane */
    //int LL=0;
    int i;
    int J; /* liczniki petli */
    /*------------------------------------*/
    /*  Obliczanie warunkow poczatkowych  */
    /*------------------------------------*/
    for( J = 0; J < M; J++ ) {
        Y[J] = 0; // zerowe warunki poczatkowe
    }
    for( J = 0; J < M; J++ ) {
        X[J] = 0; // dla zmiennych ukladu sterowania - zerowe
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

    while( 1 ) {
        TIME += ht;

        //// place actual set controller value changes here
        if ( TIME > 1500 ) {
            x11z = 1.0;
            m0 = 0.1;
        } else if ( TIME > 800 ) {
            x11z = .5;
            m0 = 0.1;
        } else if ( TIME > 300 ) {
            x11z = .5;
            m0 = 0.9;
        }

        /*
        if ( TIME > 3100 ) {
            x11z = .5;
            m0 = 0.1;
        }
        if ( TIME > 4100 ) {
            x11z = .8;
            m0 = 0.1;
        }
        if ( TIME > 5100 ) {
            x11z = .8;
            m0 = 0.9;
        }
        if ( TIME > 6100 ) {
            x11z = .8;
            m0 = 0.1;
        }
        if ( TIME > 7100 ) {
            x11z = 1.0;
            m0 = 0.1;
        }
        if ( TIME > 8100 ) {
            x11z = 1.0;
            m0 = 0.9;
        }
        if ( TIME > 9100 ) {
            x11z = 1.0;
            m0 = 0.1;
        }
         */
        //if(TIME>1100) {x11z=.7;m0=0.5;}
        //if(TIME>1800) {x11z=.7;m0=0;}
        //if(TIME>1900) {x11z=.17;m0=1;}
        //if(TIME>2200) {x11z=.1;m0=1;}
        //if(TIME>2500) {x11z=.7;m0=0.5;}

        //if(TIME>3200) x11z=.05;
        //if(TIME>9100) x11z=0.0;

        //sterowanie

        if ( sterowanie == 1 ) {
            US = .91;
            roU += US * Timp;
            if ( roU > 2 * M_PI ) {
                roU -= 2 * M_PI;
            }
            if ( roU < 0 ) {
                roU += 2 * M_PI;
            }

            //zmienne multiskalarne
            x11 = omegaR;
            x12 = frx * isy - fry*isx;
            x21 = frx * frx + fry*fry;
            x22 = frx * isx + fry*isy;


            if ( TIME > 10 ) {
                TRAPEZ( Np, Nk, Timp, X );
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
                roU = kat( usx_zad, usy_zad );
            }
            sterowanie = 0;
        }

        ///PWM
        //if (TIME<350)
        PWM( Timp, US, roU, ud, h );
        //if (TIME>350)
        //{usx=usx_zad; usy=usy_zad;}

        //usx=US*cos(roU); usy=US*sin(roU);
        //	PWMU_2WR(US,roU,ud,Timp,h);
        F4( M, h, Y ); // SILNIK rownania rozniczkowe - rozwiazanie metoda RK4

        // obliczenie pradow trojfazowych
        tran2_3i( isx, isy );
        tran2_3u( usx, usy );

        main_mon( );
    } // while(1))
}