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

if [[ $EUID -ne 0 ]]; then
    echo "This script must be run as root"
    exit 1
fi

set -euo pipefail

if [[ $# -ne 2 ]]; then
    echo "Usage: $0 <path-to-image.xz> <path-to-staging-dir>"
    exit 1
fi

IMAGE_PATH="$1"
if [[ ! -f "$IMAGE_PATH" ]]; then
    echo "Image file not found: $IMAGE_PATH"
    exit 1
fi

STAGING_PATH="$2"

if [[ ! -d "$STAGING_PATH" ]]; then
    echo "Staging directory not found: $STAGING_PATH"
    exit 1
fi

unxz "$IMAGE_PATH"
IMAGE_FILE="${IMAGE_PATH%.xz}"

LOOPDEV=$(losetup -Pf --show "$IMAGE_FILE")
echo "Loop device is: $LOOPDEV"

# Setup trap and mountpoint
MOUNTPOINT=$(mktemp -d /mnt/wf_root.XXXXXX)
cleanup() {
    echo "Cleaning up..."
    sync
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
# Wait for partitions to appear
partprobe "$LOOPDEV"
udevadm settle

# Find partitions
# mapfile -t PARTS < <(lsblk -nr -o NAME "$LOOPDEV" | tail -n +2)
# Enumerate partition block devices
PARTS=()
for p in "${LOOPDEV}"p*; do
    [[ -b "$p" ]] && PARTS+=("$p")
done

if [[ ${#PARTS[@]} -eq 0 ]]; then
    echo "No partitions found; assuming raw filesystem"
    ROOT_PARTITION="$LOOPDEV"
    PARTNUM=0
    RAWFS=true
elif [[ ${#PARTS[@]} -eq 1 ]]; then
    echo "Single-partition image detected"
    ROOT_PARTITION="${PARTS[0]}"
    PARTNUM="${PARTS[0]##*p}"
    RAWFS=false
else
    echo "Multi-partition image detected"
    ROOT_PARTITION="${PARTS[-1]}"  # last partition = rootfs
    PARTNUM="${PARTS[-1]##*p}"
    RAWFS=false
fi

echo "Using root partition: $ROOT_PARTITION"
echo "Partition number: $PARTNUM"

# Resize image
FSTYPE=$(lsblk -no FSTYPE "$ROOT_PARTITION")

if [[ "$FSTYPE" != "ext4" ]]; then
    echo "Refusing to resize non-ext4 filesystem on $ROOT_PARTITION"
    exit 1
fi

echo "Resizing image filesystem to ensure sufficient space..."
truncate -s +2G "$IMAGE_FILE"
losetup -c "$LOOPDEV"
if [[ "$RAWFS" == true ]]; then
    echo "Raw filesystem detected; skipping partition resize"
else
    echo "Resizing partition $PARTNUM to fill image"
    sgdisk --move-second-header "$LOOPDEV" >> /dev/null # This moves the backup GPT header to the end of the resized disk
    # Delete and recreate the partition with the same start sector but expanded to the end of the disk
    START_SECTOR=$(sgdisk -p "$LOOPDEV" | awk -v part="$PARTNUM" '$1 == part {print $2}')
    sgdisk -d "$PARTNUM" "$LOOPDEV" >> /dev/null
    sgdisk -n "$PARTNUM:$START_SECTOR:0" "$LOOPDEV" >> /dev/null
    sgdisk -t "$PARTNUM:8300" "$LOOPDEV" >> /dev/null  # Linux filesystem
    # Wait for kernel to notice new partition sizes
    partprobe "$LOOPDEV"
    udevadm settle
    echo "Partition resized"
fi
e2fsck -f $ROOT_PARTITION
resize2fs $ROOT_PARTITION
#lsblk -o NAME,SIZE,FSTYPE,MOUNTPOINT "$LOOPDEV"
# Mount the root partition
mount "$ROOT_PARTITION" "$MOUNTPOINT"
echo "Mounted root partition at $MOUNTPOINT"


# Copy Wayfinder files to the mounted root partition
rsync -a "$STAGING_PATH/opt/wayfinder/" "$MOUNTPOINT/opt/wayfinder/"
echo "Copied Wayfinder files to $MOUNTPOINT/opt/wayfinder"
chmod +x "$MOUNTPOINT/opt/wayfinder/scripts/"*.sh || true

echo "Mounting necessary filesystems..."
# Chroot into the mounted partition and run the install script
mount --bind /dev "$MOUNTPOINT/dev"
mount --bind /dev/pts "$MOUNTPOINT/dev/pts"
mount --bind /proc "$MOUNTPOINT/proc"
mount --bind /sys "$MOUNTPOINT/sys"
mount --bind /run "$MOUNTPOINT/run"
echo "Mounted necessary filesystems for chroot"
# This enables chrooting into an aarch64 image on an x86_64 host
cp /usr/bin/qemu-aarch64-static "$MOUNTPOINT/usr/bin/"
echo "exectuing install.sh via chroot..."
chroot "$MOUNTPOINT" /bin/bash -c "/opt/wayfinder/scripts/install.sh"
# Re-compress the image
xz "$IMAGE_FILE"
echo "Unmounted and cleaned up"
echo "Wayfinder deployed to image successfully."

