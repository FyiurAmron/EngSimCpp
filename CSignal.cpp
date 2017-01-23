#include "stdafx.h"

#include "CSignal.h"

CSignal::CSignal():sName()
{
	locked=0;
	xStep=0.001;
	signalType=SIGNAL_LINES;
	xValues=nullptr;
	yValues=nullptr;
	x=nullptr;
	y=nullptr;
	if(!InitMemory(SIGNAL_DEF_LENGTH)) exit(0);
}

int CSignal::InitMemory(int samples)
{
	if(xValues!=nullptr) delete[] xValues;
	if(yValues!=nullptr) delete[] yValues;
	if(x!=nullptr) delete[] x;
	if(y!=nullptr) delete[] y;

	try
	{
		xValues = new float[samples]; 
		yValues = new float[samples]; 
		x = new float[samples]; 
		y = new float[samples]; 
		
		for(int i=0;i<samples;i++)
		{
			xValues[i]=0;
			yValues[i]=0;
			x[i]=0;
			y[i]=0;
		}
	}
	catch(std::bad_alloc& exc)
	{
		MessageBox(NULL,_T("Nieudane przydzielanie pamięci rejestrowanych sygnałów.\nSpróbuj skrócić czas symulacji, zwiększyć krok lub zmniejszyć liczbę rejestrowanych przebiegów."),_T("Przydzielanie pamięci"),MB_OK|MB_ICONERROR);
		exit(0);
		return 0;
	}

	x[0]=0;
	y[0]=0;
	nSamples=0; 
	nSamplesV=0;
	nSize=samples;
	skippedSamples=0;
	skippedSamplesDisp=0;

	return 1;
}

void CSignal::AddSample(double x, double y)
{
	//while(locked);	//zabezpieczenie przed dodawaniem nowych probek podczas przeskalowywania (przy pracy wielowatkowej)

	if(sName==_T("x11"))
	{
		int booo;
		booo=yValues[0];
	}

	this->xValues[nSamplesV]=x;
	this->yValues[nSamplesV]=(y<1e6 && y>-1e6)?y:1e6;
	nSamplesV++;
}

void CSignal::AddSampleDisplayed(double x, double y)
{
	this->x[nSamples]=x;
	this->y[nSamples]=y;
	nSamples++;
}

void CSignal::ModifyLastSample(double x,double y)
{
	this->xValues[nSamplesV-1]=x;
	this->yValues[nSamplesV-1]=y;

	//this->x[nSamples-1]=x;
	//this->y[nSamples-1]=y;
}

void CSignal::Fetch()
{
	AddSample(*sourceX,*sourceY);
}

void CSignal::UpdateScale(double xStep)
{
	double time=0;

	locked=1;

	this->xStep=xStep;
	
	nSamples=0;

	for(int i=0;i<nSamplesV;i++)
	{
		if(xValues[i]>=time || signalType==SIGNAL_DOTS)	
		{
			x[nSamples]=xValues[i];
			y[nSamples]=yValues[i];
			nSamples++;
			if(xValues[i]==xValues[i+1]) //zapewnienie prostokatnych przebiegow (2 punkty dla tego samego czasu)
			{
				x[nSamples]=xValues[i+1];
				y[nSamples]=yValues[i+1];
				nSamples++;
				i++;
			}
			time+=xStep;
		}
	}

	//dodanie ostatniej próbki - zapewnia, że ostatnia rzeczywista próbka jest obecna w uproszczonej tablicy
	x[nSamples]=xValues[nSamplesV-1];
	y[nSamples]=yValues[nSamplesV-1];
	nSamples++;

	locked=0;
}

void CSignal::SkipOrFetch()
{
	while(locked);	//zabezpieczenie przed dodawaniem nowych probek podczas przeskalowywania (przy pracy wielowatkowej)

	if(nSamplesV && (float)(*sourceY)==yValues[nSamplesV-1])
	{
		if(skippedSamples==0)
		{
			AddSample(*sourceX,yValues[nSamplesV-1]);
		}
		skippedSamples=1;
		ModifyLastSample(*sourceX,(*sourceY*scale)+offset);
	} else
	{
		if(skippedSamples)
		{
			ModifyLastSample(*sourceX,yValues[nSamplesV-1]);
			skippedSamples=0;
		}
		AddSample(*sourceX,(*sourceY*scale)+offset);
	}

	if(!xStep || this->x[nSamples-1-skippedSamplesDisp]+xStep<= *sourceX || nSamples<2)
	{
		AddSampleDisplayed(*sourceX,yValues[nSamplesV-1]);
	}
}

void CSignal::Clear()
{
	nSamples=0;
	nSamplesV=0;
	skippedSamples=0;
	skippedSamplesDisp=0;
}

void CSignal::FindMargins()
{
	minX=INFINITE;
	maxX=-1e10;
	minY=INFINITE;
	maxY=-1e10;
	for(int i=0;i<nSamples;i++)
	{
		if(x[i]<minX)minX=x[i];
		if(x[i]>maxX)maxX=x[i];
		if(y[i]<minY)minY=y[i];
		if(y[i]>maxY)maxY=y[i];
	}
}