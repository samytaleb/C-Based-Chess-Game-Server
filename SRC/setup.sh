#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
echo "Compiling. No firewall rules will be modified."
make all
