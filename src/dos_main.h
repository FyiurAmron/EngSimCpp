#ifndef DOS_MAIN_H
#define DOS_MAIN_H

extern double TIME_s, TIME;

#if 0
extern double temp, tempa, tempb, tempc, tempdelta;
extern double uzas, u1, u2, izas, i1, i2, Ei2, i2zad, kp1, ki1, k;
extern double gamma, fi, a0, a1, Lo;
extern double t, tau; /* czas,czas wzgledny */
extern double timebe;
extern double omegaR;
extern double m0; /* moment obciazenia */
#endif

extern double x11, x12, x21, x22;
extern double x11z, x12z, x21z, x22z;

int dos_main( );

#undef IN_MAIN
#endif //DOS_MAIN_H