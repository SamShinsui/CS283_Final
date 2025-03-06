#!/usr/bin/env bats

# Filename: student_tests.sh

# Multiple pipes with three commands
@test "Multiple pipes with three commands" {
    run bash -c "echo test | tr a-z A-Z | rev"
    [ "$status" -eq 0 ]
    [ "$output" = "TSET" ]
}

# Basic input redirection
@test "Basic input redirection" {
    echo "hello world" > input.txt
    run bash -c "cat < input.txt"
    [ "$status" -eq 0 ]
    [ "$output" = "hello world" ]
    rm input.txt
}

# Input redirection with pipe
@test "Input redirection with pipe" {
    echo "hello world" > input.txt
    run bash -c "cat < input.txt | tr a-z A-Z"
    [ "$status" -eq 0 ]
    [ "$output" = "HELLO WORLD" ]
    rm input.txt
}

# Handling empty commands in pipeline
@test "Handling empty commands in pipeline" {
    run bash -c "echo test | || rev"
    [ "$status" -ne 0 ]
}

# Handling maximum number of pipes
@test "Handling maximum number of pipes" {
    run bash -c "echo test | tr a-z A-Z | rev | tr A-Z a-z | rev | tr a-z A-Z"
    [ "$status" -eq 0 ]
    [ "$output" = "TEST" ]
}



# Pipeline with redirection in middle command
@test "Pipeline with redirection in middle command" {
    echo "hello world" > input.txt
    run bash -c "cat < input.txt | tr a-z A-Z | rev"
    [ "$status" -eq 0 ]
    [ "$output" = "DLROW OLLEH" ]
    rm input.txt
}

# Command output counting with different pipelines
@test "Command output counting with different pipelines" {
    run bash -c "echo -e 'line1\nline2\nline3' | wc -l"
    [ "$status" -eq 0 ]
    [ "$output" -eq 3 ]
}

# Memory management in long pipeline
@test "Memory management in long pipeline" {
    run bash -c "seq 1 1000 | awk '{print $1*2}' | tr -d '\n'"
    [ "$status" -eq 0 ]
}

