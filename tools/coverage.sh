#!/usr/bin/env bash
# Build the unit tests instrumented for coverage, run them headless, and print a
# gcov/gcovr report (plus an HTML report). Requires gcovr (pip install gcovr).
#
#   tools/coverage.sh            # uses ./build-coverage
#   tools/coverage.sh mybuilddir
set -euo pipefail
cd "$(dirname "$0")/.."

BUILD=${1:-build-coverage}
CMAKE=$(command -v /usr/bin/cmake || command -v cmake)
GCOVR=$(command -v gcovr || echo "$HOME/.local/bin/gcovr")

"$CMAKE" -S . -B "$BUILD" -DWHATLY_TESTS=ON -DWHATLY_COVERAGE=ON -DCMAKE_BUILD_TYPE=Debug
"$CMAKE" --build "$BUILD" --target tst_logic tst_ui_assets
find "$BUILD" -name '*.gcda' -delete 2>/dev/null || true
QT_QPA_PLATFORM=offscreen ctest --test-dir "$BUILD" --output-on-failure

"$GCOVR" --root . --filter 'src/' --print-summary \
         --html-details "$BUILD/coverage.html" "$BUILD"
echo
echo "HTML report: $BUILD/coverage.html"
