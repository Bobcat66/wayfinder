#include "wfcore/network/NTDataPublisher.h"
#include "wfcore/common/marshalling/marshalling.h"
namespace wf {
    NTDataPublisher::NTDataPublisher(const nt::NetworkTable& devRootTable, const std::string& name) {
        table = devRootTable.GetSubTable(name);
        pipelineResultPublisher = table.GetRawTopic("pipeline_result").Publish();
    }
    void NTDataPublisher::publishPipelineResult(const PipelineResult& result) {
        std::vector<byte> pipelineResultBin;
        marshalPipelineResult(pipelineResultBin,result);
        pipelineResultPublisher.set
    }
}