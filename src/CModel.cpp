#include "stdafx.h"


#include "CModel.h"
#include "CSimulation.h"

CModel::CModel( int nDiffEqs ) {
    AssignMemory( nDiffEqs );
}

void CModel::DiffSolve( SolverType solver ) {
    switch( solver ) {
        default:
        case SolverRK4:
            SolveF4( ); //calkowanie
            break;
    }
}

void CModel::SolveF4( ) {
    //static double DZ[32];
    static double Q[5][32];

    int i;
    double hf = sim->t - prevCalcTime;

    if ( hf != 0 ) {
        memcpy( xTemp, x, sizeof (x[0] ) * nDiffEqs );

        ( this->*diffFcn )( dxTemp, xTemp );
        for( i = 0; i < nDiffEqs; i += 1 ) {
            //Q[0][i]=xTemp[i];
            Q[1][i] = hf * dx[i];
            xTemp[i] = x[i] + 0.5 * Q[1][i];
        }
        ( this->*diffFcn )( dxTemp, xTemp );
        for( i = 0; i < nDiffEqs; i += 1 ) {
            Q[2][i] = hf * dx[i];
            xTemp[i] = x[i] + 0.5 * Q[2][i];
        }
        ( this->*diffFcn )( dxTemp, xTemp );
        for( i = 0; i < nDiffEqs; i += 1 ) {
            Q[3][i] = hf * dx[i];
            xTemp[i] = x[i] + Q[3][i];
        }
        ( this->*diffFcn )( dxTemp, xTemp );
        for( i = 0; i < nDiffEqs; i += 1 ) {
            Q[4][i] = hf * dx[i];
            x[i] = ( Q[1][i] + Q[4][i] ) / 6 + ( Q[2][i] + Q[3][i] ) / 3 + x[i];
        }
    }
}

CModel::CModel( CSimulation *sim, int nDiffEqs ) {
    AssignMemory( nDiffEqs );
    this->sim = sim;
    parentModel = NULL;
    inheritCalcTrigger = 0;
    prevCalcTime = 0;
    sim->AddModel( this );
}

void CModel::InheritCalcTrigger( CModel *parent ) {
    parentModel = parent;
    if ( parentModel ) {
        inheritCalcTrigger = 1;
    }
}

void CModel::AssignMemory( int nStates ) {
    this->nDiffEqs = nStates;

    x = new double[nDiffEqs];
    for( int i = 0; i < nDiffEqs; i++ ) x[i] = 0;
    xTemp = new double[nDiffEqs];
    dx = new double[nDiffEqs];
    dxTemp = new double[nDiffEqs];
}

void CModel::ReAssignMemory( int nStates ) {
    if ( x ) delete[] x;
    if ( xTemp ) delete[] xTemp;
    if ( dx ) delete[] dx;
    if ( dxTemp ) delete[] dxTemp;

    AssignMemory( nStates );
}

double CModel::Limit( double val, double min, double max ) {
    if ( val > max ) return max;
    if ( val < min ) return min;
    return val;
}

void CModel::AddDelayedInput( CDelayedInput *input ) {
    if ( input ) {
        delayedInputs.push_back( input );
    }
}

void CModel::UpdateInputs( ) {
    for( size_t i = 0; i < delayedInputs.size( ); i++ ) {
        delayedInputs[i]->Update( );
    }
}

void CModel::UpdateInputsAndCalculate( double period ) {
    UpdateInputs( );
    orderedCalc = 1;
    orderedCalcTime = sim->t + period;
    sim->OrderCalc( orderedCalcTime );
}

int CModel::OrderCalc( double t ) {
    if ( sim->OrderCalc( t ) ) {
        orderedCalc = 1;
        orderedCalcTime = t;
        return 1;
    }
    return 0;
}

int CModel::OrderCalcIn( double period ) {
    if ( sim->OrderCalcIn( period ) ) {
        orderedCalc = 1;
        orderedCalcTime = sim->t + period;
        return 1;
    }
    return 0;
}

void CModel::Reset( ) {
    prevCalcTime = 0;
    for( int i = 0; i < nDiffEqs; i++ ) x[i] = 0;
    orderedCalcTime = 0;
    orderedCalc = 0;
    for( size_t i = 0; i < delayedInputs.size( ); i++ ) {
        delayedInputs[i]->delayedIn = 0;
    }
    InitModel( );
    OnReset( );
}

CDelayedInput::CDelayedInput( CModel *model ) {
    model->AddDelayedInput( this );
}

void CDelayedInput::Update( ) {
    delayedIn = *in;
}