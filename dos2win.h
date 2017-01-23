#ifndef DOS2WIN_H
#define DOS2WIN_H


#define _USE_MATH_DEFINES
#include "MDosSim.h"

extern MDosSim *dos_model;

inline double dosT2realT(double dosT);
void main_mon();
void clrscr();
void gotoxy(int,int);
//void printf(char*,...);

#endif