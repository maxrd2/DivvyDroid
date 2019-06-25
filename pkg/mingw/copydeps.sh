#!/bin/bash

set -e

_file="$1"
_dest="$2"
_arch="$3"
[[ -z "$_file" || ! -f "$_file" || -z "$_dest" ]] && echo -e "Usage: copydeps.sh <exe> <destination> [arch]\n" && exit 1
[[ -z "$_arch" ]] && _arch="i686-w64-mingw32"

processed=()
exclude=(
	advapi32.dll
	bcrypt.dll
	crypt32.dll
	dnsapi.dll
	dwmapi.dll
	gdi32.dll
	iphlpapi.dll
	kernel32.dll
	mpr.dll
	msvcrt.dll
	netapi32.dll
	ole32.dll
	opengl32.dll
	shell32.dll
	user32.dll
	userenv.dll
	uxtheme.dll
	version.dll
	winmm.dll
	ws2_32.dll
)
mkdir -p "$_dest/lib"

copydeps() {
	cp -vf "$1" "$2"
# 	objdump -p $1 | grep 'DLL Name'
	deps=(`objdump -p $1 | grep 'DLL Name' | sed -Ee 's|^.*DLL Name:\s([[:alpha:]].*[[:alpha:]])\s*$|\1|'`)
# 	echo "'${deps[@]}'"
	for dll in "${deps[@]}"; do
		# skip dlls we have already processed
		[[ "${processed[@]}" =~ $(echo "\\b$dll\\b") ]] && continue
		# skip dlls we don't want
		[[ "${exclude[@]}" =~ $(echo "\\b${dll,,}\\b") ]] && continue
		
		# find full dll path
		_dll=$(find /usr/$_arch -iname "$dll" 2>/dev/null)
		processed+=("$dll")
		[[ -z "$_dll" ]] && echo "WARNING: '$dll' was not found." && continue
		copydeps "$_dll" "$_dest/lib"
	done
}

copydeps "$_file" "$_dest"
