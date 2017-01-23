#include "stdafx.h"

#include "CGenAlg.h"

template class CGenAlg<CGenIndividualFcnMin>;

int CompareIndividuals( const void *a, const void *b ) {
    double diff = ( (const CGenIndividual *) a )->score - ( (const CGenIndividual *) b )->score;
    return diff < 0 ? -1 : diff > 0 ? 1 : 0;
}

void CGenIndividualFcnMin::Init( void *props ) {
    this->props = (CGenFcnMinProps *) props;
    vars = new double [this->props->nVars];
}

double CGenIndividualFcnMin::Evaluate( ) {
    return score = props->evalFcn( this, props->customData );
}

void CGenIndividualFcnMin::Initiate( ) {
    double a;
    for( int i = 0; i < props->nVars; i++ ) {
        a = rand( ) / (double) RAND_MAX;
        vars[i] = a * ( props->constraints[i][1] - props->constraints[i][0] ) + props->constraints[i][0];
    }
}

void CGenIndividualFcnMin::Crossover( void *result, void *ind2 ) {
    double a;
    for( int i = 0; i < props->nVars; i++ ) {
        a = rand( ) / (double) RAND_MAX;
        ( (CGenIndividualFcnMin*) result )->vars[i] = a * vars[i]+( 1 - a )*( (CGenIndividualFcnMin*) ind2 )->vars[i];
    }
}

void CGenIndividualFcnMin::Mutate( ) {
    //double a;
    int gen = rand( ) % props->nVars; ///(double)(RAND_MAX+1);

    vars[gen] = rand( ) / (double) RAND_MAX * ( props->constraints[gen][1] - props->constraints[gen][0] ) + props->constraints[gen][0];

    /*for(int i=0;i<props->nVars;i++)
    {
        a=rand()/(double)RAND_MAX;
        vars[i]*=props->mutVal*a;
        if(vars[i]<props->constraints[i][0])
        {
            vars[i]=props->constraints[i][0];
        }
        if(vars[i]>props->constraints[i][1])
        {
            vars[i]=props->constraints[i][1];
        }
    }*/
}

template <typename Individual> void CGenAlg<Individual>::Init( ) {
    if ( population ) delete[] population;
    if ( populationTemp ) delete[] populationTemp;

    population = new Individual [nPopulation];
    populationTemp = new Individual [nPopulation];

    for( int i = 0; i < nPopulation; i++ ) {
        population[i].Init( indProps );
        populationTemp[i].Init( indProps );
    }
    winner.Init( indProps );
}

template <typename Individual> void CGenAlg<Individual>::Run( ) {
    double best_score;
    best_score = 1000000;
    for( int rep = 0; rep < nRepetitions; rep++ ) {
        Initialization( );
        Evaluation( );
        //best_score=population[0].score;
        for( int gen = 0; gen < nMaxGenerations; gen++ ) {
            Crossover( );
            Mutation( );
            Evaluation( );
            if ( population[0].score < best_score ) {
                winner = population[0];
                best_score = winner.score;
            }
        }
    }
    //best_score*=1.0;
    winner.Evaluate( );
}

template <typename Individual> void CGenAlg<Individual>::Initialization( ) {
    for( int i = 0; i < nPopulation; i++ ) {
        populationTemp[i].Initiate( );
    }
}

template <typename Individual> void CGenAlg<Individual>::Crossover( ) {
    int nInd1, nInd2;

    memcpy( populationTemp, population, nPopulation * ( 1 - crossoverFrac ) * sizeof (population[0] ) );

    for( int i = nPopulation * ( 1 - crossoverFrac ); i < nPopulation; i++ ) {
        nInd1 = rand( ) % (int) ( nPopulation * ( 1 - crossoverFrac ) );
        nInd2 = rand( ) % (int) ( nPopulation * ( 1 - crossoverFrac ) );

        population[nInd1].Crossover( &populationTemp[i], &population[nInd2] );
    }

    /*for(int i=nPopulation*(1-crossoverFrac);i<nPopulation;i++)
    {
        nInd1 = min(rand()%nPopulation,rand()%nPopulation);
        nInd2 = min(rand()%nPopulation,rand()%nPopulation);

        population[nInd1].Crossover(&populationTemp[i],&population[nInd2]);
    }*/
}

template <typename Individual> void CGenAlg<Individual>::Mutation( ) {
    for( int i = 0; i < nPopulation; i++ ) {
        if ( rand( ) % 1000 < mutationFrac * 1000 ) {
            populationTemp[i].Mutate( );
        }
    }
}

template <typename Individual> void CGenAlg<Individual>::Evaluation( ) {
    for( int i = 0; i < nPopulation; i++ ) {
        populationTemp[i].Evaluate( );
    }



    qsort( populationTemp, nPopulation, sizeof (populationTemp[0] ), CompareIndividuals );
    memcpy( population, populationTemp, nPopulation * sizeof (population[0] ) );
}