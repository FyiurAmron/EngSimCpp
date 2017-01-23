#include "stdafx.h"


#include "CSimulation.h"

std::map<UINT_PTR, CSimulation *> CSimulation::timerMap;

void CALLBACK CSimulation::TimerProc(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	CSimulation *sim = timerMap[idEvent];

	//sim->threadHandle = _beginthread(ThreadedCalc,0,(void *)sim);
	sim->threadHandle = CreateThread(NULL,0,ThreadedCalc,(void *)sim,CREATE_SUSPENDED,NULL);
	//_beginthreadex(0,0,((CSimulation *)idEvent)->ThreadedCalc,(void *)sim,0,0);
}

//unsigned __stdcall CSimulation::ThreadedCalc(void *args)
DWORD WINAPI CSimulation::ThreadedCalc(void *args)
{
	CSimulation *sim=(CSimulation *)args;

	if(((CSimulation *)args)->calcInterval<=0) //gdy liczymy z max wydajnoscia
	{
		while(sim->t < sim->t_end && !sim->terminateThread)
		{
			sim->OnCalculate();
		}
	} else
	{
		for(int i=0;i<sim->nCalcPerInterval;i++)
		{
			sim->OnCalculate();
		}
	}
	sim->EndSimulation();

	return 0;
}


void CSimulation::InitTimer(UINT nMiliSeconds)
{
	UINT_PTR timerID;
	timerID = SetTimer(NULL,(UINT_PTR)this,nMiliSeconds,TimerProc);
	timerMap[timerID]=this;

}

void CSimulation::OnCalculate()
{
	CModel *model;
	
	PreCalc();

	for(int i=0;i<models.size();i++)
	{
		model=models[i];

		if(model->triggerCalcAlways || model->calcTrigger || (model->orderedCalc && model->orderedCalcTime <= t))
		{
			safeToTerminate=0;
			model->OnCalculate();
			safeToTerminate=1;
			model->prevCalcTime=t;
		}
	}

	if(nTicks%nTicksToSkip==0)
	{
		for(int i=signals.size()-1;i>=0;i--)
		{
			signals[i].SkipOrFetch();	//dodawanie nowych probek do rejestrowanych sygnalow
		}
	}

	if(vOrderedCalcTimes.size() && t_prev_regular+t_step>=vOrderedCalcTimes[0])	//sprzwdzanie czy zamowiono dodatkowe obliczenia
	{
		if(t_prev_regular+t_step!=vOrderedCalcTimes[0])
		{
			t=vOrderedCalcTimes[0];
			RemoveOrederedCalc();
			calcReason = SimCalcReasonOrdered;
			nTicksOrdered++;
		} else
		{
			t=t_prev_regular+t_step;
			RemoveOrederedCalc();
			calcReason = SimCalcReasonRegular;
			t_prev_regular=t;
		}
	} else
	{
		t=t_prev_regular+t_step;
		calcReason = SimCalcReasonRegular;
		t_prev_regular=t;
	}

	PostCalc();
	nTicks++;
	//t+=t_step;
}

///<summary>This function does somethinggg</summary>
///<param name="step">paramater comment</param>
void CSimulation::SetStep(double calcStep, double samplesStep, double simTime)
{
	t_step=calcStep;
	nTicksToSkip = samplesStep/calcStep;
	t_end = simTime;

	nDefaultSignalSize=simTime/calcStep/nTicksToSkip+1;
	
	for(int i=0;i<signals.size();i++)
	{
		signals[i].InitMemory(nDefaultSignalSize);
	}
}

void CSimulation::StartSimulation(SimStart start)
{
	t=t_start;

	if(start==SimStartWithInit)
	{
		for(int i=0;i<models.size();i++)
		{
			models[i]->OnInit();
			models[i]->OnReset();
		}
	}

	if(calcInterval<=0)
	{
		//threadHandle = _beginthread(ThreadedCalc,0,(void *)this);
		threadHandle = CreateThread(NULL,0,ThreadedCalc,(void *)this,CREATE_SUSPENDED,NULL);
		ResumeThread(threadHandle);
	} else
	{
		InitTimer(calcInterval);
	}
}

void CSimulation::EndSimulation()
{
	DWORD exitCode;
	

	for(int i=0;i<models.size();i++)
	{
		models[i]->OnEnd();
	}

	//terminateThread=1;
	while(!safeToTerminate);
	GetExitCodeThread(threadHandle,&exitCode);
	TerminateThread(threadHandle,exitCode);

	//terminateThread=1;
	//WaitForSingleObject((HANDLE)threadHandle,INFINITE);
	terminateThread=0;

	
	//CloseHandle((HANDLE)threadHandle);
}

void CSimulation::ResetSimulation()
{
	EndSimulation();

	nTicks=0;
	nTicksOrdered=0;
	t=t_start;
	t_prev_regular=t;

	vOrderedCalcTimes.clear();

	for(int i=0;i<models.size();i++)
	{
		models[i]->Reset();
		//models[i]->OnReset();
	}

	for(int i=0;i<signals.size();i++)
	{
		signals[i].Clear();
	}

	StartSimulation(SimStartReset);
	//ResumeSimulation();
}

void CSimulation::PauseSimulation()
{

	//terminateThread=1;
	//WaitForSingleObject((HANDLE)threadHandle,INFINITE);
	terminateThread=0;

	for(int i=0;i<models.size();i++)
	{
		models[i]->OnPause();
	}

	SuspendThread(threadHandle);
}

void CSimulation::ResumeSimulation()
{
	ResumeThread(threadHandle);

	for(int i=0;i<models.size();i++)
	{
		models[i]->OnResume();
	}

	/*if(calcInterval<=0)
	{
		//threadHandle = _beginthread(ThreadedCalc,0,(void *)this);
		threadHandle = CreateThread(NULL,0,ThreadedCalc,(void *)this,CREATE_SUSPENDED,NULL);
	} else
	{
		InitTimer(calcInterval);
	}*/
}

int CSimulation::RegisterSignal(const double *y, wstring name, double offset, double scale)
{
	CSignal sig;
	sig.InitMemory(nDefaultSignalSize);
	sig.sourceX=(double *)&t;
	sig.sourceY=(double *)y;
	sig.sName = name;
	sig.sYLabel = name;
	sig.sXLabel = wstring(_T("t"));
	sig.offset = offset;
	sig.scale = scale;

	signals.push_back(sig);

	return signals.size()-1;
}

int CSimulation::RegisterSignal(const double *x, const double *y, wstring name, double offset, double scale)
{
	CSignal sig;
	sig.sourceX=(double *)x;
	sig.sourceY=(double *)y;
	sig.sName = name;
	sig.sYLabel = name;
	sig.sXLabel = wstring(_T("t"));
	sig.offset = offset;
	sig.scale = scale;

	signals.push_back(sig);

	return signals.size()-1;
}

CSignal* CSimulation::FindSignal(wstring name)
{
	for(int i=0;i<signals.size();i++)
	{
		if(signals[i].sName.compare(name)==0)
		{
			return &signals[i];
		}
	}

	TCHAR	temp[100];
	wsprintf(temp,_T("Nie mogê znaleŸæ sygna³u: %s"),name.c_str());
	MessageBox(NULL,temp,_T("Nie znaleziono sygna³u"),MB_OK | MB_ICONWARNING);
	return NULL;
}

void CSimulation::AddModel(CModel *model)
{
	models.push_back(model);
}

int CSimulation::OrderCalc(double time)
{
	int i,size;

	if(time<=t) return 0;

	size = vOrderedCalcTimes.size();

	if(size==0||time>vOrderedCalcTimes[size-1])
	{
		vOrderedCalcTimes.push_back(time);
	} else
	{
		for(i=size-2;i>=0;i--)
		{
			if(time>vOrderedCalcTimes[i])
			{
				vOrderedCalcTimesIterator = vOrderedCalcTimes.begin();
				vOrderedCalcTimesIterator+=i+1;
				vOrderedCalcTimes.insert(vOrderedCalcTimesIterator,time);
				break;
			}
		}
		if(i==-1)
		{
			vOrderedCalcTimesIterator = vOrderedCalcTimes.begin();
			vOrderedCalcTimes.insert(vOrderedCalcTimesIterator,time);
		}
	}
	return 1;
}

int CSimulation::OrderCalcIn(double period)
{
	return OrderCalc(t+period);
}

void CSimulation::RemoveOrederedCalc()
{
	vOrderedCalcTimes.erase(vOrderedCalcTimes.begin());
}

void CSimulation::SetParameterValue(double *ptr, double val)
{
	;
}
