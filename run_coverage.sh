#!/bin/bash
set -e

echo "================================================"
echo "  Jasper Code Coverage Report Generator"
echo "================================================"
echo ""

# Colors for output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Configuration
COVERAGE_DIR="coverage_report"
BUILD_DIR="build/coverage"

# Step 1: Clean previous coverage data
echo -e "${BLUE}[1/6] Cleaning previous coverage data...${NC}"
find . -name "*.gcda" -type f -delete 2>/dev/null || true
rm -rf "$COVERAGE_DIR"

# Step 2: Build interpreter with coverage instrumentation
echo ""
echo -e "${BLUE}[2/6] Building interpreter with coverage instrumentation...${NC}"
make interpreter MODE=coverage -j8

# Step 3: Build test runner with coverage instrumentation
echo ""
echo -e "${BLUE}[3/6] Building test runner with coverage instrumentation...${NC}"
make tests MODE=coverage -j8

# Step 4: Run positive tests
echo ""
echo -e "${BLUE}[4/6] Running positive tests...${NC}"
./bin/run_tests

# Step 5: Run negative tests
echo ""
echo -e "${BLUE}[5/6] Running negative tests...${NC}"
./scripts/run_negative_tests.sh

# Step 6: Generate coverage report
echo ""
echo -e "${BLUE}[6/6] Generating coverage report...${NC}"

# Check if lcov is installed
if ! command -v lcov &> /dev/null; then
    echo -e "${YELLOW}Warning: lcov is not installed. Installing it is recommended for HTML reports.${NC}"
    echo -e "${YELLOW}Install with: sudo apt install lcov (Debian/Ubuntu)${NC}"
    echo ""
    echo "Generating basic gcov reports..."
    
    # Fallback to gcov for basic coverage info
    cd "$BUILD_DIR"
    gcov -r *.o 2>/dev/null || true
    cd ../..
    
    echo ""
    echo -e "${GREEN}Coverage data generated in $BUILD_DIR${NC}"
    echo "Install lcov for better HTML reports."
else
    # Use lcov to collect coverage data
    lcov --capture --directory "$BUILD_DIR" --output-file coverage.info --quiet
    
    # Remove coverage from unwanted files
    # - Test code itself (we want to measure production code coverage)
    # - System headers and libraries
    lcov --remove coverage.info \
        '*/src/test/*' \
        '/usr/include/*' \
        '/usr/lib/*' \
        '/usr/local/include/*' \
        --output-file coverage.info --quiet
    
    # Generate HTML report
    mkdir -p "$COVERAGE_DIR"
    genhtml coverage.info --output-directory "$COVERAGE_DIR" --quiet
    
    # Display summary
    echo ""
    echo "================================================"
    echo -e "${GREEN}Coverage Report Summary${NC}"
    echo "================================================"
    lcov --summary coverage.info 2>&1 | grep -E "(lines\.\.\.\.\.\.|functions\.\.\.\.)"
    
    echo ""
    echo "================================================"
    echo -e "${GREEN}HTML Report Generated${NC}"
    echo "================================================"
    echo "Open the report in your browser:"
    echo -e "${BLUE}file://$(pwd)/$COVERAGE_DIR/index.html${NC}"
    echo ""
    echo "Or run:"
    echo "  xdg-open $COVERAGE_DIR/index.html"
    echo ""
fi

echo -e "${GREEN}✓ Coverage analysis complete!${NC}"
