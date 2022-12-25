#!/bin/bash

DIR="$(dirname "$1")"
FILE="$(basename "$1")"

(cd ${DIR} && valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --trace-children=yes ./${FILE} ${@:2})
