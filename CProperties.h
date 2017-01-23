#ifndef PROPS_H
#define PROPS_H

#include <string>
#include <vector>

class CProperty
{
	

	
public:
	double *pointer;
	char shortcut;
	std::wstring name;
	double	sliderMin,sliderMax,step;
	static	std::vector<CProperty *> allProps;
	int		indexInAllProps;

	CProperty(){indexInAllProps=-1;sliderMin=0;sliderMax=100;}
	CProperty(double *pointer, std::wstring name){indexInAllProps=-1;Set(pointer,name);}
	CProperty(double *pointer, std::wstring name, char shortcut){indexInAllProps=-1;Set(pointer,name,shortcut);}
	CProperty(double *pointer, std::wstring name, char shortcut, double sliderMin, double sliderMax){indexInAllProps=-1;Set(pointer,name,shortcut,sliderMin,sliderMax);}

	void Set(double *pointer, std::wstring name);
	void Set(double *pointer, std::wstring name, char shortcut, double sliderMin=-1, double sliderMax=1);
	void SetVal(double val);
	void SetSliderRange(double min, double max);
	double GetVal();
};

class CPropertiesGroup
{
public:
	std::vector<CProperty>	props;	//dynamiczna tablica parametrów/w³aœciwoœci
	std::wstring	groupName;	//nazwa grupy parametrów/w³aœciwoœci

	/// <summary>Dodaje do grupy parametrów/w³aœciwoœci dodatkow¹ pozycjê</summary>
	///	<param name="pointer">wskaznik do parametru</param>
	///	<param name="name">wyœwietlana nazwa parametru</param>
	void AddProperty(double *pointer, std::wstring name);

	/// <summary>Dodaje do grupy parametrów/w³aœciwoœci dodatkow¹ pozycjê</summary>
	///	<param name="pointer">wskaznik do parametru</param>
	///	<param name="name">wyœwietlana nazwa parametru</param>
	///	<param name="shortcut">skrót do parametru, mo¿e byæ wielk¹ liter¹ lub cyfr¹, mo¿e przyj¹æ wartoœæ NULL - brak skrótu</param>
	void AddProperty(double *pointer, std::wstring name, char shortcut);

	/// <summary>Dodaje do grupy parametrów/w³aœciwoœci dodatkow¹ pozycjê</summary>
	///	<param name="pointer">wskaznik do parametru</param>
	///	<param name="name">wyœwietlana nazwa parametru</param>
	///	<param name="shortcut">skrót do parametru, mo¿e byæ wielk¹ liter¹ lub cyfr¹, mo¿e przyj¹æ wartoœæ NULL - brak skrótu</param>
	///	<param name="sliderMin">minimalna wartoœæ suwaka</param>
	///	<param name="sliderMax">maksymalna wartoœæ suwaka</param>
	void AddProperty(double *pointer, std::wstring name, char shortcut, double sliderMin, double sliderMax);
};

#endif