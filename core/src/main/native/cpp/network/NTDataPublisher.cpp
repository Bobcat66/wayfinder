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

#include "wips/wips_runtime.h"
#include "wfcore/network/NTDataPublisher.h"
#include "wfcore/common/serde/legacy.h"
#include <networktables/RawTopic.h>
#include <span>

namespace wf {
    NTDataPublisher::NTDataPublisher(const std::shared_ptr<nt::NetworkTable> devRootTable, const std::string& name) 
    : table(devRootTable->GetSubTable(name))
    , pipelineResultPub(table->GetRawTopic("pipeline_result").Publish("application/octet-stream")) {}

    void NTDataPublisher::publishPipelineResult(const PipelineResult& result) {
        // WIP, test
        auto bin = packPipelineResult(result);
        pipelineResultPub.Set(std::span<const uint8_t>(bin->base,bin->offset));
        wips_bin_destroy(bin);
    }
}