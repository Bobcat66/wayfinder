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

#include "wips_runtime.h"
#include "twist2.wips.h"

#define WIPS_INTERNAL
#include "wips_detail.h"

wips_twist2_t* wips_twist2_create(){
    WIPS_TRACELOG("Creating twist2 struct\n");
    wips_twist2_t* struct_ptr = calloc(1,GET_SIZE(twist2));
    if (!struct_ptr) {
        WIPS_DEBUGLOG("Error: Failed to allocate twist2 struct\n");
        return NULL;
    }
    WIPS_TRACELOG("Created twist2 struct\n");
    return struct_ptr;
}
void wips_twist2_free_resources(wips_twist2_t* struct_ptr) {
    WIPS_TRACELOG("Freeing resources held by twist2\n");
    WIPS_TRACELOG("Freed resources held by twist2\n");
}
// Function to destroy the struct and free all resources
void wips_twist2_destroy(wips_twist2_t* struct_ptr) {
    WIPS_TRACELOG("Destroying twist2\n");
    if (!struct_ptr) { return; }
    // Free resources allocated by the struct
    wips_twist2_free_resources(struct_ptr);
    
    // Free the struct itself
    free(struct_ptr);
    WIPS_TRACELOG("Destroyed twist2\n");
}


wips_status_t wips_encode_twist2(wips_bin_t* data, wips_twist2_t* in) {
    WIPS_TRACELOG("Encoding twist2\n");
    WIPS_Assert(data != NULL && in != NULL,0);
    size_t bytesEncoded = 0;
    wips_status_t status;
    WIPS_TRACELOG("Encoding twist2 field dx (fp64)\n");
    status = wips_encode_fp64(data, &(in->dx));
    bytesEncoded += status.bytes_processed;
    if (status.status_code != WIPS_STATUS_OK) return wips_make_status(bytesEncoded,status.status_code);
    WIPS_TRACELOG("Encoding twist2 field dy (fp64)\n");
    status = wips_encode_fp64(data, &(in->dy));
    bytesEncoded += status.bytes_processed;
    if (status.status_code != WIPS_STATUS_OK) return wips_make_status(bytesEncoded,status.status_code);
    WIPS_TRACELOG("Encoding twist2 field dtheta (fp64)\n");
    status = wips_encode_fp64(data, &(in->dtheta));
    bytesEncoded += status.bytes_processed;
    if (status.status_code != WIPS_STATUS_OK) return wips_make_status(bytesEncoded,status.status_code);
    WIPS_TRACELOG("Encoded twist2\n");
    return wips_make_status(bytesEncoded,WIPS_STATUS_OK);
}
wips_status_t wips_decode_twist2(wips_twist2_t* out, wips_bin_t* data) {
    WIPS_TRACELOG("Decoding twist2\n");
    WIPS_Assert(out != NULL && data != NULL,0);
    size_t bytesDecoded = 0;
    wips_status_t status;
    WIPS_TRACELOG("Decoding twist2 field dx (fp64)\n");
    status = wips_decode_fp64(&(out->dx), data);
    bytesDecoded += status.bytes_processed;
    if (status.status_code != WIPS_STATUS_OK) return wips_make_status(bytesDecoded,status.status_code);
    WIPS_TRACELOG("Decoding twist2 field dy (fp64)\n");
    status = wips_decode_fp64(&(out->dy), data);
    bytesDecoded += status.bytes_processed;
    if (status.status_code != WIPS_STATUS_OK) return wips_make_status(bytesDecoded,status.status_code);
    WIPS_TRACELOG("Decoding twist2 field dtheta (fp64)\n");
    status = wips_decode_fp64(&(out->dtheta), data);
    bytesDecoded += status.bytes_processed;
    if (status.status_code != WIPS_STATUS_OK) return wips_make_status(bytesDecoded,status.status_code);
    WIPS_TRACELOG("Decoded twist2\n");
    return wips_make_status(bytesDecoded,WIPS_STATUS_OK);
}