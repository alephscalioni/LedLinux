# LedLinux

Aplicativo pessoal em C++ e Qt 6 para ajustar a iluminação RGB pela bandeja do KDE Plasma. O clique no ícone abre um painel compacto junto à barra superior.

## Recursos

- Oito cores rápidas, seletor personalizado e brilho.
- Restauração da última cor validada e início automático opcional.
- Validação do perfil final gerado pelo OpenRGB.

## Estado do projeto

Protótipo configurado para um computador específico, em Fedora 44 com KDE Plasma e XWayland. A versão atual contém a lista de dispositivos no código e exige os seis controladores esperados para indicar sucesso. Não é ainda um instalador universal nem uma alternativa completa ao OpenRGB.

Dispositivos testados: dois módulos Corsair Vengeance RGB Pro DDR4, Gigabyte RTX 3060 Gaming OC, Logitech G502 Lightspeed, ASUS TUF GAMING B550M-PLUS e Nollie8. O reconhecimento depende do suporte do OpenRGB, das permissões de hardware e da configuração dos canais. LEDs de status sem RGB não são ajustáveis.

## Compilar

Requer compilador C++17, CMake e Qt 6 Widgets/Network.

```sh
cmake -S . -B build
cmake --build build
```

## Executar

O backend é o [OpenRGB](https://openrgb.org/), obtido separadamente. Esta publicação não inclui binários do OpenRGB, fontes SF Pro, regras de acesso pessoais ou configurações do computador.

A versão atual procura o backend em `~/.local/share/led-control/vendor/squashfs-root/AppRun`. O código deve ser instalado em `~/.local/bin/led-control`; o início automático usa `~/.local/share/led-control/led-control.desktop`. O diretório de dados e o backend precisam ser preparados antes da execução. A release contém um instalador por usuário; veja `docs/LEIA-ME.txt` para os requisitos e a preparação do backend.

```sh
./build/led-control
```

O perfil confirma o estado resultante no software, não uma leitura independente das luzes físicas. O popup usa XWayland para posicionamento; outros ambientes e escalas ainda não foram verificados.

## Licença

A licença para distribuição está aguardando escolha do autor.

## Release

O pacote binário da prévia 0.1.0 é destinado ao Fedora 44 x86_64 com KDE Plasma. O OpenRGB e as bibliotecas Qt são dependências externas. Acesse [Releases](https://github.com/alephscalioni/LedLinux/releases).

Para gerar o pacote após compilar: `bash scripts/package.sh`.
