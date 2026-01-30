#!/usr/bin/env bash
# A shell script to do everything needed to make a wayfinder image because i'm lazy
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
if [[ $EUID -ne 0 ]]; then
    echo "This script must be run as root"
    exit 1
fi

set -euo pipefail

if [[ $# -ne 2 ]]; then
    echo "Usage: $0 <path-to-image.xz> <output-path.xz>"
    exit 1
fi

IMAGE_PATH="$(readlink -f "$1")"

if [[ "$IMAGE_PATH" != *.img.xz ]]; then
    echo "Expected an .img.xz file, got: $IMAGE_PATH"
    exit 1
fi

if [[ ! -f "$IMAGE_PATH" ]]; then
    echo "Image file not found: $IMAGE_PATH"
    exit 1
fi

source "${SCRIPT_DIR}/build-aarch64.sh"
source "${SCRIPT_DIR}/package.sh"

"${SCRIPT_DIR}/modify-image-armbian.sh" "$IMAGE_PATH" "${SCRIPT_DIR}/../staging"


