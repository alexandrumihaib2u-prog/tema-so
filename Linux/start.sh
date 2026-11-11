#!/bin/bash
# Compileaza si porneste doua procese separate
gcc -Wall -o numarare numarare.c -pthread -lrt || exit 1
./numarare & ./numarare &
wait
