#include "stdafx.h"
#include "MIndMach.h"


void MIndMach::IndMachDiffFcn(double *dx, double *x)
{
	double w=Ls*Lr-Lm*Lm;
	double a1=-(Rs*Lr*Lr+Rr*Lm*Lm)/Lr/w;
	double a2=Rr*Lm/Lr/w;
	double a3=-Lm/w;
	double a4=Lr/w;
	double a5=Rr*Lm/Lr;
	double a6=-Rr/Lr;
	double a7=Lm/Lr;
	//dx[0]=-x[0]-1;

	if(sim->t>2000e-6)
	{
		sim->t+=0;
	}

	Disx=(a1*isx+a2*frx+wa*isy-a3*wr*fry+a4*(*usx_m))*w_wz;
	Disy=(a1*isy+a2*fry-wa*isx+a3*wr*frx+a4*(*usy_m))*w_wz;
	Dfrx=(a6*frx+(wa-wr)*fry+a5*isx)*w_wz;
	Dfry=(a6*fry-(wa-wr)*frx+a5*isy)*w_wz;
	Te=Lm/Lr*(frx*isy-fry*isx);
	Dwr=((Te-*mo)/J)*w_wz;
}


void MIndMach::OnCalculate()
{
	DiffSolve(SolverRK4);
}

void MIndMach::OnEnd()
{
	;
}

void MIndMach::OnInit()
{
	Rr=.04755;
	Rs=.02797;
	Lr=1.987;
	Ls=1.987;
	Lm=1.95;
	J=59;

	w_wz=50*2*3.14;
}

void MIndMach::OnPause()
{
	;
}

void MIndMach::OnResume()
{
	;
}

void MIndMach::OnReset()
{
	frx=0.01;
	fry=0.01;
}
