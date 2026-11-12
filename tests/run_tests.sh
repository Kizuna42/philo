#!/usr/bin/env bash
# Smoke tests for the philo binary.
# Usage: tests/run_tests.sh [path/to/philo]
set -u

PHILO="${1:-./philo/philo}"
PASS=0
FAIL=0

ok()   { echo "PASS: $1"; PASS=$((PASS + 1)); }
ng()   { echo "FAIL: $1"; FAIL=$((FAIL + 1)); }

# Invalid input must be rejected promptly: exit code exactly 1 and an
# "Error" message. Timeouts (124) and signal deaths (>128) are failures.
expect_error() {
	local desc="$1"; shift
	local out rc
	out=$(timeout 5 "$PHILO" "$@" 2>&1)
	rc=$?
	if [ "$rc" -ne 1 ]; then
		ng "$desc (exit code $rc, expected 1)"
	elif ! printf '%s\n' "$out" | grep -q 'Error'; then
		ng "$desc (no error message printed)"
	else
		ok "$desc"
	fi
}

expect_death() {
	local desc="$1" min="$2" max="$3"; shift 3
	local out last t
	out=$(timeout 10 "$PHILO" "$@")
	last=$(printf '%s\n' "$out" | tail -n 1)
	if ! printf '%s\n' "$last" | grep -q ' died$'; then
		ng "$desc (last line is not a death: '$last')"
		return
	fi
	if [ "$(printf '%s\n' "$out" | grep -c ' died$')" -ne 1 ]; then
		ng "$desc (more than one death message)"
		return
	fi
	t=${last%% *}
	if [ "$t" -lt "$min" ] || [ "$t" -gt "$max" ]; then
		ng "$desc (death at ${t}ms, expected ${min}-${max}ms)"
		return
	fi
	ok "$desc"
}

expect_no_death_until_done() {
	local desc="$1" n="$2" meals="$3"; shift 3
	local out rc i count
	out=$(timeout 20 "$PHILO" "$@")
	rc=$?
	if [ "$rc" -ne 0 ]; then
		ng "$desc (exit code $rc)"
		return
	fi
	if printf '%s\n' "$out" | grep -q ' died$'; then
		ng "$desc (a philosopher died)"
		return
	fi
	for i in $(seq 1 "$n"); do
		count=$(printf '%s\n' "$out" | grep -c "^[0-9]* $i is eating$")
		if [ "$count" -lt "$meals" ]; then
			ng "$desc (philosopher $i ate $count < $meals times)"
			return
		fi
	done
	ok "$desc"
}

# The simulation must still be running when the window ends (timeout's
# exit code 124), print no death, and every philosopher must have eaten.
# stdout is line-buffered via stdbuf so that nothing printed before the
# SIGTERM (e.g. a death followed by a hang) is lost in the stdio buffer.
expect_alive_for() {
	local desc="$1" secs="$2" n="$3"; shift 3
	local out rc i
	out=$(timeout "$secs" stdbuf -oL "$PHILO" "$@")
	rc=$?
	if printf '%s\n' "$out" | grep -q ' died$'; then
		ng "$desc (a philosopher died)"
		return
	fi
	if [ "$rc" -ne 124 ]; then
		ng "$desc (exited early with code $rc)"
		return
	fi
	for i in $(seq 1 "$n"); do
		if ! printf '%s\n' "$out" | grep -q "^[0-9]* $i is eating$"; then
			ng "$desc (philosopher $i never ate)"
			return
		fi
	done
	ok "$desc"
}

if [ ! -x "$PHILO" ]; then
	echo "binary not found: $PHILO" >&2
	exit 1
fi

# Argument validation
expect_error "no arguments"
expect_error "too few arguments" 5 800 200
expect_error "too many arguments" 5 800 200 200 7 1
expect_error "zero philosophers" 0 800 200 200
expect_error "negative value" 5 -800 200 200
expect_error "non-numeric value" 5 abc 200 200
expect_error "more than 200 philosophers" 201 800 200 200
expect_error "value overflowing int" 4294967297 800 200 200
expect_error "zero meals" 5 800 200 200 0

# Death scenarios
expect_death "single philosopher dies" 800 810 1 800 200 200
expect_death "4 310 200 100 dies" 310 320 4 310 200 100

# Simulations that must end without death
expect_no_death_until_done "5 800 200 200 7" 5 7 5 800 200 200 7
expect_no_death_until_done "4 410 200 200 10" 4 10 4 410 200 200 10

# Simulations that must not die
expect_alive_for "5 800 200 200 survives" 5 5 5 800 200 200
expect_alive_for "4 410 200 200 survives" 5 4 4 410 200 200

echo "----"
echo "passed: $PASS, failed: $FAIL"
[ "$FAIL" -eq 0 ]
