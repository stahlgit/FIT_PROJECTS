#!/usr/bin/env bash
# End-to-end integration tests for ipk-rdt.
# Run from the repository root:  bash tests/e2e.sh
# Or via Makefile:                make e2e

set -uo pipefail

# ── Paths ────────────────────────────────────────────────────────────────────
REPO="$(cd "$(dirname "$0")/.." && pwd)"
EXE="$REPO/ipk-rdt"
TMP="$(mktemp -d /tmp/ipk_e2e.XXXXXX)"
PORT=19100      # start of test port range
PASS=0
FAIL=0
SKIP=0

# ── Cleanup ──────────────────────────────────────────────────────────────────
cleanup() {
    # Kill any background server we may have left behind
    jobs -p 2>/dev/null | xargs -r kill 2>/dev/null || true
    rm -rf "$TMP"
}
trap cleanup EXIT INT TERM

# ── Helpers ──────────────────────────────────────────────────────────────────
ok()   { printf '\033[32mPASS\033[0m  %s\n' "$1"; PASS=$((PASS + 1)); }
fail() { printf '\033[31mFAIL\033[0m  %s: %s\n' "$1" "$2"; FAIL=$((FAIL + 1)); }
skip() { printf '\033[33mSKIP\033[0m  %s\n' "$1"; SKIP=$((SKIP + 1)); }

next_port() { PORT=$((PORT + 1)); echo $((PORT - 1)); }

# Wait up to 2 s for a UDP socket to appear on the given port.
wait_port() {
    local p=$1 i
    for i in $(seq 1 20); do
        ss -uln 2>/dev/null | grep -q ":$p " && return 0
        sleep 0.1
    done
    return 1
}

# ── Core transfer helper ──────────────────────────────────────────────────────
# transfer_file NAME INPUT_FILE SERVER_ADDR
#   Starts server (-o tmp_out), runs client (-i INPUT_FILE), diffs output.
transfer_file() {
    local name="$1" input="$2" addr="${3:-127.0.0.1}"
    local port; port=$(next_port)
    local output="$TMP/out_$port"

    "$EXE" -s -p "$port" -o "$output" -w 5 &
    local spid=$!

    if ! wait_port "$port"; then
        kill "$spid" 2>/dev/null; wait "$spid" 2>/dev/null || true
        fail "$name" "server did not bind in time"; return
    fi

    if ! "$EXE" -c -a "$addr" -p "$port" -i "$input" -w 5 2>/dev/null; then
        kill "$spid" 2>/dev/null; wait "$spid" 2>/dev/null || true
        fail "$name" "client exited non-zero"; return
    fi

    if ! wait "$spid" 2>/dev/null; then
        fail "$name" "server exited non-zero"; return
    fi

    if diff -q "$input" "$output" >/dev/null 2>&1; then
        ok "$name"
    else
        fail "$name" "output differs from input"
    fi
}

# pipe_transfer NAME INPUT_FILE
#   Server writes to stdout (captured), client reads from stdin (INPUT_FILE).
pipe_transfer() {
    local name="$1" input="$2"
    local port; port=$(next_port)
    local output="$TMP/pipe_$port"

    "$EXE" -s -p "$port" -w 5 >"$output" &
    local spid=$!

    if ! wait_port "$port"; then
        kill "$spid" 2>/dev/null; wait "$spid" 2>/dev/null || true
        fail "$name" "server did not bind in time"; return
    fi

    if ! "$EXE" -c -a 127.0.0.1 -p "$port" -w 5 <"$input" 2>/dev/null; then
        kill "$spid" 2>/dev/null; wait "$spid" 2>/dev/null || true
        fail "$name" "client exited non-zero"; return
    fi

    if ! wait "$spid" 2>/dev/null; then
        fail "$name" "server exited non-zero"; return
    fi

    if diff -q "$input" "$output" >/dev/null 2>&1; then
        ok "$name"
    else
        fail "$name" "output differs from input"
    fi
}

# ── Build ─────────────────────────────────────────────────────────────────────
cd "$REPO"
if ! make build_target -s 2>&1; then
    echo "Build failed — cannot run tests" >&2
    exit 1
fi

printf '\n\033[1m═══════════════════════════════════════\033[0m\n'
printf '\033[1m ipk-rdt — end-to-end integration tests\033[0m\n'
printf '\033[1m═══════════════════════════════════════\033[0m\n\n'

# ── Prepare test inputs ───────────────────────────────────────────────────────
EMPTY="$TMP/empty"
SMALL="$TMP/small.txt"
BIN_4K="$TMP/4k.bin"
BIN_1M="$TMP/1m.bin"
BIN_50M="$TMP/50m.bin"

touch "$EMPTY"
printf 'Hello, IPK RDT!\nThis is a small text file.\n' >"$SMALL"
dd if=/dev/urandom bs=4K   count=1  of="$BIN_4K"  status=none
dd if=/dev/urandom bs=1M   count=1  of="$BIN_1M"  status=none
dd if=/dev/urandom bs=1M   count=50 of="$BIN_50M" status=none

# ── Section 1: file → file (IPv4) ────────────────────────────────────────────
printf '--- file → file (IPv4) ---\n'
transfer_file "empty input"          "$EMPTY"
transfer_file "small text"           "$SMALL"
transfer_file "binary 4 KB"          "$BIN_4K"
transfer_file "binary 1 MB"          "$BIN_1M"
transfer_file "binary 50 MB"         "$BIN_50M"

# ── Section 2: stdin → stdout ────────────────────────────────────────────────
printf '\n--- stdin → stdout ---\n'
pipe_transfer "empty input (stdin→stdout)"    "$EMPTY"
pipe_transfer "small text (stdin→stdout)"     "$SMALL"
pipe_transfer "binary 1 MB (stdin→stdout)"    "$BIN_1M"

# ── Section 3: stdin → file ──────────────────────────────────────────────────
printf '\n--- stdin → file ---\n'
port=$(next_port)
out_sf="$TMP/stdin_file_$port"
"$EXE" -s -p "$port" -o "$out_sf" -w 5 &
spid=$!
if wait_port "$port" && "$EXE" -c -a 127.0.0.1 -p "$port" -w 5 <"$BIN_1M" 2>/dev/null && wait "$spid" 2>/dev/null; then
    diff -q "$BIN_1M" "$out_sf" >/dev/null 2>&1 && ok "binary 1 MB (stdin→file)" || fail "binary 1 MB (stdin→file)" "output mismatch"
else
    kill "$spid" 2>/dev/null; wait "$spid" 2>/dev/null || true
    fail "binary 1 MB (stdin→file)" "transfer failed"
fi

# ── Section 4: file → stdout ─────────────────────────────────────────────────
printf '\n--- file → stdout ---\n'
port=$(next_port)
out_fs="$TMP/file_stdout_$port"
"$EXE" -s -p "$port" -w 5 >"$out_fs" &
spid=$!
if wait_port "$port" && "$EXE" -c -a 127.0.0.1 -p "$port" -i "$BIN_1M" -w 5 2>/dev/null && wait "$spid" 2>/dev/null; then
    diff -q "$BIN_1M" "$out_fs" >/dev/null 2>&1 && ok "binary 1 MB (file→stdout)" || fail "binary 1 MB (file→stdout)" "output mismatch"
else
    kill "$spid" 2>/dev/null; wait "$spid" 2>/dev/null || true
    fail "binary 1 MB (file→stdout)" "transfer failed"
fi

# ── Section 5: IPv6 ──────────────────────────────────────────────────────────
printf '\n--- IPv6 ---\n'
if ip -6 addr show lo 2>/dev/null | grep -q '::1'; then
    transfer_file "small text (IPv6)"    "$SMALL"   "::1"
    transfer_file "binary 1 MB (IPv6)"  "$BIN_1M"  "::1"
else
    skip "IPv6 (::1 not available on this host)"
    skip "IPv6 binary 1 MB"
fi

# ── Section 6: Exit code / timeout behaviour ──────────────────────────────────
printf '\n--- Exit codes & timeouts ---\n'

# Client must exit non-zero when server is unreachable
port=$(next_port)
if ! "$EXE" -c -a 127.0.0.1 -p "$port" -i "$SMALL" -w 2 2>/dev/null; then
    ok "client exits non-zero (no server)"
else
    fail "client exits non-zero (no server)" "got exit code 0"
fi

# Server must exit non-zero after -w seconds with no client
port=$(next_port)
"$EXE" -s -p "$port" -o /dev/null -w 2 2>/dev/null &
spid=$!
if ! wait "$spid" 2>/dev/null; then
    ok "server exits non-zero (no client, timeout)"
else
    fail "server exits non-zero (no client, timeout)" "got exit code 0"
fi

# ── Summary ───────────────────────────────────────────────────────────────────
printf '\n\033[1m═══════════════════════════════════════\033[0m\n'
printf "\033[1mResults:\033[0m  \033[32m%d passed\033[0m  \033[31m%d failed\033[0m  \033[33m%d skipped\033[0m\n" \
    "$PASS" "$FAIL" "$SKIP"
printf '\033[1m═══════════════════════════════════════\033[0m\n\n'

[[ $FAIL -eq 0 ]]
