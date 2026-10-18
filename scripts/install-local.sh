#!/usr/bin/env bash
set -euo pipefail
project_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
cmake -S "$project_dir" -B "$project_dir/build"
cmake --build "$project_dir/build" --parallel 2
/usr/bin/python3 "$project_dir/scripts/install-effects.py"
mkdir -p "$HOME/.local/bin"
install -m 755 "$project_dir/build/led-control" "$HOME/.local/bin/.led-control-new"
mv -- "$HOME/.local/bin/.led-control-new" "$HOME/.local/bin/led-control"
echo 'Versão atualizada. Reinicie o LedLinux para usar os efeitos globais.'
