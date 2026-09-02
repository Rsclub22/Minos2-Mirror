#!/usr/bin/env bash
# Publishes the built packages as the GitHub release for $TAG.
#
#   usage: publish-release.sh <directory of package files>
#
# Re-runnable: an existing release keeps its identity and only gains or
# replaces assets, so a re-run of one failed target tops the release up
# instead of replacing it.
set -euo pipefail

DIST="${1:?usage: publish-release.sh <dist dir>}"
TAG="${TAG:?}"

shopt -s nullglob
mapfile -t assets < <(find "$DIST" -type f \
    \( -name '*.deb' -o -name '*.rpm' -o -name '*.pkg.tar.zst' -o -name '*.AppImage' \) | sort)

if [ "${#assets[@]}" -eq 0 ]; then
    echo "!! no packages were built for $TAG, not publishing a release" >&2
    exit 1
fi

# Build facts recorded by packaging/build.sh on the reference target.
VERSION=""
if [ -f "$DIST/minos-build.env" ]; then
    # shellcheck disable=SC1091
    . "$DIST/minos-build.env"
    VERSION="${MINOS_VERSION:-}"
fi

( cd "$DIST" && sha256sum "${assets[@]#"$DIST"/}" > SHA256SUMS )
assets+=("$DIST/SHA256SUMS")

# Beta and release-candidate tags are marked as pre-releases so the "Latest
# release" badge keeps pointing at a stable build.
PRERELEASE=""
case "$TAG" in
    *[Bb]eta*|*RC[0-9]*|*[Rr]c[0-9]*|*_999*|*[Pp]re*) PRERELEASE="--prerelease" ;;
esac

NOTES="$(mktemp)"
{
    echo "Automated build of the upstream tag \`$TAG\`."
    [ -n "$VERSION" ] && echo "Minos version **$VERSION**."
    echo
    if [ "${BUILD_RESULT:-success}" != "success" ]; then
        echo "> [!WARNING]"
        echo "> Not every target built. The packages below are the ones that succeeded;"
        echo "> re-run the *Build Linux packages* workflow for this tag to fill in the rest."
        echo
    fi
    echo '## Installing'
    echo
    echo '```sh'
    echo '# Debian / Ubuntu / Raspberry Pi OS'
    echo 'sudo apt install ./minos2_*.deb'
    echo
    echo '# Fedora'
    echo 'sudo dnf install ./minos2-*.rpm'
    echo
    echo '# Arch'
    echo 'sudo pacman -U ./minos2-*.pkg.tar.zst'
    echo
    echo '# AppImage (no installation)'
    echo 'chmod +x minos2-*.AppImage && ./minos2-*.AppImage'
    echo '```'
    echo
    echo 'Start the logger with `minos2`. The first start creates `~/minos2-runtime`;'
    echo 'your configuration lives there and upgrades never overwrite it.'
    echo
    echo '## Packages'
    echo
    echo '| File | Size |'
    echo '|---|---|'
    for a in "${assets[@]}"; do
        printf '| `%s` | %s |\n' "$(basename "$a")" "$(du -h "$a" | cut -f1)"
    done
    if [ -f "$DIST/minos-build.env" ]; then
        echo
        echo '<details><summary>Build details</summary>'
        echo
        echo '```'
        cat "$DIST/minos-build.env"
        echo '```'
        echo
        echo '</details>'
    fi
    echo
    echo "Mirrored from <https://sourceforge.net/p/minos/minos/ci/$TAG/>."
} > "$NOTES"

if gh release view "$TAG" >/dev/null 2>&1; then
    echo "==> updating existing release $TAG"
    gh release edit "$TAG" --notes-file "$NOTES"
else
    echo "==> creating release $TAG"
    # shellcheck disable=SC2086
    gh release create "$TAG" --title "$TAG" --notes-file "$NOTES" --verify-tag $PRERELEASE
fi

echo "==> uploading ${#assets[@]} asset(s)"
gh release upload "$TAG" "${assets[@]}" --clobber

echo "==> done: $(gh release view "$TAG" --json url --jq .url)"
