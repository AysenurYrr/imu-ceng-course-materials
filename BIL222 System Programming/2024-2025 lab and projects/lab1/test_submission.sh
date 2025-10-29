#!/usr/bin/env bats

# Test Exercise 1: System Report
@test "System Report script creates log file" {
  run ./system_report.sh
  [ "$status" -eq 0 ]
  [[ "$output" == *"log"* ]]
  logfile=$(echo "$output" | grep -o "system_report_[0-9_]*\.log")
  [ -f "$logfile" ]
}

# Test Exercise 2: Even/Odd Checker
@test "Even/Odd script works correctly" {
  run ./even_odd.sh 10
  [ "$status" -eq 0 ]
  [ "$output" == "10 is even." ]

  run ./even_odd.sh 7
  [ "$output" == "7 is odd." ]

  # Test error handling
  run ./even_odd.sh
  [ "$status" -eq 1 ]
  [ "$output" == "Error: Please provide a number as an argument." ]
}


# Test Exercise 3: Backup Script
@test "Backup script creates valid backup" {
  rm -rf test_dir
  mkdir test_dir
  touch test_dir/file{1..3}.txt

  run ./custom_backup.sh test_dir
  [ "$status" -eq 0 ]
  [[ "$output" == *"backup_"*".tar.gz"* ]]
  backup_name=$(echo "$output" | grep -o "backup_[0-9]*\.tar\.gz")
  [ -f "$backup_name" ]

  # Cleanup
  rm -rf test_dir $backup_name
}
