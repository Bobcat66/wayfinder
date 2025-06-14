cmake_minimum_required(VERSION 3.25)
project(wayfinder VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# TODO: Fix dependency hierarchy. Right now the dependencies are a mess, eventually 
# I should configure the build system to make sure dependencies are imported once and only once
# Particular points i've identified as needing improvement are fmt and Eigen
# Also worried this stuff might break the ODR rule
# Ideally none of the dependencies would bundle any dependencies that are used by other dependencies, and everything
# would rely on a shared set of libraries bundled with this project

# Allowed values for IMG_BACKEND
set(ALLOWED_IMG_BACKENDS
    "cpu" # No hardware acceleration
    "rkvpu" # Rockchip VPU acceleration
    "raspi" # Raspberry Pi V4L2 M2M acceleration
    "nvidia" # Nvidia NVENC/NVDEC acceleration
    "amf" # AMD Advanced Media Framework acceleration
    "videotoolbox" # Apple VideoToolbox acceleration
    "vaapi" # VA-API acceleration
    "quicksync" # Intel Quick Sync acceleration
    "dx11" # DirectX 11 acceleration
    "dx12" # DirectX 12 acceleration
    "vulkan" # Vulkan acceleration
    "opengl" # OpenGL acceleration
)

# Allowed values for AI_BACKEND
set(ALLOWED_AI_BACKENDS
    "cpu" # No hardware acceleration
    "rknpu" # Rockchip NPU acceleration
    "cuda" # Nvidia CUDA acceleration (TensorRT)
    "coreml" # CoreML acceleration
    "directml" # DirectML acceleration
    "rocm" # AMD ROCm acceleration (MlGraphX)
    "openvino" # Intel OpenVINO acceleration
    "hailo" # Hailo acceleration
    "edgetpu" # Edge TPU acceleration
    "armnn" # Arm NN acceleration
    "qnn" # QNN acceleration
)

set(WF_IMG_BACKEND "cpu" CACHE STRING "Backend for hardware-accelerated image processing")
set_property(CACHE WF_IMG_BACKEND PROPERTY STRINGS ${ALLOWED_IMG_BACKENDS})

set(WF_AI_BACKEND "cpu" CACHE STRING "Backend for hardware-accelerated AI inference")
set_property(CACHE AI_BACKEND PROPERTY STRINGS ${ALLOWED_AI_BACKENDS})

option(WF_CV_OPENCL "Enable OpenCL acceleration in OpenCV" ON)
option(WF_CV_CUDA "Enable CUDA acceleration in OpenCV" OFF)

option(WF_USE_ONNX "Use ONNX runtime inference engine" ON)

if(WF_CV_OPENCL AND WF_CV_CUDA)
    message(FATAL_ERROR "WF_CV_OPENCL and WF_CV_CUDA are mutually exclusive. Please enable only one.")
endif()

# Function to validate a value against a list of allowed values
function(validate_backend option_name option_value allowed_values)
    list(FIND ${allowed_values} ${option_value} found_index)
    if(found_index EQUAL -1)
        message(FATAL_ERROR
            "Invalid value '${option_value}' for option ${option_name}.\n"
            "Allowed values are: ${${allowed_values}}"
        )
    endif()
endfunction()

validate_backend("IMG_BACKEND" "${WF_IMG_BACKEND}" "ALLOWED_IMG_BACKENDS")
validate_backend("AI_BACKEND" "${WF_AI_BACKEND}" "ALLOWED_AI_BACKENDS")

add_subdirectory(thirdparty/googletest)
add_subdirectory(thirdparty/apriltag)
add_subdirectory(thirdparty/wpilib)
add_subdirectory(thirdparty/gtsam)
add_subdirectory(thirdparty/spdlog)
if(WF_USE_ONNX)
    add_subdirectory(thirdparty/onnxruntime)
endif()
add_subdirectory(wfvtk)
add_subdirectory(wfcore)

