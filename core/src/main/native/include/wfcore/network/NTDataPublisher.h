#pragma once

#include "wfcore/network/NetworkTablesManager.h"
#include "wfcore/pipeline/Pipeline.h"
#include <networktables/NetworkTableInstance.h>
#include <networktables/NetworkTable.h>
#include <networktables/StructArrayTopic.h>

#include <string>

namespace wf {
    class NTDataPublisher {
    public:
        NTDataPublisher(const std::string& name);
        void publishPipelineResult(const PipelineResult& result);
    private:
        const NetworkTablesManager& manager;
    }
}