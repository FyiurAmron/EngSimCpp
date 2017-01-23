#ifndef NNET_H
#define NNET_H

#define NN_AF_PURELIN 1
#define NN_AF_TANSIG 2


typedef double (*PACTFUNC )(double x);
typedef int NNACTFUNC;

class NeuralNetwork {

    void AddErrorSample( double e ) {
        error_sum += e*e;
        error_den++;
    }
public:
    int nInputs; //liczba wejsc sieci
    int nOutputs; //liczba wyjsc = liczbie neuronow w osatniej warstwie
    int nLayers; //liczba warstw
    int *nNeurons; //wskaznik do tablicy zawierajacej liczby neuronow w poszczegolnych warstwach
    int *nLayerInputs; //wskaznik do tablicy zawierajacej liczby wejsc poszczegolnych warstw
    double ***dWeights; //wagi sieci
    double **dBiases; //progi sieci
    double **outputsVals; //obliczone wyjscia w kazdej z warstw - aktualizowane po wywolaniu funkcji Calc
    double **s; //czulosci poszczegolnych warstw (w metodzie propagacji wstecznej)
    double alpha; //krok uczenia
    double error_sum; //licznik bledu uczenia sieci
    int error_den; //mianownik bledu uczenia sieci - liczba zsumowanych bledow

    PACTFUNC *pActFunc; //wskaznik do tablicy wskaznikow funkcji aktywacji w poszczegolnych warstwach
    PACTFUNC *pActFuncDer; //wskaznik do tablicy wskaznikow pochodnych funkcji aktywacji w poszczegolnych warstwach


    NeuralNetwork( ); //konstruktor - na razie nic nie robi, trzeba wywolac funkcje init
    void init( int inputs, int layers, int *neurons, char const *fileName, NNACTFUNC *actFuncs ); //inicjalizacja sieci
    double Calc( double const *inputs, double *outputs ); //oczlicza wyjscia i zapisuje wyniki pod wskazanym adresie
    double Calc( double const *inputs, double *outputs, double const *targets ); //liczy wyjscia i bledy
    double Calc( double const *inputs ); //oblicza wyjscia i wyjscia poszczegolnych warstw zapisuje jedynie w 'outputsVals[][]'

    double BackProp( double *inputs, double *outputs ); //uczenie - metoda propagacji wstecznej

    //funkcje aktywacji oraz ich pochodne

    static double tansig( double x ) {
        return 2 / ( 1 + exp( -2 * x ) ) - 1;
    }

    static double tansigD( double x ) {
        return 4 * exp( -2 * x ) / ( 1 + exp( -2 * x ) ) / ( 1 + exp( -2 * x ) );
    }

    static double purelin( double x ) {
        return x;
    }

    static double purelinD( double x ) {
        return 1;
    }

    void ResetError( ) {
        error_sum = 0;
        error_den = 0;
    } //zerowanie bledu uczenia

    double GetError( ) {
        return error_sum / (double) error_den;
    } //obliczanie bledu
};

#endif