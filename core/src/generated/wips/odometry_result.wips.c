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
#include "odometry_result.wips.h"

#define WIPS_INTERNAL
#include "wips_detail.h"

wips_odometry_result_t* wips_odometry_result_create(){
    WIPS_TRACELOG("Creating odometry_result struct\n");
    wips_odometry_result_t* struct_ptr = calloc(1,GET_SIZE(odometry_result));
    if (!struct_ptr) {
        WIPS_DEBUGLOG("Error: Failed to allocate odometry_result struct\n");
        return NULL;
    }
    WIPS_TRACELOG("Created odometry_result struct\n");
    return struct_ptr;
}
void wips_odometry_result_free_resources(wips_odometry_result_t* struct_ptr) {
    WIPS_TRACELOG("Freeing resources held by odometry_result\n");
    if (struct_ptr->timestamps) {
        WIPS_TRACELOG("Freeing odometry_result field timestamps (u64,VLA,size=%u)\n",struct_ptr->GET_DETAIL(timestamps,vlasize));
        for (wips_u32_t i = 0; i < struct_ptr->GET_DETAIL(timestamps,vlasize); i++) {
            wips_u64_free_resources(struct_ptr->timestamps + i);
        }
        free(struct_ptr->timestamps);
    }
    if (struct_ptr->twists) {
        WIPS_TRACELOG("Freeing odometry_result field twists (twist2,VLA,size=%u)\n",struct_ptr->GET_DETAIL(twists,vlasize));
        for (wips_u32_t i = 0; i < struct_ptr->GET_DETAIL(twists,vlasize); i++) {
            wips_twist2_free_resources(struct_ptr->twists + i);
        }
        free(struct_ptr->twists);
    }
    WIPS_TRACELOG("Freed resources held by odometry_result\n");
}
// Function to destroy the struct and free all resources
void wips_odometry_result_destroy(wips_odometry_result_t* struct_ptr) {
    WIPS_TRACELOG("Destroying odometry_result\n");
    if (!struct_ptr) { return; }
    // Free resources allocated by the struct
    wips_odometry_result_free_resources(struct_ptr);
    
    // Free the struct itself
    free(struct_ptr);
    WIPS_TRACELOG("Destroyed odometry_result\n");
}


wips_status_t wips_encode_odometry_result(wips_bin_t* data, wips_odometry_result_t* in) {
    WIPS_TRACELOG("Encoding odometry_result\n");
    WIPS_Assert(data != NULL && in != NULL,0);
    size_t bytesEncoded = 0;
    wips_status_t status;
    WIPS_TRACELOG("Encoding odometry_result field DETAILvlasize__timestamps (u32)\n");
    status = wips_encode_u32(data, &(in->DETAILvlasize__timestamps));
    bytesEncoded += status.bytes_processed;
    if (status.status_code != WIPS_STATUS_OK) return wips_make_status(bytesEncoded,status.status_code);
    WIPS_TRACELOG("Encoding odometry_result field timestamps (u64,VLA,size=%u)\n",in->GET_DETAIL(timestamps,vlasize));
    for (wips_u32_t i = 0; i < in->GET_DETAIL(timestamps,vlasize); i++) {
        status = wips_encode_u64(data, in->timestamps + i);
        bytesEncoded += status.bytes_processed;
        if (status.status_code != WIPS_STATUS_OK) return wips_make_status(bytesEncoded,status.status_code);
    }
    WIPS_TRACELOG("Encoding odometry_result field DETAILvlasize__twists (u32)\n");
    status = wips_encode_u32(data, &(in->DETAILvlasize__twists));
    bytesEncoded += status.bytes_processed;
    if (status.status_code != WIPS_STATUS_OK) return wips_make_status(bytesEncoded,status.status_code);
    WIPS_TRACELOG("Encoding odometry_result field twists (twist2,VLA,size=%u)\n",in->GET_DETAIL(twists,vlasize));
    for (wips_u32_t i = 0; i < in->GET_DETAIL(twists,vlasize); i++) {
        status = wips_encode_twist2(data, in->twists + i);
        bytesEncoded += status.bytes_processed;
        if (status.status_code != WIPS_STATUS_OK) return wips_make_status(bytesEncoded,status.status_code);
    }
    WIPS_TRACELOG("Encoded odometry_result\n");
    return wips_make_status(bytesEncoded,WIPS_STATUS_OK);
}
wips_status_t wips_decode_odometry_result(wips_odometry_result_t* out, wips_bin_t* data) {
    WIPS_TRACELOG("Decoding odometry_result\n");
    WIPS_Assert(out != NULL && data != NULL,0);
    size_t bytesDecoded = 0;
    wips_status_t status;
    WIPS_TRACELOG("Decoding odometry_result field DETAILvlasize__timestamps (u32)\n");
    status = wips_decode_u32(&(out->DETAILvlasize__timestamps), data);
    bytesDecoded += status.bytes_processed;
    if (status.status_code != WIPS_STATUS_OK) return wips_make_status(bytesDecoded,status.status_code);
    WIPS_TRACELOG("Decoding odometry_result field timestamps (u64,VLA,size=%u)\n",out->GET_DETAIL(timestamps,vlasize));
    out->timestamps = malloc(out->GET_DETAIL(timestamps,vlasize) * GET_SIZE(u64));
    if (!out->timestamps){
        WIPS_DEBUGLOG("Fatal error while decoding odometry_result: OOM\n");
        return wips_make_status(bytesDecoded,WIPS_STATUS_OOM);
    }
    for (wips_u32_t i = 0; i < out->GET_DETAIL(timestamps,vlasize); i++) {
        status = wips_decode_u64(out->timestamps + i, data);
        bytesDecoded += status.bytes_processed;
        if (status.status_code != WIPS_STATUS_OK){
            // Free any partially decoded elements to avoid leaks
            for (wips_u32_t j = 0; j < i; j++) {
                wips_u64_free_resources(out->timestamps + j);
            }
            free(out->timestamps);
            out->timestamps = NULL;
            return wips_make_status(bytesDecoded,status.status_code);
        }
    }
    WIPS_TRACELOG("Decoding odometry_result field DETAILvlasize__twists (u32)\n");
    status = wips_decode_u32(&(out->DETAILvlasize__twists), data);
    bytesDecoded += status.bytes_processed;
    if (status.status_code != WIPS_STATUS_OK) return wips_make_status(bytesDecoded,status.status_code);
    WIPS_TRACELOG("Decoding odometry_result field twists (twist2,VLA,size=%u)\n",out->GET_DETAIL(twists,vlasize));
    out->twists = malloc(out->GET_DETAIL(twists,vlasize) * GET_SIZE(twist2));
    if (!out->twists){
        WIPS_DEBUGLOG("Fatal error while decoding odometry_result: OOM\n");
        return wips_make_status(bytesDecoded,WIPS_STATUS_OOM);
    }
    for (wips_u32_t i = 0; i < out->GET_DETAIL(twists,vlasize); i++) {
        status = wips_decode_twist2(out->twists + i, data);
        bytesDecoded += status.bytes_processed;
        if (status.status_code != WIPS_STATUS_OK){
            // Free any partially decoded elements to avoid leaks
            for (wips_u32_t j = 0; j < i; j++) {
                wips_twist2_free_resources(out->twists + j);
            }
            free(out->twists);
            out->twists = NULL;
            return wips_make_status(bytesDecoded,status.status_code);
        }
    }
    WIPS_TRACELOG("Decoded odometry_result\n");
    return wips_make_status(bytesDecoded,WIPS_STATUS_OK);
}