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
#include "apriltag_relative_pose_observation.wips.h"

#define WIPS_INTERNAL
#include "wips_detail.h"

wips_apriltag_relative_pose_observation_t* wips_apriltag_relative_pose_observation_create(){
    WIPS_TRACELOG("Creating apriltag_relative_pose_observation struct\n");
    wips_apriltag_relative_pose_observation_t* struct_ptr = calloc(1,GET_SIZE(apriltag_relative_pose_observation));
    if (!struct_ptr) {
        WIPS_DEBUGLOG("Error: Failed to allocate apriltag_relative_pose_observation struct\n");
        return NULL;
    }
    WIPS_TRACELOG("Created apriltag_relative_pose_observation struct\n");
    return struct_ptr;
}
void wips_apriltag_relative_pose_observation_free_resources(wips_apriltag_relative_pose_observation_t* struct_ptr) {
    WIPS_TRACELOG("Freeing resources held by apriltag_relative_pose_observation\n");
    WIPS_TRACELOG("Freed resources held by apriltag_relative_pose_observation\n");
}
// Function to destroy the struct and free all resources
void wips_apriltag_relative_pose_observation_destroy(wips_apriltag_relative_pose_observation_t* struct_ptr) {
    WIPS_TRACELOG("Destroying apriltag_relative_pose_observation\n");
    if (!struct_ptr) { return; }
    // Free resources allocated by the struct
    wips_apriltag_relative_pose_observation_free_resources(struct_ptr);
    
    // Free the struct itself
    free(struct_ptr);
    WIPS_TRACELOG("Destroyed apriltag_relative_pose_observation\n");
}


wips_status_t wips_encode_apriltag_relative_pose_observation(wips_bin_t* data, wips_apriltag_relative_pose_observation_t* in) {
    WIPS_TRACELOG("Encoding apriltag_relative_pose_observation\n");
    WIPS_Assert(data != NULL && in != NULL,0);
    size_t bytesEncoded = 0;
    wips_status_t status;
    WIPS_TRACELOG("Encoding apriltag_relative_pose_observation field fiducial_id (i32)\n");
    status = wips_encode_i32(data, &(in->fiducial_id));
    bytesEncoded += status.bytes_processed;
    if (status.status_code != WIPS_STATUS_OK) return wips_make_status(bytesEncoded,status.status_code);
    WIPS_TRACELOG("Encoding apriltag_relative_pose_observation field cam_pose_0 (pose3)\n");
    status = wips_encode_pose3(data, &(in->cam_pose_0));
    bytesEncoded += status.bytes_processed;
    if (status.status_code != WIPS_STATUS_OK) return wips_make_status(bytesEncoded,status.status_code);
    WIPS_TRACELOG("Encoding apriltag_relative_pose_observation field error_0 (fp64)\n");
    status = wips_encode_fp64(data, &(in->error_0));
    bytesEncoded += status.bytes_processed;
    if (status.status_code != WIPS_STATUS_OK) return wips_make_status(bytesEncoded,status.status_code);
    WIPS_TRACELOG("Encoding apriltag_relative_pose_observation field cam_pose_1 (pose3)\n");
    status = wips_encode_pose3(data, &(in->cam_pose_1));
    bytesEncoded += status.bytes_processed;
    if (status.status_code != WIPS_STATUS_OK) return wips_make_status(bytesEncoded,status.status_code);
    WIPS_TRACELOG("Encoding apriltag_relative_pose_observation field error_1 (fp64)\n");
    status = wips_encode_fp64(data, &(in->error_1));
    bytesEncoded += status.bytes_processed;
    if (status.status_code != WIPS_STATUS_OK) return wips_make_status(bytesEncoded,status.status_code);
    WIPS_TRACELOG("Encoded apriltag_relative_pose_observation\n");
    return wips_make_status(bytesEncoded,WIPS_STATUS_OK);
}
wips_status_t wips_decode_apriltag_relative_pose_observation(wips_apriltag_relative_pose_observation_t* out, wips_bin_t* data) {
    WIPS_TRACELOG("Decoding apriltag_relative_pose_observation\n");
    WIPS_Assert(out != NULL && data != NULL,0);
    size_t bytesDecoded = 0;
    wips_status_t status;
    WIPS_TRACELOG("Decoding apriltag_relative_pose_observation field fiducial_id (i32)\n");
    status = wips_decode_i32(&(out->fiducial_id), data);
    bytesDecoded += status.bytes_processed;
    if (status.status_code != WIPS_STATUS_OK) return wips_make_status(bytesDecoded,status.status_code);
    WIPS_TRACELOG("Decoding apriltag_relative_pose_observation field cam_pose_0 (pose3)\n");
    status = wips_decode_pose3(&(out->cam_pose_0), data);
    bytesDecoded += status.bytes_processed;
    if (status.status_code != WIPS_STATUS_OK) return wips_make_status(bytesDecoded,status.status_code);
    WIPS_TRACELOG("Decoding apriltag_relative_pose_observation field error_0 (fp64)\n");
    status = wips_decode_fp64(&(out->error_0), data);
    bytesDecoded += status.bytes_processed;
    if (status.status_code != WIPS_STATUS_OK) return wips_make_status(bytesDecoded,status.status_code);
    WIPS_TRACELOG("Decoding apriltag_relative_pose_observation field cam_pose_1 (pose3)\n");
    status = wips_decode_pose3(&(out->cam_pose_1), data);
    bytesDecoded += status.bytes_processed;
    if (status.status_code != WIPS_STATUS_OK) return wips_make_status(bytesDecoded,status.status_code);
    WIPS_TRACELOG("Decoding apriltag_relative_pose_observation field error_1 (fp64)\n");
    status = wips_decode_fp64(&(out->error_1), data);
    bytesDecoded += status.bytes_processed;
    if (status.status_code != WIPS_STATUS_OK) return wips_make_status(bytesDecoded,status.status_code);
    WIPS_TRACELOG("Decoded apriltag_relative_pose_observation\n");
    return wips_make_status(bytesDecoded,WIPS_STATUS_OK);
}