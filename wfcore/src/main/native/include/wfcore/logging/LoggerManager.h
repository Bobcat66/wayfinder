// Copyright (c) 2025 Jesse Kane
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/sink.h>

#include <unordered_map>
#include <memory>
#include <mutex>
#include <vector>

namespace wf {

    using loggerPtr = std::shared_ptr<spdlog::logger>;

    // Aliases for logging levels, meant to simplify instantiation of loggers for related subsystems
    namespace LogGroup {
        constexpr spdlog::level::level_enum General   = spdlog::level::info;
        constexpr spdlog::level::level_enum Pipeline  = spdlog::level::info;
        constexpr spdlog::level::level_enum Gstreamer = spdlog::level::info;
        constexpr spdlog::level::level_enum Network   = spdlog::level::warn;
        constexpr spdlog::level::level_enum Config    = spdlog::level::debug;
        constexpr spdlog::level::level_enum System    = spdlog::level::debug;
    } // TODO: tune these logging levels

    class LoggerManager {
        public:
        static LoggerManager& getInstance() {
            static LoggerManager instance;
            return instance;
        }

        loggerPtr getLogger(const std::string& name, const spdlog::level::level_enum logLevel);
        
        loggerPtr getLogger(const std::string& name);

        private:
        static LoggerManager* instancePtr;
        std::vector<spdlog::sink_ptr> sinks_; // All loggers output to the same sinks for simplicity
        std::unordered_map<std::string, std::shared_ptr<spdlog::logger>> loggers_;
        std::mutex mutex_;
        LoggerManager();
        ~LoggerManager();

        // Prevent copying and moving
        LoggerManager(const LoggerManager&) = delete;
        LoggerManager& operator=(const LoggerManager&) = delete;
        LoggerManager(LoggerManager&&) = delete;
        LoggerManager& operator=(LoggerManager&&) = delete;
    };
}