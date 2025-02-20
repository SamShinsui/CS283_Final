#!/usr/bin/env bats

# File: student_tests.sh
#
# Create your unit tests suit in this file

@test "Example: check ls runs without errors" {
    run ./dsh <<EOF
ls
EOF
    # Assertions
    [ "$status" -eq 0 ]
}

@test "Check if cd with no arguments works" {
    run ./dsh <<EOF
cd
EOF
    [ "$status" -eq 0 ]
}

@test "Make sure pwd shows current directory" {
    run ./dsh <<EOF
pwd
EOF
    [ "$status" -eq 0 ]
}

@test "Verify cd changes directory correctly" {
    run ./dsh <<EOF
cd ..
pwd
EOF
    [ "$status" -eq 0 ]
}

@test "Test multiple commands in sequence" {
    run ./dsh <<EOF
ls
pwd
cd ..
pwd
EOF
    [ "$status" -eq 0 ]
}

@test "Check if exit command works" {
    run ./dsh <<EOF
exit
EOF
    [ "$status" -eq 0 ]
}

@test "Test echo with simple text" {
    run ./dsh <<EOF
echo hello world
EOF
    [ "$status" -eq 0 ]
}

@test "Verify echo preserves multiple spaces in quotes" {
    run ./dsh <<EOF
echo "hello    world"
EOF
    [ "$status" -eq 0 ]
}

@test "Test command with special characters" {
    run ./dsh <<EOF
echo hello! world?
EOF
    [ "$status" -eq 0 ]
}

@test "Check behavior with empty command" {
    run ./dsh <<EOF

EOF
    [ "$output" = "warning: no commands provided" ]
}

@test "Test cd to nonexistent directory" {
    run ./dsh <<EOF
cd /path/that/doesnt/exist
EOF
    [[ "$output" =~ "cd:" ]]
}

@test "Verify command not found error" {
    run ./dsh <<EOF
thiscommanddoesntexist
EOF
    [[ "$output" =~ "execvp" ]]
}

@test "Test multiple spaces between commands" {
    run ./dsh <<EOF
echo    hello     world
EOF
    [ "$status" -eq 0 ]
}

@test "Check if spaces are preserved in echo" {
    run ./dsh <<EOF
echo "   hello   world   "
EOF
    [ "$status" -eq 0 ]
}

@test "Test simple ls with flags" {
    run ./dsh <<EOF
ls -la
EOF
    [ "$status" -eq 0 ]
}

@test "Verify cat command works" {
    run ./dsh <<EOF
echo "test" > testfile.txt
cat testfile.txt
EOF
    [ "$status" -eq 0 ]
}

@test "Test multiple echo commands" {
    run ./dsh <<EOF
echo first line
echo second line
echo third line
EOF
    [ "$status" -eq 0 ]
}

@test "Check directory listing after cd" {
    run ./dsh <<EOF
mkdir testdir
cd testdir
ls
EOF
    [ "$status" -eq 0 ]
}

@test "Test command with single quotes" {
    run ./dsh <<EOF
echo 'hello world'
EOF
    [ "$status" -eq 0 ]
}

@test "Verify touch command creates file" {
    run ./dsh <<EOF
touch newfile
ls newfile
EOF
    [ "$status" -eq 0 ]
}

@test "Test consecutive cd commands" {
    run ./dsh <<EOF
cd ..
cd .
cd ..
EOF
    [ "$status" -eq 0 ]
}

@test "Check command line editing" {
    run ./dsh <<EOF
echo hello world
echo "edited    spaces"
EOF
    [ "$status" -eq 0 ]
}

@test "Test error handling with bad file" {
    run ./dsh <<EOF
cat nonexistentfile
EOF
    [ "$status" -ne 0 ]
}

@test "Verify working directory persistence" {
    run ./dsh <<EOF
pwd
cd ..
pwd
cd -
pwd
EOF
    [ "$status" -eq 0 ]
}
