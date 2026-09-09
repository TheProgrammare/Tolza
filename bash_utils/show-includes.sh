#!/bin/bash

find $1 -type f \( -name '*.cpp' -o -name '*.hpp' \) -print0 |
while IFS= read -r -d '' f; do
    echo
    echo "========== $f =========="
    grep -nE '^[[:space:]]*#include[[:space:]]*[<"]' "$f"
done

