# LedLinux

Controle pessoal de iluminação RGB em C++/Qt 6 para Fedora 44 e KDE Plasma. O painel da bandeja aplica cor, brilho e efeitos aos seis controladores do computador e ao teclado ATTACK SHARK M87.

## Efeitos globais

Estático, respiração, espectro, onda, perseguição, transição de cores e desligado. O motor Python mantém uma conexão com o servidor local do OpenRGB e envia cores por LED, inclusive aos 1008 LEDs configurados na Nollie. Os efeitos animados exigem que o LedLinux permaneça aberto.

Os dispositivos esperados são dois Corsair Vengeance RGB Pro DDR4, Gigabyte RTX 3060 Gaming OC, Logitech G502 Lightspeed, ASUS TUF GAMING B550M-PLUS e Nollie 8CH. Esta instalação exige esses seis controladores; ainda não é um configurador universal.

As ondas seguem a ordem lógica dos LEDs de cada dispositivo. Não há mapa de posição física do gabinete nem captura de regiões do monitor. O teclado usa efeitos nativos correspondentes e pode ter fase e aparência diferentes. Em opções, é possível escolher um efeito independente para ele; desligar globalmente apaga também o teclado.

## Water cooler

O painel mostra temperaturas de CPU/GPU, uso de CPU/RAM e estado do serviço da tela Rise Mode Aura Ice, com controles para iniciar e pausar. Dados antigos deixam de aparecer como atuais. Requer a instalação separada do [driver da tela](https://github.com/alephscalioni/rise-mode-aura-ice-linux), privado do proprietário. O serviço atualiza a tela independentemente de o painel estar aberto. Esta integração não controla a bomba ou as ventoinhas.

## Instalação local

Requer C++17, CMake, Qt 6 Widgets/Network, Python 3 com venv/pip e OpenRGB com SDK. Prepare o backend em `~/.local/share/led-control/vendor/squashfs-root/AppRun`, ou use o instalador do pacote com OpenRGB disponível no PATH.

```sh
bash scripts/install-local.sh
~/.local/bin/led-control
```

O instalador prepara o ambiente Python com a dependência fixada em `requirements.txt`. O menu permite início automático. Configurações ficam em `~/.config/Aleph/led-control.conf`; o motor e os logs ficam em `~/.local/share/led-control/`. O servidor SDK é um processo próprio, limitado a localhost e encerrado junto ao motor.

Para o M87, execute `packaging/install-ledlinux.py` no projeto [attack-shark-m87-linux](https://github.com/alephscalioni/attack-shark-m87-linux), privado do proprietário. O adaptador usa USB ou receptor 2,4 GHz. A lateral e a seta continuam com controle manual pelo teclado. Não foram confirmados visualmente todos os efeitos em todos os firmwares.

## Verificação

```sh
cmake -S . -B build
cmake --build build
/usr/bin/python3 -m unittest discover -s tests -v
```

São 11 testes de cores, brilho, animação em dispositivos de um LED, envio a todos os controladores, parâmetros do M87 e integração do executável com motor simulado. Na máquina do proprietário, o teste SDK real observou cores variando nos seis controladores e cores distintas em LEDs consecutivos da Nollie; o M87 respondeu ao comando pelo receptor. Isso verifica o estado do software e as respostas de protocolo, sem leitura independente das luzes físicas.

A tela do water cooler acendeu com confirmação do proprietário; a precisão de cada indicador ainda não foi confirmada visualmente. O posicionamento do painel usa XWayland; outros ambientes e escalas não foram verificados.

O motor usa a [API openrgb-python](https://openrgb-python.readthedocs.io/en/latest/pages/api.html) e o [SDK OpenRGB](https://github.com/CalcProgrammer1/OpenRGB/blob/master/Documentation/OpenRGBSDK.md). OpenRGB, fontes e configurações pessoais não são redistribuídos. A licença de distribuição aguarda escolha do autor.

Pacote local: `bash scripts/package.sh`. Releases antigas não incluem o motor de efeitos globais.
