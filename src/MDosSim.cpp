#include "stdafx.h"
#include "MDosSim.h"
#include "dos2win.h"

extern volatile int DOS_IS_STILL_CALCULATING;
extern volatile int LOCK_DOS;

void MDosSim::InitDosModel( CSimulation *sim, MAINFUNC main, EVENTFUNC onReset, double *time ) {
    this->main = main;
    dos_time_src = time;
    triggerCalcAlways = 1;
    onResetExt = onReset;
    //this->time = time;
    //thread=_beginthread(DosThread,0,(void *)this);
}

DWORD WINAPI MDosSim::DosThread( void *args ) {
    ( (MDosSim*) args )->main( );

    return 1;
}

void MDosSim::UpdateDosTime( ) {
    dos_time_real = *dos_time_src / 1000.0;
}

void MDosSim::OnCalculate( ) {
    while( dos_time_real < sim->t ) { }
}

void MDosSim::OnEnd( ) {
    DWORD exitCode;
    GetExitCodeThread( thread, &exitCode );
    TerminateThread( thread, exitCode );
}

void MDosSim::OnInit( ) {
    thread = CreateThread( NULL, 0, DosThread, (void *) this, CREATE_SUSPENDED, NULL );
    //ResumeThread(thread);
}

void MDosSim::OnPause( ) {
    SuspendThread( thread );
}

void MDosSim::OnResume( ) {
    ResumeThread( thread );
}

void MDosSim::OnReset( ) {
    //ResumeThread(thread);
    DWORD exitCode;
    GetExitCodeThread( thread, &exitCode );
    TerminateThread( thread, exitCode );

    thread = CreateThread( NULL, 0, DosThread, (void *) this, CREATE_SUSPENDED, NULL );
    ResumeThread( thread );

    *dos_time_src = 0;
    dos_time_real = 0;

    if ( onResetExt ) onResetExt( );
}
