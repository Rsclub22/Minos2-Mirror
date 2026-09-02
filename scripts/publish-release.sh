#!/usr/bin/env bash
# Publishes the built packages as the GitHub release for $TAG.
#
#   usage: publish-release.sh <directory of package files>
#
# Re-runnable, and that is the whole difficulty: a run may carry only some of
# the targets - one target re-run after a failure, or a format added later. So
# the release notes and the checksum file are built from what the release
# actually holds after this run's assets are uploaded, never from this run's
# artifacts alone. Deriving them from the artifacts would shrink the package
# table and drop the other packages' checksums every time a single target is
# re-run.
set -euo pipefail

DIST="${1:?usage: publish-release.sh <dist dir>}"
TAG="${TAG:?}"

mapfile -t assets < <(find "$DIST" -type f \
    \( -name '*.deb' -o -name '*.rpm' -o -name '*.pkg.tar.zst' -o -name '*.AppImage' \) | sort)

if [ "${#assets[@]}" -eq 0 ]; then
    echo "!! no packages were built for $TAG, not publishing a release" >&2
    exit 1
fi
echo "==> this run produced ${#assets[@]} package(s)"
printf '    %s\n' "${assets[@]##*/}"

# Build facts recorded by packaging/build.sh. Every target stashes its own, so
# a run without the reference target still has something to report; prefer the
# reference target when it is there, for a stable set of facts across runs.
BUILDINFO=""
for candidate in "$DIST/buildinfo-deb-ubuntu-24.04-amd64.env" "$DIST"/buildinfo-*.env; do
    if [ -f "$candidate" ]; then BUILDINFO="$candidate"; break; fi
done

VERSION=""
if [ -n "$BUILDINFO" ]; then
    # shellcheck disable=SC1090
    . "$BUILDINFO"
    VERSION="${MINOS_VERSION:-}"
fi

# Beta and release-candidate tags are marked as pre-releases so the "Latest
# release" badge keeps pointing at a stable build.
PRERELEASE=""
case "$TAG" in
    *[Bb]eta*|*RC[0-9]*|*[Rr]c[0-9]*|*_999*|*[Pp]re*) PRERELEASE="--prerelease" ;;
esac

WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT

# Checksums the release already carries, from before this run touched it.
if gh release view "$TAG" >/dev/null 2>&1; then
    gh release download "$TAG" -p SHA256SUMS -O "$WORK/old-sums" 2>/dev/null || : > "$WORK/old-sums"
else
    : > "$WORK/old-sums"
    echo "==> creating release $TAG"
    # shellcheck disable=SC2086
    gh release create "$TAG" --title "$TAG" --notes "Build in progress." \
        --verify-tag $PRERELEASE
fi

( cd "$DIST" && sha256sum "${assets[@]##*/}" ) > "$WORK/new-sums"

echo "==> uploading ${#assets[@]} package(s)"
gh release upload "$TAG" "${assets[@]}" --clobber

# The release's full asset list, now that this run's uploads are in.
gh release view "$TAG" --json assets \
    --jq '.assets[] | select(.name != "SHA256SUMS") | "\(.name)\t\(.size)"' \
    | sort > "$WORK/release-assets"

# One line per asset the release actually holds: this run's checksum when it
# rebuilt that asset, the previous one otherwise. Assets that have since
# disappeared fall out because the release's list drives the loop.
: > "$WORK/SHA256SUMS"
while IFS=$'\t' read -r name _; do
    sum="$(awk -v n="$name" '$2 == n { print $1; exit }' "$WORK/new-sums")"
    [ -z "$sum" ] && sum="$(awk -v n="$name" '$2 == n { print $1; exit }' "$WORK/old-sums")"
    if [ -n "$sum" ]; then
        printf '%s  %s\n' "$sum" "$name" >> "$WORK/SHA256SUMS"
    else
        echo "!! no checksum for $name (uploaded by an older pipeline?)" >&2
    fi
done < "$WORK/release-assets"

echo "==> checksums for $(wc -l < "$WORK/SHA256SUMS") asset(s)"
gh release upload "$TAG" "$WORK/SHA256SUMS" --clobber

NOTES="$WORK/notes.md"
{
    echo "Automated build of the upstream tag \`$TAG\`."
    [ -n "$VERSION" ] && echo "Minos version **$VERSION**."
    echo
    if [ "${BUILD_RESULT:-success}" != "success" ]; then
        echo "> [!WARNING]"
        echo "> Not every target built in the most recent run. Re-run the"
        echo "> *Build Linux packages* workflow for this tag to fill in the rest;"
        echo "> the packages below are what the release currently holds."
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
    while IFS=$'\t' read -r name size; do
        printf '| `%s` | %s |\n' "$name" "$(numfmt --to=iec --suffix=B "$size")"
    done < "$WORK/release-assets"
    printf '| `%s` | %s |\n' "SHA256SUMS" \
        "$(numfmt --to=iec --suffix=B "$(stat -c%s "$WORK/SHA256SUMS")")"
    if [ -n "$BUILDINFO" ]; then
        echo
        echo '<details><summary>Build details</summary>'
        echo
        echo '```'
        cat "$BUILDINFO"
        echo '```'
        echo
        echo '</details>'
    fi
    echo
    echo "Mirrored from <https://sourceforge.net/p/minos/minos/ci/$TAG/>."
} > "$NOTES"

gh release edit "$TAG" --notes-file "$NOTES"
echo "==> done: $(gh release view "$TAG" --json url --jq .url)"
