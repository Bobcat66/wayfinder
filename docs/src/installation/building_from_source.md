# Building Wayfinder from Source

>[!WARNING]
>Building from source is a very involved process, and is generally not recommended for most use cases.

>[!IMPORTANT]
>Wayfinder is designed to be built with GCC 13 using the C++ 20 standard. Building with Clang and MSVC are not supported at the moment

>[!NOTE]
>This guide is designed for Ubuntu 24.04.1 LTS

## Installing prerequisites

To install the necessary prerequisites, run the following command in a terminal:
<!-- Eventually OpenCV will be built from source. For now we rely on system installations -->
```
sudo apt update && apt install -y build-essential cmake ninja-build libopencv-dev 
```
(This is outdated btw)
