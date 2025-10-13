#!/bin/bash

# This script automatically creates a distributable package of the Wayfinder application.
# It assumes that the application has already been built and staged using build.sh.
# The resulting package will be placed in the 'dist' directory.

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
STAGING_DIR="${SCRIPT_DIR}/../staging"
