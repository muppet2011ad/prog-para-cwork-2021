#!/bin/bash

#This should give no warnings or errors
gcc -Wextra -Wall -pedantic -std=c11 -o test1 test1.c connect4.c

./test1
if [ $? -eq 0 ]
then
  echo Test passed
else
  echo Test failed
fi
diff  --ignore-trailing-space --ignore-blank-lines -q output1.txt test_output1.txt
if [ $? -eq 0 ]
then
  echo Test passed
else
  echo Test failed
fi

