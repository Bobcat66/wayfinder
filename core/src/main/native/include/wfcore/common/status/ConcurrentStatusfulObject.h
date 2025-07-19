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

#include "wfcore/common/status/StatusfulObject.h"
#include <mutex>

// Somewhat naive implementation, there's probably a better way to do this
namespace wf {
    template <status_code T, T nominal_status>
    class ConcurrentStatusfulObject : public StatusfulObject<T,nominal_status> {
    public:
        [[nodiscard]]
        virtual T getStatus() const noexcept { 
            std::lock_guard lock(status_mtx);
            return this->status_; 
        }

        [[ nodiscard ]]
        virtual bool ok() const noexcept { 
            std::lock_guard lock(status_mtx);
            return this->status_ == nominal_status; 
        }

        /*
         * Returns a human-readable string describing error in more detail.
         */
        [[ nodiscard ]]
        virtual std::optional<std::string> getError() const noexcept {
            std::lock_guard lock(status_mtx);
            if (this->status_ == nominal_status) {
                return std::nullopt;
            }
            return this->errorMsg_;
        }

        virtual void clearFaults() const noexcept {
            std::lock_guard lock(status_mtx);
            this->status_ = nominal_status;
            this->errorMsg_.clear();
        }
    protected:
        mutable std::mutex status_mtx;
        // error message is only meaningful when the status code isn't nominal
        // Note, as this function merely masks the StatusfulObject's function, rather than overriding it,
        // It will NOT have runtime polymorphic behavior
        template <typename... Args>
        void reportError(T status, std::string_view fmt, Args&&... args) const noexcept {
            std::lock_guard lock(status_mtx);
            this->StatusfulObject<T,nominal_status>::reportError(status,fmt,std::forward<Args>(args)...);
        }

        virtual void reportOk() const noexcept {
            std::lock_guard lock(status_mtx);
            this->status_ = nominal_status;
        }
    };
}