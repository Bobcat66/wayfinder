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

# This script is wayfinder's launcher. It handles startup, restart, and shutdown
# It relies on the WF_ENV_PATH environment variable being set

running=true

function load_env {
    if [[ -z "$WF_ENV_PATH" ]]; then
        echo "WF_ENV_PATH is not set"
        exit 1
    fi
    echo "Loading environment from $WF_ENV_PATH"
    set -a
    source "$WF_ENV_PATH"
    set +a
}

# Initial environment load
load_env

while $running; do
    if [[ ! -x "./$WF_EXEC" ]]; then
        echo "WF_EXEC either is not set or is not executable"
        exit 1
    fi
    "./$WF_EXEC_PATH"
    ret_code=$?
    case $ret_code in
        0)
            echo "Wayfinder exited successfully"
            running=false
            ;;
        1)
            echo "Wayfinder exited with an error"
            running=false
            ;;
        2)
            echo "Wayfinder requested a restart"
            running=true
            ;;
        3)
            echo "Wayfinder requested an environment reload"
            running=true
            load_env
            ;;
        4)
            echo "Wayfinder requested a shutdown"
            running=false
            shutdown now
            ;;
        5)
            echo "Wayfinder requested a reboot"
            running=false
            shutdown -r now
            ;;
        *)
            echo "Wayfinder exited with unknown code $ret_code"
            running=false
            ;;
    esac
done

exit 0