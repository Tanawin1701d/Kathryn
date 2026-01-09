#!/usr/bin/env bash
set -euo pipefail

# Directory containing this script
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Recursively delete files with the given suffixes under SCRIPT_DIR
find "$SCRIPT_DIR" -type f \( \
    -name '*.prof' -o \
    -name '*.sl'    -o \
    -name '*.vcd' \
\) -print -delete
