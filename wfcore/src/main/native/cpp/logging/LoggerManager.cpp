// Copyright (c) 2025 Jesse Kane
// SPDX-License-Identifier: BSD-3-Clause

#include "wfcore/logging/LoggerManager.h"

#include <spdlog/sinks/stdout_color_sinks.h>

using namespace wf;

loggerPtr LoggerManager::getLogger(const std::string& name, const spdlog::level::level_enum logLevel){
    auto logger = spdlog::get(name);
    if (!logger) {
        logger = std::make_shared<spdlog::logger>(name, sinks_.begin(), sinks_.end());
        spdlog::register_logger(logger);
    }
    logger->set_level(logLevel);
    return logger;
}

loggerPtr LoggerManager::getLogger(const std::string& name){
    auto logger = spdlog::get(name);
    if (!logger) {
        logger = std::make_shared<spdlog::logger>(name, sinks_.begin(), sinks_.end());
        spdlog::register_logger(logger);
    }
    return logger;
}

LoggerManager::LoggerManager(){
    // TODO: Make this configurable with environment variables
    sinks_.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
}