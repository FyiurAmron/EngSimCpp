#include "stdafx.h"
#include "dos2win.h"

MDosSim *dos_model;

void main_mon() {
    dos_model->UpdateDosTime( );
    while( dos_model->dos_time_real >= dos_model->sim->t );
}