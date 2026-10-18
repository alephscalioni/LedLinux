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

## ATTACK SHARK M87 de Aleph

Esta versão pode sincronizar também a iluminação estática do M87 pelo adaptador
Python do projeto M87 Linux. Em `~/.config/Aleph/led-control.conf`, configure
`m87Enabled=true`, `m87Driver=/caminho/para/ledlinux.py` e `m87Connection=auto`
(ou `usb` / `wireless`). O instalador `packaging/install-ledlinux.py` do M87
prepara o adaptador na pasta de dados do LedLinux e preserva a cor e o brilho.
Reinicie o LedLinux após instalar. Sem essa configuração, o comportamento anterior é preservado.

O M87 recebe a mesma cor RGB escalada pelo brilho de 0–100%, com brilho interno
máximo. O transporte automático prefere o cabo USB e usa o receptor 2,4 GHz
quando não há interface USB. Uma falha não dispara reenvio por outro transporte.
O processo roda sem bloquear o painel, tem limite de tempo e escreve o resultado
em `~/.local/share/led-control/last-m87-run.log`. Uma falha do M87 aparece
separadamente do resultado dos LEDs do computador. O programa só salva a nova
cor como confirmada quando os dois controles têm sucesso.

A cor estática por 2,4 GHz foi confirmada fisicamente no M87 desta máquina.
A integração automática e os demais níveis de brilho ainda precisam de
confirmação visual; resposta de protocolo não é leitura física dos LEDs.

### Efeitos RGB do teclado

O painel integrado agora oferece **Efeito do M87** com 19 efeitos e desligado.
Em **Opções e dispositivos**, ajuste velocidade (0–5), sentido e multicolorido.
Os efeitos são executados pelo teclado; a cor estática dos demais dispositivos
continua sincronizada. As preferências `m87Mode`, `m87Speed`, `m87Direction` e
`m87Rainbow` são salvas somente quando a operação completa tem sucesso.
Edições durante uma aplicação são enfileiradas com os novos valores, sem misturar
parâmetros da operação em andamento. Os LEDs lateral e seta permanecem físicos.

Requer o adaptador M87 atualizado com `packaging/install-ledlinux.py` no projeto
[attack-shark-m87-linux](https://github.com/alephscalioni/attack-shark-m87-linux)
(repositório privado do proprietário). O configurador M87 permite salvar/exportar
perfis completos com iluminação, conexão, mapa e macros e oferece 12 receitas RGB.
Não há confirmação visual de todos os efeitos em todos os firmwares.

Verificação com backends simulados, sem tocar no hardware:

```sh
cmake --build build
/usr/bin/python3 -m unittest discover -s tests -v
```
