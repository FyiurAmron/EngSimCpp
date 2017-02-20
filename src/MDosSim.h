#ifndef MDOSSIM_H
#define MDOSSIM_H

#include "CModel.h"

typedef int(*MAINFUNC )(void);
typedef void(*EVENTFUNC )(void);

class MDosSim : public CModel {
    //static void DosThread(void *args);
    static DWORD WINAPI DosThread( LPVOID args );
    MAINFUNC main;
    EVENTFUNC onResetExt;
    HANDLE thread;

    double *dos_time_src;
public:
    volatile double dos_time_real;

    MDosSim( CSimulation *sim, MAINFUNC main, double *timeCnt ) :
    CModel( sim, 0 ) {
        InitDosModel( sim, main, NULL, timeCnt );
    }

    MDosSim( CSimulation *sim, MAINFUNC main, EVENTFUNC onReset, double *timeCnt ) :
    CModel( sim, 0 ) {
        InitDosModel( sim, main, onReset, timeCnt );
    }

    void InitDosModel( CSimulation *sim, MAINFUNC main, EVENTFUNC onReset, double *timeCnt );

    void OnCalculate( );
    void OnEnd( );
    void OnInit( );
    void OnPause( );
    void OnResume( );
    void OnReset( );

    void UpdateDosTime( );
};

#endif