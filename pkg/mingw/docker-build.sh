#!/bin/bash

set -e

_gitroot="$(cd $(dirname "$0") && echo $PWD)"
while [[ ! -d "$_gitroot/.git" ]]; do _gitroot="$(dirname "$_gitroot")" ; [[ "$_gitroot" == "/" ]] && echo "ERROR: cannot find .git directory" && exit 1 ; done
cd "$_gitroot"

appver="$(git describe --always --abbrev=8 | sed 's/-g/-/;s/-/-git-/;s/^v//g')"
rm -rf build && mkdir build

sudo docker run -v "$PWD":/home/devel -it maxrd2/arch-mingw /bin/bash -c "\
	cd build && \
	i686-w64-mingw32-cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr -DAPP_VERSION=$appver .. && \
	make -j\$(nproc) nsis"
