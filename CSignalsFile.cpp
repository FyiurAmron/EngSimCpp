#include "stdafx.h"
#include "CSignalsFile.h"

void CSignalsFile::SaveSignals( std::vector<CChart*> &charts, SaveSignalFormat format ) {
    TCHAR delimiter = ';';
    TCHAR formatBuffer[255];
    int line = 0;
    int finished = 1;
    int nSignal = 0;
    int *vectNums;

    double time, timeMin;

    if ( SelectAndSave( _T( "dat" ), _T( "Plik z przebiegami" ) ) ) {
        // NAGLOWEK

        if ( format == SaveSignalCSVOneTimeColumn ) printf( _T( "t%c" ), delimiter );
        for( int i = 0; i < charts.size( ); i++ ) {
            for( int j = 0; j < charts[i]->signals.size( ); j++ ) {
                if ( format == SaveSignalCSVMultipleTimeColumn ) printf( _T( "t%c" ), delimiter );
                printf( _T( "%s%c" ), charts[i]->signals[j]->sName.c_str( ), delimiter );
                nSignal++;
            }
        }
        printf( _T( "\r\n" ) );



        // DANE

        if ( format == SaveSignalCSVMultipleTimeColumn ) {
            line = 0;
            do {
                finished = 1;
                for( int i = 0; i < charts.size( ); i++ ) {
                    for( int j = 0; j < charts[i]->signals.size( ); j++ ) {
                        if ( line < charts[i]->signals[j]->nSamples ) {
                            printf( _T( "%f%c%f%c" ), charts[i]->signals[j]->x[line], delimiter, charts[i]->signals[j]->y[line], delimiter );
                            finished = 0;
                        } else {
                            printf( _T( ";;" ) );
                        }
                    }
                }
                printf( _T( "\r\n" ) );
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
                for( int i = 0; i < charts.size( ); i++ ) {
                    for( int j = 0; j < charts[i]->signals.size( ); j++ ) {
                        if ( vectNums[nSignal] < charts[i]->signals[j]->nSamples ) {
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
                printf( _T( "%f%c" ), timeMin, delimiter );
                for( int i = 0; i < charts.size( ); i++ ) {
                    for( int j = 0; j < charts[i]->signals.size( ); j++ ) {
                        if ( charts[i]->signals[j]->x[vectNums[nSignal]] <= timeMin ) {
                            vectNums[nSignal]++;
                        }
                        printf( _T( "%f%c" ), charts[i]->signals[j]->y[vectNums[nSignal]], delimiter );
                        nSignal++;
                    }
                }
                printf( _T( "\r\n" ) );
            } while( 1 ); //petla konczona brakeiem

            delete[] vectNums;
        }

        CloseFile( );
    }
}