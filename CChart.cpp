#include "stdafx.h"
#include "CSignal.h"
#include "CGraphics.h"
#include "CChartWindow.h"

#include "CChart.h"

CSimulation *CChart::sim;
int CChart::markerFontSize=14;

CChart::CChart()
{
	scaleX=1000;
	scaleY=50;
	x0=-10;
	y0=0;

	x1=0;
	y1=0;
	x2=200;
	y2=100;
}

CChart::CChart(CGraphics *graph):signals(),signalsVect(),signalsElement(),plotColors(),prevXNum(),prevXVal(),xLabel(),yLabel()
{
	this->graph = graph;
	scaleX=1000;
	scaleY=100;
	x0=0;
	y0=0;

	gridResX=50;
	gridResY=50;

	marginLeft = 50;
	marginRight = 5;
	marginTop = 5;
	marginBottom = 20;

	nSignalNum=0;
	transforming=0;

	markerX=5;
	xWidth=1;
	yHeight=1;

	x1=-1;
	y1=-1;
	x2=-1;
	y2=-1;

	SetColors(RGB_(0,0,0),RGB_(255,255,255),RGB_(255,255,255));
	resapleRequired=0;
}

/*CChart::CChart(const CChart& obj)
{
	Copy(obj);
}

void CChart::Copy(const CChart& obj)
{
	//memcpy(this,&obj,sizeof(obj));
	graph = obj.graph;
}*/

void CChart::SetHeight(double height)
{
	yHeight=height;
}

void CChart::SetWidth(double width)
{
	xWidth=width;
}

void CChart::SetYPos(double y)
{
	y0=-y;
}

void CChart::SetColors(DWORD cBackground, DWORD cGrid, DWORD cLabels)
{
	colorBackground=cBackground;
	colorGrid=cGrid;
	colorLabels=cLabels;
}

void CChart::DrawChart()
{
	CRect	clip;
	//int	frameX1,frameY1,frameX2,frameY2;
	
	double	divX,divY;
	int		divXpow=0,divYpow=0;
	double	divXmult=1,divYmult=1;
	int		divXdigit,divYdigit;
	double	divShift;

	int mouseX;

	TCHAR	wstr[100],wstr2[100];

	UpdateSignalsPointers();
	
	x1<0 ? frameX1=0 : frameX1=x1;
	y1<0 ? frameY1=0 : frameY1=y1;
	x2<0 ? frameX2=graph->window->width : frameX2=x2;
	y2<0 ? frameY2=graph->window->height : frameY2=y2;

	scaleX=(abs(frameX2-frameX1)-(marginLeft+marginRight))/xWidth;
	scaleY=(abs(frameY2-frameY1)-(marginTop+marginBottom))/yHeight;

	divX=gridResX/scaleX;
	if(divX<1)
	{
		while((divX*=10.0f) < 10)
		{
			divXpow--;
			divXmult/=10;
		}
		divXdigit = (int)(divX/10);
	} else
	{
		while((divX/=10.0f) >= 1)
		{
			divXpow++;
			divXmult*=10;
		}
		divXdigit = (int)(divX*10);
	}
	if(divXdigit>2)
	{
		if(divXdigit<7)
		{
			divXdigit=5;
		}
		else
		{
			divXdigit=1;
			divXmult*=10;
			divXpow++;
		}
	}

	divY=gridResY/scaleY;
	if(divY<1)
	{
		while((divY*=10.0f) < 10)
		{
			divYpow--;
			divYmult/=10;
		}
		divYdigit = (int)(divY/10);
	} else
	{
		while((divY/=10.0f) >= 1)
		{
			divYpow++;
			divYmult*=10;
		}
		divYdigit = (int)(divY*10);
	}
	if(divYdigit>2)
	{
		if(divYdigit<7)
		{
			divYdigit=5;
		}
		else
		{
			divYdigit=1;
			divYmult*=10;
			divYpow++;
		}
	}

	//x1=5;

	//scaleX=20;
	//scaleY=100;

	graph->ClearViewMatrix();
	graph->ClearModelMatrix();
	graph->SetScale(1);

	graph->SetClipping(0,graph->window->width,0,graph->window->height);

	graph->SetLine(colorGrid,LINE_SOLID);
	graph->SetFillColor(colorBackground);
	graph->SetTextColor(colorLabels);
	graph->DrawRect(frameX1,frameY1,frameX2,frameY2);	//ramka calego obszaru wykresu
	graph->DrawRect(frameX1+marginLeft,frameY1+marginBottom,frameX2-marginRight,frameY2-marginTop);	//ramka samego wykresu
	//graph->SetFillColor(RGBA(255,255,255,0));

	//*************************************************
	//********** RYSOWANIE SIATKI ORAZ SKALI **********
	graph->SetTextSize(14);
	divShift=-x0/divXdigit/divXmult;
	for(int i=x0/(double)divXdigit/divXmult; i<(x0+(frameX2-frameX1-(marginLeft+marginRight))/scaleX)/(double)divXdigit/divXmult; i++)
	{
		if(((i+divShift)*divXdigit*divXmult*scaleX) < 0) continue;
		graph->DrawLine(frameX1+(i+divShift)*divXdigit*divXmult*scaleX+marginLeft,frameY1+marginBottom,frameX1+(i+divShift)*divXdigit*divXmult*scaleX+marginLeft,frameY2-marginTop);
		
		if(divXpow>=0)
		{
			swprintf(wstr,_T("%.0f"),i*divXdigit*divXmult);
		} else
		{
			if(divXpow>5)
			{
				divXpow++;
			}
			swprintf(wstr2,_T("%%.%df"),-divXpow);
			swprintf(wstr,wstr2,i*divXdigit*divXmult);
		}
		graph->DrawText((i+divShift)*divXdigit*divXmult*scaleX+marginLeft,frameY1+17,wstr);
	}

	divShift=y0/divYdigit/divYmult;
	for(int i=-y0/(double)divYdigit/divYmult; i<(-y0+(frameY2-frameY1-(marginBottom+marginTop))/scaleY)/(double)divYdigit/divYmult; i++)
	{
		if(((i+divShift)*divYdigit*divYmult*scaleY) < 0) continue;
		graph->DrawLine(frameX1+marginLeft,frameY1+(i+divShift)*divYdigit*divYmult*scaleY+marginBottom,frameX2-marginRight,frameY1+(i+divShift)*divYdigit*divYmult*scaleY+marginBottom);
		
		if(divYpow>=0)
		{
			swprintf(wstr,_T("%.0f"),i*divYdigit*divYmult);
		} else
		{
			swprintf(wstr2,_T("%%.%df"),-divYpow);
			swprintf(wstr,wstr2,i*divYdigit*divYmult);
		}
		graph->DrawText(frameX1+22,frameY1+(i+divShift)*divYdigit*divYmult*scaleY+marginBottom,wstr);
	}
	//********* KONIEC RYSOWANIA SIATKI **********
	//********************************************

	
	
	graph->ClearViewMatrix();
	graph->ClearModelMatrix();
	graph->SetScale(1);

	graph->SetClipping(frameX1+marginLeft,frameX2-marginRight,frameY1+marginBottom,frameY2-marginTop);
	//graph->TranslateView(10-10*x1,10+y1);
	graph->TranslateView(frameX1+marginLeft,frameY1+marginBottom);
	graph->ScaleView(scaleX,scaleY);
	graph->SetScale(max(scaleX,scaleY));
	graph->TranslateView(-x0,y0);

	graph->SetTextSize(markerFontSize);

	//graph->TranslateModel(-x1,0);
	
	graph->GetClipping(&clip);

	xWidth=clip.right-clip.left;
	
	graph->SetLine(RGB_(255,150,150,128),LINE_SOLID);
	graph->DrawLine(markerX,clip.bottom,markerX,clip.top);
	
	int markerNum=-1;	//numer probki z markerem
	for(int i=0;i<nSignalNum;i++)
	{
		if(signals[i]->sName==_T("obserwator"))
		{
			int ggg;
			ggg=3;
		}
		graph->SetLine(plotColors[i],LINE_SOLID);
		switch(signals[i]->signalType)
		{
		case SIGNAL_DOTS:
			markerNum=-1;
			graph->DrawPoints(signals[i]->xValues,signals[i]->yValues,signals[i]->nSamplesV);
			break;
		default:
			markerNum = graph->DrawLines(signals[i]->x,signals[i]->y,signals[i]->nSamples,clip.left,clip.right,markerX);
			break;
		}
		
		if(markerNum>=0)
		{
			graph->SetLine((plotColors[i])&(0x88ffffff),LINE_SOLID);
			graph->DrawLine(clip.left,signals[i]->y[markerNum],clip.right,signals[i]->y[markerNum]);
			//graph->SetFillColor(RGBA(0,0,0,128));
			//graph->DrawRect(clip.left,signals[i]->y[markerNum],clip.left+20,signals[i]->y[markerNum]+10);
			graph->SetTextColor(plotColors[i]);
			graph->DrawNumber(clip.left,signals[i]->y[markerNum],signals[i]->y[markerNum]);
		}
	}
	if(markerNum>=0)
	{
		graph->SetTextColor(colorLabels);
		graph->DrawNumber(signals[nSignalNum-1]->x[markerNum],clip.top,signals[nSignalNum-1]->x[markerNum],_T(".6"));
	}
	mouseX=((CChartWindow*)(graph->window))->prevPosX;
	DrawLegend((mouseX<frameX2-150)?(mouseX>(frameX2-frameX1)/2)?((frameX2-150-mouseX)*255/(frameX2-150-frameX1-(frameX2-frameX1)/2)):255:0);
}


void CChart::UpdateMarker(int mouseX, int mouseY)
{
	CPoint pt;

	graph->Cursor2Pos(mouseX,mouseY,&pt);

	markerX=(mouseX-frameX1-marginLeft)/scaleX+x0;
}



void CChart::SetPlotOffset(double x, double y)
{
	x0+=x/scaleX;
	y0+=y/scaleY;

	//if(x0<0)x0=0;
}

void CChart::ChangeScale(double x, double y, int increment, int forceRecalc)
{
	double scaleFactor=1.005;

	if(increment)
	{
		//scaleX=(scaleX*pow(scaleFactor,x))>1e8?1e8:(scaleX*pow(scaleFactor,x));
		//scaleY=(scaleY*pow(scaleFactor,y))>1e8?1e8:(scaleY*pow(scaleFactor,y));
		if(x!=0) resapleRequired=1;
		xWidth=(xWidth*pow(scaleFactor,x))>1e8?1e8:(xWidth*pow(scaleFactor,x));
		yHeight=(yHeight*pow(scaleFactor,y))>1e8?1e8:(yHeight*pow(scaleFactor,y));
	} else
	{
		if(x!=0) scaleX=x;
		if(y!=0) scaleY=y;
	}
	
	if(forceRecalc && resapleRequired)
	{
		for(int i=0;i<signals.size();i++)
		{
			signals[i]->UpdateScale(1.0/scaleX);
		}
		resapleRequired=0;
	}
}

void CChart::RewriteSignalsAfterScale()
{
	if(resapleRequired)
	{
		for(int i=0;i<signals.size();i++)
		{
			signals[i]->UpdateScale(1/scaleX);
		}
	}
}

void CChart::SetArea(int x1, int y1, int x2, int y2)
{
	this->x1=x1;
	this->y1=y1;
	this->x2=x2;
	this->y2=y2;
}

void CChart::AddSignal(CSignal *signal, DWORD color)
{
	signals.push_back(signal);
	signalsVect.push_back(0);
	signalsElement.push_back(-1);
	plotColors.push_back(color);
	nSignalNum=signals.size();	
}

void CChart::AddSignal(std::vector<CSignal> *signalVect, int signalElement, DWORD color)
{
	signals.push_back(&((*signalVect)[signalElement]));
	signalsVect.push_back(signalVect);
	signalsElement.push_back(signalElement);
	plotColors.push_back(color);
	nSignalNum=signals.size();
}

void CChart::AddSignal(const double *y, std::wstring name, DWORD color, double offset, double scale)
{
	AddSignal(&(sim->signals), sim->RegisterSignal(y,name,offset,scale), color);//zamienic wskaznik do sygnalu na cos innego
}

void CChart::AddSignal(const double *x, const double *y, std::wstring name, DWORD color, double offset, double scale)
{
	AddSignal(&(sim->signals), sim->RegisterSignal(x,y,name,offset,scale), color);
}

void CChart::UpdateSignalsPointers()
{
	for(int i=0;i<signals.size();i++)
	{
		if(signalsElement[i]>=0)
		{
			//signals[i] = &(*(signalsVect[i])[signalsElement[i]]);
			signals[i] = &(*(signalsVect[i]))[signalsElement[i]];
		}
	}
}

int CChart::Contains(int mouseX, int mouseY)
{
	CPoint pt;

	graph->Cursor2Pos(mouseX, mouseY, &pt);
	if(pt.x>frameX1+marginLeft && pt.x<frameX2-marginRight && pt.y>frameY1+marginBottom && pt.y<frameY2-marginTop)
	{
		return 1;
	} else
	{
		return 0;
	}
}

void CChart::DrawLegend(int transparency)
{
	int fontHeight;
	int n;
	int legendFrameX1,legendFrameX2,legendFrameY1,legendFrameY2;
	TCHAR temp[255];
	TCHAR *strPtr;

	graph->SetTextSize(11);
	fontHeight = graph->GetFontHeight();
	n=signals.size();

	legendFrameX1=frameX2-200;
	legendFrameX2=frameX2-10;
	legendFrameY1=frameY2-10-20-n*fontHeight;
	legendFrameY2=frameY2-10;

	graph->ClearViewMatrix();
	graph->ClearModelMatrix();
	graph->SetScale(1);

	graph->SetLine((colorGrid&0x00ffffff)|(transparency<<24),LINE_SOLID);
	graph->SetFillColor((colorBackground&0x00ffffff)|(transparency<<24));
	graph->DrawRect(legendFrameX1,legendFrameY1,legendFrameX2,legendFrameY2);

	

	for(int i=0;i<n;i++)
	{
		if(signals[i]->offset!=0 || signals[i]->scale!=1)
		{
			swprintf(temp,_T("%s (offset:%.1f; scale:%.1f)"),signals[i]->sName.c_str(),signals[i]->offset,signals[i]->scale);
			strPtr = temp;
		} else
		{
			strPtr = (TCHAR*)signals[i]->sName.c_str();
		}
		graph->SetTextColor(((DWORD) plotColors[i] & ~(255<<24)) | (((DWORD) (BYTE) (transparency)) << 24));
		graph->DrawText(legendFrameX1+10,legendFrameY1+10+(i+1)*fontHeight,strPtr);
	}
}

void CChart::FindMargins()
{
	for(int signal=0;signal<signals.size();signal++)
	{
		signals[signal]->FindMargins();
		minX=INFINITE;
		maxX=-1e10;
		minY=INFINITE;
		maxY=-1e10;

		if(signals[signal]->minX<minX)minX=signals[signal]->minX;
		if(signals[signal]->maxX>maxX)maxX=signals[signal]->maxX;
		if(signals[signal]->minY<minY)minY=signals[signal]->minY;
		if(signals[signal]->maxY>maxY)maxY=signals[signal]->maxY;
	}
}

void CChart::SetMargins()
{
	x0=minX;
	y0=-minY;

	xWidth = maxX-minX;
	yHeight = maxY-minY;
}

void CChart::SetMarkerFontSize(double fontSize)
{
	markerFontSize=fontSize;
}