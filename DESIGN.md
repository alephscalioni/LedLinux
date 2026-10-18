---
name: Cores dos LEDs
description: Popup Qt local para iluminação e estado da telinha Rise Mode.
colors:
  surface: "#222732"
  border: "#50596b"
  text: "#f3f5fa"
  control: "#363e4e"
  control-hover: "#46536a"
  focus: "#8bbcff"
  checkbox-text: "#d6dce7"
  swatch-outline: "#f0f0f0"
typography:
  body:
    fontFamily: "SF Pro Text"
    fontSize: "13px"
    fontWeight: 400
    letterSpacing: "0.15px"
  title:
    fontFamily: "SF Pro Text"
    fontSize: "16px"
    fontWeight: 600
    letterSpacing: "0.1px"
rounded:
  popup: "16px"
  button: "8px"
spacing:
  outer: "20px"
  group: "12px"
  presets: "8px"
  advanced: "10px"
components:
  button:
    backgroundColor: "{colors.control}"
    textColor: "{colors.text}"
    rounded: "{rounded.button}"
    padding: "8px 6px"
  button-hover:
    backgroundColor: "{colors.control-hover}"
---

# Design System: Cores dos LEDs

## Overview

**Creative North Star: "Controle de bandeja compacto"**

Popup Qt escuro de operação local, ancorado à bandeja do KDE. A composição preserva cores e brilho no início, efeito global e estado da telinha Rise Mode no corpo, opções avançadas recolhíveis e ações no rodapé. Este documento registra a interface existente em src/main.cpp e .impeccable/review/global-and-display.png, sem propor redesign.

**Key Characteristics:**

- Superfície escura compacta e cantos suaves.
- Amostras de cor acompanhadas de valor hexadecimal e brilho.
- Estado RGB e telemetria Rise Mode visíveis.
- Opções avançadas com rolagem e controles Qt.

## Colors

A superfície azul-escura, os botões ligeiramente mais claros e o texto claro pertencem ao stylesheet existente. O contorno delimita o popup; o azul de foco identifica navegação por teclado. Checkboxes têm texto secundário próprio. As amostras circulares usam contorno claro e representam cores operacionais escolhidas pelo usuário, não uma paleta de marca. As oito receitas são azul, ciano, verde, amarelo, vermelho, roxo, branco e apagar. Campos de seleção, sliders, spinboxes, toolbuttons e barras de rolagem preservam os tratamentos Qt não substituídos pelo stylesheet; a captura mostra a convivência de campos claros com o popup escuro.

## Typography

O aplicativo solicita SF Pro Text nas medidas dos tokens; se indisponível, Qt resolve a substituição de fonte. O título usa peso DemiBold, e o título “Water cooler · Aura Ice” usa DemiBold no tamanho do corpo. Rótulos e estados são em português. Mensagens longas e telemetria quebram linha; status e métricas permitem seleção de texto. Não há tipografia web ou escala de títulos adicional.

## Layout

A fonte atual fixa a largura do popup em 390 unidades lógicas Qt; referências anteriores em torno de 380 não definem outro breakpoint. O corpo usa o espaçamento externo e o ritmo de grupos dos tokens, e a grade de receitas tem duas linhas de quatro colunas. A amostra com hexadecimal e porcentagem tem altura mínima de 38; botões de receitas, 44, com ícones de 26. O rodapé posiciona “Reaplicar” à esquerda e “Pronto” à direita.

O corpo inteiro está em QScrollArea sem moldura. O avançado tem outra área rolável, com altura mínima de 190 e máxima de 240. Ao abrir ou expandir opções, a altura respeita a área útil da tela menos 16 e o posicionamento mantém folga de 8 das bordas; a âncora vem da bandeja ou do cursor. Preserve acesso por rolagem em telas baixas, inclusive às opções inferiores e ao rodapé. Não há adaptação web por breakpoints.

## Elevation & Depth

Não existem sombras personalizadas. A separação vem do contorno fino do popup, contraste tonal dos botões, espaços e bordas nativas dos controles. A janela é Qt::Popup sem moldura de janela convencional.

## Shapes

O popup e os botões usam os raios dos tokens. As amostras são círculos desenhados por QPainter em ícone transparente de 32 × 32, com contorno de 2; não são indicadores de estado do hardware. Combos, sliders e demais controles conservam sua geometria Qt.

## Components

### Cores e brilho

A amostra superior mostra hexadecimal e porcentagem e abre o seletor de cor Qt; as oito receitas também têm tooltip e nome acessível. “Escolher outra cor…” oferece a mesma escolha. O slider de brilho vai de 0 a 100, atualiza a amostra e agrupa alterações com atraso de 500 ms; soltar o slider também aplica. Escolhas de cor iniciam aplicação. Botões usam hover tonal e borda de foco de 1 conforme os tokens; não há animação decorativa.

### Efeito global e teclado

“Todos os LEDs” seleciona Estático, Respiração, Ciclo de cores RGB, Onda RGB, Rastro, Transição de cores ou Desligado. Velocidade global fica no avançado. O teclado acompanha a escolha global, salvo quando “Efeito próprio no teclado” está marcado e a integração M87 está habilitada. Nesse caso, o editor oferece modo, velocidade, sentido e multicolorido; controles incompatíveis ficam desabilitados. Lateral e setinha continuam pelo controle físico, conforme o aviso visível.

### Estado e recuperação

O status informa inicialização, aplicação, quantidade de controladores atendidos ou falha e orienta conferir as luzes. O menu e tooltip da bandeja reproduzem esse estado. “Reaplicar” recupera ou repete a operação; “Pronto” oculta o popup. Configurações RGB são persistidas após o evento de aplicação correspondente. A amostra mostra a escolha pendente e pode diferir do último envio concluído; não deve ser apresentada como leitura dos LEDs. Funcionalidade e testes permanecem descritos no README.

### Telinha Rise Mode

“Water cooler · Aura Ice” precede temperaturas de CPU/GPU e uso de CPU/RAM. Consulta a cada 2 segundos; dados com mais de 8 segundos, serviço ausente ou parado têm mensagens próprias, em lugar de métricas apresentadas como atuais. O avançado mostra estado de envio e “Iniciar telinha” ou “Parar telinha”, desabilitado quando o serviço não está disponível ou há comando em andamento. A captura é referência visual de telemetria, não prova de funcionamento físico de todos os dispositivos.

### Opções e dispositivos

QToolButton recolhível usa seta lateral/para baixo. A área avançada reúne controladores, velocidade global, estado e controle da telinha, opções específicas do M87 e início automático. Preserve a rolagem e os estados Qt de foco, seleção e desabilitado.

## Do's and Don'ts

### Do:

- Do preservar a hierarquia de cor, brilho, efeito global, telemetria e opções avançadas.
- Do manter nomes acessíveis e tooltips nas amostras sem texto.
- Do distinguir envio de efeito, falha de comunicação e conferência física das luzes.
- Do manter rolagem e limites de altura para alcançar todos os controles.

### Don't:

- Don’t transformar o popup em dashboard ou importar o tema do painel M87.
- Don’t interpretar a amostra de cor como leitura física dos LEDs.
- Don’t apresentar dados antigos da telinha como medições atuais.
- Don’t tratar o evento de aplicação como validação visual de todos os efeitos.
