#!/bin/bash

gh_repo=maxrd2/DivvyDroid

set -e

[[ -z "$1" || ! -f "$1" ]] && echo "ERROR: File '$1' doesn't exist." 1>&2 && exit 1

[[ ! -f "$HOME/.github/token" ]] && echo "ERROR: Cannot find GitHub token." 1>&2 && exit 1
. "$HOME/.github/token"

pathname="$1"
basename="$(basename "$1")"
#curl=(curl -u "maxrd2:${GITHUB_TOKEN}")
curl=(curl -H "Authorization: token ${GITHUB_TOKEN}")

gh_is_tag=
gh_release="$(cd "$(dirname "$0")" && git describe --exact-match 2>/dev/null)" && gh_is_tag=1
[[ -z "$gh_release" ]] && gh_release="$(cd "$(dirname "$0")" && git branch --show-current)"
[[ "$gh_release" = "master" ]] && gh_release="continuous"
[[ -z "$gh_release" ]] && echo "ERROR: unable to figure current branch/tag."  1>&2 && exit 1

url="https://api.github.com/repos/$gh_repo/releases/tags/$gh_release"
echo "Getting release info from '$url'..."
release_info=$("${curl[@]}" -s -XGET "${url}" | jq .)
# echo $release_info | jq -C .

release_tag="$(echo $release_info | jq -r .tag_name)"
[[ "$release_tag" = "null" ]] && echo "ERROR: Release for branch/tag '$gh_release' doesn't exist." 1>&2 && exit 1

echo "Updating release w/ tag '$release_tag' named '$(echo $release_info | jq -r .name)'"
read -r -p "Continue? [Y/n] " r ; [[ "$r" != "" && "${r,,}" != "y" ]] && exit

url="$(echo $release_info | jq -r .assets_url)"
echo "Getting asset list from '$url'..."
assets=$("${curl[@]}" -s -XGET "${url}" | jq .)
asset_url=
declare -i i=0
declare -i len=$(echo $assets | jq -r '. | length')
while [[ $i -lt $len ]]; do
	asset_name="$(echo $assets | jq -r ".[$i].name")"
	echo "asset[$i]: '$asset_name'"
	[[ "$asset_name" = "$basename" ]] && asset_url="$(echo $assets | jq -r ".[$i].url")"
	i=i+1
done

if [[ ! -z "$asset_url" ]]; then
	echo "Deleting existing asset '$basename' from '$asset_url'..."
	#read -r -p "Continue? [Y/n] " r ; [[ "$r" != "" && "${r,,}" != "y" ]] && exit
	"${curl[@]}" -s -XDELETE "$asset_url"
fi

upload_url="$(echo $release_info | jq -r '.upload_url|gsub("{.*}";"")')?name=$basename"
echo "Uploading '$pathname' to '$upload_url'..."
"${curl[@]}" \
	-H "Accept: application/vnd.github.manifold-preview" \
	-H "Content-Type: application/octet-stream" \
	--data-binary @$pathname \
	"$upload_url" | jq -C .
