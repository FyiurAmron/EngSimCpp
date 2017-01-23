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
	std::vector<CProperty>	props;	//dynamiczna tablica parametr�w/w�a�ciwo�ci
	std::wstring	groupName;	//nazwa grupy parametr�w/w�a�ciwo�ci

	/// <summary>Dodaje do grupy parametr�w/w�a�ciwo�ci dodatkow� pozycj�</summary>
	///	<param name="pointer">wskaznik do parametru</param>
	///	<param name="name">wy�wietlana nazwa parametru</param>
	void AddProperty(double *pointer, std::wstring name);

	/// <summary>Dodaje do grupy parametr�w/w�a�ciwo�ci dodatkow� pozycj�</summary>
	///	<param name="pointer">wskaznik do parametru</param>
	///	<param name="name">wy�wietlana nazwa parametru</param>
	///	<param name="shortcut">skr�t do parametru, mo�e by� wielk� liter� lub cyfr�, mo�e przyj�� warto�� NULL - brak skr�tu</param>
	void AddProperty(double *pointer, std::wstring name, char shortcut);

	/// <summary>Dodaje do grupy parametr�w/w�a�ciwo�ci dodatkow� pozycj�</summary>
	///	<param name="pointer">wskaznik do parametru</param>
	///	<param name="name">wy�wietlana nazwa parametru</param>
	///	<param name="shortcut">skr�t do parametru, mo�e by� wielk� liter� lub cyfr�, mo�e przyj�� warto�� NULL - brak skr�tu</param>
	///	<param name="sliderMin">minimalna warto�� suwaka</param>
	///	<param name="sliderMax">maksymalna warto�� suwaka</param>
	void AddProperty(double *pointer, std::wstring name, char shortcut, double sliderMin, double sliderMax);
};

#endif