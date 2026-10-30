#pragma once

#include "LogAnalyzer.h"
#include "LogReader.h"

#include <string>
#include <vector>

class ReportFormatter {
public:
    std::string format(const AnalysisResult& analysis,
                       const std::vector<ReaderDiagnostic>& diagnostics = {}) const;
};
