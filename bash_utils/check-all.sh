#!/bin/bash

LOG_FILE="$2/clang-tidy.log"
: > "$LOG_FILE"

run-clang-tidy -p $1 -quiet -j "$(nproc)" > "$LOG_FILE"
