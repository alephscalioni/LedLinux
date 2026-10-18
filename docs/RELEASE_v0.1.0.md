Primeira prévia do LedLinux para **Fedora 44, KDE Plasma e x86_64**.

- Popover na barra superior com oito cores, seletor personalizado e brilho.
- Restauração da última cor validada e início automático opcional.
- Aplicativo compilado, instalador por usuário e arquivo SHA256SUMS.

**Instalação:** baixe `LedLinux-0.1.0-fedora44-x86_64.tar.gz`, extraia e execute `./install.sh`. O pacote traz `LEIA-ME.txt` com requisitos e preparação do OpenRGB.

**Dependência:** OpenRGB 1.1+ instalado separadamente. Bibliotecas Qt 6 e XWayland precisam estar disponíveis no Fedora. Não é um executável universal para qualquer distribuição Linux.

**Limitação desta prévia:** o código está configurado para seis controladores específicos: dois módulos Corsair Vengeance RGB Pro DDR4, RTX 3060 Gaming OC, G502 Lightspeed, ASUS TUF B550M-PLUS e Nollie8. Outros computadores exigem adaptar a configuração no código. Os canais RGB devem ser configurados no OpenRGB.
