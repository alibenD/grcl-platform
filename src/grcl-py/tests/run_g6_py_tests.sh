#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../../.." && pwd)"
PACKAGE_ROOT="$REPO_ROOT/src/grcl-py"
ARTIFACT_ROOT="${GRCL_PLATFORM_ARTIFACT_ROOT:-$REPO_ROOT/../artifacts}/g6/grcl-py"

mkdir -p "$ARTIFACT_ROOT"

export PYTHONDONTWRITEBYTECODE=1
export PYTHONPATH="$PACKAGE_ROOT${PYTHONPATH:+:$PYTHONPATH}"

cd "$REPO_ROOT"
python -m unittest discover -s src/grcl-py/tests -p 'test_*.py'
