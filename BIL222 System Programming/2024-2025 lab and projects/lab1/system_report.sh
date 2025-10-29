#!/bin/bash

# Greet the user
echo "Hello!"

# List all files in the home directory
echo "Listing all files in the home directory:"
ls -l ~/

# Create a timestamped log file
timestamp=$(date +"%Y%m%d_%H%M%S")
logfile="system_report_$timestamp.log"
echo "Creating log file: $logfile"
ls -l ~/ > "$logfile"

# Output the log file name for the test to capture
echo "Log file created: $logfile"
