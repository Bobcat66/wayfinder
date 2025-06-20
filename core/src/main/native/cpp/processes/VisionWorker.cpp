#include "wfcore/processes/VisionWorker.h"
#include "wfcore/video/video_utils.h"

namespace wf {
    VisionWorker::VisionWorker(
        Pipeline& pipeline_, 
        FrameProvider& frameProvider_, 
        ResultConsumer resultConsumer_
    )
    : pipeline(pipeline_)
    , frameProvider(frameProvider_)
    , resultConsumer(std::move(resultConsumer_)) {
        inputBuffer = generateEmptyFrameBuf(frameProvider.getStreamFormat().frameFormat);
        running = false;
    }
}