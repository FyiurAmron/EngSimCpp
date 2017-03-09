#ifndef DOS_MAIN_H
#define DOS_MAIN_H

#if 0
extern double temp, tempa, tempb, tempc, tempdelta;
extern double uzas, u1, u2, izas, i1, i2, Ei2, i2zad, kp1, ki1, k;
extern double gamma, fi, a0, a1, Lo;
extern double t, tau; /* czas,czas wzgledny */
extern double timebe;
extern double omegaR;
extern double m0; /* moment obciazenia */
#endif

#if 0
extern double x11, x12, x21, x22;
extern double x11z, x12z, x21z, x22z;
#endif

extern double usx, usy;
//extern double isx, isy;
extern double is1, is3;
extern double usx1, usy1, usx3, usy3;

extern double rhoU, phiU;

extern double timeCnt;

int dos_main( );

#endif //DOS_MAIN_H