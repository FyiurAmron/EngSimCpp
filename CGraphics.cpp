#include "stdafx.h"


#include "CGraphics.h"

void CGraphics::SetScale(double scale)
{
	this->scale=scale;
}

void CGraphics::DrawNumber(float x, float y, int number)
{
	TCHAR	str_tmp[100];

	swprintf(str_tmp,_T("%d"),number);
	DrawText(x,y,str_tmp);
}

void CGraphics::DrawNumber(float x, float y, float number, TCHAR *precision)
{
	TCHAR	str_tmp[100],str_tmp2[100];
	swprintf(str_tmp2,_T("%%%sf"),precision);
	swprintf(str_tmp,str_tmp2,number);
	DrawText(x,y,str_tmp);
}