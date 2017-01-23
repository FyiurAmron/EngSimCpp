#include "stdafx.h"
#include "MPoles.h"

void MPoles::OnCalculate( ) {
    /*int i=1000000,j=2;
    for(;i>0;i--)
    {
        j*=1.1;
    }*/
    Update( );
    SaveAsSignal( );
}

void MPoles::NewTick( ) {
    ;
}

void MPoles::GetPoles( ) {
    arma::eig_gen( eig_vals, eig_vecs, A );
    for( int i = 0; i < size; i++ ) {
        eig_values[0][i] = eig_vals[i].real( );
        eig_values[1][i] = eig_vals[i].imag( );
    }

    arma::eig_gen( eig_vals, eig_vecs, B );
    for( int i = 0; i < size; i++ ) {
        eig_values2[0][i] = eig_vals[i].real( );
        eig_values2[1][i] = eig_vals[i].imag( );
    }
}

void MPoles::SetSize( int size ) {
    A.set_size( size, size );
    B.set_size( size, size );
    eig_values[0] = new double[size];
    eig_values[1] = new double[size];
    eig_values2[0] = new double[size];
    eig_values2[1] = new double[size];
    this->size = size;
}

void MPoles::Update( ) {
    double w = obs->Ls * obs->Lr - obs->Lm * obs->Lm;
    double a1 = -( obs->Rs * obs->Lr * obs->Lr + obs->Rr * obs->Lm * obs->Lm ) / obs->Lr / w;
    double a2 = obs->Rr * obs->Lm / obs->Lr / w;
    double a3 = -obs->Lm / w;
    double a4 = obs->Lr / w;
    double a5 = obs->Rr * obs->Lm / obs->Lr;
    double a6 = -obs->Rr / obs->Lr;
    double a7 = obs->Lm / obs->Lr;

    if ( wr_ptr ) wr = *wr_ptr;
    if ( T_ptr ) T = *T_ptr;
    if ( frq_ptr ) frq = *frq_ptr;


    if ( size != 6 ) SetSize( 6 );

    isd = frq / obs->Lm;
    isq = T / a7 / frq;
    //wa=wr;
    //wa=0;
    //wa=wr+a5*(0*isq-frq*isd)/frq/frq;
    wa = wr + a5 * isq / frq;
    frd = frq;

    //A(0,0)=a1*(1.0-obs->k13);
    //A(0,1)=obs->k14*a1+wa;
    //A(0,2)=a2;
    //A(0,3)=-a3*wr;
    //A(0,4)=-obs->k11*a3;
    //A(0,5)=-a3*(1-obs->k12);

    //A(1,0)=-(obs->k14*a1+wa+a3*a5*obs->k24);
    //A(1,1)=a1*(1-obs->k13)+a3*a5*(obs->k23-1.0);
    ////A(1,2)=a3*(2.0*wr-wa);
    ////A(1,2)=a3*(wr-a5*isq/frq);
    //A(1,2)=a3*(wr-a5*3*isq/frq);
    ////A(1,3)=a2+a3*a5*(isd+2.0*isq)/frq;
    //A(1,3)=a2+a3*a5*(isd)/frq;
    //A(1,4)=a3*(1.0-obs->k12+obs->k22);
    //A(1,5)=-a3*(obs->k11-obs->k21);

    ////A(1,0)=-(obs->k14*a1+wa);
    ////A(1,1)=a1*(1-obs->k13);
    ////A(1,2)=a3*(wr);
    ////A(1,3)=a2;
    ////A(1,4)=a3*(1.0-obs->k12);
    ////A(1,5)=-a3*(obs->k11);

    //A(2,0)=a5*(1-obs->k23);
    //A(2,1)=obs->k24*a5;
    //A(2,2)=a6;
    //A(2,3)=-(wr-wa);
    //A(2,4)=-obs->k21;
    //A(2,5)=-(1.0-obs->k22);

    //A(3,0)=0;
    //A(3,1)=0;
    ////A(3,2)=2.0*(wr-wa);
    ////A(3,2)=(wr-wa)-a5*isq/frq;
    //A(3,2)=(wr-wa)-a5*3*isq/frq;
    ////A(3,3)=a6+a5*(isd+2.0*isq)/frq;
    //A(3,3)=a6+a5*(isd)/frq;
    //A(3,4)=1;
    //A(3,5)=0;

    ////A(3,0)=-obs->k24*a5;
    ////A(3,1)=a5*(1-obs->k23);
    ////A(3,2)=(wr-wa);
    ////A(3,3)=a6;
    ////A(3,4)=(1.0-obs->k22);
    ////A(3,5)=-obs->k21;

    //A(4,0)=a5*(wr*obs->k23-a6*obs->k33);
    //A(4,1)=-a5*(wr*obs->k24-a6*obs->k34);
    //A(4,2)=0;
    //A(4,3)=0;
    //A(4,4)=a6+wr*(obs->k21-obs->k31);
    //A(4,5)=-(wr*(obs->k22-obs->k32)-wa);

    //A(5,0)=a5*(wr*obs->k24-a6*obs->k34);
    //A(5,1)=a5*(wr*obs->k23-a6*obs->k33);
    //A(5,2)=0;
    //A(5,3)=0;
    //A(5,4)=wr*(obs->k22-obs->k32)-wa;
    //A(5,5)=a6+wr*(obs->k21-obs->k31);

    //A*=314;

    //nowy blad:

    if ( 1 ) {
        A( 0, 0 ) = ( a1 - obs->k13 * a1 );
        A( 0, 1 ) = obs->k14 * a1 + wa;
        A( 0, 2 ) = a2;
        A( 0, 3 ) = -obs->k12 * a3*wr;
        A( 0, 4 ) = 0;
        A( 0, 5 ) = a3 - obs->k12*a3;

        A( 1, 0 ) = -obs->k14 * a1 - wa;
        A( 1, 1 ) = ( a1 - obs->k13 * a1 );
        A( 1, 2 ) = 0;
        A( 1, 3 ) = a2 + obs->k11 * a3*wr;
        A( 1, 4 ) = -a3;
        A( 1, 5 ) = obs->k11*a3;

        A( 2, 0 ) = a5 - obs->k23*a5;
        A( 2, 1 ) = obs->k24*a5;
        A( 2, 2 ) = a6;
        A( 2, 3 ) = -obs->k22 * wr + wa;
        A( 2, 4 ) = 0;
        A( 2, 5 ) = 1 - obs->k22;

        A( 3, 0 ) = -obs->k24*a5;
        A( 3, 1 ) = a5 - obs->k23*a5;
        A( 3, 2 ) = 0 - wa;
        A( 3, 3 ) = a6 + obs->k21*wr;
        A( 3, 4 ) = -1;
        A( 3, 5 ) = obs->k21;

        A( 4, 0 ) = obs->k33 * a5 * a6 - a5*wr;
        A( 4, 1 ) = -obs->k34 * a5*a6;
        A( 4, 2 ) = a5 * wr * isd / frd;
        A( 4, 3 ) = obs->k32*wr;
        A( 4, 4 ) = a6 + a5 * isd / frd;
        A( 4, 5 ) = obs->k32 - wr + wa;

        A( 5, 0 ) = obs->k34 * a5*a6;
        A( 5, 1 ) = obs->k33 * a5 * a6 - a5*wr;
        A( 5, 2 ) = a5 * wr * isd / frd + wr*wr;
        A( 5, 3 ) = -obs->k31*wr;
        A( 5, 4 ) = a5 * isd / frd + 2 * wr - wa;
        A( 5, 5 ) = a6 - obs->k31;
    } else {
        A( 0, 0 ) = ( a1 - obs->k13 * a1 );
        A( 0, 1 ) = obs->k14*a1; //+wa;
        A( 0, 2 ) = a2;
        A( 0, 3 ) = 0;
        A( 0, 4 ) = obs->k11*a3;
        A( 0, 5 ) = a3 - obs->k12*a3;

        A( 1, 0 ) = -obs->k14 * a1 - wa;
        A( 1, 1 ) = ( a1 - obs->k13 * a1 );
        A( 1, 2 ) = 0;
        A( 1, 3 ) = a2;
        A( 1, 4 ) = -a3 + obs->k12*a3;
        A( 1, 5 ) = obs->k11*a3;

        A( 2, 0 ) = a5 - obs->k23*a5;
        A( 2, 1 ) = obs->k24*a5;
        A( 2, 2 ) = a6;
        A( 2, 3 ) = 0; //+wa;
        A( 2, 4 ) = obs->k21;
        A( 2, 5 ) = 1 - obs->k22;

        A( 3, 0 ) = -obs->k24*a5;
        A( 3, 1 ) = a5 - obs->k23*a5;
        A( 3, 2 ) = 0; //-wa;
        A( 3, 3 ) = a6;
        A( 3, 4 ) = -1 + obs->k22;
        A( 3, 5 ) = obs->k21;

        A( 4, 0 ) = obs->k33 * a5*a6;
        A( 4, 1 ) = -obs->k34 * a5*a6;
        A( 4, 2 ) = 0;
        A( 4, 3 ) = 0;
        A( 4, 4 ) = a6 - obs->k31;
        A( 4, 5 ) = obs->k32; //+wa;

        A( 5, 0 ) = obs->k34 * a5*a6;
        A( 5, 1 ) = obs->k33 * a5*a6;
        A( 5, 2 ) = 0;
        A( 5, 3 ) = 0;
        A( 5, 4 ) = -obs->k32; //-wa;
        A( 5, 5 ) = a6 - obs->k31;
    }

    B( 0, 0 ) = a1;
    B( 0, 1 ) = 0;
    B( 0, 2 ) = a2;
    B( 0, 3 ) = 0;
    B( 0, 4 ) = 0;
    B( 0, 5 ) = -a3;

    B( 1, 0 ) = 0;
    B( 1, 1 ) = a1;
    B( 1, 2 ) = 0;
    B( 1, 3 ) = a2;
    B( 1, 4 ) = a3;
    B( 1, 5 ) = 0;

    B( 2, 0 ) = a5;
    B( 2, 1 ) = 0;
    B( 2, 2 ) = a6;
    B( 2, 3 ) = 0;
    B( 2, 4 ) = 0;
    B( 2, 5 ) = -1;

    B( 3, 0 ) = 0;
    B( 3, 1 ) = a5;
    B( 3, 2 ) = 0;
    B( 3, 3 ) = a6;
    B( 3, 4 ) = 1;
    B( 3, 5 ) = 0;

    B( 4, 0 ) = a5*wr;
    B( 4, 1 ) = 0;
    B( 4, 2 ) = 0;
    B( 4, 3 ) = 0;
    B( 4, 4 ) = a6;
    B( 4, 5 ) = -wr;

    B( 5, 0 ) = 0;
    B( 5, 1 ) = a5*wr;
    B( 5, 2 ) = 0;
    B( 5, 3 ) = 0;
    B( 5, 4 ) = wr;
    B( 5, 5 ) = a6;

    //A*=314;
    //B*=314;

    GetPoles( );

    poles_qi = 0;
    for( int i = 0; i < 6; i++ ) {
        if ( eig_values[0][i] >= 0 ) {
            poles_qi += 1000;
            break;
        }
        poles_qi += 1.0 / eig_values[0][i];
    }

}

void MPoles::SaveAsSignal( ) {
    signal->Clear( );
    signal2->Clear( );
    for( int i = 0; i < size; i++ ) {
        signal->AddSample( eig_values[0][i], eig_values[1][i] );
        signal2->AddSample( eig_values2[0][i], eig_values2[1][i] );
    }
}