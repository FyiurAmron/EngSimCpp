#include "stdafx.h"
#include "MIndMachObs.h"

void MIndMachObs::IndMachObsDiffFcn( double *dx, double *x ) {
    double w = Ls * Lr - Lm*Lm;
    double a1 = -( Rs * Lr * Lr + Rr * Lm * Lm ) / Lr / w;
    double a2 = Rr * Lm / Lr / w;
    double a3 = -Lm / w;
    double a4 = Lr / w;
    double a5 = Rr * Lm / Lr;
    double a6 = -Rr / Lr;
    double a7 = Lm / Lr;

    //if(frx_est>10)frx_est=10;
    //if(frx_est<-10)frx_est=-10;
    //if(fry_est>10)fry_est=10;
    //if(fry_est<-10)fry_est=-10;

    wr_est = ( frx_est * zetax_est + fry_est * zetay_est ) / ( frx_est * frx_est + fry_est * fry_est );
    zetax_err = ( wr_est * frx_est - zetax_est );
    zetay_err = ( wr_est * fry_est - zetay_est );
    isx_err = ( *isx + 0.000 ) - isx_est;
    isy_err = ( *isy - 0.000 ) - isy_est;

    //if(wr_est>10)wr_est=10;
    //if(wr_est<-10)wr_est=-10;

    //dwr_est=(wr_est-wr_est_prev);
    dwr_est = 0;

    Disx_est = ( a1 * isx_est + a2 * frx_est - a3 * zetay_est + a4 * ( *usx ) + k11 * a3 * zetax_err - k12 * a3 * zetay_err + k13 * a1 * isx_err - k14 * a1 * isy_err ) * w_wz;
    Disy_est = ( a1 * isy_est + a2 * fry_est + a3 * zetax_est + a4 * ( *usy ) + k11 * a3 * zetay_err - k12 * a3 * zetax_err + k13 * a1 * isy_err - k14 * a1 * isx_err ) * w_wz;
    Dfrx_est = ( a5 * isx_est + a6 * frx_est - zetay_est + k21 * zetax_err - k22 * zetay_err + k23 * a5 * isx_err - k24 * a5 * isy_err ) * w_wz;
    Dfry_est = ( a5 * isy_est + a6 * fry_est + zetax_est + k21 * zetay_err - k22 * zetax_err + k23 * a5 * isy_err - k24 * a5 * isx_err ) * w_wz;
    Dzetax_est = ( dwr_est * frx_est + a5 * wr_est * isx_est + a6 * zetax_est - wr_est * zetay_est + k31 * zetax_err - k32 * zetay_err + a5 * a6 * k33 * isx_err - a5 * a6 * k34 * isy_err ) * w_wz;
    Dzetay_est = ( dwr_est * fry_est + a5 * wr_est * isy_est + a6 * zetay_est + wr_est * zetax_est + k31 * zetay_err - k32 * zetax_err + a5 * a6 * k33 * isy_err - a5 * a6 * k34 * isx_err ) * w_wz;

}

void MIndMachObs::OnCalculate( ) {
    DiffSolve( SolverRK4 );
}

void MIndMachObs::OnEnd( ) {
    ;
}

void MIndMachObs::OnInit( ) {
    Rr = .04755;
    Rs = .02797;
    Lr = 1.987;
    Ls = 1.987;
    Lm = 1.95;
    J = 59;

    //k11=1; k12=1; k13=-1; k14=0;
    //k21=0; k22=-1; k23=1; k24=0;
    //k31=0; k32=0; k33=-100; k34=-1000;

    k11 = -2.887721;
    k12 = 1.257090;
    k13 = -12.312011;
    k14 = -1.242348;
    k21 = 1.162127;
    k22 = -0.065045;
    k23 = 4.324311;
    k24 = 19.357347;
    k31 = 4.954680;
    k32 = 26.693511;
    k33 = 924.319956;
    k34 = -2384.553357;

    k11 = -2.887721;
    k12 = -1.257090;
    k13 = -12.312011;
    k14 = 1.242348;
    k21 = 1.162127;
    k22 = 0.065045;
    k23 = 4.324311;
    k24 = -19.357347;
    k31 = 4.954680;
    k32 = -26.693511;
    k33 = 924.319956;
    k34 = 2384.553357;

    w_wz = 50 * 2 * 3.14;
}

void MIndMachObs::OnPause( ) {
    ;
}

void MIndMachObs::OnResume( ) {
    ;
}

void MIndMachObs::OnReset( ) {
    frx_est = 0.01;
    fry_est = 0.01;
}
