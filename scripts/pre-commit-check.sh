#!/usr/bin/env bash
# Pre-commit gate: blocks git commit if format, build, or tests fail.
# Reads Claude Code hook JSON from stdin; exits 0 (outputs block JSON) on failure.
set -uo pipefail

ROOT="/home/mih/cs/vi/robotics/proj1-virt-sig-cond"

INPUT=$(cat)
CMD=$(printf '%s' "$INPUT" | jq -r '.tool_input.command // ""')

# Only act on git commit invocations
if ! printf '%s' "$CMD" | grep -qE '(^|[;&|[:space:]])git commit'; then
    exit 0
fi

cd "$ROOT"

FAILED=0
MESSAGES=""

# ── 1. Format ────────────────────────────────────────────────────────────────
printf '==> Checking formatting...\n' >&2
if ! clang-format --dry-run --Werror include/*.hpp src/*.cpp tests/*.cpp 2>/dev/null; then
    MESSAGES="${MESSAGES}\n  • Formatting: run: clang-format -i include/*.hpp src/*.cpp tests/*.cpp"
    FAILED=1
fi

# ── 2. Build ─────────────────────────────────────────────────────────────────
printf '==> Building (Release)...\n' >&2
if ! cmake --build build --parallel >/dev/null 2>&1; then
    MESSAGES="${MESSAGES}\n  • Build failed: cmake --build build"
    FAILED=1
fi

# ── 3. Tests ─────────────────────────────────────────────────────────────────
printf '==> Running tests...\n' >&2
if ! (cd build && ctest -q --output-on-failure >/dev/null 2>&1); then
    MESSAGES="${MESSAGES}\n  • Tests failing: cd build && ctest --output-on-failure"
    FAILED=1
fi

# ── Result ───────────────────────────────────────────────────────────────────
if [ "$FAILED" -eq 1 ]; then
    REASON="Pre-commit checks FAILED — commit blocked.$(printf '%b' "$MESSAGES")"
    jq -n --arg r "$REASON" '{
        "continue": false,
        "stopReason": $r
    }'
    exit 0
fi

printf '==> All pre-commit checks passed.\n' >&2
exit 0
