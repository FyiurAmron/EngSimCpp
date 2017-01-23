#ifndef DOS_MAIN_H
#define DOS_MAIN_H

#ifdef IN_MAIN
#define _EXT
#else
#define _EXT extern
#endif

//PONIZEJ DEFINIUJEMY WSPOLDZIELONE ZMIENNE

_EXT double TIME_s, TIME;


/*---------------------------------------------*/
/* Deklaracje zmiennych                        */
/*---------------------------------------------*/
_EXT double temp, tempa, tempb, tempc, tempdelta;
_EXT double uzas,u1,u2,izas,i1,i2,Ei2,i2zad,kp1,ki1,k;
_EXT double gamma,fi,a0,a1,Lo;
_EXT double t,tau;   /* czas,czas wzgledny */
_EXT double timebe;
_EXT double omegaR;
_EXT double m0; /* moment obciazenia */

void main();

#undef IN_MAIN
#endif	//DOS_MAIN_H