#ifndef SIGNAL_H
#define SIGNAL_H

#define SIGNAL_DEF_LENGTH 4000000

#include <string>

using namespace std;

enum SignalType{SIGNAL_LINES, SIGNAL_DOTS};

class CSignal
{
	unsigned int nSize;
	int	skippedSamples,skippedSamplesDisp;
	volatile int locked;
	double skippedX;
	double skippedY;
	double xStep;
	
	

public:
	float	*y;
	float	*x;
	float	*xValues;
	float	*yValues;
	double	*sourceY;
	double	*sourceX;
	unsigned int	nSamples;	/*probki do wyswietlenia*/
	unsigned int	nSamplesV; //wszystkie probki
	double minX,maxX,minY,maxY;
	double	offset;
	double	scale;

	SignalType	signalType;
	
	wstring	sName;
	wstring	sDescription;
	wstring	sXLabel;
	wstring	sYLabel;
	
	CSignal();
	int InitMemory(int samples);
	void AddSample(double x, double y);
	void AddSampleDisplayed(double x, double y);
	void ModifyLastSample(double x,double y);
	void Fetch();
	void SkipOrFetch();
	void UpdateScale(double scale);
	void Clear();
	void FindMargins();
	

};

#endif