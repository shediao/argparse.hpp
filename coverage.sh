#!/usr/bin/env bash
set -euo pipefail
#
# coverage.sh — Generate line-by-line coverage for include/argparse/argparse.hpp
#              using LLVM source-based code coverage (Clang + llvm-profdata + llvm-cov).
#
# Prerequisites:
#   - clang++ (or clang++-20), llvm-profdata, llvm-cov in PATH
#   - LLVM_PROFILE_FILE not already set (script will set it)
#
# Usage:
#   ./coverage.sh              # build + test + report (text summary)
#   ./coverage.sh html         # also generate HTML report in build/coverage/html/
#   ./coverage.sh lines        # print full line-by-line text to stdout
#   ./coverage.sh clean        # remove build/coverage directory

readonly PROJECT_ROOT="$(cd "$(dirname "$0")" && pwd)"
readonly HEADER_FILE="include/argparse/argparse.hpp"
readonly BUILD_DIR="build/coverage"
readonly PROFRAW_DIR="${BUILD_DIR}/profraw"
readonly PROFDATA="${BUILD_DIR}/coverage.profdata"
readonly TEST_BINARY="${BUILD_DIR}/tests/all_test"

# -- find LLVM tools (prefer versioned names, fall back to unversioned) ----------
find_tool() {
  local name="$1"
  for candidate in "${name}-20" "${name}-19" "${name}-18" "${name}-17" "${name}"; do
    if command -v "$candidate" &>/dev/null; then
      echo "$candidate"
      return 0
    fi
  done
  echo "ERROR: could not find ${name} in PATH" >&2
  return 1
}

CLANGPP=$(find_tool clang++)
LLVM_PROFDATA=$(find_tool llvm-profdata)
LLVM_COV=$(find_tool llvm-cov)

info()  { echo -e "\033[1;32m[INFO]\033[0m  $*"; }
warn()  { echo -e "\033[1;33m[WARN]\033[0m  $*"; }
error() { echo -e "\033[1;31m[ERROR]\033[0m $*"; exit 1; }

cmd="${1:-summary}"

# -- clean ---------------------------------------------------------------------
if [[ "$cmd" == "clean" ]]; then
  info "Removing ${BUILD_DIR} …"
  rm -rf "${PROJECT_ROOT:?}/${BUILD_DIR}"
  info "Done."
  exit 0
fi

# -- configure -----------------------------------------------------------------
if [[ ! -f "${PROJECT_ROOT}/${BUILD_DIR}/build.ninja" ]]; then
  info "Configuring CMake (compiler=${CLANGPP}) …"
  cmake -B "${PROJECT_ROOT}/${BUILD_DIR}" -S "${PROJECT_ROOT}" \
    -G Ninja \
    -DCMAKE_CXX_COMPILER="${CLANGPP}" \
    -DCMAKE_C_COMPILER="${CLANGPP//clang\+\+/clang}" \
    -DCMAKE_BUILD_TYPE=Debug \
    -DARGPARSE_BUILD_TESTS=ON \
    -DCMAKE_CXX_FLAGS="-fprofile-instr-generate -fcoverage-mapping" \
    -DCMAKE_EXE_LINKER_FLAGS="-fprofile-instr-generate"
else
  info "Build directory already exists, skipping cmake configure."
fi

# -- build ---------------------------------------------------------------------
info "Building tests …"
cmake --build "${PROJECT_ROOT}/${BUILD_DIR}" -j"$(nproc)"

# -- run tests -----------------------------------------------------------------
info "Running tests (profraw → ${PROFRAW_DIR}) …"
rm -rf "${PROJECT_ROOT:?}/${PROFRAW_DIR}"
mkdir -p "${PROJECT_ROOT}/${PROFRAW_DIR}"
export LLVM_PROFILE_FILE="${PROJECT_ROOT}/${PROFRAW_DIR}/%p.profraw"
ctest --test-dir "${PROJECT_ROOT}/${BUILD_DIR}" --output-on-failure

# -- merge profdata ------------------------------------------------------------
info "Merging profraw files → ${PROFDATA} …"
"${LLVM_PROFDATA}" merge -sparse "${PROJECT_ROOT}/${PROFRAW_DIR}"/*.profraw \
  -o "${PROJECT_ROOT}/${PROFDATA}"

# -- report --------------------------------------------------------------------
case "$cmd" in
  summary)
    info "Coverage summary:"
    "${LLVM_COV}" report \
      -instr-profile="${PROJECT_ROOT}/${PROFDATA}" \
      "${PROJECT_ROOT}/${TEST_BINARY}" \
      "${PROJECT_ROOT}/${HEADER_FILE}"
    ;;
  html)
    info "Generating HTML report → ${BUILD_DIR}/html …"
    mkdir -p "${PROJECT_ROOT}/${BUILD_DIR}/html"
    "${LLVM_COV}" show \
      -instr-profile="${PROJECT_ROOT}/${PROFDATA}" \
      "${PROJECT_ROOT}/${TEST_BINARY}" \
      -format=html \
      -show-line-counts-or-regions \
      -show-instantiation-summary \
      -output-dir="${PROJECT_ROOT}/${BUILD_DIR}/html" \
      "${PROJECT_ROOT}/${HEADER_FILE}"
    info "Open file://${PROJECT_ROOT}/${BUILD_DIR}/html/index.html in a browser."
    ;;
  lines)
    info "Line-by-line coverage for ${HEADER_FILE}:"
    "${LLVM_COV}" show \
      -instr-profile="${PROJECT_ROOT}/${PROFDATA}" \
      "${PROJECT_ROOT}/${TEST_BINARY}" \
      -format=text \
      -show-line-counts-or-regions \
      "${PROJECT_ROOT}/${HEADER_FILE}"
    ;;
  *)
    error "Unknown command: '${cmd}'.  Use: summary | html | lines | clean"
    ;;
esac
