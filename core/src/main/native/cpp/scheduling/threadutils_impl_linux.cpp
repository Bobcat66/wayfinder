// Linux threadutils implementation. When built for windows or mac, this file compiles to nothing
#ifdef __linux__

#define _GNU_SOURCE
#include "wfcore/scheduling/threadutils.h"
#include <pthread.h>
#include <sched.h>

namespace wf {
    int setCPUAffinity(const std::thread& thread, const std::vector<int>& cpus) {
        pthread_t pt = thread.native_handle();
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        for (int cpu : cpus) {
            CPU_SET(cpu, &cpuset);
        }
        return pthread_setaffinity_np(pt, sizeof(cpu_set_t), &coreset);
    }

    int setThreadPriority(const std::thread& thread, int priority) {
        
    }

    int setThreadPolicy(const std::thread& thread, int policy);

    int setProcessPolicyNT(int policy);

    std::vector<int> getCPUAffinity(const std::thread& thread);
}

#endif // _POSIX_VERSION


