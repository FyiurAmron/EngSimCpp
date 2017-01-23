#ifndef MPOLES_H
#define MPOLES_H

#include "CModel.h"
#include "MIndMachObs.h"
#define ARMA_USE_LAPACK
#define ARMA_USE_BLAS
#include <armadillo/armadillo.h>

class MPoles : public CModel {
public:
    int size;

    arma::mat A, B;
    arma::cx_vec eig_vals;
    arma::cx_mat eig_vecs;

    double *eig_values[2], *eig_values2[2];

    MIndMachObs *obs;
    CSignal *signal, *signal2;


    double wa, wr, T;
    double isd, isq, frd, frq;

    double poles_qi;

    double *wr_ptr, *T_ptr, *frq_ptr;

    MPoles( CSimulation *sim ) :
    CModel( sim, 0 ) {
        wr = 1;
        T = 0.9;
        frq = 0.75;
    }

    void GetPoles( );
    void SetSize( int size );
    void Update( );
    void OnCalculate( );
    void NewTick( );
    void SaveAsSignal( );
};

#endif