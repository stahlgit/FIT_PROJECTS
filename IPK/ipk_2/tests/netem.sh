#!/usr/bin/env bash
# Network-impairment tests for ipk-rdt using tc-netem.
# Applies qdisc rules to the loopback interface — requires sudo.
#
# Run from the repository root:
#   sudo bash tests/netem.sh
# Or via Makefile:
#   make netem          (will sudo internally if needed)

set -uo pipefail

# ── Paths ─────────────────────────────────────────────────────────────────────
REPO="$(cd "$(dirname "$0")/.." && pwd)"
EXE="$REPO/ipk-rdt"
TMP="$(mktemp -d /tmp/ipk_netem.XXXXXX)"
PORT=19400      # start of netem test port range (separate from e2e range)
PASS=0
FAIL=0
SKIP=0
NETEM_ACTIVE=0  # track whether we have an active qdisc on lo

# ── Privilege check ───────────────────────────────────────────────────────────
if [[ $EUID -ne 0 ]]; then
    echo "Netem tests require root. Re-running with sudo..." >&2
    exec sudo "$0" "$@"
fi

# ── Cleanup ───────────────────────────────────────────────────────────────────
clear_netem() {
    if [[ $NETEM_ACTIVE -eq 1 ]]; then
        tc qdisc del dev lo root 2>/dev/null || true
        NETEM_ACTIVE=0
    fi
}

cleanup() {
    jobs -p 2>/dev/null | xargs -r kill 2>/dev/null || true
    clear_netem
    rm -rf "$TMP"
}
trap cleanup EXIT INT TERM

# ── Helpers ───────────────────────────────────────────────────────────────────
ok()   { printf '\033[32mPASS\033[0m  %s\n' "$1"; PASS=$((PASS + 1)); }
fail() { printf '\033[31mFAIL\033[0m  %s: %s\n' "$1" "$2"; FAIL=$((FAIL + 1)); }
skip() { printf '\033[33mSKIP\033[0m  %s\n' "$1"; SKIP=$((SKIP + 1)); }

next_port() { PORT=$((PORT + 1)); echo $((PORT - 1)); }

wait_port() {
    local p=$1 i
    for i in $(seq 1 30); do
        ss -uln 2>/dev/null | grep -q ":$p " && return 0
        sleep 0.1
    done
    return 1
}

# Apply a netem rule to lo (del first so we never double-stack).
apply_netem() {
    clear_netem
    tc qdisc add dev lo root netem "$@"
    NETEM_ACTIVE=1
}

# Run one file→file transfer under the current netem conditions.
# Usage: netem_transfer NAME INPUT_FILE TIMEOUT
netem_transfer() {
    local name="$1" input="$2" timeout_s="${3:-20}"
    local port; port=$(next_port)
    local output="$TMP/out_$port"

    "$EXE" -s -p "$port" -o "$output" -w "$timeout_s" 2>/dev/null &
    local spid=$!

    if ! wait_port "$port"; then
        kill "$spid" 2>/dev/null; wait "$spid" 2>/dev/null || true
        fail "$name" "server did not bind in time"; return
    fi

    if ! "$EXE" -c -a 127.0.0.1 -p "$port" -i "$input" -w "$timeout_s" 2>/dev/null; then
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
    echo "Build failed — cannot run netem tests" >&2
    exit 1
fi

# ── Check netem module ────────────────────────────────────────────────────────
if ! modprobe sch_netem 2>/dev/null && ! tc qdisc add dev lo root netem 2>/dev/null; then
    echo "tc-netem not available on this kernel — skipping all netem tests" >&2
    exit 0
fi
tc qdisc del dev lo root 2>/dev/null || true
NETEM_ACTIVE=0

printf '\n\033[1m═══════════════════════════════════════════\033[0m\n'
printf '\033[1m ipk-rdt — network impairment tests (netem)\033[0m\n'
printf '\033[1m═══════════════════════════════════════════\033[0m\n\n'

# ── Prepare test inputs ───────────────────────────────────────────────────────
SMALL="$TMP/small.txt"
BIN_4K="$TMP/4k.bin"
BIN_1M="$TMP/1m.bin"

printf 'Hello, IPK RDT!\nThis is a small text file.\n' >"$SMALL"
dd if=/dev/urandom bs=4K  count=1  of="$BIN_4K"  status=none
dd if=/dev/urandom bs=1M  count=1  of="$BIN_1M"  status=none

# ── Section 1: Packet loss ─────────────────────────────────────────────────────
printf -- '--- Packet loss ---\n'

apply_netem loss 5%
netem_transfer "5% loss — small text"  "$SMALL"  15
netem_transfer "5% loss — 1 MB"        "$BIN_1M" 30

apply_netem loss 10%
netem_transfer "10% loss — 1 MB"       "$BIN_1M" 45

apply_netem loss 20%
netem_transfer "20% loss — 4 KB"       "$BIN_4K" 30
netem_transfer "20% loss — 1 MB"       "$BIN_1M" 60

clear_netem

# ── Section 2: Packet duplication ─────────────────────────────────────────────
printf '\n--- Packet duplication ---\n'

apply_netem duplicate 10%
netem_transfer "10% duplicate — 1 MB"  "$BIN_1M" 20

apply_netem duplicate 30%
netem_transfer "30% duplicate — 1 MB"  "$BIN_1M" 20

clear_netem

# ── Section 3: Packet reordering ──────────────────────────────────────────────
printf '\n--- Packet reordering ---\n'
# netem reorder requires a base delay to hold packets for reshuffling

apply_netem delay 20ms reorder 25%
netem_transfer "25% reorder (20ms base) — 4 KB"  "$BIN_4K" 20
netem_transfer "25% reorder (20ms base) — 1 MB"  "$BIN_1M" 30

apply_netem delay 10ms reorder 50%
netem_transfer "50% reorder (10ms base) — 1 MB"  "$BIN_1M" 30

clear_netem

# ── Section 4: Delay and jitter ────────────────────────────────────────────────
printf '\n--- Delay and jitter ---\n'

apply_netem delay 30ms
netem_transfer "30ms fixed delay — 1 MB"          "$BIN_1M" 20

apply_netem delay 50ms 20ms distribution normal
netem_transfer "50ms ± 20ms jitter (normal) — 1 MB"  "$BIN_1M" 30

clear_netem

# ── Section 5: Combined impairment ────────────────────────────────────────────
printf '\n--- Combined impairment ---\n'
# loss + reorder + delay — closest to real adverse networks

apply_netem delay 30ms 15ms loss 5% reorder 20%
netem_transfer "5% loss + 20% reorder + 30ms±15ms — 4 KB"  "$BIN_4K" 30
netem_transfer "5% loss + 20% reorder + 30ms±15ms — 1 MB"  "$BIN_1M" 60

apply_netem delay 50ms loss 10% duplicate 5%
netem_transfer "10% loss + 5% dup + 50ms — 1 MB"  "$BIN_1M" 60

clear_netem

# ── Summary ───────────────────────────────────────────────────────────────────
printf '\n\033[1m═══════════════════════════════════════════\033[0m\n'
printf "\033[1mResults:\033[0m  \033[32m%d passed\033[0m  \033[31m%d failed\033[0m  \033[33m%d skipped\033[0m\n" \
    "$PASS" "$FAIL" "$SKIP"
printf '\033[1m═══════════════════════════════════════════\033[0m\n\n'

[[ $FAIL -eq 0 ]]
