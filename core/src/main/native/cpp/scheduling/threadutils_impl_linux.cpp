// Linux threadutils implementation. When built for windows or mac, this file compiles to nothing
#ifdef __linux__

#define _GNU_SOURCE
#include "wfcore/scheduling/threadutils.h"
#include <pthread.h>
#include <sched.h>
#include <unordered_set>
#include <unordered_map>
#include <sys/resource.h> 

namespace wf {
    static std::unordered_set<ThreadPriority> RTPriorities = {
        ThreadPriority::HIGH,
        ThreadPriority::HIGHEST,
        ThreadPriority::SENSITIVE,
        ThreadPriority::CRITICAL,
        ThreadPriority::REALTIME,
        ThreadPriority::ELEVEN
    };
    static std::unordered_map<ThreadPriority,int> rt_priority_map {
        {ThreadPriority::HIGH,1},
        {ThreadPriority::HIGHEST,10},
        {ThreadPriority::SENSITIVE,30},
        {ThreadPriority::CRITICAL,50},
        {ThreadPriority::REALTIME,80},
        {ThreadPriority::ELEVEN,99}
    };
    int setCPUAffinity(std::thread& thread, const std::vector<int>& cpus) {
        pthread_t pt = thread.native_handle();
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        for (int cpu : cpus) {
            CPU_SET(cpu, &cpuset);
        }
        return pthread_setaffinity_np(pt, sizeof(cpu_set_t), &cpuset);
    }

    int setThreadPriority(std::thread& thread, ThreadPriority priority) {
        pthread_t pt = thread.native_handle();
        if (rt_priority_map.contains(priority)) {
            sched_param param;
            param.sched_priority = rt_priority_map.at(priority);
            int policy = SCHED_RR; // Round robin scheduling
            return pthread_setschedparam(pt, policy, &param);
        } else {
            // Non-rt niceness is handled on a process-level basis, do nothing
            return 0;
        }
    }

    std::vector<int> getCPUAffinity(std::thread& thread) {
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);

        pthread_t nativeHandle = thread.native_handle();

        int ret = pthread_getaffinity_np(nativeHandle, sizeof(cpu_set_t), &cpuset);
        if (ret != 0) {
            throw std::runtime_error("Failed to get thread CPU affinity");
        }

        std::vector<int> cpus;
        int nproc = CPU_SETSIZE;

        for (int cpu = 0; cpu < nproc; ++cpu) {
            if (CPU_ISSET(cpu, &cpuset)) {
                cpus.push_back(cpu);
            }
        }

        return cpus;
    }
}

#endif // __linux__


