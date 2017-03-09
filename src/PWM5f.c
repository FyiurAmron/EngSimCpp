#include <math.h>
#include <stdbool.h>

#define SIN72  0.30901699437494742410229341718282 //cos 2PI/5
#define COS72  0.95105651629515357211643933337938 //sin 2PI/5
#define COS144    -0.80901699437494742410229341718282 //cos 4PI/5
#define SIN144     0.58778525229247312916870595463907 //sin 4PI/5
//#define kat1sect	0.6283185307179586476925286766559 //odpelglosc katowa pomiedzy sektorami
#define HALF_SQRT_2DIV5 0.31622776601683793319988935444327 /* * 2 */ //0,5*pierwiastek 2/5

double time0;

struct vectParam {
    int w;
    //	double wsk;
};
vectParam vect[32]; //32

struct selVectParam {
    int w;
};
selVectParam selVect[5], selVect2[5]; //5

struct timeOnDesc {
    int faza; //a=1, b=2, c=3, d=4, e=5
    double timeOn; //w danej fazie
};
timeOnDesc timesOnDesc[6]; //6

struct timeOnTmp {
    double timeOn; //zmienna tymczasowa
};
timeOnTmp timesOnTmp[6]; //6

struct outVectParam {
    int w;
};
outVectParam outVect[6]; //6

double ux1[32], uy1[32], ux3[32], uy3[32];

double ux1out[32], uy1out[32], ux3out[32], uy3out[32];

//wspolczynniki do PWm 4 wek
//double A4y1;
double a1x1, a1y1, a1x3, a1y3;
double a2x1, a2y1, a2x3, a2y3;
double a3x1, a3y1, a3x3, a3y3;
double a4x1, a4y1, a4x3, a4y3;

//czasy
double tCnt[5]; //czas do za��czenia tranzystora w fazie (czas trwania zera)
double t[6]; //6
//skladowe zadane
double uSX1, uSY1, uSX3, uSY3;

/*
inline double limit( const double v, const double limit ) {
    if ( v > limit ) {
        return limit;
    } else if ( v < -limit ) {
        return -limit;
    }
    return v;
}
 */

//stany gornych tranzystorow

double fux1( int tA, int tB, int tC, int tD, int tE ) {
    //return 0.2*((6+2*cos72+2*cos144)*(Ta-(1-Ta))+((1+7*cos72+2*cos144)*((Tb-(1-Tb))+(Te-(1-Te))))+((1+2*cos72+7*cos144)*((Tc-(1-Tc))+(Td-(1-Td)))));
    return 0.2 * ( ( 4 - 2 * COS72 - 2 * COS144 ) * ( tA - ( 1 - tA ) )
            + ( ( -1 + 3 * COS72 - 2 * COS144 ) * ( ( tB - ( 1 - tB ) ) + ( tE - ( 1 - tE ) ) ) )
            + ( ( -1 - 2 * COS72 + 3 * COS144 ) * ( ( tC - ( 1 - tC ) ) + ( tD - ( 1 - tD ) ) ) ) );
}

double fuy1( int tA, int tB, int tC, int tD, int tE ) {
    return ( COS72 * ( ( tB - ( 1 - tB ) ) - ( tE - ( 1 - tE ) ) ) )
            + ( SIN144 * ( ( tC - ( 1 - tC ) ) - ( tD - ( 1 - tD ) ) ) );
}

double fux3( int tA, int tB, int tC, int tD, int tE ) {
    //return 0.2*((6+2*cos144+2*cos72)*(Ta-(1-Ta))+((1+7*cos144+2*cos72)*((Tb-(1-Tb))+(Te-(1-Te))))+((1+2*cos144+7*cos72)*((Tc-(1-Tc))+(Td-(1-Td)))));
    return 0.2 * ( ( 4 - 2 * COS144 - 2 * COS72 ) * ( tA - ( 1 - tA ) )
            + ( ( -1 + 3 * COS144 - 2 * COS72 ) * ( ( tB - ( 1 - tB ) ) + ( tE - ( 1 - tE ) ) ) )
            + ( ( -1 - 2 * COS144 + 3 * COS72 ) * ( ( tC - ( 1 - tC ) ) + ( tD - ( 1 - tD ) ) ) ) );
}

double fuy3( int tA, int tB, int tC, int tD, int tE ) {
    return ( SIN144 * ( ( tB - ( 1 - tB ) ) - ( tE - ( 1 - tE ) ) ) )
            - ( COS72 * ( ( tC - ( 1 - tC ) ) - ( tD - ( 1 - tD ) ) ) );
}

inline int cmp( int a, int b ) {
    return (a > b ) - ( a < b );
}

int cmpTimeOn( const void * a, const void * b ) {
    return cmp( ( (timeOnDesc*) a )->timeOn, ( (timeOnDesc*) b )->timeOn );
}

int cmpTimeOnTmp( const void * a, const void * b ) {
    return cmp( ( (timeOnTmp*) a )->timeOn, ( (timeOnTmp*) b )->timeOn );
}

void PWM5f( double tImp, double uS1, double rhoU1, double uS3, double rhoU3, double ud, double h,
        double *usx1, double *usy1, double *usx3, double *usy3,
        double *usx1wyg, double *usy1wyg, double *usx3wyg, double *usy3wyg,
        bool *przerwanie ) {
    //int przerwanie;
    static bool initNeeded = true;
    static double impuls;
    static int cykl = 1;

    int stan[5][5];

    if ( initNeeded ) {
        //wyznaczenie wspolczynnikow skladowych wektorow, ich numery wynikaja z zapisu binarnego (f*2^0+e*2^1+....a*2^0)
        //kolejnosc faz w zapise wektorow (a,b,c,d,e,f)
        //uwaga - rzeczywiste dlugosci wektorow uzyskujemy po pomnozeniu przez sqrt(2/5)*uDC/2
        //kat 0*36st (uklad wsp 1)
        ux1[25] = fux1( 1, 1, 0, 0, 1 );
        uy1[25] = fuy1( 1, 1, 0, 0, 1 );
        ux3[25] = fux3( 1, 1, 0, 0, 1 );
        uy3[25] = fuy3( 1, 1, 0, 0, 1 ); //w. Dlugi  (ukl wsp 1)
        ux1[9] = fux1( 0, 1, 0, 0, 1 );
        uy1[9] = fuy1( 0, 1, 0, 0, 1 );
        ux3[9] = fux3( 0, 1, 0, 0, 1 );
        uy3[9] = fuy3( 0, 1, 0, 0, 1 ); //w. Krotki (ukl wsp 1)
        ux1[16] = fux1( 1, 0, 0, 0, 0 );
        uy1[16] = fuy1( 1, 0, 0, 0, 0 );
        ux3[16] = fux3( 1, 0, 0, 0, 0 );
        uy3[16] = fuy3( 1, 0, 0, 0, 0 ); //w. Sredni (ukl wsp 1)

        //kat 1*36st (uklad wsp 1)
        ux1[24] = fux1( 1, 1, 0, 0, 0 );
        uy1[24] = fuy1( 1, 1, 0, 0, 0 );
        ux3[24] = fux3( 1, 1, 0, 0, 0 );
        uy3[24] = fuy3( 1, 1, 0, 0, 0 ); //w. Dlugi  (ukl wsp 1)
        ux1[26] = fux1( 1, 1, 0, 1, 0 );
        uy1[26] = fuy1( 1, 1, 0, 1, 0 );
        ux3[26] = fux3( 1, 1, 0, 1, 0 );
        uy3[26] = fuy3( 1, 1, 0, 1, 0 ); //w. Krotki (ukl wsp 1)
        ux1[29] = fux1( 1, 1, 1, 0, 1 );
        uy1[29] = fuy1( 1, 1, 1, 0, 1 );
        ux3[29] = fux3( 1, 1, 1, 0, 1 );
        uy3[29] = fuy3( 1, 1, 1, 0, 1 ); //w. Sredni (ukl wsp 1)

        //kat 2*36st (uklad wsp 1)
        ux1[28] = fux1( 1, 1, 1, 0, 0 );
        uy1[28] = fuy1( 1, 1, 1, 0, 0 );
        ux3[28] = fux3( 1, 1, 1, 0, 0 );
        uy3[28] = fuy3( 1, 1, 1, 0, 0 ); //w. Dlugi  (ukl wsp 1)
        ux1[20] = fux1( 1, 0, 1, 0, 0 );
        uy1[20] = fuy1( 1, 0, 1, 0, 0 );
        ux3[20] = fux3( 1, 0, 1, 0, 0 );
        uy3[20] = fuy3( 1, 0, 1, 0, 0 ); //w. Krotki (ukl wsp 1)
        ux1[8] = fux1( 0, 1, 0, 0, 0 );
        uy1[8] = fuy1( 0, 1, 0, 0, 0 );
        ux3[8] = fux3( 0, 1, 0, 0, 0 );
        uy3[8] = fuy3( 0, 1, 0, 0, 0 ); //w. Sredni (ukl wsp 1)

        //kat 3*36st (uklad wsp 1)
        ux1[12] = fux1( 0, 1, 1, 0, 0 );
        uy1[12] = fuy1( 0, 1, 1, 0, 0 );
        ux3[12] = fux3( 0, 1, 1, 0, 0 );
        uy3[12] = fuy3( 0, 1, 1, 0, 0 ); //w. Dlugi  (ukl wsp 1)
        ux1[13] = fux1( 0, 1, 1, 0, 1 );
        uy1[13] = fuy1( 0, 1, 1, 0, 1 );
        ux3[13] = fux3( 0, 1, 1, 0, 1 );
        uy3[13] = fuy3( 0, 1, 1, 0, 1 ); //w. Krotki (ukl wsp 1)
        ux1[30] = fux1( 1, 1, 1, 1, 0 );
        uy1[30] = fuy1( 1, 1, 1, 1, 0 );
        ux3[30] = fux3( 1, 1, 1, 1, 0 );
        uy3[30] = fuy3( 1, 1, 1, 1, 0 ); //w. Sredni (ukl wsp 1)

        //kat 4*36st (uklad wsp 1)
        ux1[14] = fux1( 0, 1, 1, 1, 0 );
        uy1[14] = fuy1( 0, 1, 1, 1, 0 );
        ux3[14] = fux3( 0, 1, 1, 1, 0 );
        uy3[14] = fuy3( 0, 1, 1, 1, 0 ); //w. Dlugi  (ukl wsp 1)
        ux1[10] = fux1( 0, 1, 0, 1, 0 );
        uy1[10] = fuy1( 0, 1, 0, 1, 0 );
        ux3[10] = fux3( 0, 1, 0, 1, 0 );
        uy3[10] = fuy3( 0, 1, 0, 1, 0 ); //w. Krotki (ukl wsp 1)
        ux1[4] = fux1( 0, 0, 1, 0, 0 );
        uy1[4] = fuy1( 0, 0, 1, 0, 0 );
        ux3[4] = fux3( 0, 0, 1, 0, 0 );
        uy3[4] = fuy3( 0, 0, 1, 0, 0 ); //w. Sredni (ukl wsp 1)

        //kat 5*36st (uklad wsp 1)
        ux1[6] = fux1( 0, 0, 1, 1, 0 );
        uy1[6] = fuy1( 0, 0, 1, 1, 0 );
        ux3[6] = fux3( 0, 0, 1, 1, 0 );
        uy3[6] = fuy3( 0, 0, 1, 1, 0 ); //w. Dlugi  (ukl wsp 1)
        ux1[22] = fux1( 1, 0, 1, 1, 0 );
        uy1[22] = fuy1( 1, 0, 1, 1, 0 );
        ux3[22] = fux3( 1, 0, 1, 1, 0 );
        uy3[22] = fuy3( 1, 0, 1, 1, 0 ); //w. Krotki (ukl wsp 1)
        ux1[15] = fux1( 0, 1, 1, 1, 1 );
        uy1[15] = fuy1( 0, 1, 1, 1, 1 );
        ux3[15] = fux3( 0, 1, 1, 1, 1 );
        uy3[15] = fuy3( 0, 1, 1, 1, 1 ); //w. Sredni (ukl wsp 1)

        //kat 6*36st (uklad wsp 1)
        ux1[7] = fux1( 0, 0, 1, 1, 1 );
        uy1[7] = fuy1( 0, 0, 1, 1, 1 );
        ux3[7] = fux3( 0, 0, 1, 1, 1 );
        uy3[7] = fuy3( 0, 0, 1, 1, 1 ); //w. Dlugi  (ukl wsp 1)
        ux1[5] = fux1( 0, 0, 1, 0, 1 );
        uy1[5] = fuy1( 0, 0, 1, 0, 1 );
        ux3[5] = fux3( 0, 0, 1, 0, 1 );
        uy3[5] = fuy3( 0, 0, 1, 0, 1 ); //w. Krotki (ukl wsp 1)
        ux1[2] = fux1( 0, 0, 0, 1, 0 );
        uy1[2] = fuy1( 0, 0, 0, 1, 0 );
        ux3[2] = fux3( 0, 0, 0, 1, 0 );
        uy3[2] = fuy3( 0, 0, 0, 1, 0 ); //w. Sredni (ukl wsp 1)

        //kat 7*36st (uklad wsp 1)
        ux1[3] = fux1( 0, 0, 0, 1, 1 );
        uy1[3] = fuy1( 0, 0, 0, 1, 1 );
        ux3[3] = fux3( 0, 0, 0, 1, 1 );
        uy3[3] = fuy3( 0, 0, 0, 1, 1 ); //w. Dlugi  (ukl wsp 1)
        ux1[11] = fux1( 0, 1, 0, 1, 1 );
        uy1[11] = fuy1( 0, 1, 0, 1, 1 );
        ux3[11] = fux3( 0, 1, 0, 1, 1 );
        uy3[11] = fuy3( 0, 1, 0, 1, 1 ); //w. Krotki (ukl wsp 1)
        ux1[23] = fux1( 1, 0, 1, 1, 1 );
        uy1[23] = fuy1( 1, 0, 1, 1, 1 );
        ux3[23] = fux3( 1, 0, 1, 1, 1 );
        uy3[23] = fuy3( 1, 0, 1, 1, 1 ); //w. Sredni (ukl wsp 1)

        //kat 8*36st (uklad wsp 1)
        ux1[19] = fux1( 1, 0, 0, 1, 1 );
        uy1[19] = fuy1( 1, 0, 0, 1, 1 );
        ux3[19] = fux3( 1, 0, 0, 1, 1 );
        uy3[19] = fuy3( 1, 0, 0, 1, 1 ); //w. Dlugi  (ukl wsp 1)
        ux1[18] = fux1( 1, 0, 0, 1, 0 );
        uy1[18] = fuy1( 1, 0, 0, 1, 0 );
        ux3[18] = fux3( 1, 0, 0, 1, 0 );
        uy3[18] = fuy3( 1, 0, 0, 1, 0 ); //w. Krotki (ukl wsp 1)
        ux1[1] = fux1( 0, 0, 0, 0, 1 );
        uy1[1] = fuy1( 0, 0, 0, 0, 1 );
        ux3[1] = fux3( 0, 0, 0, 0, 1 );
        uy3[1] = fuy3( 0, 0, 0, 0, 1 ); //w. Sredni (ukl wsp 1)

        //kat 9*36st (uklad wsp 1)
        ux1[17] = fux1( 1, 0, 0, 0, 1 );
        uy1[17] = fuy1( 1, 0, 0, 0, 1 );
        ux3[17] = fux3( 1, 0, 0, 0, 1 );
        uy3[17] = fuy3( 1, 0, 0, 0, 1 ); //w. Dlugi  (ukl wsp 1)
        ux1[21] = fux1( 1, 0, 1, 0, 1 );
        uy1[21] = fuy1( 1, 0, 1, 0, 1 );
        ux3[21] = fux3( 1, 0, 1, 0, 1 );
        uy3[21] = fuy3( 1, 0, 1, 0, 1 ); //w. Krotki (ukl wsp 1)
        ux1[27] = fux1( 1, 1, 0, 1, 1 );
        uy1[27] = fuy1( 1, 1, 0, 1, 1 );
        ux3[27] = fux3( 1, 1, 0, 1, 1 );
        uy3[27] = fuy3( 1, 1, 0, 1, 1 ); //w. Sredni (ukl wsp 1)

        //werktory pasywne
        ux1[0] = 0;
        uy1[0] = 0;
        ux3[0] = 0;
        uy3[0] = 0; //00000
        ux1[31] = 0;
        uy1[31] = 0;
        ux3[31] = 0;
        uy3[31] = 0; //11111

        //wyb�r 4 dowolnych wektor�w (tworz�cych macierz odwracaln�)
        selVect[1].w = 25;
        selVect[2].w = 28;
        selVect[3].w = 10;
        selVect[4].w = 11;

        /*wektor_sel[1].W=1;
        wektor_sel[2].W=2;
        wektor_sel[3].W=3;
        wektor_sel[4].W=30;*/

        double invDenom = 1 /
                ( ux1[selVect[1].w] * ux3[selVect[2].w] * uy1[selVect[3].w] * uy3[selVect[4].w]
                - ux1[selVect[1].w] * ux3[selVect[2].w] * uy3[selVect[3].w] * uy1[selVect[4].w]
                - ux1[selVect[1].w] * ux3[selVect[3].w] * uy1[selVect[2].w] * uy3[selVect[4].w]
                + ux1[selVect[1].w] * ux3[selVect[3].w] * uy3[selVect[2].w] * uy1[selVect[4].w]
                + ux1[selVect[1].w] * ux3[selVect[4].w] * uy1[selVect[2].w] * uy3[selVect[3].w]
                - ux1[selVect[1].w] * ux3[selVect[4].w] * uy3[selVect[2].w] * uy1[selVect[3].w]
                - ux3[selVect[1].w] * ux1[selVect[2].w] * uy1[selVect[3].w] * uy3[selVect[4].w]
                + ux3[selVect[1].w] * ux1[selVect[2].w] * uy3[selVect[3].w] * uy1[selVect[4].w]
                + ux3[selVect[1].w] * ux1[selVect[3].w] * uy1[selVect[2].w] * uy3[selVect[4].w]
                - ux3[selVect[1].w] * ux1[selVect[3].w] * uy3[selVect[2].w] * uy1[selVect[4].w]
                - ux3[selVect[1].w] * ux1[selVect[4].w] * uy1[selVect[2].w] * uy3[selVect[3].w]
                + ux3[selVect[1].w] * ux1[selVect[4].w] * uy3[selVect[2].w] * uy1[selVect[3].w]
                + ux1[selVect[2].w] * ux3[selVect[3].w] * uy1[selVect[1].w] * uy3[selVect[4].w]
                - ux1[selVect[2].w] * ux3[selVect[3].w] * uy3[selVect[1].w] * uy1[selVect[4].w]
                - ux1[selVect[2].w] * ux3[selVect[4].w] * uy1[selVect[1].w] * uy3[selVect[3].w]
                + ux1[selVect[2].w] * ux3[selVect[4].w] * uy3[selVect[1].w] * uy1[selVect[3].w]
                - ux3[selVect[2].w] * ux1[selVect[3].w] * uy1[selVect[1].w] * uy3[selVect[4].w]
                + ux3[selVect[2].w] * ux1[selVect[3].w] * uy3[selVect[1].w] * uy1[selVect[4].w]
                + ux3[selVect[2].w] * ux1[selVect[4].w] * uy1[selVect[1].w] * uy3[selVect[3].w]
                - ux3[selVect[2].w] * ux1[selVect[4].w] * uy3[selVect[1].w] * uy1[selVect[3].w]
                + ux1[selVect[3].w] * ux3[selVect[4].w] * uy1[selVect[1].w] * uy3[selVect[2].w]
                - ux1[selVect[3].w] * ux3[selVect[4].w] * uy3[selVect[1].w] * uy1[selVect[2].w]
                - ux3[selVect[3].w] * ux1[selVect[4].w] * uy1[selVect[1].w] * uy3[selVect[2].w]
                + ux3[selVect[3].w] * ux1[selVect[4].w] * uy3[selVect[1].w] * uy1[selVect[2].w] );

        a1x1 = invDenom *
                ( ux3[selVect[2].w] * uy1[selVect[3].w] * uy3[selVect[4].w]
                - ux3[selVect[2].w] * uy3[selVect[3].w] * uy1[selVect[4].w]
                - ux3[selVect[3].w] * uy1[selVect[2].w] * uy3[selVect[4].w]
                + ux3[selVect[3].w] * uy3[selVect[2].w] * uy1[selVect[4].w]
                + ux3[selVect[4].w] * uy1[selVect[2].w] * uy3[selVect[3].w]
                - ux3[selVect[4].w] * uy3[selVect[2].w] * uy1[selVect[3].w] );
        a2x1 = -invDenom
                * ( ux3[selVect[1].w] * uy1[selVect[3].w] * uy3[selVect[4].w]
                - ux3[selVect[1].w] * uy3[selVect[3].w] * uy1[selVect[4].w]
                - ux3[selVect[3].w] * uy1[selVect[1].w] * uy3[selVect[4].w]
                + ux3[selVect[3].w] * uy3[selVect[1].w] * uy1[selVect[4].w]
                + ux3[selVect[4].w] * uy1[selVect[1].w] * uy3[selVect[3].w]
                - ux3[selVect[4].w] * uy3[selVect[1].w] * uy1[selVect[3].w] );
        a3x1 = invDenom *
                ( ux3[selVect[1].w] * uy1[selVect[2].w] * uy3[selVect[4].w]
                - ux3[selVect[1].w] * uy3[selVect[2].w] * uy1[selVect[4].w]
                - ux3[selVect[2].w] * uy1[selVect[1].w] * uy3[selVect[4].w]
                + ux3[selVect[2].w] * uy3[selVect[1].w] * uy1[selVect[4].w]
                + ux3[selVect[4].w] * uy1[selVect[1].w] * uy3[selVect[2].w]
                - ux3[selVect[4].w] * uy3[selVect[1].w] * uy1[selVect[2].w] );
        a4x1 = -invDenom *
                ( ux3[selVect[1].w] * uy1[selVect[2].w] * uy3[selVect[3].w]
                - ux3[selVect[1].w] * uy3[selVect[2].w] * uy1[selVect[3].w]
                - ux3[selVect[2].w] * uy1[selVect[1].w] * uy3[selVect[3].w]
                + ux3[selVect[2].w] * uy3[selVect[1].w] * uy1[selVect[3].w]
                + ux3[selVect[3].w] * uy1[selVect[1].w] * uy3[selVect[2].w]
                - ux3[selVect[3].w] * uy3[selVect[1].w] * uy1[selVect[2].w] );

        a1y1 = invDenom *
                ( ux1[selVect[2].w] * ux3[selVect[3].w] * uy3[selVect[4].w]
                - ux1[selVect[2].w] * ux3[selVect[4].w] * uy3[selVect[3].w]
                - ux3[selVect[2].w] * ux1[selVect[3].w] * uy3[selVect[4].w]
                + ux3[selVect[2].w] * ux1[selVect[4].w] * uy3[selVect[3].w]
                + ux1[selVect[3].w] * ux3[selVect[4].w] * uy3[selVect[2].w]
                - ux3[selVect[3].w] * ux1[selVect[4].w] * uy3[selVect[2].w] );
        a2y1 = -invDenom *
                ( ux1[selVect[1].w] * ux3[selVect[3].w] * uy3[selVect[4].w]
                - ux1[selVect[1].w] * ux3[selVect[4].w] * uy3[selVect[3].w]
                - ux3[selVect[1].w] * ux1[selVect[3].w] * uy3[selVect[4].w]
                + ux3[selVect[1].w] * ux1[selVect[4].w] * uy3[selVect[3].w]
                + ux1[selVect[3].w] * ux3[selVect[4].w] * uy3[selVect[1].w]
                - ux3[selVect[3].w] * ux1[selVect[4].w] * uy3[selVect[1].w] );
        a3y1 = invDenom *
                ( ux1[selVect[1].w] * ux3[selVect[2].w] * uy3[selVect[4].w]
                - ux1[selVect[1].w] * ux3[selVect[4].w] * uy3[selVect[2].w]
                - ux3[selVect[1].w] * ux1[selVect[2].w] * uy3[selVect[4].w]
                + ux3[selVect[1].w] * ux1[selVect[4].w] * uy3[selVect[2].w]
                + ux1[selVect[2].w] * ux3[selVect[4].w] * uy3[selVect[1].w]
                - ux3[selVect[2].w] * ux1[selVect[4].w] * uy3[selVect[1].w] );
        a4y1 = -invDenom *
                ( ux1[selVect[1].w] * ux3[selVect[2].w] * uy3[selVect[3].w]
                - ux1[selVect[1].w] * ux3[selVect[3].w] * uy3[selVect[2].w]
                - ux3[selVect[1].w] * ux1[selVect[2].w] * uy3[selVect[3].w]
                + ux3[selVect[1].w] * ux1[selVect[3].w] * uy3[selVect[2].w]
                + ux1[selVect[2].w] * ux3[selVect[3].w] * uy3[selVect[1].w]
                - ux3[selVect[2].w] * ux1[selVect[3].w] * uy3[selVect[1].w] );

        a1x3 = -invDenom *
                ( ux1[selVect[2].w] * uy1[selVect[3].w] * uy3[selVect[4].w]
                - ux1[selVect[2].w] * uy3[selVect[3].w] * uy1[selVect[4].w]
                - ux1[selVect[3].w] * uy1[selVect[2].w] * uy3[selVect[4].w]
                + ux1[selVect[3].w] * uy3[selVect[2].w] * uy1[selVect[4].w]
                + ux1[selVect[4].w] * uy1[selVect[2].w] * uy3[selVect[3].w]
                - ux1[selVect[4].w] * uy3[selVect[2].w] * uy1[selVect[3].w] );
        a2x3 = invDenom *
                ( ux1[selVect[1].w] * uy1[selVect[3].w] * uy3[selVect[4].w]
                - ux1[selVect[1].w] * uy3[selVect[3].w] * uy1[selVect[4].w]
                - ux1[selVect[3].w] * uy1[selVect[1].w] * uy3[selVect[4].w]
                + ux1[selVect[3].w] * uy3[selVect[1].w] * uy1[selVect[4].w]
                + ux1[selVect[4].w] * uy1[selVect[1].w] * uy3[selVect[3].w]
                - ux1[selVect[4].w] * uy3[selVect[1].w] * uy1[selVect[3].w] );
        a3x3 = -invDenom *
                ( ux1[selVect[1].w] * uy1[selVect[2].w] * uy3[selVect[4].w]
                - ux1[selVect[1].w] * uy3[selVect[2].w] * uy1[selVect[4].w]
                - ux1[selVect[2].w] * uy1[selVect[1].w] * uy3[selVect[4].w]
                + ux1[selVect[2].w] * uy3[selVect[1].w] * uy1[selVect[4].w]
                + ux1[selVect[4].w] * uy1[selVect[1].w] * uy3[selVect[2].w]
                - ux1[selVect[4].w] * uy3[selVect[1].w] * uy1[selVect[2].w] );
        a4x3 = invDenom * ( ux1[selVect[1].w] * uy1[selVect[2].w] * uy3[selVect[3].w]
                - ux1[selVect[1].w] * uy3[selVect[2].w] * uy1[selVect[3].w]
                - ux1[selVect[2].w] * uy1[selVect[1].w] * uy3[selVect[3].w]
                + ux1[selVect[2].w] * uy3[selVect[1].w] * uy1[selVect[3].w]
                + ux1[selVect[3].w] * uy1[selVect[1].w] * uy3[selVect[2].w]
                - ux1[selVect[3].w] * uy3[selVect[1].w] * uy1[selVect[2].w] );

        a1y3 = -invDenom *
                ( ux1[selVect[2].w] * ux3[selVect[3].w] * uy1[selVect[4].w]
                - ux1[selVect[2].w] * ux3[selVect[4].w] * uy1[selVect[3].w]
                - ux3[selVect[2].w] * ux1[selVect[3].w] * uy1[selVect[4].w]
                + ux3[selVect[2].w] * ux1[selVect[4].w] * uy1[selVect[3].w]
                + ux1[selVect[3].w] * ux3[selVect[4].w] * uy1[selVect[2].w]
                - ux3[selVect[3].w] * ux1[selVect[4].w] * uy1[selVect[2].w] );
        a2y3 = invDenom *
                ( ux1[selVect[1].w] * ux3[selVect[3].w] * uy1[selVect[4].w]
                - ux1[selVect[1].w] * ux3[selVect[4].w] * uy1[selVect[3].w]
                - ux3[selVect[1].w] * ux1[selVect[3].w] * uy1[selVect[4].w]
                + ux3[selVect[1].w] * ux1[selVect[4].w] * uy1[selVect[3].w]
                + ux1[selVect[3].w] * ux3[selVect[4].w] * uy1[selVect[1].w]
                - ux3[selVect[3].w] * ux1[selVect[4].w] * uy1[selVect[1].w] );
        a3y3 = -invDenom *
                ( ux1[selVect[1].w] * ux3[selVect[2].w] * uy1[selVect[4].w]
                - ux1[selVect[1].w] * ux3[selVect[4].w] * uy1[selVect[2].w]
                - ux3[selVect[1].w] * ux1[selVect[2].w] * uy1[selVect[4].w]
                + ux3[selVect[1].w] * ux1[selVect[4].w] * uy1[selVect[2].w]
                + ux1[selVect[2].w] * ux3[selVect[4].w] * uy1[selVect[1].w]
                - ux3[selVect[2].w] * ux1[selVect[4].w] * uy1[selVect[1].w] );
        a4y3 = invDenom *
                ( ux1[selVect[1].w] * ux3[selVect[2].w] * uy1[selVect[3].w]
                - ux1[selVect[1].w] * ux3[selVect[3].w] * uy1[selVect[2].w]
                - ux3[selVect[1].w] * ux1[selVect[2].w] * uy1[selVect[3].w]
                + ux3[selVect[1].w] * ux1[selVect[3].w] * uy1[selVect[2].w]
                + ux1[selVect[2].w] * ux3[selVect[3].w] * uy1[selVect[1].w]
                - ux3[selVect[2].w] * ux1[selVect[3].w] * uy1[selVect[1].w] );

        initNeeded = false;
    }

    double k_ud = ud * HALF_SQRT_2DIV5;

    /******************************w przerwaniu***********************************************************************/
    if ( impuls == 0 ) { //czyli co okres impulsowania (uwaga MUSI byc Timp - h bo inaczej  niezgodnosc czestotliwosci
        //wylicz polozenia i skladowe wektorow zadanych
        rhoU1 = wrapAngle( rhoU1 );
        rhoU3 = wrapAngle( rhoU3 );

        uSX1 = uS1 * cos( rhoU1 );
        uSY1 = uS1 * sin( rhoU1 );
        uSX3 = uS3 * cos( rhoU3 );
        uSY3 = uS3 * sin( rhoU3 );

        //od tego miejsca musi by� w przerwaniu
        double k = tImp / k_ud;
        t[1] = ( a1x1 * uSX1 + a1y1 * uSY1 + a1x3 * uSX3 + a1y3 * uSY3 ) * k; //czasy tu wyliczone dotycz� 4 wybranych wektor_sel�w i mog� mie� dowolne warto�ci (te� ujemne). to ju� muzi byc w przerwaniu
        t[2] = ( a2x1 * uSX1 + a2y1 * uSY1 + a2x3 * uSX3 + a2y3 * uSY3 ) * k;
        t[3] = ( a3x1 * uSX1 + a3y1 * uSY1 + a3x3 * uSX3 + a3y3 * uSY3 ) * k;
        t[4] = ( a4x1 * uSX1 + a4y1 * uSY1 + a4x3 * uSX3 + a4y3 * uSY3 ) * k;

        //przepisanie wybranych wektor�w (wstawionych w inicjacji) dla kt�rych wyliczono czasy (dalej wektory s� nadpisywane wi�c trzeba przepisa� je tutaj)
        //jezeli czasy ujemne - to we� wektory przeciwne
        for( int i = 1; i < 5; i++ ) {
            selVect2[i].w = selVect[i].w;
            if ( t[i] < 0 ) {
                t[i] *= -1;
                selVect2[i].w = abs( 31 - selVect2[i].w );
            }
        }

        //suma czasu trwania zer i jedynek w fazach
        //dla 4 wybranych wektor_sel�w wyluskanie bitu wektor_sel�w okre�laj�cego faz�, 1 to stan za��czenia, zero - wy��czenia
        for( int i = 1; i <= 4; i++ ) { //do 4 bo 4 wektry
            stan[0][i] = ( (selVect2[i].w ) & 0x10 ) >> 4;
            stan[1][i] = ( (selVect2[i].w ) & 0x08 ) >> 3;
            stan[2][i] = ( (selVect2[i].w ) & 0x04 ) >> 2;
            stan[3][i] = ( (selVect2[i].w ) & 0x02 ) >> 1;
            stan[4][i] = ( (selVect2[i].w ) & 0x01 ) >> 0;
        }

        //wyzerowanie czasu trwania jedynek na pocz�tku oblicze�. Przyj�te przyporzadkowanie nr fazom (a=1, b=2... e=5)
        for( int i = 1; i <= 5; i++ ) {
            timesOnDesc[i].faza = i;
            timesOnDesc[i].timeOn = 0;
        }

        //i zsumowanie czas�w trwania jedynek w fazach
        //<= 4 bo 4 czasy trwania wektorow
        for( int i = 1; i <= 4; i++ ) {
            for( int j = 0; j < 5; j++ ) {
                if ( stan[j][i] == 1 ) {
                    timesOnDesc[j + 1].timeOn += t[i];
                }
            }
        }

        //przpisujemy te czasy do zmiennnych pomocniczych
        for( int i = 1; i <= 5; i++ ) {
            timesOnTmp[i].timeOn = timesOnDesc[i].timeOn;
        }
        //sortowanie od najkr�tszego  do najd�u�szego czasu trwania jedynki - im kr�tsza tym jest ona p�niej. najkr�sza to wektor 11111
        qsort( &timesOnTmp[1], 5, sizeof (timeOnTmp ), cmpTimeOnTmp ); //alternatywna metoda sortowania
        //tu najkr�tszy czas to 1111. ten czas odejmujemy od wszystkich czas�w trwania jedynek
        for( int i = 1; i <= 5; i++ ) {
            timesOnDesc[i].timeOn -= timesOnTmp[1].timeOn;
        }
        //teraz czasy jedynek dotycza wylacznie wektorow aktywnych. Kolejny etap to wyluskanie zer.
        time0 = 0.5 * ( tImp - ( timesOnTmp[5].timeOn - timesOnTmp[1].timeOn ) );
        //czasy do za�aczenia tranzystora w fazie (przejscie z 0 na 1)
        for( int i = 0; i < 5; i++ ) {
            tCnt[i] = tImp - timesOnDesc[i + 1].timeOn - time0;
        } // na razie wektory aktywne s� na koncu cyklu. Kazdy cykl startuje z 00000

        //teraz odtwarzamy wektory. To jest te� potrzebne do wyliczenia czas�w do prze�aczenia

        //sortowanie od najkr�tszego  do najd�u�szego czasu trwania jedynki - im kr�tsza tym jest ona p�niej
        qsort( &timesOnDesc[1], 5, sizeof (timeOnDesc ), cmpTimeOn ); //alternatywna metoda sortowania

        //usuwamy ostatni wektor (to jest wektor 11111)

        //wektory s� odtwarzane od ko�ca
        //wektor 11111 jest pi�ty
        //wyzerowanie numer�w wektor�w
        for( int i = 1; i <= 5; i++ ) {
            outVect[i].w = 0;
        }

        t[5] = timesOnDesc[1].timeOn;
        for( int i = 1; i <= 5; i++ ) {
            timesOnDesc[i].timeOn -= t[5];
            if ( timesOnDesc[i].timeOn >= 0 ) {
                outVect[5].w |= 1 << ( 5 - timesOnDesc[i].faza );
            }
        }

        t[4] = timesOnDesc[2].timeOn;
        for( int i = 2; i <= 5; i++ ) {
            timesOnDesc[i].timeOn -= t[4];
            if ( timesOnDesc[i].timeOn >= 0 ) {
                outVect[4].w |= 1 << ( 5 - timesOnDesc[i].faza );
            }
        }

        t[3] = timesOnDesc[3].timeOn;
        for( int i = 3; i <= 5; i++ ) {
            timesOnDesc[i].timeOn -= t[3];
            if ( timesOnDesc[i].timeOn >= 0 ) {
                outVect[3].w |= 1 << ( 5 - timesOnDesc[i].faza );
            }
        }

        t[2] = timesOnDesc[4].timeOn;
        for( int i = 4; i <= 5; i++ ) {
            timesOnDesc[i].timeOn -= t[2];
            if ( timesOnDesc[i].timeOn >= 0 ) {
                outVect[2].w |= 1 << ( 5 - timesOnDesc[i].faza );
            }
        }

        t[1] = timesOnDesc[5].timeOn;
        for( int i = 5; i <= 5; i++ ) {
            timesOnDesc[i].timeOn -= t[1];
            if ( timesOnDesc[i].timeOn >= 0 ) {
                outVect[1].w |= 1 << ( 5 - timesOnDesc[i].faza );
            }
        }

        //wyczyszczenie wektor�w o zerowej d�ugo�ci (przyjmij poprzedni)
        for( int i = 2; i <= 5; i++ ) {
            if ( t[i] == 0 ) {
                outVect[i].w = outVect[i - 1].w;
            }
        }

        /// w tym miejscu jest sekwencja 00000-a1-a2-a3-a4-11111 przy minimalnej liczbie przelaczen poiedzy kolejnymi wektorami

        //czasy paswyne
        t[0] = 0.5 * ( tImp - ( t[1] + t[2] + t[3] + t[4] ) );
        vect[0].w = 0;
        vect[5].w = 31;

        /*
                for( int i = 1; i <= 30; i++ ) {
                    ux1out[i] = ux1[i] * k_ud; //wartosci skladowych wektorow
                    uy1out[i] = uy1[i] * k_ud;
                    ux3out[i] = ux3[i] * k_ud;
                    uy3out[i] = uy3[i] * k_ud;
                }
         */

        //wygenerowane skladowe w ukladzie wsp.1 i 3
        /*
         *usx1wyg = ( t[1] * ux1out[outVect[1].w] + t[2] * ux1out[outVect[2].w] + t[3] * ux1out[outVect[3].w] + t[4] * ux1out[outVect[4].w] ) / tImp;
         *usy1wyg = ( t[1] * uy1out[outVect[1].w] + t[2] * uy1out[outVect[2].w] + t[3] * uy1out[outVect[3].w] + t[4] * uy1out[outVect[4].w] ) / tImp;
         *usx3wyg = ( t[1] * ux3out[outVect[1].w] + t[2] * ux3out[outVect[2].w] + t[3] * ux3out[outVect[3].w] + t[4] * ux3out[outVect[4].w] ) / tImp;
         *usy3wyg = ( t[1] * uy3out[outVect[1].w] + t[2] * uy3out[outVect[2].w] + t[3] * uy3out[outVect[3].w] + t[4] * uy3out[outVect[4].w] ) / tImp;
         */

        cykl *= -1;

        *przerwanie = 1; //pozwala na uruchomienie sterowania
    }

    /*
    if ( cykl > 0 ) {
        for( int i = 0; i < 5; i++ ) {
            stanOut[i] = ( impuls <= tCnt[i] ) ? 0 : 1;
        }
    } else {
        for( int i = 0; i < 5; i++ ) {
            stanOut[i] = ( impuls <= tImp - tCnt[i] ) ? 1 : 0;
        }
    }
    int bits = stanOut[0] << 4 | stanOut[1] << 3 | stanOut[2] << 2 | stanOut[3] << 1 | stanOut[4];
     */

    int bits = 0;
    if ( cykl == 1 ) {
        for( int i = 0; i < 5; i++ ) {
            bits <<= 1;
            bits |= ( impuls <= tCnt[i] ) ? 0 : 1;
        }
    } else {
        for( int i = 0; i < 5; i++ ) {
            bits <<= 1;
            bits |= ( impuls <= tImp - tCnt[i] ) ? 1 : 0;
        }
    }
    *usx1 = k_ud * ux1[bits];
    *usy1 = k_ud * uy1[bits];
    *usx3 = k_ud * ux3[bits];
    *usy3 = k_ud * uy3[bits];

    impuls += h;
    if ( impuls > tImp + h ) { //ob1?
        impuls = 0; //czyli co okres impulsowania
    }
}