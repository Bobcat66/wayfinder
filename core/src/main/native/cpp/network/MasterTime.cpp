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

#include "wfcore/network/MasterTime.h"
#include <ntcore.h>
#include <shared_mutex>
#include <mutex>
#include <cstdint>
#include <atomic>

namespace impl {
    static std::atomic<int64_t(*)(void)> mt_getter_atomic{nullptr};
}

namespace wf {
    int64_t getMasterTime() {
        auto getter = impl::mt_getter_atomic.load();
        if (getter) return getter();
        return nt::Now();
    }
    void loadMasterTimeSupplier(int64_t (*mt_getter_)(void)) {
        if (!mt_getter_) return;
        impl::mt_getter_atomic.store(mt_getter_);
    }
    void unloadMasterTimeSupplier() {
        impl::mt_getter_atomic.store(nullptr);
    }
    bool masterTimeLoaded() {
        return static_cast<bool>(impl::mt_getter_atomic.load());
    }
}