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
double uSx1, uSy1, uSX3, uSY3;

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
#define SCALE_FACTOR  0.5

inline double fux1( int tA, int tB, int tC, int tD, int tE ) {
    //return 0.2*((6+2*cos72+2*cos144)*(Ta-(1-Ta))+((1+7*cos72+2*cos144)*((Tb-(1-Tb))+(Te-(1-Te))))+((1+2*cos72+7*cos144)*((Tc-(1-Tc))+(Td-(1-Td)))));
    return SCALE_FACTOR
            * ( 0.2 * ( ( 4 - 2 * COS72 - 2 * COS144 ) * ( tA - ( 1 - tA ) )
            + ( ( -1 + 3 * COS72 - 2 * COS144 ) * ( ( tB - ( 1 - tB ) ) + ( tE - ( 1 - tE ) ) ) )
            + ( ( -1 - 2 * COS72 + 3 * COS144 ) * ( ( tC - ( 1 - tC ) ) + ( tD - ( 1 - tD ) ) ) ) ) );
}

inline double fuy1( int tA, int tB, int tC, int tD, int tE ) {
    return SCALE_FACTOR
            * ( ( SIN72 * ( ( tB - ( 1 - tB ) ) - ( tE - ( 1 - tE ) ) ) )
            + ( SIN144 * ( ( tC - ( 1 - tC ) ) - ( tD - ( 1 - tD ) ) ) ) );
}

inline double fux3( int tA, int tB, int tC, int tD, int tE ) {
    //return 0.2*((6+2*cos144+2*cos72)*(Ta-(1-Ta))+((1+7*cos144+2*cos72)*((Tb-(1-Tb))+(Te-(1-Te))))+((1+2*cos144+7*cos72)*((Tc-(1-Tc))+(Td-(1-Td)))));
    return SCALE_FACTOR
            * ( 0.2 * ( ( 4 - 2 * COS144 - 2 * COS72 ) * ( tA - ( 1 - tA ) )
            + ( ( -1 + 3 * COS144 - 2 * COS72 ) * ( ( tB - ( 1 - tB ) ) + ( tE - ( 1 - tE ) ) ) )
            + ( ( -1 - 2 * COS144 + 3 * COS72 ) * ( ( tC - ( 1 - tC ) ) + ( tD - ( 1 - tD ) ) ) ) ) );
}

inline double fuy3( int tA, int tB, int tC, int tD, int tE ) {
    return SCALE_FACTOR
            * ( ( SIN144 * ( ( tB - ( 1 - tB ) ) - ( tE - ( 1 - tE ) ) ) )
            - ( SIN72 * ( ( tC - ( 1 - tC ) ) - ( tD - ( 1 - tD ) ) ) ) );
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
#include <time.h>
#include <stdlib.h>
#define PHI  0.618
//#define K_VEC  PHI*PHI
#define K_VEC  PHI * 0.9
// przybliżona wartość analityczna

    //static int baseVec[] = { 0b10000, 0b01000, 0b00100, 0b00010, 0b00001 };
    //static int baseVec[] = { 0b01001, 0b01000, 0b00100, 0b00010, 0b00001 }; // 2-vec compensation

int baseVecNr = 0;

extern double is[];

#define PHASE_DAM  0 // fault faza 1

#define BASE_VEC_COUNT  10
int baseVecAll[4][BASE_VEC_COUNT] = {
    { 0b10000, 0b11000, 0b01000, 0b01100, 0b00100, 0b00110, 0b00010, 0b00011, 0b00001, 0b10001 }, // normalne
    { 0b00000, 0b01000, 0b01000, 0b01100, 0b00100, 0b00110, 0b00010, 0b00011, 0b00001, 0b00001 }, // fault faza 1
    //{ 0b10000, 0b10000, 0b00000, 0b00100, 0b00100, 0b00110, 0b00010, 0b00011, 0b00001, 0b10001 }, // fault faza 2
    
    //{ 0b10000, 0b11000, 0b01000, 0b01100, 0b00100, 0b00100, 0b00000, 0b00001, 0b00001, 0b10001 }, // fault faza 4
    //{ 0b10000, 0b11000, 0b01000, 0b01100, 0b00100, 0b00110, 0b00010, 0b00010, 0b00000, 0b10000 }, // fault faza 5

    { 0b01001, -1, 0b01000, 0b01100, 0b00100, 0b00110, 0b00010, 0b00011, 0b00001, -1 }, // kompensacja 1 f1
    //{ 0b01001, -1, 0b00100, 0b00101, 0b00001, 0b01001, 0b01000, 0b01010, 0b00010, -1 }, // kompensacja 2 f1
    { -1, -1, 0b01000, 0b01100, 0b00100, 0b00110, 0b00010, 0b00011, 0b00001, -1 }, // kompensacja alt.
};

void PWM5f( double tImp, double uS1, double rhoU1, double uS3, double rhoU3, double ud, double h,
        double *usx1, double *usy1, double *usx3, double *usy3,
        double *usx1wyg, double *usy1wyg, double *usx3wyg, double *usy3wyg,
        bool *przerwanie ) {
    static bool initNeeded = true;
    static double t;
    static int kierunek = 1;

    int* baseVec = baseVecAll[baseVecNr];

    if ( initNeeded ) {
        for( int i = 0; i < 31; i++ ) {
            ux1[i] = fux1( bit( i, 4 ), bit( i, 3 ), bit( i, 2 ), bit( i, 1 ), bit( i, 0 ) );
            uy1[i] = fuy1( bit( i, 4 ), bit( i, 3 ), bit( i, 2 ), bit( i, 1 ), bit( i, 0 ) );
            //printf( "[%d] %f %f\n", i, ux1[i], uy1[i] );
            ux3[i] = fux3( bit( i, 4 ), bit( i, 3 ), bit( i, 2 ), bit( i, 1 ), bit( i, 0 ) );
            uy3[i] = fuy3( bit( i, 4 ), bit( i, 3 ), bit( i, 2 ), bit( i, 1 ), bit( i, 0 ) );
        }
        ux1[31] = 0;
        uy1[31] = 0;
        ux3[31] = 0;
        uy3[31] = 0; //11111

        initNeeded = false;
    }

    //double k_ud = ud * HALF_SQRT_2DIV5;
    //double k_ud = ud / 2;

    if ( t == 0 ) {
        rhoU1 = wrapAngle( rhoU1 );
        rhoU3 = wrapAngle( rhoU3 );

        uS1 /= ud;
        uSx1 = uS1 * cos( rhoU1 );
        uSy1 = uS1 * sin( rhoU1 );
        //uSX3 = uS3 * cos( rhoU3 );
        //uSY3 = uS3 * sin( rhoU3 );

        //printf("%f\n", rhoU1);
        int section = (int) ( rhoU1 / ( 2 * M_PI / BASE_VEC_COUNT ) );
        //printf("%d\n", part);
        if ( baseVecNr == 1 && baseVecAll[0][section] ^ baseVecAll[1][section] ) { // symulacja uszkodzenia w sekcji z uszkodzona faza
            if ( is[PHASE_DAM] < 0 ) {
                baseVec = baseVecAll[0]; // nie widac uszkodzenia
            }
        }
        int vec1 = baseVec[section];
        int offset = 0;
        while ( vec1 == -1 ) {
            offset++;
            vec1 = baseVec[( section - offset + BASE_VEC_COUNT ) % BASE_VEC_COUNT];
        }
        int vec2 = baseVec[( section + 1 ) % BASE_VEC_COUNT];
        offset = 1;
        while ( vec2 == -1 ) {
            offset++;
            vec2 = baseVec[( section + offset ) % BASE_VEC_COUNT];
        }
        double x1 = ux1[vec1], x2 = ux1[vec2];
        double y1 = uy1[vec1], y2 = uy1[vec2];
        //printf( "%d %d %f\n", vec1, vec2, ux1[vec1]*ux1[vec1] + uy1[vec1]*uy1[vec1] );

        // inverse matrix
        double tImp_detInv = tImp / ( x1 * y2 - x2 * y1 );
        double t1 = tImp_detInv * ( uSx1 * y2 - uSy1 * x2 ) * K_VEC;
        double t2 = tImp_detInv * ( uSy1 * x1 - uSx1 * y1 ) * K_VEC;

        double t0 = 1.0 - t1 - t2;

        //printf( "[%f] {%f %f} in {%f %f}, {%f %f} [%d %d] {%f %f %f}\n", rhoU1, uSx1, uSy1, x1, y1, x2, y2, vec1, vec2, t0, t1, t2 );
        //printf( "[%f] {%f %f} in {%f %f}, {%f %f} [%d %d] {%f %f}\n", rhoU1, uSx1, uSy1, x1, y1, x2, y2, vec1, vec2, t1, t2 );

        for( int i = 0; i < 5; i++ ) {
            int mask = 1 << i;
            if ( vec1 & mask ) {
                tCnt[i] = t1;
            } else {
                tCnt[i] = 0;
            }
            if ( vec2 & mask ) {
                tCnt[i] += t2;
            }
            if ( tCnt[i] == 0 ) {
                tCnt[i] = -tImp;
            }
        }

        kierunek *= -1;
        *przerwanie = 1; // uruchamia sterowanie
#ifdef VERBOSE
        FILE* fpOut = fopen( "out.txt", "a" );
        fprintf( fpOut, "*** %d\n", kierunek );
        for( int i = 4; i >= 0; i-- ) {
            fprintf( fpOut, "%f ", tCnt[i] );
        }
        fprintf( fpOut, "\n***\n" );
        fclose( fpOut );
#endif
    }

    int bits = 0;
    int bitz[5];

    if ( kierunek == 1 ) {
        for( int i = 0; i < 5; i++ ) {
            bitz[i] = ( t > tImp - tCnt[i] ) ? 1 : 0;
        }
    } else {
        for( int i = 0; i < 5; i++ ) {
            bitz[i] = ( t < tCnt[i] ) ? 1 : 0;
        }
    }

    bits = 0;
    for( int i = 4; i >= 0; i-- ) {
        bits <<= 1;
        bits |= bitz[i];
    }

#ifdef VERBOSE
#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c"/*"%c%c%c"*/
#define BYTE_TO_BINARY(byte)  \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0')
    FILE* fpOut = fopen( "out.txt", "a" );
    fprintf( fpOut, BYTE_TO_BINARY_PATTERN "\n", BYTE_TO_BINARY( bits ) );
    fclose( fpOut );
#endif

    *usx1 = ud * ux1[bits];
    *usy1 = ud * uy1[bits];
    *usx3 = ud * ux3[bits];
    *usy3 = ud * uy3[bits];

    if ( t > tImp ) {
        t = 0;
    } else {
        t += h;
    }
}