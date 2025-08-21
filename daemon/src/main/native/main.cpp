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

#include <iostream>

#include "version.h"
#include "wfcore/processes/WFOrchestrator.h"
#include "wfd.h"
#include <exception>

// signals to the launcher

// Success, will cause the launcher to exit
#define WAYFINDER_SUCCESS 0
// General error, will cause the launcher to exit
#define WAYFINDER_ERROR 1
// Restart required, signals the launcher to restart wayfinder
#define WAYFINDER_RESTART 2
// Reload required, signals the launcher to reload environment variables and restart wayfinder
#define WAYFINDER_RELOAD 3
// Shutdown request, signals the launcher to execute shutdown now
#define WAYFINDER_SHUTDOWN 4
// Reboot request, signals the launcher to execute shutdown -r now
#define WAYFINDER_REBOOT 5


int main() {
    std::cout << "Starting Wayfinder v" << PROJECT_VERSION << std::endl;
    try {
        auto orch = wf::WFOrchestrator::createFromEnv();
        orch.configureHardware();
        orch.configureWorkers();
        while (true) {
            // main loop
            auto lock = wfd::getLock();
            orch.periodic();
        }
    } catch (const std::exception& e) {
        // TODO: More descriptive error handling
        std::cerr << "Caught an unhandled exception: " << e.what() << std::endl;
        return WAYFINDER_ERROR;
    }
    return WAYFINDER_SUCCESS;
}