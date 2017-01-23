#include "stdafx.h"
#include "CSignalsFile.h"
#include <limits>

void CSignalsFile::SaveSignals( std::vector<CChart*> &charts, SaveSignalFormat format ) {
    wchar_t delimiter = ';';
    //wchar_t formatBuffer[255];
    unsigned int line = 0;
    int finished = 1;
    int nSignal = 0;
    int *vectNums;

    double time, timeMin;

    if ( SelectAndSave( L"dat", L"Plik z przebiegami" ) ) {
        // NAGLOWEK

        if ( format == SaveSignalCSVOneTimeColumn ) printf( L"t%c", delimiter );
        for( size_t i = 0; i < charts.size( ); i++ ) {
            for( size_t j = 0; j < charts[i]->signals.size( ); j++ ) {
                if ( format == SaveSignalCSVMultipleTimeColumn ) printf( L"t%c", delimiter );
                printf( L"%s%c", charts[i]->signals[j]->sName.c_str( ), delimiter );
                nSignal++;
            }
        }
        printf( L"\r\n" );



        // DANE

        if ( format == SaveSignalCSVMultipleTimeColumn ) {
            line = 0;
            do {
                finished = 1;
                for( size_t i = 0; i < charts.size( ); i++ ) {
                    for( size_t j = 0; j < charts[i]->signals.size( ); j++ ) {
                        if ( line < charts[i]->signals[j]->nSamples ) {
                            printf( L"%f%c%f%c", charts[i]->signals[j]->x[line], delimiter, charts[i]->signals[j]->y[line], delimiter );
                            finished = 0;
                        } else {
                            printf( L";;" );
                        }
                    }
                }
                printf( L"\r\n" );
                line++;
            } while( !finished );
        } else
            if ( format == SaveSignalCSVOneTimeColumn ) {
            vectNums = new int[nSignal];
            for( int i = 0; i < nSignal; i++ ) vectNums[i] = 0;
            time = 0;

            do {
                finished = 1;
                timeMin = std::numeric_limits<double>::infinity( );
                nSignal = 0;
                for( size_t i = 0; i < charts.size( ); i++ ) {
                    for( size_t j = 0; j < charts[i]->signals.size( ); j++ ) {
                        if ( vectNums[nSignal] < (int) charts[i]->signals[j]->nSamples ) {
                            if ( charts[i]->signals[j]->x[vectNums[nSignal]] < timeMin ) timeMin = charts[i]->signals[j]->x[vectNums[nSignal]]; //poszukiwanie sygnalu z kolejnym najmniejszym czasem
                            finished = 0;
                            nSignal++;
                        }
                    }
                }

                if ( finished ) break;
                if ( time == timeMin ) continue;
                time = timeMin;

                nSignal = 0;
                printf( L"%f%c", timeMin, delimiter );
                for( size_t i = 0; i < charts.size( ); i++ ) {
                    for( size_t j = 0; j < charts[i]->signals.size( ); j++ ) {
                        if ( charts[i]->signals[j]->x[vectNums[nSignal]] <= timeMin ) {
                            vectNums[nSignal]++;
                        }
                        printf( L"%f%c", charts[i]->signals[j]->y[vectNums[nSignal]], delimiter );
                        nSignal++;
                    }
                }
                printf( L"\r\n" );
            } while( 1 ); //petla konczona brakeiem

            delete[] vectNums;
        }

        CloseFile( );
    }
}