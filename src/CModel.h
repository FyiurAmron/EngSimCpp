#include "CSimulation.h"
#include <vector>

#ifndef MODEL_H
#define MODEL_H

#define MODEL_IN double*
#define MODEL_OUT double
#define MODEL_DELAYED_IN CDelayedInput

class CSimulation;
class CModel;
class CDelayedInput;

typedef void(CModel::*DIFF_EQUS_FCN )(double *dx, double *x );

enum SolverType {
    SolverRK4
};

/// <summary>Wejście z opóźnieniem</summary>

class CDelayedInput {
public:
    MODEL_IN in; //wejście (wskaźnik)
    MODEL_OUT delayedIn; //opóźnione wejście

    CDelayedInput( ) {
        delayedIn = 0;
    }
    CDelayedInput( CModel *model );
    void Update( ); //Aktualizuje stan opóźnionego wejścia przyjmując bieżącą wartość wskazywaną przez zmienna 'in'

    MODEL_OUT operator*( ) {
        return delayedIn;
    }

    CDelayedInput & operator=(double *right ) {
        this->in = right;
        return *this;
    }
};

// Klasa zawierające wspólne metody i pola obiektów. Powinna być dziedziczona przez nowe modele.

class CModel {
    /// <summary>Przydziela pamięć dla zmiennych stanu modelu</summary>
    ///	<param name="nStates">liczba zmiennych stanu</param>
    void AssignMemory( int nStates );

protected:
    DIFF_EQUS_FCN diffFcn;

    void DiffSolve( SolverType solver );
    void SolveF4( );

public:
    CSimulation *sim; //wskaźnik do symulacji, w której zarejestruje się model
    int calcTrigger; //określa czy w danym kroku model będzie obliczany (TRUE/FALSE)
    int inheritCalcTrigger; //określa czy kroki, w których wykonane będa obliczenia modelu, są dziedziczone z innego modelu (TRUE/FALSE)
    int triggerCalcAlways; //określa czy obliczenia będą wykonywane w każdym kroku (TRUE/FALSE)
    int nDiffEqs; //liczba równań różniczkowych
    CModel *parentModel; //wskaźnik do modelu nadrzędnego, z którego dziedziczone będą kroki obliczeń
    std::vector<CDelayedInput*> delayedInputs; //wektor zawierający wskaźniki do wszystkich opóźnionych wejść modelu

    double prevCalcTime; //czas symulacji, w którym zakończono poprzedni krok obliczeń
    double orderedCalcTime; //czas, w którym wykonane będzie dodatkowe obliczenie na żądanie
    int orderedCalc; //czy zamówione zostało obliczanie na żądanie (TRUE/FALSE)

    double *x; //tablica z wartościami zmiennych stanu
    double *xTemp; //tablica pomocnicza
    double *dx; //tablica z pochodnymi zmiennych stanu
    double *dxTemp; //tablica pomocnicza

    CModel( ) { }
    CModel( int nDiffEqs );
    CModel( CSimulation *sim, int nDiffEqs = 0 );

    /// <summary>Włącza dziedziczenie kroków obliczeń - obliczenia wykonywane w tych samych krokach co model nadrzędny</summary>
    ///	<param name="parent">wskaźnik do modelu nadrzędnego</param>
    void InheritCalcTrigger( CModel *parent );

    virtual void OnCalculate( ) { }

    virtual void OnReset( ) { }

    virtual void OnInit( ) { }

    virtual void OnPause( ) { }

    virtual void OnResume( ) { }

    virtual void OnEnd( ) { }

    virtual void Calculate( double t ) {
        //UNREFERENCED_PARAMETER( t );
        t = t;
    }

    virtual void Calculate2( double t ) {
        //UNREFERENCED_PARAMETER( t );
        t = t;
    }

    void TriggerCalcOnce( ) {
        calcTrigger = 1;
        inheritCalcTrigger = 0;
        orderedCalc = 0;
        triggerCalcAlways = 0;
    }

    virtual void NewTick( ) {
        ;
    }
    void ReAssignMemory( int nStates );
    double Limit( double val, double min, double max );
    void AddDelayedInput( CDelayedInput *input );
    void UpdateInputs( );
    void UpdateInputsAndCalculate( double period );
    int OrderCalc( double t );
    int OrderCalcIn( double period );
    void Reset( );

    virtual void InitModel( ) {
        ;
    }

    virtual void PrepareToStart( ) {
        ;
    }
};



#endif