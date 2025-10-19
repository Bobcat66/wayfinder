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

# This script adds necessary scripts and configurations to a staged wayfinder build
# so that it can be deployed in an OS image.
# It assumes that the staging directory is at ../staging relative to this script.

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
STAGING_DIR="${SCRIPT_DIR}/../staging"
mkdir -p "$STAGING_DIR/opt/wayfinder/local"
mkdir -p "$STAGING_DIR/opt/wayfinder/resources"
mkdir -p "$STAGING_DIR/opt/wayfinder/scripts"
mkdir -p "$STAGING_DIR/opt/wayfinder/etc"
# Copy scripts
cp -r "$SCRIPT_DIR/../scripts/" "$STAGING_DIR/opt/wayfinder/scripts/"
# Copy resource files
cp -r "$SCRIPT_DIR/../resources/" "$STAGING_DIR/opt/wayfinder/resources/"
# Copy config files
cp -r "$SCRIPT_DIR/../etc/" "$STAGING_DIR/opt/wayfinder/etc/"



