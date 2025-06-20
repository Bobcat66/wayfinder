#pragma once

#include <vector>
#include <thread>

namespace wf {

    enum class ThreadPriority {
        LOWEST,
        LOW,
        NORMAL,
        HIGH,
        HIGHEST,
        SENSITIVE,
        CRITICAL,
        REALTIME,
        ELEVEN
    };

    // Sets a thread to only run on certain CPU cores
    int setCPUAffinity(const std::thread& thread, const std::vector<int>& cores);

    // Sets thread priority of a thread
    int setThreadPriority(const std::thread& thread, ThreadPriority priority);

    // Returns the CPUs that a given thread is allowed to run on
    std::vector<int> getCPUAffinity(const std::thread& thread);
}