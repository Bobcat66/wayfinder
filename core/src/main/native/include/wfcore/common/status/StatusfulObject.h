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

#include <concepts>
#include <type_traits>
#include <string>
#include <optional>
#include <utility>
#include <format>
#include "wfcore/common/status/StatusType.h"
#include "wfcore/common/status/StatusfulResult.h"

/*
 * A base class for components with status reporting and error information.
 * 
 * This class avoids exceptions and instead uses explicit status codes and optional
 * error messages. All operations that can fail update the internal status.
 *
 * NOTE: `reportError()` and `clearFaults()` are marked `const`, as status may change
 * even in logically `const` methods. The `status_` and `errorMsg_` members are therefore
 * marked `mutable` to enable this without breaking const-correct interfaces.
 * 
 * In general, a StatusfulObject's status should only be updated in the case of a severe fault,
 * transient errors (timeouts, dropped frames, etc.) should NOT update the internal status
 * 
 * TODO: Maybe switch to CRTP?
 */

namespace wf {

    
    template <status_code status_type, status_type nominal_status, const char* (*StringMapper) (status_type)>
    class StatusfulObject {
    public:
        virtual ~StatusfulObject() = default;

        [[nodiscard]]
        virtual status_type getStatus() const noexcept { return status_; }

        [[ nodiscard ]]
        virtual bool ok() const noexcept { return this->status_ == nominal_status; }

        /*
         * Returns a human-readable string describing error in more detail.
         * TODO: add bad_alloc guards
         */
        [[ nodiscard ]]
        virtual std::string getError() const noexcept {
            if (this->status_ == nominal_status) {
                return "Nominal";
            } else if (this->errorMsg_.empty()) {
                return StringMapper(this->status_);
            }
            return errorMsg_;
        }

        virtual void clearFaults() const noexcept {
            this->status_ = nominal_status;
            this->errorMsg_.clear();
        }

    protected:
        // error message is only meaningful when the status code isn't nominal
        template <typename... Args>
        void reportError(status_type status, std::string_view fmt, Args&&... args) const noexcept {
            this->status_ = status;
            try {
                this->errorMsg_ = std::vformat(fmt, std::make_format_args(args...));
            } catch (...) {
                this->errorMsg_ = "MSGERR unknown";
            }
        }

        void reportError(status_type status) {
            this->status_ = status;
        }

        template <typename T>
        void reportError(const StatusfulResult<T,status_type,nominal_status,StringMapper>& result) const noexcept {
            this->status_ = result.status();
            this->errorMsg_ = result.what();
        }

        virtual void reportOk() const noexcept {
            this->status_ = nominal_status;
        }

        mutable status_type status_;

        // A human-readable message describing an error in more detail. It is only meaningful when the status code isn't nominal
        mutable std::string errorMsg_;
    };
}