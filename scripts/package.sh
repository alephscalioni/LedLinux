#!/usr/bin/env bash
set -euo pipefail
repo_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
version="0.2.0"
package_name="LedLinux-${version}-fedora44-x86_64"
if [[ "$(uname -m)" != x86_64 ]]; then
    echo 'Esta release binária é para x86_64.' >&2
    exit 1
fi
stage_dir="$(mktemp -d)"
trap 'rm -rf -- "$stage_dir"' EXIT
mkdir -p "$repo_dir/dist" "$stage_dir/$package_name"
install -m 755 "$repo_dir/build/led-control" "$stage_dir/$package_name/led-control"
install -m 755 "$repo_dir/scripts/install.sh" "$stage_dir/$package_name/install.sh"
install -m 644 "$repo_dir/docs/LEIA-ME.txt" "$stage_dir/$package_name/LEIA-ME.txt"
install -m 644 "$repo_dir/README.md" "$stage_dir/$package_name/README.md"
mkdir -p "$stage_dir/$package_name/src" "$stage_dir/$package_name/scripts"
install -m 644 "$repo_dir/src/rgb_engine.py" "$stage_dir/$package_name/src/rgb_engine.py"
install -m 644 "$repo_dir/requirements.txt" "$stage_dir/$package_name/requirements.txt"
install -m 755 "$repo_dir/scripts/install-effects.py" "$stage_dir/$package_name/scripts/install-effects.py"
tar -czf "$repo_dir/dist/$package_name.tar.gz" -C "$stage_dir" "$package_name"
(cd "$repo_dir/dist" && sha256sum "$package_name.tar.gz" > SHA256SUMS)
printf 'Pacote criado: %s/dist/%s.tar.gz\n' "$repo_dir" "$package_name"
