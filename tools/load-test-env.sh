# This script should be sourced. It will point the WF_ENV_PATH environment to the mock env file,
# So that all of wayfinders testing utilities will work as normal
# NOTE: WF_EXEC_PATH is only used by the launcher, so we don't set it here

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

# Resolve wayfinder root directory
script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
WF_ROOT="$(dirname "$script_dir")"
echo "Wayfinder root directory resolved to '$WF_ROOT'"

export WF_ENV_PATH="$WF_ROOT/test-resources/.env.mock"

load_env
echo "Loaded test environment"