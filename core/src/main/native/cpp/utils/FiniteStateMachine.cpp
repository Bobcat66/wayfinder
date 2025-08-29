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

#include "wfcore/utils/FiniteStateMachine.h"

namespace wf {
    uint32_t FSMInterface::getState() { return fsm->getState(); }
    void FSMInterface::halt() { fsm->halt(); }

    FiniteStateMachine::FiniteStateMachine(Dispatcher dispatcher_, void* closure_, void* dispatchContext_)
    : dispatcher(dispatcher_), closure(closure_), dispatchContext(dispatchContext_) {
        // This code is meant to be easily portable to C, which is why I use a raw function pointer
        interface = FSMInterface(this);
    }

    void FiniteStateMachine::reset(uint32_t newstate) {
        state = newstate;
        halted = false;
    }
    // Returns 0 if the machine did not halt, 1 if the machine did halt, and all negative values are reserved as error codes.
    // In general, error codes are reserved for structural errors (e.g. a malformed statemap). Errors in business logic should
    // be reported and propagated with states
    int FiniteStateMachine::run() {
        if (halted) {
            // Machine is halted, return immediately
            return fsmHalted;
        }
        auto handler = dispatcher(state,dispatchContext);
        if (!handler) return fsmBadState;
        state = handler(&interface, closure);
        return halted ? fsmHalted : fsmRunning;
    }
}