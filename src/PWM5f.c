#include <math.h>
#include <stdbool.h>

#define SIN72  0.95105651629515357211643933337938 //sin 2PI/5
#define COS72  0.30901699437494742410229341718282 //cos 2PI/5
#define SIN144     0.58778525229247312916870595463907 //sin 4PI/5
#define COS144    -0.80901699437494742410229341718282 //cos 4PI/5
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

inline double fux1( int tA, int tB, int tC, int tD, int tE ) {
    //return 0.2*((6+2*cos72+2*cos144)*(Ta-(1-Ta))+((1+7*cos72+2*cos144)*((Tb-(1-Tb))+(Te-(1-Te))))+((1+2*cos72+7*cos144)*((Tc-(1-Tc))+(Td-(1-Td)))));
    return 0.2 * ( ( 4 - 2 * COS72 - 2 * COS144 ) * ( tA - ( 1 - tA ) )
            + ( ( -1 + 3 * COS72 - 2 * COS144 ) * ( ( tB - ( 1 - tB ) ) + ( tE - ( 1 - tE ) ) ) )
            + ( ( -1 - 2 * COS72 + 3 * COS144 ) * ( ( tC - ( 1 - tC ) ) + ( tD - ( 1 - tD ) ) ) ) );
}

inline double fuy1( int tA, int tB, int tC, int tD, int tE ) {
    return ( SIN72 * ( ( tB - ( 1 - tB ) ) - ( tE - ( 1 - tE ) ) ) )
            + ( SIN144 * ( ( tC - ( 1 - tC ) ) - ( tD - ( 1 - tD ) ) ) );
}

inline double fux3( int tA, int tB, int tC, int tD, int tE ) {
    //return 0.2*((6+2*cos144+2*cos72)*(Ta-(1-Ta))+((1+7*cos144+2*cos72)*((Tb-(1-Tb))+(Te-(1-Te))))+((1+2*cos144+7*cos72)*((Tc-(1-Tc))+(Td-(1-Td)))));
    return 0.2 * ( ( 4 - 2 * COS144 - 2 * COS72 ) * ( tA - ( 1 - tA ) )
            + ( ( -1 + 3 * COS144 - 2 * COS72 ) * ( ( tB - ( 1 - tB ) ) + ( tE - ( 1 - tE ) ) ) )
            + ( ( -1 - 2 * COS144 + 3 * COS72 ) * ( ( tC - ( 1 - tC ) ) + ( tD - ( 1 - tD ) ) ) ) );
}

inline double fuy3( int tA, int tB, int tC, int tD, int tE ) {
    return ( SIN144 * ( ( tB - ( 1 - tB ) ) - ( tE - ( 1 - tE ) ) ) )
            - ( SIN72 * ( ( tC - ( 1 - tC ) ) - ( tD - ( 1 - tD ) ) ) );
}


inline int cmp( double a, double b ) {
    return ( a > b ) - ( a < b );
}

int cmpTimeOn( const void * a, const void * b ) {
    return cmp( ( (timeOnDesc*) a )->timeOn, ( (timeOnDesc*) b )->timeOn );
}

int cmpTimeOnTmp( const void * a, const void * b ) {
    return cmp( ( (timeOnTmp*) a )->timeOn, ( (timeOnTmp*) b )->timeOn );
}

inline int bit( int val, int bitNr ) {
    return ( val & ( 1 << bitNr ) ) >> bitNr;
}

#define PHASES  5

void PWM5f( double tImp, double uS1, double rhoU1, double uS3, double rhoU3, double ud, double h,
        double *usx1, double *usy1, double *usx3, double *usy3,
        double *usx1wyg, double *usy1wyg, double *usx3wyg, double *usy3wyg,
        bool *przerwanie ) {
    static bool initNeeded = true;
    static double impuls;
    static int cykl = 1;

    int stan[5][5];

    if ( initNeeded ) {
        //wyznaczenie wspolczynnikow skladowych wektorow, ich numery wynikaja z zapisu binarnego (f*2^0+e*2^1+....a*2^0)
        //uwaga - rzeczywiste dlugosci wektorow uzyskujemy po pomnozeniu przez sqrt(2/5)*uDC/2
        //kat 0*36st (uklad wsp 1)
        for( int i = 0; i < 31; i++ ) {
            ux1[i] = fux1( bit( i, 4 ), bit( i, 3 ), bit( i, 2 ), bit( i, 1 ), bit( i, 0 ) );
            uy1[i] = fuy1( bit( i, 4 ), bit( i, 3 ), bit( i, 2 ), bit( i, 1 ), bit( i, 0 ) );
            ux3[i] = fux3( bit( i, 4 ), bit( i, 3 ), bit( i, 2 ), bit( i, 1 ), bit( i, 0 ) );
            uy3[i] = fuy3( bit( i, 4 ), bit( i, 3 ), bit( i, 2 ), bit( i, 1 ), bit( i, 0 ) );
        }
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
        a4x3 = invDenom *
                ( ux1[selVect[1].w] * uy1[selVect[2].w] * uy3[selVect[3].w]
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
        double k = tImp / k_ud; //czasy tu wyliczone dotycz� 4 wybranych wektor_sel�w i mog� mie� dowolne warto�ci (te� ujemne)
        t[1] = ( a1x1 * uSX1 + a1y1 * uSY1 + a1x3 * uSX3 + a1y3 * uSY3 ) * k;
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
            for( int j = 0; j < 5; j++ ) {
                stan[j][i] = bit( selVect2[i].w, 4 - j );
            }
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
        for( int j = 1, k = 5; j <= 5; j++, k-- ) {
            t[k] = timesOnDesc[j].timeOn;
            for( int i = j; i <= 5; i++ ) {
                timesOnDesc[i].timeOn -= t[k];
                if ( timesOnDesc[i].timeOn >= 0 ) {
                    outVect[k].w |= 1 << ( 5 - timesOnDesc[i].faza );
                }
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

        cykl *= -1;

        *przerwanie = 1; //pozwala na uruchomienie sterowania
    }

    int bits = 0;
    if ( cykl == 1 ) {
        for( int i = 0; i < 5; i++ ) {
            bits <<= 1;
            if ( i == PWM_FAULT ) {
                bits |= 0;
            } else if ( i == PWM_FAULT - PHASES ) {
                bits |= 1;
            } else {
                bits |= ( impuls <= tCnt[i] ) ? 0 : 1;
            }
        }
    } else {
        for( int i = 0; i < 5; i++ ) {
            bits <<= 1;
            if ( i == PWM_FAULT ) {
                bits |= 1;
            } else if ( i == PWM_FAULT - PHASES ) {
                bits |= 0;
            } else {
                bits |= ( impuls <= tImp - tCnt[i] ) ? 1 : 0;
            }
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