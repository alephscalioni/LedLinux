#!/usr/bin/env bash
set -euo pipefail
bundle_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
install_root="${DESTDIR:-$HOME}"
bin_dir="$install_root/.local/bin"
data_dir="$install_root/.local/share/led-control"
if [[ ! -x "$bundle_dir/led-control" ]]; then
    echo 'Execute install.sh dentro do pacote extraído da release.' >&2
    exit 1
fi
mkdir -p "$bin_dir" "$data_dir" "$install_root/.local/share/applications"
install -m 755 "$bundle_dir/led-control" "$bin_dir/led-control"
python3 - "$HOME" "$data_dir/led-control.desktop" <<'PY'
from pathlib import Path
import sys
home=Path(sys.argv[1])
entry='\n'.join(['[Desktop Entry]','Type=Application','Name=LedLinux — Cores dos LEDs','Comment=Ajustar a iluminação RGB pela barra superior','Exec="'+str(home/'.local/bin/led-control')+'"','Icon=preferences-desktop-color','Terminal=false','Categories=Settings;HardwareSettings;','StartupNotify=false',''])
Path(sys.argv[2]).write_text(entry)
PY
install -m 644 "$data_dir/led-control.desktop" "$install_root/.local/share/applications/led-control.desktop"
if [[ -x "$data_dir/vendor/squashfs-root/AppRun" ]]; then
    echo 'Backend OpenRGB existente preservado.'
elif command -v openrgb >/dev/null 2>&1; then
    mkdir -p "$data_dir/vendor/squashfs-root"
    ln -s "$(command -v openrgb)" "$data_dir/vendor/squashfs-root/AppRun"
else
    echo 'Instale o OpenRGB 1.1+ antes de usar; consulte LEIA-ME.txt.'
fi
printf 'Aplicativo instalado em %s\n' "$bin_dir/led-control"
echo 'A instalação não inicia o app nem altera o início automático ou os LEDs.'
