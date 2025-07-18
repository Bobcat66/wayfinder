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

// This is a prototype implementation
// I will refactor it with a proper inheritance structure later
// All I need right now is a minimal working example

namespace wf {
    template <status_code T, T nominal_status>
    class ConcurrentLoggedStatusfulObject : public StatusfulObject<T,nominal_status> {
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

        ConcurrentLoggedStatusfulObject(const std::string& name, const spdlog::level::level_enum logLevel) {
            this->logger_ = LoggerManager::getInstance().getLogger(name,logLevel);
        }
        // error message is only meaningful when the status code isn't nominal
        // Note, as this function merely masks the StatusfulObject's function, rather than overriding it,
        // It will NOT have runtime polymorphic behavior
        template <typename... Args>
        void reportError(T status, std::string_view fmt, Args&&... args) const noexcept {
            std::lock_guard lock(status_mtx);
            this->StatusfulObject<T,nominal_status>::reportError(status,fmt,std::forward<Args>(args)...);
            logger_->error(this->errorMsg_);
        }

        // same functionality as the reportError method, but logs the message as a warning and not an error
        template <typename... Args>
        void reportWarning(T status, std::string_view fmt, Args&&... args) const noexcept {
            std::lock_guard lock(status_mtx);
            // The base method only formats and sets status, does not log
            this->StatusfulObject<T,nominal_status>::reportError(status,fmt,std::forward<Args>(args)...);
            logger_->warn(this->errorMsg_);
        }

        virtual void reportOk() const noexcept {
            std::lock_guard lock(status_mtx);
            this->status_ = nominal_status;
        }

        loggerPtr& logger() const noexcept { return this->logger_; }

        mutable std::mutex status_mtx;
        mutable loggerPtr logger_;
    };
}