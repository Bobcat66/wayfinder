#include "wfcore/network/NTDataPublisher.h"
#include "wfcore/common/serde/serde.h"
#include <networktables/RawTopic.h>

namespace wf {
    NTDataPublisher::NTDataPublisher(const nt::NetworkTable& devRootTable, const std::string& name) 
    : table(devRootTable.GetSubTable(name))
    , pipelineResultPub(table->GetRawTopic("pipeline_result").Publish("application/octet-stream")) {}

    void NTDataPublisher::publishPipelineResult(const PipelineResult& result) {
        std::vector<byte> pipelineResultBin;
        packPipelineResult(pipelineResultBin,result);
        pipelineResultPub.Set(pipelineResultBin);
    }
}