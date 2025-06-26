#pragma once

#include "wfcore/pipeline/Pipeline.h"
#include <networktables/NetworkTableInstance.h>
#include <networktables/NetworkTable.h>
#include <networktables/RawTopic.h>

#include <string>

namespace wf {

    class NTDataPublisher {
    public:
        NTDataPublisher(const nt::NetworkTable& devRootTable, const std::string& name);
        void publishPipelineResult(const PipelineResult& result);
    private:
        std::shared_ptr<nt::NetworkTable> table;
        nt::RawPublisher pipelineResultPub;
    };
}