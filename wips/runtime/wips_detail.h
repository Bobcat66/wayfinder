/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2025 Jesse Kane
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 * contributors may be used to endorse or promote products derived from this
 * software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS “AS IS”
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

// This file was automatically generated by WIPS. Do not attempt to modify manually

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define EXPAND(x) x
#define CONCAT3(a, b, c) a ## b ## c
#define EXPAND_CONCAT3(a, b, c) CONCAT3(a, b, c)

#define GET_CTYPE(wips_typename) EXPAND_CONCAT3(wips_, wips_typename, _t)
#define GET_SIZE(wips_typename) sizeof(GET_CTYPE(wips_typename))

#define GET_DETAIL_IMPL(field,detail) DETAIL ## detail ## __ ## field
#define GET_DETAIL(field,detail) GET_DETAIL_IMPL(field,detail)

#define DEFINE_TRIVIAL_ENCODE(wips_typename)                                                                    \
    size_t wips_encode_##wips_typename(wips_bin_t* data, GET_CTYPE(wips_typename)* in){                         \
        size_t newOffset = data->offset + GET_SIZE(wips_typename);                                              \
        if (newOffset > data->allocated) {                                                                      \
            size_t new_allocated = data->allocated * 2 >= newOffset                                             \
                ? data->allocated * 2                                                                           \
                : newOffset;                                                                                    \
            unsigned char* newBase = realloc(data->base, new_allocated);                                        \
            if (!newBase) {return 0;}                                                                           \
            data->allocated = new_allocated;                                                                    \
            data->base = newBase;                                                                               \
        }                                                                                                       \
        memcpy(data->base+data->offset,in, GET_SIZE(wips_typename));                                            \
        data->offset = newOffset;                                                                               \
        return GET_SIZE(wips_typename);                                                                         \
    }
    
#define DEFINE_TRIVIAL_DECODE(wips_typename)                                                                    \
    size_t wips_decode_##wips_typename(GET_CTYPE(wips_typename)* out, wips_bin_t* data){                        \
        size_t newOffset = data->offset + GET_SIZE(wips_typename);                                              \
        assert(newOffset <= data->allocated);                                                                   \
        memcpy(out,data->base+data->offset, GET_SIZE(wips_typename));                                           \
        data->offset = newOffset;                                                                               \
        return GET_SIZE(wips_typename);                                                                         \
    }

#ifdef __cplusplus
}
#endif