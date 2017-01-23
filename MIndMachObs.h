#ifndef MINDMACHOBS_H
#define MINDMACHOBS_H

#include "CModel.h"



class MIndMachObs : public CModel
{
protected:
	void IndMachObsDiffFcn(double *dx, double *x);
public:
	double *usx;
	double *usy;
	double *isx;
	double *isy;

	double Rs;
	double Rr;
	double Ls;
	double Lr;
	double Lm;
	double J;
	double wa;
	double w_wz;

	double k11,k12,k13,k14;
	double k21,k22,k23,k24;
	double k31,k32,k33,k34;

	double Te;
	double wr_est;
	double isx_err,isy_err,frx_err,fry_err,zetax_err,zetay_err,dwr_est;
	double &isx_est,&isy_est,&frx_est,&fry_est,&zetax_est,&zetay_est;
	double &Disx_est,&Disy_est,&Dfrx_est,&Dfry_est,&Dzetax_est,&Dzetay_est;

	MIndMachObs(CSimulation *sim) : 
		CModel(sim,6), isx_est(x[0]), isy_est(x[1]), frx_est(x[2]), fry_est(x[3]), zetax_est(x[4]), zetay_est(x[5]),
		Disx_est(dx[0]), Disy_est(dx[1]), Dfrx_est(dx[2]), Dfry_est(dx[3]), Dzetax_est(dx[4]), Dzetay_est(dx[5])
	{
		diffFcn=(DIFF_EQUS_FCN)(&MIndMachObs::IndMachObsDiffFcn);
	}
	void OnCalculate();
	void OnEnd();
	void OnInit();
	void OnPause();
	void OnResume();
	void OnReset();

	void UpdateDosTime();
};

#endif