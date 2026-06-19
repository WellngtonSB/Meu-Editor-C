              ## Meu Editor de Texto em C (Kilo Clone) 🚀

Este projeto é um esforço dedicado ao estudo prático de engenharia de software de baixo nível, sistemas operacionais e programação na linguagem C. O objetivo principal é construir um editor de texto totalmente funcional diretamente para o terminal Linux do zero, sem o uso de bibliotecas externas complexas, baseando-se no famoso editor `kilo` criado por Salvatore Sanfilippo (antirez).

---

## 🎯 Objetivos de Estudo e Conceitos Absorvidos

Construir este software permite explorar os bastidores de como o computador gerencia interfaces de texto, buffers de memória e interações de teclado em tempo real. Os principais pilares teóricos e práticos abordados até o momento incluem:

### 1. Manipulação do Terminal (Raw Mode vs. Canonical Mode)
Por padrão, os terminais operam em modo canônico (processam o texto linha por linha apenas quando você aperta Enter). Para criar um editor interativo, configuramos o terminal para o **Modo Bruto (Raw Mode)** manipulando a estrutura `termios` do Linux:
* **Desativação de Flags Locais (`ECHO`, `ICANON`, `ISIG`, `IEXTEN`):** Impede o terminal de imprimir automaticamente o que é digitado, desativa a espera pelo Enter e desliga atalhos padrão do sistema como `Ctrl+C` e `Ctrl+Z`.
* **Configurações de Timeout (`VMIN`, `VTIME`):** Ajuste fino do comportamento da função `read()` para ler bytes imediatamente ou após frações de segundo, garantindo que o programa não fique travado esperando digitação.

### 2. Operações de Bits e Atalhos de Teclado (Bitwise Operations)
* Estudo prático de como a tecla `Ctrl` altera os bits enviados pelo teclado.
* Implementação de **Macros de Pré-processamento** (`#define CTRL_KEY(k)`) usando o operador lógico **AND bit a bit** (`& 0x1f`) para criar máscaras e capturar comandos de controle de forma ultraveloz e legível, como o botão de emergência de saída do editor (`Ctrl+Q`).

### 3. Chamadas de Sistema de Baixo Nível (Low-Level I/O)
* Uso direto das funções nativas do POSIX/Linux como `read()` e `write()` para entrada e saída de dados diretamente nos descritores de arquivos (`STDIN_FILENO` e `STDOUT_FILENO`).
* Tratamento robusto de erros capturando códigos de erro globais via `<errno.h>` e exibindo mensagens limpas com `perror()`.

### 4. Sequências de Escape ANSI (ANSI Escape Sequences)
* Comunicação direta com o monitor de vídeo enviando bytes de comandos físicos como `\x1b[2J` (para limpar a tela inteira instantaneamente) e `\x1b[H` (para mover o cursor de volta à posição inicial: Linha 1, Coluna 1).

### 5. Arquitetura Orientada a Estado e Redimensionamento Dinâmico
* Organização do estado global do software dentro de uma única estrutura unificada (`struct editorConfig`).
* Integração com o subsistema do Linux através da biblioteca `<sys/ioctl.h>` e do comando `TIOCGWINSZ`, permitindo que o programa pergunte dinamicamente ao sistema operacional o tamanho físico da janela do terminal (linhas e colunas), adaptando a interface em tempo real caso o usuário maximize ou redimensione o terminal.

---

## 🏗️ Estrutura de Funções do Projeto (A Linha de Produção)

O código foi cuidadosamente arquitetado dividindo as responsabilidades em operários especializados:

* **Gerenciamento de Ambiente:**
  * `enableRawMode()`: Prepara e configura o terminal do Linux para o Modo Bruto.
  * `disableRawMode()`: Restaura as configurações originais do terminal automaticamente ao fechar o programa usando a diretiva de saída `atexit`.
  * `initEditor()`: Inicializa a estrutura global com as informações do ambiente.

* **Subsistema de Entrada (Input):**
  * `editorReadKey()`: Escuta em baixo nível o recebimento de bytes vindos do teclado.
  * `editorProcessKeypress()`: O Tomador de Decisões. Analisa qual tecla foi pressionada e dispara a ação correspondente no editor (ex: `Ctrl+Q` para fechar).

* **Subsistema de Saída (Output/Renderização):**
  * `getWindowSize()`: Consulta as dimensões físicas atuais do monitor do usuário através do sistema operacional.
  * `editorRefreshScreen()`: Limpa o ecrã e gerencia a ordem correta de desenho da interface.
  * `editorDrawRows()`: O desenhador visual da tela. Responsável por projetar a coluna de tils (`~`) na margem esquerda (como no Vim) indicando linhas de texto vazias, aplicando lógica inteligente para evitar pulos indesejados na última linha da tela.

* **Gerenciamento de Crise:**
  * `die()`: Função centralizadora de falhas graves que limpa o terminal do usuário antes de encerrar o programa de forma segura com `exit(1)`.

---

## 🛠️ Como Compilar e Rodar o Projeto

O projeto utiliza um arquivo automatizado de compilação (**Makefile**) para facilitar o fluxo de desenvolvimento no ambiente Ubuntu/Linux:

1. **Compilar o editor:**
    make
   
2. **Executar o Programa**
   ./kilo
   
3. **Sair do Programa**
   Ctrl + Q
