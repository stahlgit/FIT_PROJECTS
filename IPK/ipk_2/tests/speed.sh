#!/usr/bin/env bash
# Throughput / speed tests for ipk-rdt.
#
# Section 1 (clean loopback) runs without root.
# Sections 2–4 (netem impairment) require root — the script auto-escalates.
#
# Run from repository root:  bash tests/speed.sh
# Or via Makefile:            make speed

set -uo pipefail
export LC_ALL=C   # force '.' decimal separator for printf/awk regardless of system locale

# ── Paths ─────────────────────────────────────────────────────────────────────
REPO="$(cd "$(dirname "$0")/.." && pwd)"
EXE="$REPO/ipk-rdt"
TMP="$(mktemp -d /tmp/ipk_speed.XXXXXX)"
PORT=19700      # port range separate from e2e (19100) and netem (19400)
PASS=0
FAIL=0
NETEM_ACTIVE=0

# ── Privilege check ───────────────────────────────────────────────────────────
if [[ $EUID -ne 0 ]]; then
    echo "Impairment sections require root. Re-running with sudo..." >&2
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
next_port() { PORT=$((PORT + 1)); echo $((PORT - 1)); }

wait_port() {
    local p=$1 i
    for i in $(seq 1 30); do
        ss -uln 2>/dev/null | grep -q ":$p " && return 0
        sleep 0.1
    done
    return 1
}

apply_netem() {
    clear_netem
    tc qdisc add dev lo root netem "$@"
    NETEM_ACTIVE=1
}

# time_transfer INPUT_FILE TIMEOUT_S
#   Runs a file→file transfer and prints elapsed milliseconds to stdout.
#   Timing stops when the SERVER exits — this excludes the client's TIME_WAIT
#   period (2 × MAX_RETRANSMIT_MS = 6 s) so the result reflects actual transfer
#   throughput rather than the fixed teardown delay.
#   Checks byte-for-byte integrity via diff. Returns non-zero on any failure.
time_transfer() {
    local input="$1" timeout_s="$2"
    local port; port=$(next_port)
    local output="$TMP/out_$port"

    "$EXE" -s -p "$port" -o "$output" -w "$timeout_s" 2>/dev/null &
    local spid=$!

    if ! wait_port "$port"; then
        kill "$spid" 2>/dev/null; wait "$spid" 2>/dev/null || true
        return 1
    fi

    local t0; t0=$(date +%s%3N)
    "$EXE" -c -a 127.0.0.1 -p "$port" -i "$input" -w "$timeout_s" 2>/dev/null &
    local cpid=$!

    # Wait for server — it exits after the final ACK, before client TIME_WAIT
    if ! wait "$spid" 2>/dev/null; then
        kill "$cpid" 2>/dev/null; wait "$cpid" 2>/dev/null || true
        return 1
    fi
    local t1; t1=$(date +%s%3N)

    if ! wait "$cpid" 2>/dev/null; then
        return 1
    fi

    if ! diff -q "$input" "$output" >/dev/null 2>&1; then
        return 1
    fi

    echo $((t1 - t0))
}

# speed_test NAME INPUT_FILE TIMEOUT_S MIN_MBPS
#   Runs a timed transfer and reports throughput.
#   FAIL if the transfer fails or throughput is below MIN_MBPS.
speed_test() {
    local name="$1" input="$2" timeout_s="$3" min_mbps="$4"
    local size; size=$(wc -c < "$input")
    local size_label; size_label=$(awk "BEGIN { printf \"%.0f MB\", $size/1048576 }")

    local elapsed_ms
    if ! elapsed_ms=$(time_transfer "$input" "$timeout_s"); then
        printf '\033[31mFAIL\033[0m  %-46s  %6s  transfer failed\n' "$name" "$size_label"
        FAIL=$((FAIL + 1))
        return
    fi

    local elapsed_s; elapsed_s=$(awk "BEGIN { printf \"%.2f\", $elapsed_ms/1000 }")
    local mbps;      mbps=$(awk      "BEGIN { printf \"%.2f\", $size/(1048576*($elapsed_ms/1000.0)) }")

    if awk "BEGIN { exit ($mbps >= $min_mbps) ? 0 : 1 }"; then
        printf '\033[32mPASS\033[0m  %-46s  %6s  %6s s  %6s MB/s  (min %.2f)\n' \
            "$name" "$size_label" "$elapsed_s" "$mbps" "$min_mbps"
        PASS=$((PASS + 1))
    else
        printf '\033[31mFAIL\033[0m  %-46s  %6s  %6s s  %6s MB/s  (min %.2f — too slow)\n' \
            "$name" "$size_label" "$elapsed_s" "$mbps" "$min_mbps"
        FAIL=$((FAIL + 1))
    fi
}

# ── Build ─────────────────────────────────────────────────────────────────────
cd "$REPO"
if ! make -s 2>&1; then
    echo "Build failed — cannot run speed tests" >&2
    exit 1
fi

printf '\n\033[1m═══════════════════════════════════════════\033[0m\n'
printf '\033[1m ipk-rdt — throughput / speed tests\033[0m\n'
printf '\033[1m═══════════════════════════════════════════\033[0m\n\n'

# ── Prepare test inputs ───────────────────────────────────────────────────────
BIN_1M="$TMP/1m.bin"
BIN_10M="$TMP/10m.bin"
BIN_50M="$TMP/50m.bin"

printf 'Generating test inputs...\n'
dd if=/dev/urandom bs=1M  count=1  of="$BIN_1M"  status=none
dd if=/dev/urandom bs=1M  count=10 of="$BIN_10M" status=none
dd if=/dev/urandom bs=1M  count=50 of="$BIN_50M" status=none
printf '\n'

# ── Section 1: Clean loopback ─────────────────────────────────────────────────
# Timing excludes TIME_WAIT (6 s fixed overhead). Thresholds are set well above
# stop-and-wait (~1 MB/s on sub-ms loopback) while leaving room for I/O overhead.
printf -- '--- Clean loopback (no impairment) ---\n'
speed_test "loopback  1 MB"  "$BIN_1M"   10   2.0
speed_test "loopback 10 MB"  "$BIN_10M"  30   5.0
speed_test "loopback 50 MB"  "$BIN_50M"  60  10.0

# ── Sections 2–4: Netem impairment ───────────────────────────────────────────
if ! modprobe sch_netem 2>/dev/null && ! tc qdisc add dev lo root netem 2>/dev/null; then
    printf '\ntc-netem not available — skipping impairment sections\n'
    tc qdisc del dev lo root 2>/dev/null || true
else
    tc qdisc del dev lo root 2>/dev/null || true

    # ── Section 2: Fixed delay ────────────────────────────────────────────────
    # Thresholds are set to ~30% of the theoretical window-fill rate
    # (window × segment / RTT), clearly above stop-and-wait throughput.
    #
    # 10 ms one-way → RTT 20 ms:
    #   pipelined: 34 KB / 0.020 s ≈ 1.69 MB/s   stop-and-wait: ~0.056 MB/s
    # 30 ms one-way → RTT 60 ms:
    #   pipelined: 34 KB / 0.060 s ≈ 0.56 MB/s   stop-and-wait: ~0.019 MB/s
    printf '\n--- Fixed delay ---\n'
    apply_netem delay 10ms
    speed_test "10 ms delay  1 MB"  "$BIN_1M"  30  0.50
    apply_netem delay 30ms
    speed_test "30 ms delay  1 MB"  "$BIN_1M"  60  0.15
    clear_netem

    # ── Section 3: Delay + jitter ─────────────────────────────────────────────
    printf '\n--- Delay + jitter ---\n'
    apply_netem delay 30ms 15ms distribution normal
    speed_test "30ms +/-15ms jitter  1 MB"  "$BIN_1M"  60  0.10
    clear_netem

    # ── Section 4: Loss + delay ───────────────────────────────────────────────
    # Retransmit timer starts at 200 ms; ~5% of ~848 segments (1 MB) need a
    # retransmit, adding ~8–10 s of overhead at 30 ms RTT. Threshold of
    # 0.05 MB/s = 1 MB in 20 s is achievable and far above stop-and-wait.
    printf '\n--- Loss + delay ---\n'
    apply_netem delay 30ms loss 5%
    speed_test "5% loss + 30ms delay  1 MB"  "$BIN_1M"  120  0.05
    apply_netem delay 30ms loss 10%
    speed_test "10% loss + 30ms delay  1 MB" "$BIN_1M"  120  0.03
    clear_netem
fi

# ── Summary ───────────────────────────────────────────────────────────────────
printf '\n\033[1m═══════════════════════════════════════════\033[0m\n'
printf "\033[1mResults:\033[0m  \033[32m%d passed\033[0m  \033[31m%d failed\033[0m\n" "$PASS" "$FAIL"
printf '\033[1m═══════════════════════════════════════════\033[0m\n\n'

[[ $FAIL -eq 0 ]]
