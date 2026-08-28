#!/bin/bash

LOG_FILE="./clang-tidy.log"
: > "$LOG_FILE"
CONF_FILE="./.clang-tidy"

run-clang-tidy -p "$(pwd)/build/debug" -quiet -config-file="$CONF_FILE" "$" -j "$(nproc)" > "$LOG_FILE"