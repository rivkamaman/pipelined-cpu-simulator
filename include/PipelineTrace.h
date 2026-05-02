#ifndef PIPELINE_TRACE_H
#define PIPELINE_TRACE_H

#include <string>

struct PipelineTrace {
    std::string fetch;
    std::string decode;
    std::string execute;
    std::string memory;
    std::string writeBack;
};

#endif
