#!/bin/bash

# Compile the server and client
echo "Compiling server and client..."
g++ server.cpp utils.cpp eventloop.cpp commands.cpp -o server -std=c++17
g++ client.cpp utils.cpp clientUtils.cpp -o client -std=c++17

# Start the server in the background
echo "Starting server..."
./server &

# Capture the server process ID so we can kill it later
SERVER_PID=$!

# Wait for the server to initialize
sleep 1

# Initialize a counter for passed tests
passed_tests=0
total_tests=0

# Function to run a client command and check response
run_test() {
    cmd=$1
    expected_response=$2

    # Run the client command and remove leading/trailing spaces
    response=$(eval "$cmd" | xargs)

    # Trim leading/trailing spaces from the expected response
    expected_response=$(echo "$expected_response" | xargs)

    # Compare the actual response with the expected response
    if [[ "$response" == "$expected_response" ]]; then
        echo "PASS: $cmd"
        passed_tests=$((passed_tests + 1))  # Increment pass count
    else
        echo "FAIL: $cmd"
        echo "Expected: $expected_response"
        echo "Got: $response"
    fi
}

# Define the tests (client commands) and expected responses
declare -a tests=(
    "./client get k"
    "./client set k v"
    "./client get k"
    "./client del k"
    "./client get k"
    "./client aaa bbb"
)

declare -a expected_responses=(
    "server says: [2]"
    "server says: [0]"
    "server says: [0] v"
    "server says: [0]"
    "server says: [2]"
    "server says: [1] Unknown cmd"
)

# Run each test by iterating over the tests and expected responses simultaneously
for i in "${!tests[@]}"; do
    cmd="${tests[$i]}"
    expected_response="${expected_responses[$i]}"
    
    # Run the test
    run_test "$cmd" "$expected_response"
    total_tests=$((total_tests + 1))  # Increment total test count
done

# Stop the server
echo "Stopping server..."
kill $SERVER_PID

# Print the number of passed tests
echo "Tests passed: $passed_tests/$total_tests"
