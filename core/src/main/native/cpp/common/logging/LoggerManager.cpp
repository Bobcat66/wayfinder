/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * This file is part of Wayfinder.
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

#include "wfcore/common/logging/LoggerManager.h"

#include <spdlog/sinks/stdout_color_sinks.h>

namespace wf {

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

}