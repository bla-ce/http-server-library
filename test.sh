#!/bin/bash

# Define colors
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No color

# Define the tests - format: url method status_code (response)
tests=(
  "localhost:1337/api GET 404"
  "localhost:1337/health GET 200 ok"
  "localhost:1337/ GET 200"
  "localhost:1337/index GET 200"
  "localhost:1337/ind GET 404"
  "localhost:1337/ POST 404"
  "localhost:1337/ CONNECT 405"
  "localhost:1337/ DELETE 405"
  "localhost:1337/ GE 405"
  "localhost:1337/api/v1 GET 200"
  "localhost:1337/api/v1/ GET 200"
  "localhost:1337/i/v1/ GET 404"
  "localhost:1337/api/v1/ PUT 404"
  "localhost:1337/api/v1/ CON 405"
  "localhost:1337/not-found-not-found-not-found-not-found-not-found GET 400"
  "localhost:1337/index?id=1 GET 200"
  "localhost:1337/index#anchor GET 200"
)

total_tests=${#tests[@]}
passed_tests=0

# Function to perform the test
perform_test() {
  local url=$1
  local method=$2
  local expected_status=$3
  local expected_response=$4
  local test_number=$5
  local total_tests=$6

  # Perform the request, capturing the HTTP status code and response body
  response=$(curl -s -w "\n%{http_code}" -X "$method" "$url")
  status_code=$(echo "$response" | tail -n1)
  body=$(echo "$response" | sed '$d') # Removes the last line (status code)

  # Check if the actual status code matches the expected one
  if [ "$status_code" -eq "$expected_status" ] && [[ "$body" == *"$expected_response"* ]]; then
    echo -e "[${YELLOW}TEST $test_number/$total_tests${NC}] : $method $url -> ${GREEN}PASSED${NC} (expected: $expected_status, response: '$expected_response')"
    passed_tests=$((passed_tests + 1))
  else
    echo -e "[${YELLOW}TEST $test_number/$total_tests${NC}] : $method $url -> ${RED}FAILED${NC} (expected: $expected_status, got: $status_code, expected response: '$expected_response', got: '$body')"
  fi
}

echo -e "\n---------- HTTP Server Tests ----------\n"

# Loop through each test
test_number=1
for test in "${tests[@]}"; do
  # Split the test into URL, method, expected status code, and expected response
  IFS=' ' read -r -a parts <<< "$test"
  url="${parts[0]}"
  method="${parts[1]}"
  expected_status="${parts[2]}"
  expected_response="${parts[3]-}"

  # Perform the test
  perform_test "$url" "$method" "$expected_status" "$expected_response" "$test_number" "$total_tests"

  # Increment the test number
  test_number=$((test_number + 1))

  sleep 0.1
done

echo -e "\n--------------- Results ---------------\n"

# Print total results
echo -e "Total Tests: $total_tests, ${GREEN}PASSED${NC}: $passed_tests, ${RED}FAILED${NC}: $((total_tests - passed_tests))\n"

