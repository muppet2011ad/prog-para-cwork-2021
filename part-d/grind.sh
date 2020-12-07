#!/bin/bash
gcc sort.c -o sort
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose --log-file=grind-out.txt ./sort astar