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

# This script installs Wayfinder to the system, and sets up necessary services and permissions.
# It must be run with root privileges.
# Prebuilt images already have run this script during their creation, so it is not necessary to run it again on those images.

set -euo pipefail

echo "Installing Wayfinder..."

echo "Installing dependencies..."
# Install necessary dependencies
apt-get update -qq
apt-get install -y -qq \
    network-manager \
    avahi-daemon >> /dev/null
echo "Dependencies installed."

echo "Configuring..."
# Set permissions and ownerships
chown root:root /opt/wayfinder/etc/NetworkManager/system-connections/frc-eth0.nmconnection
chmod 600 /opt/wayfinder/etc/NetworkManager/system-connections/frc-eth0.nmconnection
chown root:root /opt/wayfinder/etc/systemd/system/wayfinder.service
chmod 644 /opt/wayfinder/etc/systemd/system/wayfinder.service

# Create symlinks
ln -s /opt/wayfinder/etc/NetworkManager/system-connections/frc-eth0.nmconnection /etc/NetworkManager/system-connections/frc-eth0.nmconnection
ln -s /opt/wayfinder/etc/systemd/system/wayfinder.service /etc/systemd/system/wayfinder.service

# wfdev user for ssh access and admin tasks
if ! id -u wfdev >/dev/null 2>&1; then
    useradd --create-home --shell /bin/bash wfdev
fi
# Create wayfinder group if not exists
if ! getent group wayfinder >/dev/null; then
    groupadd --system wayfinder
fi
# Create wayfinder system user if not exists
if ! id -u wayfinder >/dev/null 2>&1; then
    useradd --system --create-home --shell /usr/sbin/nologin --gid wayfinder wayfinder
fi

usermod -aG wayfinder,sudo wfdev
usermod -aG video wayfinder

# TODO: Figure out permissions needed and set them appropriately

systemctl daemon-reload
systemctl enable wayfinder.service
systemctl enable avahi-daemon.service

echo "Installation complete."