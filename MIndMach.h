#ifndef MINDMACH_H
#define MINDMACH_H

#include "CModel.h"



class MIndMach : public CModel
{
protected:
	void IndMachDiffFcn(double *dx, double *x);
public:
	double *mo;
	double *usx_m;
	double *usy_m;

	double Rs;
	double Rr;
	double Ls;
	double Lr;
	double Lm;
	double J;
	double wa;
	double w_wz;

	double Te;
	double &isx,&isy,&frx,&fry,&wr;
	double &Disx,&Disy,&Dfrx,&Dfry,&Dwr;

	MIndMach(CSimulation *sim) : 
		CModel(sim,5), isx(x[0]), isy(x[1]), frx(x[2]), fry(x[3]), wr(x[4]), Disx(dx[0]), Disy(dx[1]), Dfrx(dx[2]), Dfry(dx[3]), Dwr(dx[4])
	{
		diffFcn=(DIFF_EQUS_FCN)(&MIndMach::IndMachDiffFcn);
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