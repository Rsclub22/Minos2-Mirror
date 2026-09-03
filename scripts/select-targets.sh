#!/usr/bin/env bash
# Turns the TARGETS input into a build matrix.
#
# A job-level `if:` cannot see the matrix context, so filtering the targets has
# to happen before the matrix exists - hence this script and the prepare job.
#
#   TARGETS       "all", or a comma-separated list of target names
#   ARM64_RUNNER  runner label for arm64 targets; an x64 label switches them
#                 to QEMU emulation. Default: the native arm64 runner.
#
# Writes matrix=<json> to $GITHUB_OUTPUT.
set -euo pipefail
HERE="$(dirname "$(readlink -f "$0")")"
DEF="$HERE/../packaging/targets.json"

TARGETS="${TARGETS:-all}"

# Native arm64 runners are free for public repositories and on the larger plans;
# elsewhere the arm64 build has to be emulated, which turns ~20 minutes into
# about two hours. Set the ARM64_RUNNER repository variable to an x64 label
# (ubuntu-latest) if the native ones are not available here.
ARM64_RUNNER="${ARM64_RUNNER:-ubuntu-24.04-arm}"

if [ -z "$TARGETS" ] || [ "$TARGETS" = "all" ]; then
    matrix="$(jq -c '{include: .}' "$DEF")"
else
    wanted="$(printf '%s' "$TARGETS" | tr ',' '\n' \
        | sed 's/^[[:space:]]*//; s/[[:space:]]*$//' | grep . | jq -R . | jq -sc .)"
    matrix="$(jq -c --argjson w "$wanted" \
        '{include: [.[] | select(.name as $n | $w | index($n))]}' "$DEF")"

    unknown="$(jq -r --argjson w "$wanted" \
        '[.[].name] as $all | $w | map(select(. as $n | $all | index($n) | not)) | .[]' "$DEF")"
    if [ -n "$unknown" ]; then
        echo "!! unknown target(s): $(echo "$unknown" | tr '\n' ' ')" >&2
        echo "!! known targets: $(jq -r '.[].name' "$DEF" | tr '\n' ' ')" >&2
        exit 1
    fi
fi

count="$(printf '%s' "$matrix" | jq '.include | length')"
if [ "$count" -eq 0 ]; then
    echo "!! no target selected by TARGETS='$TARGETS'" >&2
    exit 1
fi

# Fill in the runner for each target, and mark the arm64 ones that will have to
# go through QEMU because they landed on an x64 runner.
matrix="$(printf '%s' "$matrix" | jq -c --arg armrunner "$ARM64_RUNNER" '
    .include |= map(
        if .arch == "arm64" then
            .runner = $armrunner
            | .qemu = (if ($armrunner | test("arm")) then false else true end)
        else
            .runner = "ubuntu-latest" | .qemu = false
        end)')"

printf 'building %s target(s):\n' "$count" >&2
printf '%s' "$matrix" | jq -r \
    '.include[] | "  \(.name)  on \(.runner)\(if .qemu then "  (emulated)" else "" end)"' >&2
echo "matrix=$matrix" >> "${GITHUB_OUTPUT:-/dev/stdout}"
