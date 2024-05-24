#!/usr/bin/env bash

# set -x

toplevel=$(git rev-parse --show-toplevel)

trap cleanup INT EXIT

cleanup() {
    pkill WPEWebDriver
}

run_wpewebdriver() {
    local env=(
        WEBKIT_EXEC_PATH=/app/webkit/WebKitBuild/Release/bin
        WEBKIT_INJECTED_BUNDLE_PATH=/app/webkit/WebKitBuild/Release/lib
        COG_MODULEDIR=/app/webkit/WebKitBuild/Release/Tools/cog-prefix/src/cog-build/platform
    )
    local cmd="WebKitBuild/WPE/Release/bin/WPEWebDriver --host=all --port=8088"
    env="${env[@]}"
    $toplevel/Tools/Scripts/webkit-flatpak --wpe --release -c /bin/bash -c "${env} ${cmd}" &
}

run_mvt_tests() {
    local cmd="$(dirname "$0")/run-mvt-tests --browser /app/webkit/WebKitBuild/WPE/Release/Tools/cog-prefix/src/cog-build/launcher/cog --platform gtk4 127.0.0.1:8088"
    $toplevel/Tools/Scripts/webkit-flatpak --wpe --release -c /bin/bash -c "${cmd}"
}

run_wpewebdriver
sleep 2
run_mvt_tests
