// MacOS threadutils implementation. When built for windows or linux, this file compiles to nothing
#if defined(__APPLE__) && defined(__MACH__)

#include "wfcore/scheduling/threadutils.h"

namespace wf {
    // Pins a thread to only run on certain CPU cores
    int setCPUAffinity(const std::thread& thread, const std::vector<int>& cores);

    // Sets thread priority of a thread
    int setThreadPriority(const std::thread& thread, int priority);

}
#endif //defined(__APPLE__) && defined(__MACH__)