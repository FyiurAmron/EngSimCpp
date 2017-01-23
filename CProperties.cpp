#include "stdafx.h"
#include "CProperties.h"

std::vector<CProperty *> CProperty::allProps;

void CProperty::Set(double *pointer, std::wstring name, char shortcut, double sliderMin, double sliderMax)
{
	this->pointer = pointer;
	this->name = name;
	this->shortcut = shortcut;
	this->sliderMin=sliderMin;
	this->sliderMax=sliderMax;

	if(indexInAllProps==-1)
	{
		allProps.push_back(this);
		indexInAllProps = allProps.size()-1;
	}
}

void CProperty::Set(double *pointer, std::wstring name)
{
	Set(pointer,name,0);
}

void CProperty::SetVal(double val)
{
	*pointer = val;
}

double CProperty::GetVal()
{
	return *pointer;
}

void CProperty::SetSliderRange(double min, double max)
{
	sliderMin=min;
	sliderMax=max;
}







void CPropertiesGroup::AddProperty(double *pointer, std::wstring name)
{
	props.push_back(CProperty(pointer,name));
}

void CPropertiesGroup::AddProperty(double *pointer, std::wstring name, char shortcut)
{
	props.push_back(CProperty(pointer,name,shortcut));
}

void CPropertiesGroup::AddProperty(double *pointer, std::wstring name, char shortcut, double sliderMin, double sliderMax)
{
	props.push_back(CProperty(pointer,name,shortcut,sliderMin,sliderMax));
}
