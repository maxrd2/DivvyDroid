#!/bin/bash

set -e

_file="$1"
_arch="$2"
[[ -z "$_file" || ! -f "$_file" || -z "$_arch" ]] && echo -e "Usage: nsi-installer.sh <exe> <arch>\n" && exit 1

echo "Searching $_file dependencies on $_arch..." 1>&2

sdir="$(dirname "$0")"

# add adb
wget -c -nc "https://dl.google.com/android/repository/platform-tools-latest-windows.zip"
unzip -qn platform-tools-latest-windows.zip
adb="platform-tools/adb.exe"

deps=(
	"$_file"
	"$adb"
	# add qt plugin dependencies
	"/usr/$_arch/lib/qt/plugins/platforms/qwindows.dll"
	"/usr/$_arch/lib/qt/plugins/audio/qtaudio_windows.dll"
	"/usr/$_arch/lib/qt/plugins/printsupport/windowsprintersupport.dll"
	"/usr/$_arch/lib/qt/plugins/styles/qwindowsvistastyle.dll"
	"/usr/$_arch/lib/qt/plugins/iconengines/qsvgicon.dll"
	"/usr/$_arch/lib/qt/plugins/imageformats/qico.dll"
	"/usr/$_arch/lib/qt/plugins/imageformats/qjpeg.dll"
	"/usr/$_arch/lib/qt/plugins/imageformats/qsvg.dll"
	"/usr/$_arch/lib/qt/plugins/imageformats/qtiff.dll"
)

dlls=(
	"$adb" # adb.exe
	`"$sdir/deps-find.sh" "$_arch" "${deps[@]}"` # all dlls
	# qt plugins are added in installer.nsi into their own directory
)

nsi=$(cat "$sdir/installer.nsi")
for dll in "${dlls[@]}"; do
	nsi=$(echo -n "$nsi" | sed -Ee 's|^(.*)InstallDeps(.*)$|\1File "'"${dll/|/\\|}"'"\2\n\0|g')
done
echo -n "$nsi" | sed '/InstallDeps/d'
