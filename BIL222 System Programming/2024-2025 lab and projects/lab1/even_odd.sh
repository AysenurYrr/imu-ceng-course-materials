#!/bin/bash

# Check if a number is provided
if [ -z "$1" ]; then
  echo "Error: Please provide a number as an argument."
  exit 1
fi

# Store the input number
number=$1

# Check if the number is even or odd
if [ $((number % 2)) -eq 0 ]; then
  echo "$number is even."
else
  echo "$number is odd."
fi
