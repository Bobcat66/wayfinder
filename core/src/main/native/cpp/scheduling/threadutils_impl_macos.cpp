// MacOS threadutils implementation. When built for windows or linux, this file compiles to nothing
#if defined(__APPLE__) && defined(__MACH__)

#include "wfcore/scheduling/threadutils.h"

namespace wf {
    // Pins a thread to only run on certain CPU cores
    int setCPUAffinity(const std::thread& thread, const std::vector<int>& cores);

    // Sets thread priority of a thread
    int setThreadPriority(const std::thread& thread, int priority);

    // Sets the scheduling policy of a thread
    int setThreadPolicy(const std::thread& thread, int policy);

    // Sets the scheduling policy of the process (Windows only)
    int setProcessPolicyNT(int policy);
}
#endif //defined(__APPLE__) && defined(__MACH__)