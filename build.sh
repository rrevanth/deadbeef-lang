#!/bin/sh
gcc -Wall -fPIC -std=c99 -shared -O2 -o lang.so lang.c && echo "Built lang.so"
