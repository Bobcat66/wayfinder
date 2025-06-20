// Windows NT threadutils implementation. When built for a POSIX system, this file compiles to nothing
#ifdef _WIN32

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
#endif // _WIN32