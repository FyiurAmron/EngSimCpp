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

/// <summary>Wejœcie z opóŸnieniem</summary>
class CDelayedInput
{
public:
	MODEL_IN in;			//wejœcie (wskaŸnik)
	MODEL_OUT delayedIn;	//opóŸnione wejœcie

	CDelayedInput()
	{
		delayedIn=0;
	}
	CDelayedInput(CModel *model);
	void Update();	//Aktualizuje stan opóŸnionego wejœcia przyjmuj¹c bie¿¹c¹ wartoœæ wskazywan¹ przez zmienna 'in'
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

// Klasa zawieraj¹ce wspólne metody i pola obiektów. Powinna byæ dziedziczona przez nowe modele.
class CModel
{
	/// <summary>Przydziela pamiêæ dla zmiennych stanu modelu</summary>
	///	<param name="nStates">liczba zmiennych stanu</param>
	void AssignMemory(int nStates);

protected:	
	DIFF_EQUS_FCN	diffFcn;
	
	void DiffSolve(SolverType solver);
	void SolveF4();
	
public:
	CSimulation *sim;	//wskaŸnik do symulacji, w której zarejestruje siê model
	int		calcTrigger;	//okreœla czy w danym kroku model bêdzie obliczany (TRUE/FALSE)
	int		inheritCalcTrigger; //okreœla czy kroki, w których wykonane bêda obliczenia modelu, s¹ dziedziczone z innego modelu (TRUE/FALSE)
	int		triggerCalcAlways;	//okreœla czy obliczenia bêd¹ wykonywane w ka¿dym kroku (TRUE/FALSE)
	int		nDiffEqs;	//liczba równañ ró¿niczkowych
	CModel	*parentModel;	//wskaŸnik do modelu nadrzêdnego, z którego dziedziczone bêd¹ kroki obliczeñ
	std::vector<CDelayedInput*> delayedInputs;	//wektor zawieraj¹cy wskaŸniki do wszystkich opóŸnionych wejœæ modelu

	double	prevCalcTime;	//czas symulacji, w którym zakoñczono poprzedni krok obliczeñ
	double	orderedCalcTime;	//czas, w którym wykonane bêdzie dodatkowe obliczenie na ¿¹danie
	int		orderedCalc;	//czy zamówione zosta³o obliczanie na ¿¹danie (TRUE/FALSE)

	double	*x;	//tablica z wartoœciami zmiennych stanu
	double	*xTemp;	//tablica pomocnicza
	double	*dx;	//tablica z pochodnymi zmiennych stanu
	double	*dxTemp;	//tablica pomocnicza

	CModel(){}
	CModel(int nDiffEqs);
	CModel(CSimulation *sim, int nDiffEqs=0);

	/// <summary>W³¹cza dziedziczenie kroków obliczeñ - obliczenia wykonywane w tych samych krokach co model nadrzêdny</summary>
	///	<param name="parent">wskaŸnik do modelu nadrzêdnego</param>
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