// Copyright (c) 2025 Jesse Kane
// SPDX-License-Identifier: GPL-3.0-or-later

#include <iostream>

#include "wfcore/scheduling/ThreadPool.h"
#include <thread>
#include <chrono>

#include <gtest/gtest.h>

// Tests the thread pool
TEST(processTests, ThreadPoolTest){
    using clock = std::chrono::steady_clock;
    auto pool = wf::ThreadPool(4);
    auto start = clock::now();
    auto message1time = pool.enqueue([] {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        std::cout << "Printed after two seconds" << std::endl;
        return clock::now();
    });
    auto message2time = pool.enqueue([] {
        std::this_thread::sleep_for(std::chrono::seconds(4));
        std::cout << "Printed after four seconds" << std::endl;
        return clock::now();
    });
    auto message3time = pool.enqueue([] {
        std::this_thread::sleep_for(std::chrono::seconds(6));
        std::cout << "Printed after six seconds" << std::endl;
        return clock::now();
    });
    auto message4time = pool.enqueue([] {
        std::this_thread::sleep_for(std::chrono::seconds(8));
        std::cout << "Printed after eight seconds" << std::endl;
        return clock::now();
    });
    EXPECT_NEAR((std::chrono::duration<double>(message1time.get()-start)).count(),2.0,0.1);
    EXPECT_NEAR((std::chrono::duration<double>(message2time.get()-start)).count(),4.0,0.1);
    EXPECT_NEAR((std::chrono::duration<double>(message3time.get()-start)).count(),6.0,0.1);
    EXPECT_NEAR((std::chrono::duration<double>(message4time.get()-start)).count(),8.0,0.1);
}