#ifndef GENALG_H
#define GENALG_H

typedef double (*PACTFUNC )(double x);
typedef int NNACTFUNC;

int CompareIndividuals( const void *a, const void *b );

class CGenIndividual {
public:
    double score;

    virtual void Crossover( void *result, void *ind2 ) = 0;
    virtual void Mutate( ) = 0;
    virtual double Evaluate( ) = 0;
    virtual void Initiate( ) = 0;
};

class CGenFcnMinProps {
public:

    CGenFcnMinProps( int nVars ) {
        constraints = new double* [nVars];
        for( int i = 0; i < nVars; i++ ) {
            constraints[i] = new double[2];
            constraints[i][0] = -10000;
            constraints[i][1] = 10000;
        }
        this->nVars = nVars;
        mutVal = 2;
    }

    ~CGenFcnMinProps( ) {

        for( int i = 0; i < nVars; i++ ) {
            delete[] constraints[i];
        }
        delete[] constraints;
    }

    int nVars;
    double **constraints;
    double (*evalFcn )(void *, void *);
    void *customData; //wskaznik do dowolnego obiektu przekazywanego do funkcji celu
    double mutVal;
};

class CGenIndividualFcnMin : public CGenIndividual {
public:
    double *vars;
    CGenFcnMinProps *props;

    CGenIndividualFcnMin( ) {
        ;
    }

    CGenIndividualFcnMin( void *props ) {
        Init( props );
    }

    void Init( void *props );

    virtual void Crossover( void *result, void *ind2 );
    virtual void Mutate( );
    virtual double Evaluate( );
    virtual void Initiate( );

    CGenIndividualFcnMin operator=( CGenIndividualFcnMin obj ) {
        score = obj.score;
        props = obj.props;
        memcpy( vars, obj.vars, props->nVars * sizeof (vars[0] ) );

        return *this;
    }
};

template <typename Individual>
class CGenAlg {
public:
    int nPopulation;
    int nMaxGenerations;
    int nRepetitions;
    double crossoverFrac;
    double mutationFrac;
    Individual *population;
    Individual *populationTemp;
    Individual winner;
    void *indProps;

    CGenAlg( ) {
        crossoverFrac = 0.5;
        mutationFrac = 0.5;
        nMaxGenerations = 100;
        nRepetitions = 1;
    }

    ~CGenAlg( ) {
        if ( population ) delete[] population;
        if ( populationTemp ) delete[] populationTemp;
    }

    void Init( );
    void Run( );

    void Initialization( );
    void Evaluation( );
    void Crossover( );
    void Mutation( );

};

#endif