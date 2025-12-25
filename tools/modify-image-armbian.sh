#!/bin/bash

# SPDX-License-Identifier: GPL-3.0-or-later
#
# Copyright (C) 2025 Jesse Kane
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

# This script deploys the Wayfinder application to an Armbian image xz archive
# The .img.xz file should be provided as the first argument
# This script mounts the image, copies the application files to the appropriate locations,
# and then unmounts the image.
# It requires root privileges to run.

# This script requires xz, losetup, and rsync to run.


SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

if [[ $# -ne 1 ]]; then
    echo "Usage: $0 <path-to-image> <path-to-staging-dir>"
    exit 1
fi

IMAGE_PATH="$1"

if [[ ! -f "$IMAGE_PATH" ]]; then
    echo "Image file not found: $IMAGE_PATH"
    exit 1
fi

if [[ $# -ne 2 ]]; then
    echo "Usage: $0 <path-to-image> <path-to-staging-dir>"
    exit 1
fi

STAGING_PATH="$2"

if [[ ! -d "$STAGING_PATH" ]]; then
    echo "Staging directory not found: $STAGING_PATH"
    exit 1
fi

unxz "$IMAGE_PATH"
if [[ $? -ne 0 ]]; then
    echo "Failed to uncompress image file"
    exit 1
fi
IMAGE_FILE="${IMAGE_PATH%.xz}"

LOOPDEV=$(losetup -Pf --show $IMAGE_FILE)
echo "Loop device is: $LOOPDEV"
BOOT_PARTITION="${LOOPDEV}p1"
ROOT_PARTITION="${LOOPDEV}p2"
echo "Boot partition: $BOOT_PARTITION"
echo "Root partition: $ROOT_PARTITION"
# Mount the root partition
MOUNTPOINT=$(mktemp -d /mnt/wf_root.XXXXXX)
mkdir -p "$MOUNTPOINT"
mount "$ROOT_PARTITION" "$MOUNTPOINT"
if [[ $? -ne 0 ]]; then
    echo "Failed to mount root partition"
    losetup -d "$LOOPDEV"
    exit 1
fi
echo "Mounted root partition at $MOUNTPOINT"
cleanup() {
    echo "Cleaning up..."
    umount -q "$MOUNTPOINT/dev/pts" 2>/dev/null || true
    umount -q "$MOUNTPOINT/dev" 2>/dev/null || true
    umount -q "$MOUNTPOINT/proc" 2>/dev/null || true
    umount -q "$MOUNTPOINT/sys" 2>/dev/null || true
    umount -q "$MOUNTPOINT/run" 2>/dev/null || true
    umount -q "$MOUNTPOINT" 2>/dev/null || true
    losetup -d "$LOOPDEV" 2>/dev/null || true
    rm -f "$MOUNTPOINT/usr/bin/qemu-aarch64-static" 2>/dev/null || true
    rmdir "$MOUNTPOINT" 2>/dev/null || true
}
trap cleanup EXIT
# Copy Wayfinder files to the mounted root partition
rsync -a "$STAGING_PATH/opt/wayfinder/" "$MOUNTPOINT/opt/wayfinder/"
if [[ $? -ne 0 ]]; then
    echo "Failed to copy Wayfinder files"
    umount "$MOUNTPOINT"
    losetup -d "$LOOPDEV"
    exit 1
fi
echo "Copied Wayfinder files to $MOUNTPOINT/opt/wayfinder"

# Chroot into the mounted partition and run the install script
mount --bind /dev "$MOUNTPOINT/dev"
mount --bind /dev/pts "$MOUNTPOINT/dev/pts"
mount --bind /proc "$MOUNTPOINT/proc"
mount --bind /sys "$MOUNTPOINT/sys"
mount --bind /run "$MOUNTPOINT/run"
# This enables chrooting into an aarch64 image on an x86_64 host
cp /usr/bin/qemu-aarch64-static "$MOUNTPOINT/usr/bin/"
chroot "$MOUNTPOINT" /bin/bash -c "/opt/wayfinder/scripts/install.sh"
if [[ $? -ne 0 ]]; then
    echo "Failed to run install script in chroot"
    cleanup()
    exit 1
fi
echo "Ran install script in chroot"
# Re-compress the image
xz "$IMAGE_FILE"
echo "Unmounted and cleaned up"
echo "Wayfinder deployed to image successfully."

