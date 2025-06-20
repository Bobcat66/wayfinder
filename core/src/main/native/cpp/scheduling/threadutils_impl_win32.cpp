// Windows NT threadutils implementation. When built for a POSIX system, this file compiles to nothing
#ifdef _WIN32

#include "wfcore/scheduling/threadutils.h"

namespace wf {
    // Pins a thread to only run on certain CPU cores
    int setCPUAffinity(const std::thread& thread, const std::vector<int>& cores) {
        return 0; // Placeholder
    }

    // Sets thread priority of a thread
    int setThreadPriority(const std::thread& thread, int priority) {
        return 0; // Placeholder
    }
}
#endif // _WIN32