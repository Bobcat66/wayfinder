#!/bin/bash

# This script is wayfinder's launcher. It handles startup, restart, and shutdown
# It relies on two environment variables being set, WF_EXEC_PATH, the relative path to the wayfinderd executable,
# and WF_ENV_PATH, the relative path to the .env file

if [[ $EUID -ne 0 ]]; then
  echo "The wayfinder launcher must be run as root."
  exit 1
fi

running=true

function load_env {
    if [[ -z "$WF_ENV_PATH" ]]; then
        echo "WF_ENV_PATH is not set"
        exit 1
    fi
    set -a
    source "$WF_ENV_PATH"
    set +a
}

# Initial environment load
load_env

while [[ "$running" == "true" ]]; do
    if [[ ! -x "./$WF_EXEC_PATH" ]]; then
        echo "WF_EXEC_PATH either is not set or is not executable"
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