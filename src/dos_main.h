#ifndef DOS_MAIN_H
#define DOS_MAIN_H


extern double TIME_s, TIME;

extern double temp, tempa, tempb, tempc, tempdelta;
extern double uzas, u1, u2, izas, i1, i2, Ei2, i2zad, kp1, ki1, k;
extern double gamma, fi, a0, a1, Lo;
extern double t, tau; /* czas,czas wzgledny */
extern double timebe;
extern double omegaR;
extern double m0; /* moment obciazenia */

struct wykres {
    char *co;
    double *war;
};

struct stan {
    double t; /* czas w symulowanym ukladzie*/
    double h; /* krok calkowania */
    double omega, omega2; /* predkosc walu_wirnika silnika, sterowania*/
    double tetaw; /* kat obrotu sprowadzany do przedzialu [0,2Pi) */
};

int dos_main( );

#undef IN_MAIN
#endif //DOS_MAIN_H