#!/bin/bash

LOG_FILE="./clang-tidy.log"
: > "$LOG_FILE"

run-clang-tidy -p "$(pwd)/build" -quiet -j "$(nproc)" > "$LOG_FILE"