#!/usr/bin/env bash
# Mirrors the upstream SourceForge repository into this GitHub repository and
# reports which release tags showed up, so the caller can build packages.
#
#   UPSTREAM            upstream git URL
#   GH_REMOTE           authenticated push URL for this repository
#   GH_REPO / GH_TOKEN  for the `gh` release lookup
#   CI_BRANCH           branch holding the pipeline; never mirrored or pruned
#   TAG_REGEX           which tags count as releases
#   MAX_NEW_TAG_BUILDS  refuse to auto-build more than this many at once
#
# Writes new_tags (a JSON array) to $GITHUB_OUTPUT.
set -euo pipefail

UPSTREAM="${UPSTREAM:?}"
GH_REMOTE="${GH_REMOTE:?}"
CI_BRANCH="${CI_BRANCH:-ci}"
TAG_REGEX="${TAG_REGEX:-^(tags/)?[Mm]qt_Rel}"
MAX_NEW_TAG_BUILDS="${MAX_NEW_TAG_BUILDS:-3}"

WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT

echo "==> cloning $UPSTREAM"
git clone --quiet --mirror "$UPSTREAM" "$WORK/up"
cd "$WORK/up"
git remote add gh "$GH_REMOTE"

# Every list here is newline-separated and must never contain a blank line:
# a stray empty entry would turn into a `:refs/heads/` push below.
clean() { sed '/^[[:space:]]*$/d' | sort -u; }

up_branches="$(git for-each-ref --format='%(refname:strip=2)' refs/heads | clean)"
up_tags="$(git for-each-ref --format='%(refname:strip=2)' refs/tags | clean)"
echo "==> upstream has $(printf '%s' "$up_branches" | grep -c . || true) branches" \
     "and $(printf '%s' "$up_tags" | grep -c . || true) tags"

# What the mirror looked like before this run, so we can tell what is new.
gh_tags_before="$( { git ls-remote --tags gh || true; } \
    | sed 's|.*refs/tags/||' | grep -v '\^{}$' | clean )"
gh_branches_before="$( { git ls-remote --heads gh || true; } \
    | sed 's|.*refs/heads/||' | clean )"

echo "==> pushing branches and tags"
git push gh "+refs/heads/*:refs/heads/*" "+refs/tags/*:refs/tags/*"

# Branches deleted upstream should disappear here too. --prune cannot be used:
# it would also delete the CI branch, which upstream does not have.
gone="$( comm -23 <(printf '%s\n' "$gh_branches_before") \
                  <(printf '%s\n' "$up_branches") \
         | clean | { grep -vx "$CI_BRANCH" || true; } )"
if [ -n "$gone" ]; then
    echo "==> deleting branches removed upstream:"
    printf '%s\n' "$gone" | sed 's/^/    /'
    # shellcheck disable=SC2046
    git push gh $(printf '%s\n' "$gone" | sed 's|^|:refs/heads/|')
fi

# Tags are never deleted from the mirror: a release published from a tag has to
# keep resolving even if upstream retags or removes one.
new_tags="$( comm -13 <(printf '%s\n' "$gh_tags_before") \
                      <(printf '%s\n' "$up_tags") \
             | clean | { grep -E "$TAG_REGEX" || true; } )"

# Only build what has no release yet, so a re-run, or a sync following a failed
# build, repairs itself instead of duplicating work.
buildable=""
if [ -n "$new_tags" ]; then
    while IFS= read -r t; do
        [ -z "$t" ] && continue
        if gh release view "$t" >/dev/null 2>&1; then
            echo "    skip $t (release already exists)"
        else
            buildable+="$t"$'\n'
        fi
    done <<< "$new_tags"
fi
buildable="$(printf '%s' "$buildable" | clean)"

count="$(printf '%s' "$buildable" | grep -c . || true)"
echo "==> $count new release tag(s)"
if [ -n "$buildable" ]; then
    printf '%s\n' "$buildable" | sed 's/^/    /'
fi

if [ "$count" -gt "$MAX_NEW_TAG_BUILDS" ]; then
    echo "!! $count new tags at once (limit $MAX_NEW_TAG_BUILDS) - building none of them"
    echo "!! start the 'Build Linux packages' workflow by hand for the ones you want"
    buildable=""
fi

if [ -n "$buildable" ]; then
    json="$(printf '%s\n' "$buildable" | grep . | jq -R . | jq -sc .)"
else
    json='[]'
fi
echo "new_tags=$json" >> "${GITHUB_OUTPUT:-/dev/stdout}"
echo "==> new_tags=$json"

{
    echo "### Mirror sync"
    echo
    echo "| | |"
    echo "|---|---|"
    echo "| Upstream branches | $(printf '%s' "$up_branches" | grep -c . || true) |"
    echo "| Upstream tags | $(printf '%s' "$up_tags" | grep -c . || true) |"
    echo "| New release tags | $count |"
    if [ -n "$buildable" ]; then
        echo
        echo "Building:"
        printf '%s\n' "$buildable" | sed 's/^/- `/;s/$/`/'
    fi
} >> "${GITHUB_STEP_SUMMARY:-/dev/null}"
