/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (C) 2025 Jesse Kane
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */


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
        #ifdef NDEBUG
        // Non-debugging log levels
        constexpr spdlog::level::level_enum General   = spdlog::level::info;
        constexpr spdlog::level::level_enum Pipeline  = spdlog::level::info;
        constexpr spdlog::level::level_enum Gstreamer = spdlog::level::info;
        constexpr spdlog::level::level_enum Network   = spdlog::level::warn;
        constexpr spdlog::level::level_enum Config    = spdlog::level::debug;
        constexpr spdlog::level::level_enum System    = spdlog::level::debug;
        #else
        // debugging log levels
        constexpr spdlog::level::level_enum General   = spdlog::level::debug;
        constexpr spdlog::level::level_enum Pipeline  = spdlog::level::debug;
        constexpr spdlog::level::level_enum Gstreamer = spdlog::level::debug;
        constexpr spdlog::level::level_enum Network   = spdlog::level::debug;
        constexpr spdlog::level::level_enum Config    = spdlog::level::debug;
        constexpr spdlog::level::level_enum System    = spdlog::level::debug;
        #endif // NDEBUG
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
        std::vector<spdlog::sink_ptr> sinks_; // All loggers output to the same sinks for simplicity
        std::unordered_map<std::string, std::shared_ptr<spdlog::logger>> loggers_;
        std::mutex mutex_;
        LoggerManager();
        ~LoggerManager() = default;

        // Prevent copying and moving
        LoggerManager(const LoggerManager&) = delete;
        LoggerManager& operator=(const LoggerManager&) = delete;
        LoggerManager(LoggerManager&&) = delete;
        LoggerManager& operator=(LoggerManager&&) = delete;
    };
}