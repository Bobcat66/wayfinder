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
#include "object_detection.wips.h"

#define WIPS_INTERNAL
#include "wips_detail.h"

wips_object_detection_t* wips_object_detection_create(){
    WIPS_TRACELOG("Creating object_detection struct\n");
    wips_object_detection_t* struct_ptr = calloc(1,GET_SIZE(object_detection));
    if (!struct_ptr) {
        WIPS_DEBUGLOG("Error: Failed to allocate object_detection struct\n");
        return NULL;
    }
    WIPS_TRACELOG("Created object_detection struct\n");
    return struct_ptr;
}
void wips_object_detection_free_resources(wips_object_detection_t* struct_ptr) {
    WIPS_TRACELOG("Freeing resources held by object_detection\n");
    WIPS_TRACELOG("Freed resources held by object_detection\n");
}
// Function to destroy the struct and free all resources
void wips_object_detection_destroy(wips_object_detection_t* struct_ptr) {
    WIPS_TRACELOG("Destroying object_detection\n");
    if (!struct_ptr) { return; }
    // Free resources allocated by the struct
    wips_object_detection_free_resources(struct_ptr);
    
    // Free the struct itself
    free(struct_ptr);
    WIPS_TRACELOG("Destroyed object_detection\n");
}


wips_status_t wips_encode_object_detection(wips_bin_t* data, wips_object_detection_t* in) {
    WIPS_TRACELOG("Encoding object_detection\n");
    WIPS_Assert(data != NULL && in != NULL,0);
    size_t bytesEncoded = 0;
    wips_status_t status;
    WIPS_TRACELOG("Encoding object_detection field object_class (i32)\n");
    status = wips_encode_i32(data, &(in->object_class));
    bytesEncoded += status.bytes_processed;
    if (status.status_code != WIPS_STATUS_OK) return wips_make_status(bytesEncoded,status.status_code);
    WIPS_TRACELOG("Encoding object_detection field confidence (fp32)\n");
    status = wips_encode_fp32(data, &(in->confidence));
    bytesEncoded += status.bytes_processed;
    if (status.status_code != WIPS_STATUS_OK) return wips_make_status(bytesEncoded,status.status_code);
    WIPS_TRACELOG("Encoding object_detection field percent_area (fp32)\n");
    status = wips_encode_fp32(data, &(in->percent_area));
    bytesEncoded += status.bytes_processed;
    if (status.status_code != WIPS_STATUS_OK) return wips_make_status(bytesEncoded,status.status_code);
    WIPS_TRACELOG("Encoding object_detection field topleft_x_pixels (fp64)\n");
    status = wips_encode_fp64(data, &(in->topleft_x_pixels));
    bytesEncoded += status.bytes_processed;
    if (status.status_code != WIPS_STATUS_OK) return wips_make_status(bytesEncoded,status.status_code);
    WIPS_TRACELOG("Encoding object_detection field topleft_y_pixels (fp64)\n");
    status = wips_encode_fp64(data, &(in->topleft_y_pixels));
    bytesEncoded += status.bytes_processed;
    if (status.status_code != WIPS_STATUS_OK) return wips_make_status(bytesEncoded,status.status_code);
    WIPS_TRACELOG("Encoding object_detection field bottomright_x_pixels (fp64)\n");
    status = wips_encode_fp64(data, &(in->bottomright_x_pixels));
    bytesEncoded += status.bytes_processed;
    if (status.status_code != WIPS_STATUS_OK) return wips_make_status(bytesEncoded,status.status_code);
    WIPS_TRACELOG("Encoding object_detection field bottomright_y_pixels (fp64)\n");
    status = wips_encode_fp64(data, &(in->bottomright_y_pixels));
    bytesEncoded += status.bytes_processed;
    if (status.status_code != WIPS_STATUS_OK) return wips_make_status(bytesEncoded,status.status_code);
    WIPS_TRACELOG("Encoding object_detection field topleft_x_norm (fp64)\n");
    status = wips_encode_fp64(data, &(in->topleft_x_norm));
    bytesEncoded += status.bytes_processed;
    if (status.status_code != WIPS_STATUS_OK) return wips_make_status(bytesEncoded,status.status_code);
    WIPS_TRACELOG("Encoding object_detection field topleft_y_norm (fp64)\n");
    status = wips_encode_fp64(data, &(in->topleft_y_norm));
    bytesEncoded += status.bytes_processed;
    if (status.status_code != WIPS_STATUS_OK) return wips_make_status(bytesEncoded,status.status_code);
    WIPS_TRACELOG("Encoding object_detection field bottomright_x_norm (fp64)\n");
    status = wips_encode_fp64(data, &(in->bottomright_x_norm));
    bytesEncoded += status.bytes_processed;
    if (status.status_code != WIPS_STATUS_OK) return wips_make_status(bytesEncoded,status.status_code);
    WIPS_TRACELOG("Encoding object_detection field bottomright_y_norm (fp64)\n");
    status = wips_encode_fp64(data, &(in->bottomright_y_norm));
    bytesEncoded += status.bytes_processed;
    if (status.status_code != WIPS_STATUS_OK) return wips_make_status(bytesEncoded,status.status_code);
    WIPS_TRACELOG("Encoded object_detection\n");
    return wips_make_status(bytesEncoded,WIPS_STATUS_OK);
}
wips_status_t wips_decode_object_detection(wips_object_detection_t* out, wips_bin_t* data) {
    WIPS_TRACELOG("Decoding object_detection\n");
    WIPS_Assert(out != NULL && data != NULL,0);
    size_t bytesDecoded = 0;
    wips_status_t status;
    WIPS_TRACELOG("Decoding object_detection field object_class (i32)\n");
    status = wips_decode_i32(&(out->object_class), data);
    bytesDecoded += status.bytes_processed;
    if (status.status_code != WIPS_STATUS_OK) return wips_make_status(bytesDecoded,status.status_code);
    WIPS_TRACELOG("Decoding object_detection field confidence (fp32)\n");
    status = wips_decode_fp32(&(out->confidence), data);
    bytesDecoded += status.bytes_processed;
    if (status.status_code != WIPS_STATUS_OK) return wips_make_status(bytesDecoded,status.status_code);
    WIPS_TRACELOG("Decoding object_detection field percent_area (fp32)\n");
    status = wips_decode_fp32(&(out->percent_area), data);
    bytesDecoded += status.bytes_processed;
    if (status.status_code != WIPS_STATUS_OK) return wips_make_status(bytesDecoded,status.status_code);
    WIPS_TRACELOG("Decoding object_detection field topleft_x_pixels (fp64)\n");
    status = wips_decode_fp64(&(out->topleft_x_pixels), data);
    bytesDecoded += status.bytes_processed;
    if (status.status_code != WIPS_STATUS_OK) return wips_make_status(bytesDecoded,status.status_code);
    WIPS_TRACELOG("Decoding object_detection field topleft_y_pixels (fp64)\n");
    status = wips_decode_fp64(&(out->topleft_y_pixels), data);
    bytesDecoded += status.bytes_processed;
    if (status.status_code != WIPS_STATUS_OK) return wips_make_status(bytesDecoded,status.status_code);
    WIPS_TRACELOG("Decoding object_detection field bottomright_x_pixels (fp64)\n");
    status = wips_decode_fp64(&(out->bottomright_x_pixels), data);
    bytesDecoded += status.bytes_processed;
    if (status.status_code != WIPS_STATUS_OK) return wips_make_status(bytesDecoded,status.status_code);
    WIPS_TRACELOG("Decoding object_detection field bottomright_y_pixels (fp64)\n");
    status = wips_decode_fp64(&(out->bottomright_y_pixels), data);
    bytesDecoded += status.bytes_processed;
    if (status.status_code != WIPS_STATUS_OK) return wips_make_status(bytesDecoded,status.status_code);
    WIPS_TRACELOG("Decoding object_detection field topleft_x_norm (fp64)\n");
    status = wips_decode_fp64(&(out->topleft_x_norm), data);
    bytesDecoded += status.bytes_processed;
    if (status.status_code != WIPS_STATUS_OK) return wips_make_status(bytesDecoded,status.status_code);
    WIPS_TRACELOG("Decoding object_detection field topleft_y_norm (fp64)\n");
    status = wips_decode_fp64(&(out->topleft_y_norm), data);
    bytesDecoded += status.bytes_processed;
    if (status.status_code != WIPS_STATUS_OK) return wips_make_status(bytesDecoded,status.status_code);
    WIPS_TRACELOG("Decoding object_detection field bottomright_x_norm (fp64)\n");
    status = wips_decode_fp64(&(out->bottomright_x_norm), data);
    bytesDecoded += status.bytes_processed;
    if (status.status_code != WIPS_STATUS_OK) return wips_make_status(bytesDecoded,status.status_code);
    WIPS_TRACELOG("Decoding object_detection field bottomright_y_norm (fp64)\n");
    status = wips_decode_fp64(&(out->bottomright_y_norm), data);
    bytesDecoded += status.bytes_processed;
    if (status.status_code != WIPS_STATUS_OK) return wips_make_status(bytesDecoded,status.status_code);
    WIPS_TRACELOG("Decoded object_detection\n");
    return wips_make_status(bytesDecoded,WIPS_STATUS_OK);
}