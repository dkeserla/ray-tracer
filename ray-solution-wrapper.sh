#!/usr/bin/env bash
set -e

# Compute FLTK lib dir via brew
FLTK_LIB_DIR="$(brew --prefix fltk@1.3)/lib"

# Prepend FLTK lib dir to DYLD_LIBRARY_PATH for this process
export DYLD_LIBRARY_PATH="${FLTK_LIB_DIR}:${DYLD_LIBRARY_PATH}"

# Call the real reference binary with all original args
exec ../reference-binaries/macos-arm/ray-solution "$@"
