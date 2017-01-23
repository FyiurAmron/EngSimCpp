#ifndef SIGFILE_H
#define SIGFILE_H

#include "CFile.h"
#include "CSignal.h"
#include "CChart.h"

#include <vector>

enum SaveSignalFormat {
    SaveSignalCSVOneTimeColumn, SaveSignalCSVMultipleTimeColumn
};

class CSignalsFile : public CFile {
public:
    void SaveSignals( std::vector<CChart*> &charts, SaveSignalFormat format );
};

#endif