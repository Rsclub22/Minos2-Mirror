#!/usr/bin/env bash
# Turns the TARGETS input into a build matrix.
#
# A job-level `if:` cannot see the matrix context, so filtering the targets has
# to happen before the matrix exists - hence this script and the prepare job.
#
#   TARGETS   "all", or a comma-separated list of target names
#
# Writes matrix=<json> to $GITHUB_OUTPUT.
set -euo pipefail
HERE="$(dirname "$(readlink -f "$0")")"
DEF="$HERE/../packaging/targets.json"

TARGETS="${TARGETS:-all}"

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

echo "building $count target(s): $(printf '%s' "$matrix" | jq -r '.include[].name' | tr '\n' ' ')" >&2
echo "matrix=$matrix" >> "${GITHUB_OUTPUT:-/dev/stdout}"
