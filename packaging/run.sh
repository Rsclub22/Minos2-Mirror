#!/usr/bin/env bash
# One entry point for the workflow: install dependencies, build, package.
#
#   usage: run.sh <deb|rpm|arch|appimage>
#
#   SRC        checkout of the Minos2 sources   [required]
#   OUTDIR     where packages are written       [default ./out]
#   MINOS_REF  tag being built, for metadata    [optional]
set -euo pipefail
HERE="$(dirname "$(readlink -f "$0")")"

FORMAT="${1:?usage: run.sh <deb|rpm|arch|appimage>}"
export SRC="${SRC:?SRC must point at the Minos2 checkout}"
export STAGE="${STAGE:-$PWD/stage}"
export OUTDIR="${OUTDIR:-$PWD/out}"
export MINOS_ENV_FILE="$PWD/minos-build.env"
mkdir -p "$OUTDIR"

"$HERE/deps.sh"
"$HERE/build.sh"

# shellcheck disable=SC1090
set -a; . "$MINOS_ENV_FILE"; set +a

case "$FORMAT" in
    deb)      "$HERE/pkg-deb.sh" ;;
    rpm)      "$HERE/pkg-rpm.sh" ;;
    arch)     "$HERE/pkg-arch.sh" ;;
    appimage) "$HERE/pkg-appimage.sh" ;;
    *) echo "run.sh: unknown format '$FORMAT'" >&2; exit 1 ;;
esac

# Surface the build facts to the workflow.
if [ -n "${GITHUB_ENV:-}" ]; then cat "$MINOS_ENV_FILE" >> "$GITHUB_ENV"; fi

echo "==> packages in $OUTDIR:"
ls -la "$OUTDIR"
