#!/usr/bin/env bash
set -euo pipefail

ROOT_DIRECTORY="$(dirname "$(realpath "$0")")/.."

pushd "$ROOT_DIRECTORY" > /dev/null

FORGE_DIRECTORY=".forge"
BOOTSTRAP_DIRECTORY="$FORGE_DIRECTORY/bootstrap"
BOOTSTRAP_LOGFILE="$BOOTSTRAP_DIRECTORY/bootstrap.log"
FORGE_EXECUTABLE="$BOOTSTRAP_DIRECTORY/forge"

STDEXT_INCLUDE="packages/stdext/include"
CORE_INCLUDE="packages/forge-core/include"
FORGE_INCLUDE="packages/forge-build/include"

FORGE_SOURCES=(
    packages/forge-build/src/*.cpp
    packages/forge-core/src/*.cpp
    packages/stdext/src/*.cpp
)

# --- Pick compiler (Clang 19+) ---
CXX=""

for candidate in clang++-20 clang++-19; do
    if command -v "$candidate" > /dev/null 2>&1; then
        CXX="$candidate"
        break
    fi
done

if [[ -z "$CXX" ]]; then
    echo ":: No supported compiler found in PATH."
    echo ":: Please install Clang 19 or newer and try again."
    popd > /dev/null
    exit 1
fi

# --- Ensure directories exist ---
mkdir -p "$BOOTSTRAP_DIRECTORY"

# --- Bootstrap build only if the executable does not exist ---
if [[ ! -f "$FORGE_EXECUTABLE" ]]; then
    echo "Bootstrap Folder:    $BOOTSTRAP_DIRECTORY"
    echo "Bootstrap Log File:  $BOOTSTRAP_LOGFILE"
    echo "Bootstrap Compiler:  $CXX"
    echo
    echo ":: Bootstrapping Forge Build Tool..."

    if ! "$CXX" -std=c++20 -Wno-c23-extensions -o "$FORGE_EXECUTABLE" \
        -I "$STDEXT_INCLUDE" \
        -I "$CORE_INCLUDE" \
        -I "$FORGE_INCLUDE" \
        "${FORGE_SOURCES[@]}" \
        > "$BOOTSTRAP_LOGFILE" 2>&1; then

        echo ":: Forge bootstrap build failed. See $BOOTSTRAP_LOGFILE."
        popd > /dev/null
        exit 1
    fi

    echo ":: Forge bootstrap build OK"
fi

# --- Safety: bare clean cannot be run from within the repo directory ---
# It would remove the .forge directory this executable is currently running from.
if [[ "${1-}" == "clean" ]]; then
    is_bare_clean=true

    for arg in "$@"; do
        case "$arg" in
            --debug|--preview|--release) is_bare_clean=false ;;
        esac
    done

    if [[ "$is_bare_clean" == true ]]; then
        echo ":: 'clean' without a profile cannot be run from this location. It would remove the .forge directory this executable is currently running from."
        popd > /dev/null
        exit 1
    fi
fi

# --- Run Forge with all provided args ---
if "$FORGE_EXECUTABLE" "$@"; then
    code=0
else
    code=$?
fi

if [[ "$code" -ne 0 ]]; then
    echo ":: Forge build failed with exit code $code."
    popd > /dev/null
    exit "$code"
fi

popd > /dev/null
