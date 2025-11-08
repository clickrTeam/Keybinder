#!/usr/bin/env bash
set -euo pipefail
FILES=$(git ls-files '*.cpp' '*.h')
if [ -n "$FILES" ]; then
  clang-format -i $FILES
  if ! git diff --quiet; then
    echo "Formating issues fixed"
  else
    echo "No formating issues found"
  fi
fi
