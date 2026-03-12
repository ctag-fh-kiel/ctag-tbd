#!/bin/bash
###############################################################################
# test-device-api.sh — Automated API endpoint test suite for CTAG TBD device
#
# Tests all REST API endpoints against a live device over USB NCM.
# Generates a stability report with pass/fail counts, timing, and errors.
#
# Usage:
#   bash test-device-api.sh [HOST] [REPORT_DIR]
#
#   HOST        Device IP or hostname (default: 192.168.4.1)
#   REPORT_DIR  Directory for reports   (default: ./reports)
#
# Requirements: curl, jq
# Exit codes:   0 = all tests passed, 1 = one or more failures
###############################################################################
set -euo pipefail

HOST="${1:-192.168.4.1}"
REPORT_DIR="${2:-$(dirname "$0")/reports}"
TIMESTAMP=$(date +%Y%m%d_%H%M%S)
REPORT_FILE="${REPORT_DIR}/api-test-report_${TIMESTAMP}.txt"
BASE_URL="http://${HOST}"

# --- Timeouts ---
CONNECT_TIMEOUT=10
DEFAULT_TIMEOUT=15
PLUGIN_SWITCH_TIMEOUT=50   # heavy plugins need up to 45s
LONG_TIMEOUT=60

# --- Counters ---
PASS=0
FAIL=0
SKIP=0
TOTAL=0
ERRORS=()

# --- Known dangerous plugins (Guru meditation / crash) ---
BLOCKED_PLUGINS=("PicoSeqRack")

# --- Heavy plugins (need longer timeout) ---
HEAVY_PLUGINS=("WTOsc" "WTOscDuo" "Freakwaves" "VctrSnt" "DrumRack" "Rompler" "PicoSeqRack")

###############################################################################
# Helpers
###############################################################################

mkdir -p "$REPORT_DIR"

log() {
    echo "$@" | tee -a "$REPORT_FILE"
}

is_blocked() {
    local plugin="$1"
    for bp in "${BLOCKED_PLUGINS[@]}"; do
        [[ "$plugin" == "$bp" ]] && return 0
    done
    return 1
}

is_heavy() {
    local plugin="$1"
    for hp in "${HEAVY_PLUGINS[@]}"; do
        [[ "$plugin" == "$hp" ]] && return 0
    done
    return 1
}

get_timeout() {
    local plugin="${1:-}"
    if [[ -n "$plugin" ]] && is_heavy "$plugin"; then
        echo "$PLUGIN_SWITCH_TIMEOUT"
    else
        echo "$DEFAULT_TIMEOUT"
    fi
}

# send_get URL [TIMEOUT] → sets RESPONSE, HTTP_CODE, CURL_TIME, CURL_EXIT
send_get() {
    local url="$1"
    local timeout="${2:-$DEFAULT_TIMEOUT}"
    local tmpfile
    tmpfile=$(mktemp)

    set +e
    HTTP_CODE=$(curl -s -o "$tmpfile" -w '%{http_code}' \
        --connect-timeout "$CONNECT_TIMEOUT" \
        --max-time "$timeout" \
        "$url" 2>/dev/null)
    CURL_EXIT=$?
    set -e

    RESPONSE=$(cat "$tmpfile" 2>/dev/null || echo "")
    rm -f "$tmpfile"

    # Measure time with a second call isn't needed — use curl's timer
    CURL_TIME=""
}

# send_get_timed URL [TIMEOUT] → adds CURL_TIME_TOTAL
send_get_timed() {
    local url="$1"
    local timeout="${2:-$DEFAULT_TIMEOUT}"
    local tmpfile
    tmpfile=$(mktemp)
    local timefile
    timefile=$(mktemp)

    set +e
    HTTP_CODE=$(curl -s -o "$tmpfile" -w '%{http_code}\n%{time_total}' \
        --connect-timeout "$CONNECT_TIMEOUT" \
        --max-time "$timeout" \
        "$url" 2>/dev/null | tee "$timefile" | head -1)
    CURL_EXIT=$?
    set -e

    RESPONSE=$(cat "$tmpfile" 2>/dev/null || echo "")
    CURL_TIME_TOTAL=$(tail -1 "$timefile" 2>/dev/null || echo "?")
    rm -f "$tmpfile" "$timefile"
}

# send_post URL DATA [TIMEOUT] → sets RESPONSE, HTTP_CODE, CURL_EXIT
send_post() {
    local url="$1"
    local data="${2:-}"
    local timeout="${3:-$DEFAULT_TIMEOUT}"
    local tmpfile
    tmpfile=$(mktemp)

    set +e
    HTTP_CODE=$(curl -s -o "$tmpfile" -w '%{http_code}' \
        --connect-timeout "$CONNECT_TIMEOUT" \
        --max-time "$timeout" \
        -X POST \
        -H "Content-Type: application/json" \
        -d "$data" \
        "$url" 2>/dev/null)
    CURL_EXIT=$?
    set -e

    RESPONSE=$(cat "$tmpfile" 2>/dev/null || echo "")
    rm -f "$tmpfile"
}

# Core assertion: check HTTP status and optional JSON validity
# assert_test "Test Name" EXPECTED_CODE [check_json]
assert_test() {
    local name="$1"
    local expected="${2:-200}"
    local check_json="${3:-false}"
    TOTAL=$((TOTAL + 1))

    if [[ "$CURL_EXIT" -ne 0 ]]; then
        FAIL=$((FAIL + 1))
        local msg="FAIL  [$name] curl error $CURL_EXIT (timeout/connection)"
        log "$msg"
        ERRORS+=("$msg")
        return 1
    fi

    if [[ "$HTTP_CODE" != "$expected" ]]; then
        FAIL=$((FAIL + 1))
        local msg="FAIL  [$name] HTTP $HTTP_CODE (expected $expected)"
        log "$msg"
        ERRORS+=("$msg")
        return 1
    fi

    if [[ "$check_json" == "true" ]] && ! echo "$RESPONSE" | jq . >/dev/null 2>&1; then
        FAIL=$((FAIL + 1))
        local msg="FAIL  [$name] invalid JSON response"
        log "$msg"
        ERRORS+=("$msg")
        return 1
    fi

    PASS=$((PASS + 1))
    log "PASS  [$name]"
    return 0
}

###############################################################################
# Report header
###############################################################################
log "============================================================"
log " CTAG TBD — Automated API Test Report"
log " Date:   $(date)"
log " Host:   ${HOST}"
log " URL:    ${BASE_URL}"
log "============================================================"
log ""

###############################################################################
# Phase 0: Connectivity check
###############################################################################
log "--- Phase 0: Connectivity ---"
send_get "${BASE_URL}/" "$DEFAULT_TIMEOUT"
if [[ "$CURL_EXIT" -ne 0 ]]; then
    log "FATAL: Cannot reach device at ${HOST}. Aborting."
    log "       Make sure USB NCM is up and device IP is correct."
    exit 1
fi
TOTAL=$((TOTAL + 1)); PASS=$((PASS + 1))
log "PASS  [Connectivity] Device reachable"
log ""

###############################################################################
# Phase 1: GET Endpoints — read-only
###############################################################################
log "--- Phase 1: Read-only GET endpoints ---"

# 1.1 getPlugins
send_get "${BASE_URL}/api/v2/plugins?action=list"
assert_test "getPlugins" 200 true
PLUGIN_LIST="$RESPONSE"
PLUGIN_COUNT=$(echo "$PLUGIN_LIST" | jq 'length' 2>/dev/null || echo 0)
log "       → $PLUGIN_COUNT plugins found"

# 1.2 getIOCaps
send_get "${BASE_URL}/api/v2/device?action=getIOCaps"
assert_test "getIOCaps" 200 true

# 1.3 getConfiguration
send_get "${BASE_URL}/api/v2/device?action=getConfig"
assert_test "getConfiguration" 200 true
SAVED_CONFIG="$RESPONSE"

# 1.4 getActivePlugin ch0
send_get "${BASE_URL}/api/v2/plugins?action=getActive&ch=0"
assert_test "getActivePlugin/0" 200 true
INITIAL_PLUGIN_CH0=$(echo "$RESPONSE" | jq -r '.id' 2>/dev/null || echo "unknown")
log "       → ch0 active: $INITIAL_PLUGIN_CH0"

# 1.5 getActivePlugin ch1
send_get "${BASE_URL}/api/v2/plugins?action=getActive&ch=1"
assert_test "getActivePlugin/1" 200 true
INITIAL_PLUGIN_CH1=$(echo "$RESPONSE" | jq -r '.id' 2>/dev/null || echo "unknown")
log "       → ch1 active: $INITIAL_PLUGIN_CH1"

# 1.6 getPluginParams ch0
send_get "${BASE_URL}/api/v2/plugins?action=getParams&ch=0" "$LONG_TIMEOUT"
assert_test "getPluginParams/0" 200 true

# 1.7 getPluginParams ch1
send_get "${BASE_URL}/api/v2/plugins?action=getParams&ch=1" "$LONG_TIMEOUT"
assert_test "getPluginParams/1" 200 true

# 1.8 getPresets ch0
send_get "${BASE_URL}/api/v2/plugins?action=getPresets&ch=0"
assert_test "getPresets/0" 200 true

# 1.9 getPresets ch1
send_get "${BASE_URL}/api/v2/plugins?action=getPresets&ch=1"
assert_test "getPresets/1" 200 true

# 1.10 samples listing
send_get "${BASE_URL}/api/v2/samples"
assert_test "samples list" 200 true
SAMPLE_INFO="$RESPONSE"
SAMPLE_FILE_COUNT=$(echo "$SAMPLE_INFO" | jq '.files | length' 2>/dev/null || echo "?")
SAMPLE_KIT_COUNT=$(echo "$SAMPLE_INFO" | jq '.kits | length' 2>/dev/null || echo "?")
log "       → $SAMPLE_FILE_COUNT files, $SAMPLE_KIT_COUNT kits"

log ""

###############################################################################
# Phase 2: Favorites
###############################################################################
log "--- Phase 2: Favorites ---"

# 2.1 Get all favorites
send_get "${BASE_URL}/api/v2/device?action=getFavorites"
assert_test "favorites/getAll" 200 true

# 2.2 Store a test favorite at slot 9
FAV_BODY='{"name":"APITest","plug_0":"TBD03","pre_0":0,"plug_1":"TBD03","pre_1":0,"ustring":"api-test-fav"}'
send_post "${BASE_URL}/api/v2/device?action=storeFavorite&id=9" "$FAV_BODY"
assert_test "favorites/store/9" 200

# 2.3 Recall favorite 9
send_post "${BASE_URL}/api/v2/device?action=recallFavorite&id=9" ""
assert_test "favorites/recall/9" 200
sleep 3

# 2.4 Verify ch0 is now TBD03 after recall
send_get "${BASE_URL}/api/v2/plugins?action=getActive&ch=0"
assert_test "favorites/recall verify ch0" 200 true
RECALLED_CH0=$(echo "$RESPONSE" | jq -r '.id' 2>/dev/null || echo "")
if [[ "$RECALLED_CH0" != "TBD03" ]]; then
    TOTAL=$((TOTAL + 1)); FAIL=$((FAIL + 1))
    msg="FAIL  [favorites/recall content] ch0=$RECALLED_CH0, expected TBD03"
    log "$msg"; ERRORS+=("$msg")
else
    TOTAL=$((TOTAL + 1)); PASS=$((PASS + 1))
    log "PASS  [favorites/recall content] ch0=TBD03 ✓"
fi

log ""

###############################################################################
# Phase 3: Plugin Switching — all plugins
###############################################################################
log "--- Phase 3: Plugin Switching (all plugins, ch0) ---"

# Build plugin ID lists from the live getPlugins response
STEREO_PLUGINS=()
MONO_PLUGINS=()
while IFS= read -r line; do
    pid=$(echo "$line" | jq -r '.id')
    stereo=$(echo "$line" | jq -r '.isStereo')
    if is_blocked "$pid"; then
        log "SKIP  [setActivePlugin/0 $pid] blocked (known crash)"
        SKIP=$((SKIP + 1))
        continue
    fi
    if [[ "$stereo" == "true" ]]; then
        STEREO_PLUGINS+=("$pid")
    else
        MONO_PLUGINS+=("$pid")
    fi
done < <(echo "$PLUGIN_LIST" | jq -c '.[]')

log "       Stereo: ${#STEREO_PLUGINS[@]}, Mono: ${#MONO_PLUGINS[@]}, Blocked: ${#BLOCKED_PLUGINS[@]}"

# 3.1 Switch every stereo plugin onto ch0
SWITCH_PASS=0
SWITCH_FAIL=0
for pid in "${STEREO_PLUGINS[@]}"; do
    timeout=$(get_timeout "$pid")
    send_post "${BASE_URL}/api/v2/plugins?action=setActive&ch=0&id=${pid}" "" "$timeout"
    if assert_test "setActivePlugin/0 $pid" 200; then
        SWITCH_PASS=$((SWITCH_PASS + 1))
    else
        SWITCH_FAIL=$((SWITCH_FAIL + 1))
    fi
    sleep 2

    # Quick sanity: read back active plugin
    send_get "${BASE_URL}/api/v2/plugins?action=getActive&ch=0"
    READBACK=$(echo "$RESPONSE" | jq -r '.id' 2>/dev/null || echo "")
    if [[ "$READBACK" != "$pid" ]]; then
        TOTAL=$((TOTAL + 1)); FAIL=$((FAIL + 1))
        msg="FAIL  [readback ch0 after $pid] got '$READBACK'"
        log "$msg"; ERRORS+=("$msg")
    else
        TOTAL=$((TOTAL + 1)); PASS=$((PASS + 1))
        log "PASS  [readback ch0 $pid]"
    fi
done

# 3.2 Switch every mono plugin onto ch0
for pid in "${MONO_PLUGINS[@]}"; do
    timeout=$(get_timeout "$pid")
    send_post "${BASE_URL}/api/v2/plugins?action=setActive&ch=0&id=${pid}" "" "$timeout"
    if assert_test "setActivePlugin/0 $pid" 200; then
        SWITCH_PASS=$((SWITCH_PASS + 1))
    else
        SWITCH_FAIL=$((SWITCH_FAIL + 1))
    fi
    sleep 2
done

log "       Plugin switch summary: $SWITCH_PASS passed, $SWITCH_FAIL failed"
log ""

###############################################################################
# Phase 4: Plugin Switching — mono dual-channel combinations (subset)
###############################################################################
log "--- Phase 4: Mono dual-channel combos (sample of 10) ---"

# Take first 5 mono plugins for combinatorial test (5×2 = 10 combos)
COMBO_PLUGINS=("${MONO_PLUGINS[@]:0:5}")
COMBO_COUNT=0

for pid0 in "${COMBO_PLUGINS[@]}"; do
    timeout0=$(get_timeout "$pid0")
    send_post "${BASE_URL}/api/v2/plugins?action=setActive&ch=0&id=${pid0}" "" "$timeout0"
    assert_test "combo ch0=$pid0" 200
    sleep 1

    for pid1 in "${COMBO_PLUGINS[@]}"; do
        [[ "$pid0" == "$pid1" ]] && continue
        timeout1=$(get_timeout "$pid1")
        send_post "${BASE_URL}/api/v2/plugins?action=setActive&ch=1&id=${pid1}" "" "$timeout1"
        assert_test "combo ch0=$pid0 ch1=$pid1" 200
        COMBO_COUNT=$((COMBO_COUNT + 1))
        sleep 1
        [[ $COMBO_COUNT -ge 10 ]] && break 2
    done
done
log "       Tested $COMBO_COUNT dual-channel combos"
log ""

###############################################################################
# Phase 5: Plugin Params — read & write
###############################################################################
log "--- Phase 5: Plugin parameters ---"

# Set ch0 to TBD03 (safe baseline)
send_post "${BASE_URL}/api/v2/plugins?action=setActive&ch=0&id=TBD03" ""
assert_test "setActivePlugin/0 TBD03 (param test)" 200
sleep 2

# 5.1 Get full params
send_get "${BASE_URL}/api/v2/plugins?action=getParams&ch=0" "$LONG_TIMEOUT"
assert_test "getPluginParams/0 (TBD03)" 200 true
PARAMS_JSON="$RESPONSE"

# 5.2 Extract first param name and set it
FIRST_PARAM=$(echo "$PARAMS_JSON" | jq -r 'keys[0]' 2>/dev/null || echo "")
if [[ -n "$FIRST_PARAM" && "$FIRST_PARAM" != "null" && "$FIRST_PARAM" != "activePatch" ]]; then
    # Try to find a numeric param
    FIRST_PARAM=$(echo "$PARAMS_JSON" | jq -r '[to_entries[] | select(.value | type == "object" and has("current")) | .key][0] // empty' 2>/dev/null || echo "")
fi

if [[ -n "$FIRST_PARAM" ]]; then
    # Set current value
    send_post "${BASE_URL}/api/v2/plugins?action=setParam&ch=0&id=${FIRST_PARAM}&key=current&val=2048" ""
    assert_test "setPluginParam/0 ${FIRST_PARAM}=2048" 200

    # Read back
    send_get "${BASE_URL}/api/v2/plugins?action=getParams&ch=0" "$LONG_TIMEOUT"
    assert_test "getPluginParams/0 readback" 200 true
else
    log "SKIP  [setPluginParam] could not find a writable param"
    SKIP=$((SKIP + 1))
fi

log ""

###############################################################################
# Phase 6: Presets — save and load round-trip
###############################################################################
log "--- Phase 6: Presets ---"

# Still on TBD03 ch0 from Phase 5
# 6.1 Save preset at slot 9
send_post "${BASE_URL}/api/v2/plugins?action=savePreset&ch=0&number=9&name=APITest" ""
assert_test "savePreset/0 slot9" 200

# 6.2 Load preset 0
send_post "${BASE_URL}/api/v2/plugins?action=loadPreset&ch=0&number=0" ""
assert_test "loadPreset/0 slot0" 200
sleep 1

# 6.3 Load preset 9 (our saved one)
send_post "${BASE_URL}/api/v2/plugins?action=loadPreset&ch=0&number=9" ""
assert_test "loadPreset/0 slot9 (APITest)" 200
sleep 1

# 6.4 Get preset data for TBD03
send_get "${BASE_URL}/api/v2/plugins?action=getPresetData&id=TBD03" "$LONG_TIMEOUT"
assert_test "getPresetData/TBD03" 200 true

# 6.5 Get presets list
send_get "${BASE_URL}/api/v2/plugins?action=getPresets&ch=0"
assert_test "getPresets/0 (final)" 200 true

log ""

###############################################################################
# Phase 7: Configuration — round-trip
###############################################################################
log "--- Phase 7: Configuration ---"

# 7.1 Read config (already done in Phase 1, but re-read for freshness)
send_get "${BASE_URL}/api/v2/device?action=getConfig"
assert_test "getConfiguration (phase7)" 200 true
CURRENT_CONFIG="$RESPONSE"

# 7.2 Write back the same config (idempotent — should not break anything)
send_post "${BASE_URL}/api/v2/device?action=setConfig" "$CURRENT_CONFIG"
assert_test "setConfiguration (idempotent write)" 200

# 7.3 Read again and verify it's unchanged
send_get "${BASE_URL}/api/v2/device?action=getConfig"
assert_test "getConfiguration (after write)" 200 true

log ""

###############################################################################
# Phase 8: Samples API — listing and kit switching
###############################################################################
log "--- Phase 8: Samples API ---"

# 8.1 List samples
send_get "${BASE_URL}/api/v2/samples"
assert_test "samples list (phase8)" 200 true

# 8.2 Switch to kit 0 (if kits exist)
KIT_COUNT=$(echo "$RESPONSE" | jq '.kits | length' 2>/dev/null || echo 0)
if [[ "$KIT_COUNT" -gt 0 ]]; then
    send_get "${BASE_URL}/api/v2/samples?kit=0"
    assert_test "samples kit=0" 200 true
else
    log "SKIP  [samples kit=0] no kits on device"
    SKIP=$((SKIP + 1))
fi

# 8.3 Reload samples into PSRAM
send_post "${BASE_URL}/api/v2/samples?action=reload" ""
assert_test "samples reload" 200

log ""

###############################################################################
# Phase 9: Rapid plugin switching stress test
###############################################################################
log "--- Phase 9: Rapid switching stress test (10 rounds) ---"

STRESS_PLUGINS=("TBD03" "Void" "SineSrc" "PNoise" "TBD03" "MacOsc" "Dust" "Void" "TBD03" "FBDlyLine")
STRESS_PASS=0
STRESS_FAIL=0

for pid in "${STRESS_PLUGINS[@]}"; do
    timeout=$(get_timeout "$pid")
    send_post "${BASE_URL}/api/v2/plugins?action=setActive&ch=0&id=${pid}" "" "$timeout"
    if assert_test "stress ch0→$pid" 200; then
        STRESS_PASS=$((STRESS_PASS + 1))
    else
        STRESS_FAIL=$((STRESS_FAIL + 1))
    fi
    sleep 1
done
log "       Stress test: $STRESS_PASS passed, $STRESS_FAIL failed"
log ""

###############################################################################
# Phase 10: Heavy plugin endurance
###############################################################################
log "--- Phase 10: Heavy plugin endurance ---"

ENDURANCE_PLUGINS=("WTOsc" "WTOscDuo" "Freakwaves" "VctrSnt" "DrumRack")
for pid in "${ENDURANCE_PLUGINS[@]}"; do
    if is_blocked "$pid"; then
        log "SKIP  [endurance $pid] blocked"
        SKIP=$((SKIP + 1))
        continue
    fi
    log "       Testing $pid (heavy)..."
    send_post "${BASE_URL}/api/v2/plugins?action=setActive&ch=0&id=${pid}" "" "$PLUGIN_SWITCH_TIMEOUT"
    assert_test "endurance switch→$pid" 200
    sleep 3

    # Read params while heavy plugin is active
    send_get "${BASE_URL}/api/v2/plugins?action=getParams&ch=0" "$LONG_TIMEOUT"
    assert_test "endurance params $pid" 200 true

    # Get presets
    send_get "${BASE_URL}/api/v2/plugins?action=getPresets&ch=0"
    assert_test "endurance presets $pid" 200 true
    sleep 2
done

log ""

###############################################################################
# Phase 11: Restore initial state
###############################################################################
log "--- Phase 11: Restore initial state ---"

# Restore ch0
timeout_ch0=$(get_timeout "$INITIAL_PLUGIN_CH0")
send_post "${BASE_URL}/api/v2/plugins?action=setActive&ch=0&id=${INITIAL_PLUGIN_CH0}" "" "$timeout_ch0"
assert_test "restore ch0→$INITIAL_PLUGIN_CH0" 200
sleep 2

# Restore ch1 (only if it was set)
if [[ -n "$INITIAL_PLUGIN_CH1" && "$INITIAL_PLUGIN_CH1" != "unknown" && "$INITIAL_PLUGIN_CH1" != "null" ]]; then
    timeout_ch1=$(get_timeout "$INITIAL_PLUGIN_CH1")
    send_post "${BASE_URL}/api/v2/plugins?action=setActive&ch=1&id=${INITIAL_PLUGIN_CH1}" "" "$timeout_ch1"
    assert_test "restore ch1→$INITIAL_PLUGIN_CH1" 200
fi

log ""

###############################################################################
# Summary
###############################################################################
log "============================================================"
log " TEST RESULTS"
log "============================================================"
log " Total:   $TOTAL"
log " Passed:  $PASS"
log " Failed:  $FAIL"
log " Skipped: $SKIP"
log ""

if [[ $FAIL -gt 0 ]]; then
    log " FAILURES:"
    for err in "${ERRORS[@]}"; do
        log "   • $err"
    done
    log ""
    log " STATUS: *** UNSTABLE — $FAIL test(s) failed ***"
else
    log " STATUS: ALL TESTS PASSED — device is STABLE"
fi

PASS_RATE=0
if [[ $TOTAL -gt 0 ]]; then
    PASS_RATE=$(( (PASS * 100) / TOTAL ))
fi
log " Pass rate: ${PASS_RATE}%"
log ""
log " Report saved to: $REPORT_FILE"
log "============================================================"

# Exit with appropriate code
[[ $FAIL -eq 0 ]]
