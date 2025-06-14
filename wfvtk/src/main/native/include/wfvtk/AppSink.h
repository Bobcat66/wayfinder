// Copyright (c) 2025 Jesse Kane
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>

namespace wfvtk {
    class AppSink {
        public:
        virtual int getRows(); // Rows in the image
        virtual int getCols(); // Columns in the image
        virtual int getStride(); // Space in between rows in the buffer
        virtual uint64_t pullFrame(uint8_t* buffer); // Writes image data to the frame, returns the timestamp of the frame in microseconds, or 0 if there was an error
        virtual ~AppSink() = default;
    };
}