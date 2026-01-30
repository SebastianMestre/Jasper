# Code Coverage for Jasper

This document explains how to use the code coverage facilities added to the Jasper project.

## Quick Start

To generate a code coverage report:

```bash
./run_coverage.sh
```

This will:
1. Build the interpreter and test runner with coverage instrumentation
2. Run the full test suite (positive and negative tests)
3. Generate an HTML coverage report in `coverage_report/`
4. Display a coverage summary in the terminal

## Viewing the Report

After running `./run_coverage.sh`, open the HTML report:

```bash
xdg-open coverage_report/index.html
```

Or navigate directly to: `file:///path/to/Jasper/coverage_report/index.html`

## Understanding Coverage Data

The coverage report shows:
- **Line Coverage**: Percentage of lines executed during tests
- **Function Coverage**: Percentage of functions called during tests
- **Branch Coverage**: Percentage of conditional branches taken

### Color Coding in HTML Report
- **Green**: Lines executed during tests
- **Red**: Lines not executed during tests
- **Orange**: Partially covered (for branches)

## Manual Coverage Build

You can also build with coverage manually:

```bash
# Build with coverage instrumentation
make interpreter MODE=coverage -j8
make tests MODE=coverage -j8

# Run tests (this generates .gcda files)
./bin/run_tests
./scripts/run_negative_tests.sh

# Generate report manually with lcov
lcov --capture --directory build/coverage --output-file coverage.info
lcov --remove coverage.info \
    '*/src/test/*' \
    '/usr/include/*' \
    '/usr/lib/*' \
    '/usr/local/include/*' \
    --output-file coverage.info
genhtml coverage.info --output-directory coverage_report
```

## Coverage Files

The following files are generated during coverage analysis:

- `*.gcno` - Coverage notes (generated at compile time)
- `*.gcda` - Coverage data (generated at runtime)
- `coverage.info` - Processed coverage data
- `coverage_report/` - HTML report directory

These files are already added to `.gitignore` and won't be committed.

## Requirements

- **gcov**: Included with GCC (already required for building Jasper)
- **lcov**: Optional but recommended for HTML reports
  - Install on Debian/Ubuntu: `sudo apt install lcov`
  - Install on Fedora/RHEL: `sudo dnf install lcov`
  - Install on Arch: `sudo pacman -S lcov`

## Cleaning Coverage Data

To clean coverage data between runs:

```bash
# Remove coverage data files
find . -name "*.gcda" -delete

# Remove reports
rm -rf coverage_report coverage.info

# Or do a full clean and rebuild
make clean
```

## Integration with CI/CD

The coverage script can be integrated into CI/CD pipelines:

```bash
# Run coverage and check minimum threshold (example)
./run_coverage.sh
lcov --summary coverage.info 2>&1 | grep "lines" | grep -oP '\d+\.\d+%' | head -1
# Parse and fail if below threshold
```

## Tips

1. **Focus on untested code**: Use the HTML report to identify functions and branches that aren't covered by tests
2. **Incremental testing**: Run coverage after adding new tests to see the impact
3. **Performance**: Coverage builds are slower due to instrumentation; use `MODE=dev` for regular development
4. **Accurate line numbers**: The coverage mode uses `-O0` (no optimization) for accurate line-to-code mapping

## Troubleshooting

### "lcov: command not found"
Install lcov using your package manager. The script will fall back to basic gcov if lcov is unavailable.

### No coverage data generated
Ensure the program ran successfully. Coverage data (`.gcda` files) are only generated when the instrumented binary exits normally.

### Old coverage data affecting results
Run `find . -name "*.gcda" -delete` to clean old data before generating new reports.
