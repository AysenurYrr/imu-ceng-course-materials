#!/bin/bash

# Check if a directory path is provided
if [ -z "$1" ]; then
  echo "Error: Please provide a directory path as an argument."
  exit 1
fi

# Store the directory path
directory=$1

# Check if the provided path is a directory
if [ ! -d "$directory" ]; then
  echo "Error: $directory is not a valid directory."
  exit 1
fi

# Create a backup filename with a timestamp
timestamp=$(date +"%Y%m%d_%H%M%S")
backupfile="backup_$(basename "$directory")_$timestamp.tar.gz"

# Create the backup
tar -czf "$backupfile" -C "$(dirname "$directory")" "$(basename "$directory")"

# Output the backup file name for the test to capture
echo "Backup created: $backupfile"
