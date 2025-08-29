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

#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <cstdint>
#include <type_traits>
#include <memory>

namespace wf {
    constexpr int fsmRunning = 0;
    constexpr int fsmHalted = 1;
    constexpr int fsmBadState = -1;

    class FiniteStateMachine;
    // This is to limit the methods that FSM callbacks have access to
    class FSMInterface {
    public:
        FSMInterface() : fsm(nullptr) {}
        FSMInterface(FiniteStateMachine* fsm_) : fsm(fsm_) {}
        uint32_t getState();
        void halt();
    private:
        FiniteStateMachine* fsm;
    };


    using StateHandler = uint32_t(*)(FSMInterface*,void*);
    // Dispatchers should return nullptr when the state passed in is invalid
    using Dispatcher = StateHandler(*)(uint32_t,void*);

    class FiniteStateMachine {
    public:
        // closure contains data that will be passed to each state handler, while dispatchContext_ will be passed to the dispatcher itself
        FiniteStateMachine(Dispatcher dispatcher_, void* closure_, void* dispatchContext_ = NULL);
        // does NOT reset the closure
        void reset(uint32_t newstate);
        // Returns 0 if the machine did not halt, 1 if the machine did halt, and all negative values are reserved as error codes.
        // In general, error codes are reserved for structural errors (e.g. a malformed statemap). Errors in business logic should
        // be reported and propagated with states
        int step();
        // Runs the machine until it halts
        void run();
        // sends a halt signal to the FSM. This is marked const because I am restricting callback access to the FSM with a const qualifier
        // ts is lowkey scuffed, but whatever
        void halt() { this->halted = true; }
        uint32_t getState() const noexcept { return state; }
    private:
        uint32_t state = 0;
        // the functions should take an FSM pointer and a closure pointer
        Dispatcher dispatcher;
        bool halted = false;
        // NOTE: FSM does NOT own the closure in any way, and its lifetime must be managed externally.
        // That's why closure is a raw void pointer, not an RAII smart pointer
        void* closure;
        void* dispatchContext;
        FSMInterface interface;
    };
}