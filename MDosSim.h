#ifndef MDOSSIM_H
#define MDOSSIM_H

#include "CModel.h"

typedef void(*MAINFCN)(void);
typedef void(*EVENTFCN)(void);

class MDosSim : public CModel
{
	//static void DosThread(void *args);
	static DWORD WINAPI DosThread(LPVOID args);
	MAINFCN main;
	EVENTFCN onResetExt;
	HANDLE thread;
	
	double *dos_time_src;
public:
	volatile double dos_time_real;

	MDosSim(CSimulation *sim, MAINFCN main, double *time) : 
		CModel(sim,0)
	{
		InitDosModel(sim, main, NULL, time);
	}

	MDosSim(CSimulation *sim, MAINFCN main, EVENTFCN onReset, double *time) : 
		CModel(sim,0)
	{
		InitDosModel(sim, main, onReset, time);
	}

	void InitDosModel(CSimulation *sim, MAINFCN main, EVENTFCN onReset, double *time);

	

	void OnCalculate();
	void OnEnd();
	void OnInit();
	void OnPause();
	void OnResume();
	void OnReset();

	void UpdateDosTime();
};

#endif