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

#include "wfcore/common/scheduling/ThreadPool.h"
#include "wfcore/network/NTDataPublisher.h"

#include <networktables/NetworkTableInstance.h>
#include <networktables/NetworkTable.h>

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

namespace wf {

    class NetworkTablesManager {
    public:
        NetworkTablesManager(const std::string& device_name,int team,bool server);
        const std::shared_ptr<nt::NetworkTable> getRootTable() const {return devRootTable;} 
        std::weak_ptr<NTDataPublisher> getDataPublisher(const std::string& name);
    private:
        std::shared_ptr<nt::NetworkTable> devRootTable;
        std::unordered_map<std::string, std::shared_ptr<NTDataPublisher>> dataPublishers;
    };
}