#!/usr/bin/env bash
# Dependency-free fallback when CMake/CTest is not installed.
set -euo pipefail
cd "$(dirname "$0")/.."
compiler="${CXX:-g++}"
mode="${1:-plain}"
flags=(-std=c++17 -Wall -Wextra -Wpedantic -Werror -Iinclude -I. -g)
case "$mode" in
  plain) flags+=(-O2) ;;
  sanitizers) flags+=(-O1 -fsanitize=address,undefined -fno-omit-frame-pointer -fno-pie -no-pie) ;;
  *) echo "Usage: $0 [plain|sanitizers]" >&2; exit 2 ;;
esac
build="build-direct-$mode"; mkdir -p "$build"
"$compiler" --version | head -1
sources=(src/lx16a.cpp src/bus.cpp src/safety.cpp src/kinematics.cpp)
for name in protocol bus safety kinematics; do
  "$compiler" "${flags[@]}" "${sources[@]}" "tests/test_$name.cpp" -o "$build/test_$name"
  "$build/test_$name"
done
"$compiler" "${flags[@]}" "${sources[@]}" examples/dry_run.cpp -o "$build/dry_run"
"$build/dry_run"
echo "PASS all 5 host executables ($mode)"
