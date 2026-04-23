#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Disable terminal flow control (Ctrl+S/Ctrl+Q) to allow Ctrl+S for save
stty -ixon 2>/dev/null || true

exec python3 "$SCRIPT_DIR/pascc_ide.py" "$@"
