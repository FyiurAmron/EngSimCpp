#ifndef SIMULATION_H
#define SIMULATION_H

#include "CSignal.h"
#include "CModel.h"
#include <windows.h>

#include <map>
#include <vector>
#include <iterator>

#define SIM_STATE_PAUSED 2
#define SIM_STATE_FINISHED 3
#define SIM_STATE_CALCULATION 1
#define SIM_STATE_READY 0

enum SimCalcReason {
    SimCalcReasonOrdered, SimCalcReasonRegular
};

enum SimStart {
    SimStartWithInit, SimStartReset
};

class CSimulation {
    int pause;

    static std::map<UINT_PTR, CSimulation *> timerMap;

    static void CALLBACK TimerProc( HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime );
    static DWORD WINAPI ThreadedCalc( void *args );
    void OnCalculate( );

    HANDLE threadHandle;
    int terminateThread;
    int nTicksToSkip;
    std::vector<double> vOrderedCalcTimes;
    std::vector<double>::iterator vOrderedCalcTimesIterator;
protected:

    virtual void ModelCalc( ) {
        ;
    }
    void InitSys( unsigned int nX, unsigned int nU, unsigned int nEqs );



public:
    vector<CSignal> signals;
    volatile double t;
    double t_start;
    double t_end;
    double t_step;
    double t_prev_regular;
    int nTicks, nTicksOrdered;
    int nDefaultSignalSize;
    volatile bool safeToTerminate; //okresla czy mozna zkonczyc watek symulacji

    double *x;
    double *u;

    int calcInterval;
    int nCalcPerInterval;

    SimCalcReason calcReason;

    vector<CModel*> models;

    CSimulation( ) {
        t_step = 1e-6;
        t_start = 0;
        t_end = 10;
        t = 0;
        calcInterval = -1;
        terminateThread = 0;
        nTicks = 0;
        nTicksOrdered = 0;
        t_prev_regular = 0;
        nDefaultSignalSize = SIGNAL_DEF_LENGTH;
        safeToTerminate = 1;
    }
    int RegisterSignal( const double *y, wstring name, double offset = 0, double scale = 1 );
    int RegisterSignal( const double *x, const double *y, wstring name, double offset = 0, double scale = 1 );
    void AddModel( CModel *model );
    void InitTimer( UINT nMiliSeconds );

    void StartSimulation( SimStart start = SimStartWithInit );
    void EndSimulation( );
    void PauseSimulation( );
    void ResumeSimulation( );
    //void ModelCalc2(double t);

    /// <summary>Definiuje kroki oraz czas maksymalne czas symulacji</summary>
    ///	<param name="calcStep">maksymalny krok obliczeń</param>
    ///	<param name="samplesStep">maksymalny okres między kolejnymi zapisanymi próbkami</param>
    ///	<param name="simTime">przemieszczenie na osi x w jednostkach wykresu</param>
    void SetStep( double calcStep, double samplesStep, double simTime );
    void ResetSimulation( );

    virtual void PreCalc( ) {
        ;
    } //wywolywane co kazdy krok przed obliczniami modeli

    virtual void PostCalc( ) {
        ;
    } //wywolywane co kazdy krok po obliczniach modeli

    int OrderCalc( double time );
    int OrderCalcIn( double period );
    void RemoveOrederedCalc( ); //usuwa najwczesniejszy czas

    void SetParameterValue( double *ptr, double val );

    CSignal* FindSignal( wstring name );
};

#endif
