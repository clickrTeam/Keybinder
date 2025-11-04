#!/usr/bin/env bash
set -euo pipefail
FILES=$(fd -e c -e cpp -e h --exclude "mac/external")
if [ -n "$FILES" ]; then
  clang-format -i $FILES
  if ! git diff --quiet; then
    echo "Formating issues fixed"
  else
    echo "No formating issues found"
  fi
fi
