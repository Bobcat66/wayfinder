#pragma once

#include <vector>

namespace wf {
    // Sets a thread to only run on certain CPU cores
    int setCPUAffinity(const std::thread& thread, const std::vector<int>& cores);

    // Sets thread priority of a thread
    int setThreadPriority(const std::thread& thread, int priority);

    // Sets the scheduling policy of a thread
    int setThreadPolicy(const std::thread& thread, int policy);

    // Sets the scheduling policy of the process (Windows only)
    int setProcessPolicyNT(int policy);

    // Returns the CPUs that a given thread is allowed to run on
    std::vector<int> getCPUAffinity(const std::thread& thread);
}