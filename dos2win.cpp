#include "stdafx.h"
#include "dos2win.h"

MDosSim *dos_model;

volatile int DOS_IS_STILL_CALCULATING=1;
volatile int LOCK_DOS=1;


void main_mon()
{
	dos_model->UpdateDosTime();
	while(dos_model->dos_time_real >= dos_model->sim->t);//{MessageBeep(NULL);}
	
	/*DOS_IS_STILL_CALCULATING=0;
	LOCK_DOS=1;//1
	while(LOCK_DOS);
	DOS_IS_STILL_CALCULATING=1;//1*/
}

void clrscr(){;}
void gotoxy(int x,int y){;}