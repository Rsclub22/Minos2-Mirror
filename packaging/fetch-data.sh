#!/usr/bin/env bash
# Downloads the callsign and contest data that mqt/Installer/buildInstall.sh
# fetches at install time, into the Configuration directory given as $1.
#
# Failures are reported but not fatal: these are third-party sites and Minos
# refreshes the files itself, so a missing download must not sink a release.
set -u
DEST="${1:?usage: fetch-data.sh <Configuration directory>}"
mkdir -p "$DEST"
rc=0

get() { # get <url> <filename>
    local code
    code=$(curl -sSL --retry 3 --retry-delay 2 --max-time 90 \
                -w '%{http_code}' -o "$DEST/$2.part" "$1" 2>/dev/null) || code="000"
    if [ "$code" = "200" ] && [ -s "$DEST/$2.part" ]; then
        mv "$DEST/$2.part" "$DEST/$2"
        printf '    ok    %s (%s bytes)\n' "$2" "$(stat -c%s "$DEST/$2")"
        return 0
    fi
    rm -f "$DEST/$2.part"
    if [ "$code" = "404" ]; then
        # Not published yet - the RSGB puts next year's calendar up in autumn.
        printf '    none  %s (not published)\n' "$2"
    else
        printf '    FAIL  %s (HTTP %s)\n' "$2" "$code"
        rc=1
    fi
    return 0
}

echo "==> downloading contest and callsign data"
get "https://www.country-files.com/cty/cty.dat" "cty.dat"
get "https://www.rsgbcc.org/cgi-bin/vhfenter.pl?afsdownload=y" "clublist.txt"

# The RSGB publishes one file per calendar year, named by its last two digits.
this_year=$(date -u +%y)
next_year=$(printf '%02d' $(( (10#$this_year + 1) % 100 )))
for y in "$this_year" "$next_year"; do
    get "https://www.rsgbcc.org/vhf/vhfcontests$y.xml"          "vhfcontests$y.xml"
    get "https://www.rsgbcc.org/vhf/hfcontests$y.xml"           "hfcontests$y.xml"
    get "https://microwave.rsgbcc.org/microcontests$y.xml"      "microcontests$y.xml"
    get "https://microwave.rsgbcc.org/bartgcontests$y.xml"      "bartgcontests$y.xml"
done

exit $rc
