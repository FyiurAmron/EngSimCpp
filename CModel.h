#include "CSimulation.h"
#include <vector>

#ifndef MODEL_H
#define MODEL_H

#define MODEL_IN double*
#define MODEL_OUT double
#define MODEL_DELAYED_IN CDelayedInput

extern class CSimulation;
extern class CModel;
extern class CDelayedInput;

typedef void(CModel::*DIFF_EQUS_FCN)(double *dx, double *x);
enum SolverType{SolverRK4};

/// <summary>Wej�cie z op�nieniem</summary>
class CDelayedInput
{
public:
	MODEL_IN in;			//wej�cie (wska�nik)
	MODEL_OUT delayedIn;	//op�nione wej�cie

	CDelayedInput()
	{
		delayedIn=0;
	}
	CDelayedInput(CModel *model);
	void Update();	//Aktualizuje stan op�nionego wej�cia przyjmuj�c bie��c� warto�� wskazywan� przez zmienna 'in'
	MODEL_OUT operator*()
	{
		return delayedIn;
	}

	CDelayedInput & operator=(double *right)
	{
		this->in = right;
		return *this;
	}
};

// Klasa zawieraj�ce wsp�lne metody i pola obiekt�w. Powinna by� dziedziczona przez nowe modele.
class CModel
{
	/// <summary>Przydziela pami�� dla zmiennych stanu modelu</summary>
	///	<param name="nStates">liczba zmiennych stanu</param>
	void AssignMemory(int nStates);

protected:	
	DIFF_EQUS_FCN	diffFcn;
	
	void DiffSolve(SolverType solver);
	void SolveF4();
	
public:
	CSimulation *sim;	//wska�nik do symulacji, w kt�rej zarejestruje si� model
	int		calcTrigger;	//okre�la czy w danym kroku model b�dzie obliczany (TRUE/FALSE)
	int		inheritCalcTrigger; //okre�la czy kroki, w kt�rych wykonane b�da obliczenia modelu, s� dziedziczone z innego modelu (TRUE/FALSE)
	int		triggerCalcAlways;	//okre�la czy obliczenia b�d� wykonywane w ka�dym kroku (TRUE/FALSE)
	int		nDiffEqs;	//liczba r�wna� r�niczkowych
	CModel	*parentModel;	//wska�nik do modelu nadrz�dnego, z kt�rego dziedziczone b�d� kroki oblicze�
	std::vector<CDelayedInput*> delayedInputs;	//wektor zawieraj�cy wska�niki do wszystkich op�nionych wej�� modelu

	double	prevCalcTime;	//czas symulacji, w kt�rym zako�czono poprzedni krok oblicze�
	double	orderedCalcTime;	//czas, w kt�rym wykonane b�dzie dodatkowe obliczenie na ��danie
	int		orderedCalc;	//czy zam�wione zosta�o obliczanie na ��danie (TRUE/FALSE)

	double	*x;	//tablica z warto�ciami zmiennych stanu
	double	*xTemp;	//tablica pomocnicza
	double	*dx;	//tablica z pochodnymi zmiennych stanu
	double	*dxTemp;	//tablica pomocnicza

	CModel(){}
	CModel(int nDiffEqs);
	CModel(CSimulation *sim, int nDiffEqs=0);

	/// <summary>W��cza dziedziczenie krok�w oblicze� - obliczenia wykonywane w tych samych krokach co model nadrz�dny</summary>
	///	<param name="parent">wska�nik do modelu nadrz�dnego</param>
	void InheritCalcTrigger(CModel *parent);
	

	virtual void OnCalculate(){;}
	virtual void OnReset(){;}
	virtual void OnInit(){;}
	virtual void OnPause(){;}
	virtual void OnResume(){;}
	virtual void OnEnd(){;}

	virtual void Calculate(double t){;}
	virtual void Calculate2(double t){;}
	void TriggerCalcOnce(){calcTrigger=1;inheritCalcTrigger=0;orderedCalc=0;triggerCalcAlways=0;}
	virtual void NewTick(){;}
	void ReAssignMemory(int nStates);
	double Limit(double val, double min, double max);
	void AddDelayedInput(CDelayedInput *input);
	void UpdateInputs();
	void UpdateInputsAndCalculate(double period);
	int OrderCalc(double t);
	int OrderCalcIn(double period);
	void Reset();
	virtual void InitModel(){;}
	virtual void PrepareToStart(){;}
};



#endif