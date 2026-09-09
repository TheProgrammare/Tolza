#!/bin/bash

find $1 -type f \( -name "*.cpp" -o -name "*.hpp" \) -exec clang-format -i {} +
