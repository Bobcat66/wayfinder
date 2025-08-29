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

#include <cstdint>

#define WFTS_MSG_LEADER             0b00000001 // Whether or not the message is from the leader
#define WFTS_MSG_BROADCAST          0b00000010 // Whether or not the message is broadcasted
#define WFTS_MSG_CRITICAL           0b00000100 // Whether or not the message is time-critical
#define WFTS_MSG_HASTIME            0b00001000 // Whether or not the message has a meaningful timestamp
#define WFTS_MSG_ERROR              0b10000000 // Error flag

#define WFTS_MSG_SYNCFLAGS          (WFTS_MSG_LEADER | WFTS_MSG_BROADCAST | WFTS_MSG_CRITICAL)    // Required flags for a sync message
#define WFTS_MSG_FOLLOWUPFLAGS      (WFTS_MSG_LEADER | WFTS_MSG_BROADCAST | WFTS_MSG_HASTIME)     // Required flags for a followup message
#define WFTS_MSG_FOLLOWUPNFLAGS     (WFTS_MSG_CRITICAL)                                           // Disallowed flags for a followup message
#define WFTS_MSG_DELAYREQFLAGS      (WFTS_MSG_CRITICAL)                                           // Required flags for a delayreq message
#define WFTS_MSG_DELAYREQNFLAGS     (WFTS_MSG_LEADER | WFTS_MSG_BROADCAST | WFTS_MSG_HASTIME)     // Disallowed flags for a delayreq message
#define WFTS_MSG_DELAYRESPFLAGS     (WFTS_MSG_LEADER | WFTS_MSG_HASTIME)                          // Required flags for a delayresp message
#define WFTS_MSG_DELAYRESPNFLAGS    (WFTS_MSG_BROADCAST | WFTS_MSG_CRITICAL)                      // Disallowed flags for a delayresp message

#define WFTS_TSPACKET_SIZE 13 // This is the size of the *packed* packet, the WIPS struct is padded

namespace detail {
    inline constexpr bool flagcheck(uint8_t flags, uint8_t reqs, uint8_t disallowed = 0) {
        return ((flags & reqs) == reqs) && ((flags & disallowed) == 0);
    }
}
