#!/bin/bash
set -e

# Build the interpreter
echo "Building interpreter..."
make interpreter MODE=dev -j8

# Build the test runner
echo "Building test runner..."
make tests MODE=dev -j8

# Run positive tests
echo ""
echo "Running positive tests..."
./bin/run_tests

# Run negative tests
echo ""
echo "Running negative tests..."
./scripts/run_negative_tests.sh
