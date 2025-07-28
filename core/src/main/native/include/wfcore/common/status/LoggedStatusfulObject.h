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

#include "wfcore/common/logging.h"
#include "wfcore/common/status/StatusfulObject.h"

namespace wf {

   
    template <status_code status_type, status_type nominal_status, const char* (*StringMapper) (status_type)>
    class LoggedStatusfulObject : public StatusfulObject<status_type,nominal_status,StringMapper> {
    protected:
        LoggedStatusfulObject(const std::string& name, const spdlog::level::level_enum logLevel) {
            this->logger_ = LoggerManager::getInstance().getLogger(name,logLevel);
        }

        // error message is only meaningful when the status code isn't nominal
        // Note, as this function merely masks the StatusfulObject's function, rather than overriding it,
        // It will NOT have runtime polymorphic behavior
        template <typename... Args>
        void reportError(status_type status, std::string_view fmt, Args&&... args) const noexcept {
            this->StatusfulObject<status_type,nominal_status,StringMapper>::reportError(status,fmt,std::forward<Args>(args)...);
            logger_->error(this->errorMsg_);
        }

        // scuffed, fix later
        void reportError(status_type status) {
            this->status_ = status;
            logger_->error(getError());
        }

        // scuffed, fix later
        void reportError(const StatusfulResult<status_type,status_type,StringMapper>& result) const noexcept {
            this->status_ = result.status();
            this->errorMsg_ = result.what();
            logger_->error(this->errorMsg_);
        }

        // same functionality as the reportError method, but logs the message as a warning and not an error
        template <typename... Args>
        void reportWarning(status_type status, std::string_view fmt, Args&&... args) const noexcept {
            // The base method only formats and sets status, does not log
            this->StatusfulObject<status_type,nominal_status,StringMapper>::reportError(status,fmt,std::forward<Args>(args)...);
            logger_->warn(this->errorMsg_);
        }

        loggerPtr& logger() const noexcept { return this->logger_; }

        mutable loggerPtr logger_;
    };
}

