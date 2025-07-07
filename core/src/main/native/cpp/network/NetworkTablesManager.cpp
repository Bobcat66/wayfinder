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

#include "wfcore/network/NetworkTablesManager.h"
#include "wfcore/common/logging/LoggerManager.h"
#include <cstdlib>
#include <string>
#include <format>

namespace wf {

    static loggerPtr logger = LoggerManager::getInstance().getLogger("NetworkTablesManager",LogGroup::Network);

    std::weak_ptr<NTDataPublisher> NetworkTablesManager::getDataPublisher(const std::string& name) {
        std::weak_ptr<NTDataPublisher> ptr;
        auto it = this->dataPublishers.find(name);
        if (it != this->dataPublishers.end()) {
            ptr = it->second;
        } else {
            this->dataPublishers.emplace(name,std::make_shared<NTDataPublisher>(this->devRootTable,name));
            ptr = this->dataPublishers.at(name);
        }
        return ptr;
    }

    NetworkTablesManager::NetworkTablesManager() {
        const char* devname = std::getenv("DEVICE_NAME");
        const char* team = std::getenv("TEAM");
        int teamnum;
        if (team) {
            teamnum = std::atoi(team);
        } else {
            logger->warn("Team number is not set!");
            teamnum = 1234;
        }
        if (devname) {
            std::string devname_str(devname);
            auto inst = nt::NetworkTableInstance::GetDefault();
            inst.StartClient4(devname_str);
            inst.SetServerTeam(teamnum);
            this->devRootTable = inst.GetTable(std::format("wayfinder/{}",devname_str));
        } else {
            logger->info("Device name not set. Defaulting to 'wayfinder'");
            auto inst = nt::NetworkTableInstance::GetDefault();
            inst.StartClient4("wayfinder");
            inst.SetServerTeam(teamnum);
            this->devRootTable = inst.GetTable("wayfinder/wayfinder");
        }
    }
}