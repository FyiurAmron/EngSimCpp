#include "stdafx.h"

#include "CNeuralNet.h"

NeuralNetwork::NeuralNetwork( ) {
    ;
}

void NeuralNetwork::init( int inputs, int layers, int *neurons, char const *fileName, NNACTFUNC *actFuncs ) {
    //FILE	*plik;

    alpha = 0.025;

    nInputs = inputs;
    nLayers = layers;
    nNeurons = neurons;
    nOutputs = neurons[layers - 1];

    pActFunc = new PACTFUNC[nLayers];
    pActFuncDer = new PACTFUNC[nLayers];
    dWeights = new double **[nLayers];
    dBiases = new double *[nLayers];
    outputsVals = new double *[nLayers];
    s = new double *[nLayers];
    nLayerInputs = new int [nLayers];
    nLayerInputs[0] = nInputs;
    for( int i = 1; i < nLayers; i++ ) {
        nLayerInputs[i] = nNeurons[i - 1];
    }

    for( int nLay = 0; nLay < nLayers; nLay++ ) {
        dBiases[nLay] = new double [nNeurons[nLay]];
        for( int j = 0; j < nNeurons[nLay]; j++ ) {
            dBiases[nLay][j] = ( rand( ) / (double) RAND_MAX - 0.5 )*2 * sqrt( 1.0 / inputs );
        }

        dWeights[nLay] = new double *[nNeurons[nLay]];
        for( int i = 0; i < nNeurons[nLay]; i++ ) {
            dWeights[nLay][i] = new double [nLayerInputs[nLay]];
            for( int j = 0; j < nLayerInputs[nLay]; j++ ) {
                dWeights[nLay][i][j] = ( rand( ) / (double) RAND_MAX - 0.5 )*2 * sqrt( 1.0 / inputs );
            }
        }
        outputsVals[nLay] = new double[nNeurons[nLay]];
        s[nLay] = new double[nNeurons[nLay]];
        switch( actFuncs[nLay] ) {
            case NN_AF_TANSIG:
                pActFunc[nLay] = tansig;
                pActFuncDer[nLay] = tansigD;
                break;
            case NN_AF_PURELIN:
            default:
                pActFunc[nLay] = purelin;
                pActFuncDer[nLay] = purelinD;
                break;
        }
    }



    //MessageBox(NULL,L"Przydzielam wartosci",L"dssdf",MB_OK);

    //przydzial wag i progow
    //plik = fopen(fileName,"r");

    for( int nLay = 0; nLay < nLayers; nLay++ ) {
        //MessageBox(NULL,L"Przydzielam wagi",L"dssdf",MB_OK);
        //ReadMatrix(plik, nNeurons[nLay], nLayerInputs[nLay], dWeights[nLay]);
    }
    for( int nLay = 0; nLay < nLayers; nLay++ ) {
        //MessageBox(NULL,L"Przydzielam progi",L"dssdf",MB_OK);
        //ReadVector(plik, nNeurons[nLay], dBiases[nLay]);
    }

    //fclose(plik);

}

double NeuralNetwork::Calc( double const *inputs, double *outputs ) {
    Calc( inputs );
    memcpy( outputs, outputsVals[nLayers - 1], sizeof (double )*nOutputs );

    return 0;
}

double NeuralNetwork::Calc( double const *inputs, double *outputs, double const *targets ) {
    Calc( inputs );
    memcpy( outputs, outputsVals[nLayers - 1], sizeof (double )*nOutputs );

    for( int i = 0; i < nOutputs; i++ )
        AddErrorSample( outputs[i] - targets[i] );

    return 0;
}

double NeuralNetwork::Calc( double const *inputs ) {
    //double **temp_outs;
    double sum;

    //temp_outs = new double *[nLayers];

    for( int nLay = 0; nLay < nLayers; nLay++ ) {
        //outputs[nLay] = new double [nNeurons[nLay]];
        for( int i = 0; i < nNeurons[nLay]; i++ ) {
            sum = dBiases[nLay][i];
            for( int j = 0; j < nLayerInputs[nLay]; j++ ) {
                if ( nLay == 0 ) {
                    sum += inputs[j] * dWeights[nLay][i][j];
                } else {
                    sum += outputsVals[nLay - 1][j] * dWeights[nLay][i][j];
                }
            }

            outputsVals[nLay][i] = ( *pActFunc[nLay] )( sum );
            //printf("\n%f",outputs[nLay][i]);
        }
    }
    //temp_outs[2][0]=0.9;

    //outputs[0] = temp_outs[nLayers-1][0];
    //outputs[0] = inputs[11];

    return 0;
}

double NeuralNetwork::BackProp( double *inputs, double *outputs ) {
    double sum, sum2;

    Calc( inputs );
    for( int neuron = 0; neuron < nNeurons[nLayers - 1]; neuron++ ) {
        sum = 0;
        for( int i = 0; i < nLayerInputs[nLayers - 1]; i++ ) {
            sum += dWeights[nLayers - 1][neuron][i] * outputsVals[nLayers - 2][i];
        }
        s[nLayers - 1][neuron] = -2 * ( *pActFuncDer[nLayers - 1] )( sum + dBiases[nLayers - 1][neuron] )*( outputs[neuron] - outputsVals[nLayers - 1][neuron] );
    }

    for( int nLay = nLayers - 2; nLay >= 0; nLay-- ) {
        for( int neuron = 0; neuron < nNeurons[nLay]; neuron++ ) {
            sum = 0;
            for( int i = 0; i < nLayerInputs[nLay]; i++ ) {
                if ( nLay != 0 ) {
                    sum += dWeights[nLay][neuron][i] * outputsVals[nLay - 1][i];
                } else {
                    sum += dWeights[nLay][neuron][i] * inputs[i];
                }
            }
            sum2 = 0;
            for( int i = 0; i < nNeurons[nLay + 1]; i++ ) {
                sum2 += dWeights[nLay + 1][i][neuron] * s[nLay + 1][i];
            }
            s[nLay][neuron] = ( *pActFuncDer[nLay] )( sum + dBiases[nLay][neuron] ) * sum2;
        }
    }

    for( int nLay = 0; nLay < nLayers; nLay++ ) {
        for( int neuron = 0; neuron < nNeurons[nLay]; neuron++ ) {
            if ( nLay != 0 ) {
                for( int i = 0; i < nLayerInputs[nLay]; i++ ) {
                    dWeights[nLay][neuron][i] -= alpha * s[nLay][neuron] * outputsVals[nLay - 1][i];
                }
            } else {
                for( int i = 0; i < nLayerInputs[nLay]; i++ ) {
                    dWeights[nLay][neuron][i] -= alpha * s[nLay][neuron] * inputs[i];
                }
            }
            dBiases[nLay][neuron] -= alpha * s[nLay][neuron]; //*outputsVals[nLay][neuron];
        }
    }

    return 0;
}