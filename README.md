# 🏝️ Ilha dos Paradoxos

> *"A lógica é a linguagem da ilha..."*

Jogo indie 2D de mistério investigativo e lógica proposicional — desenvolvido em **C + Raylib**.

---

## 👨‍💻 Desenvolvedores

A Ilha dos Paradoxos foi desenvolvida por:

- Lucas Sereno
- Luiz Guilherme Silvestre
- Christopher Mark
- Lucas Farias
- Rafael Lucas Viana

---


## 🎮 Controles

| Tecla | Ação |
|-------|------|
| `WASD` ou `↑↓←→` | Mover o personagem |
| `E` ou `ESPAÇO` | Interagir / Falar com NPC |
| `TAB` ou `J` | Abrir Caderno de Dedução |
| `ESC` ou `P` | Pausar jogo |
| `ENTER` / `ESPAÇO` | Avançar diálogo |

---

## 🍎 Instalação no macOS (recomendado)

### Opção A — Script automático (mais fácil)

```bash
bash instalar_macos.sh
```

O script instala automaticamente: Xcode Tools → Homebrew → Raylib → compila o jogo.

---

### Opção B — Passo a passo manual

**1. Instalar Xcode Command Line Tools**
```bash
xcode-select --install
```

**2. Instalar Homebrew** (se ainda não tiver)
```bash
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

> Apple Silicon (M1/M2/M3): após instalar, rode também:
> ```bash
> echo 'eval "$(/opt/homebrew/bin/brew shellenv)"' >> ~/.zprofile
> eval "$(/opt/homebrew/bin/brew shellenv)"
> ```

**3. Instalar Raylib**
```bash
brew install raylib
```

**4. Compilar o jogo**
```bash
make
```

**5. Jogar!**
```bash
./ilha_dos_paradoxos
# ou
make run
```

---

## 💻 Configurando no VS Code (macOS)

### Extensões necessárias
Instale no marketplace do VS Code:
- **C/C++** — `ms-vscode.cpptools`

### Passos
1. Abra o VS Code
2. `Arquivo → Abrir Pasta` → selecione a pasta `ilha_dos_paradoxos`
3. Quando aparecer a notificação sobre extensões recomendadas, clique em **Instalar**
4. Selecione o kit de compilação correto:
   - **Apple Silicon (M1/M2/M3):** escolha `macOS (Apple Silicon - Homebrew)`
   - **Intel:** escolha `macOS (Intel - Homebrew)`

### Compilar e rodar pelo VS Code
| Atalho | Ação |
|--------|------|
| `⌘+Shift+B` | Compilar (make) |
| `⌘+Shift+P` → "Run Task" → **Build and Run** | Compilar e jogar |
| `⌘+Shift+P` → "Run Task" → **Instalar Raylib (macOS)** | Instalar dependência |
| `F5` | Debug com LLDB |

---


## 🐧 Instalação no Linux

**1. Instalar dependências**

Debian / Ubuntu / Mint:
```bash
sudo apt update
sudo apt install gcc make libraylib-dev
```

Arch / Manjaro:
```bash
sudo pacman -S gcc make raylib
```

Fedora:
```bash
sudo dnf install gcc make raylib-devel
```

Se o `raylib` não estiver disponível no repositório da sua distro, compile manualmente:
```bash
git clone https://github.com/raysan5/raylib.git
cd raylib/src
make PLATFORM=PLATFORM_DESKTOP
sudo make install
```

**2. Compilar o jogo**
```bash
make
```

**3. Jogar!**
```bash
./ilha_dos_paradoxos
# ou
make run
```

---

## 🪟 Instalação no Windows

### Pré-requisitos

**1. Instalar MinGW-w64** (compilador GCC para Windows)

Baixe em [winlibs.com](https://winlibs.com) → escolha **GCC 13+ / UCRT / Win64 / without LLVM**.  
Extraia em `C:\mingw64` e adicione `C:\mingw64\bin` ao PATH do sistema.

> Para adicionar ao PATH: Pesquise "variáveis de ambiente" → Variáveis do Sistema → `Path` → Editar → Novo → `C:\mingw64\bin`

Verifique com:
```bat
gcc --version
```

**2. Instalar Raylib para Windows**

Baixe `raylib-*_win64_mingw-w64.zip` nas [releases do Raylib](https://github.com/raysan5/raylib/releases).  
Extraia em `C:\raylib` — a estrutura deve ficar assim:
```
C:\raylib\
├── include\
│   └── raylib.h
└── lib\
    └── libraylib.a
```

Se extraiu em outro local, edite a variável `RAYLIB_PATH` no `Makefile`:
```makefile
RAYLIB_PATH = C:/seu/caminho/aqui
```

**3. Instalar o `make` para Windows**

Via `winget` (forma mais fácil):
```bat
winget install GnuWin32.Make
```

Ou baixe em [gnuwin32.sourceforge.net](https://gnuwin32.sourceforge.net/packages/make.htm) e adicione ao PATH.

**4. Compilar o jogo**

Abra o **Prompt de Comando** ou **PowerShell** na pasta do projeto:
```bat
make
```

**5. Jogar!**
```bat
ilha_dos_paradoxos.exe
```

### Problemas comuns no Windows

**`gcc: command not found`**  
MinGW não está no PATH — revise o passo 1.

**`cannot find -lraylib`**  
O `RAYLIB_PATH` no `Makefile` está errado ou `lib\libraylib.a` não existe — revise o passo 2.

**`make: command not found`**  
O `make` não está instalado ou não está no PATH — revise o passo 3.

---

## 📁 Estrutura do Projeto

```
ilha_dos_paradoxos/
├── instalar_macos.sh       ← rode este primeiro!
├── Makefile
├── README.md
├── src/
│   ├── main.c              — loop principal
│   ├── game.h              — structs, constantes, protótipos
│   ├── game.c              — orquestração, save/load, telas
│   ├── menu.c              — menu principal animado
│   ├── map.c               — mapa tile-based com fog of war
│   ├── player.c            — movimento e pixel art do jogador
│   ├── npc.c               — NPCs (Veraz / Falaz / Paradoxal)
│   ├── dialogue.c          — sistema de diálogo + banco narrativo
│   ├── deduction.c         — Caderno de Dedução + puzzles lógicos
│   ├── ui.c                — HUD, notificações, helpers
│   ├── environment.c       — clima, névoa, chuva, dia/noite
│   └── utils.c             — funções auxiliares
└── .vscode/
    ├── tasks.json           — Compilar / Rodar / Instalar Raylib
    ├── launch.json          — Debug com LLDB (macOS)
    ├── c_cpp_properties.json — IntelliSense (Apple Silicon + Intel)
    └── settings.json
```

---

## 🧩 O que está implementado

- ✅ Menu principal com animação de ilha, farol e estrelas
- ✅ Tela de intro narrativa com efeito typewriter
- ✅ Mapa tile-based com fog of war
- ✅ Personagem com pixel art e animação de caminhada
- ✅ 3 NPCs com tipos lógicos: **Veraz**, **Falaz**, **Paradoxal**
- ✅ Sistema de diálogo completo com revelação de pistas
- ✅ **Caderno de Dedução** com proposições e operadores lógicos visuais
- ✅ Porta nas ruínas que abre com proposição confirmada
- ✅ Ciclo dinâmico dia/noite e sistema de clima (névoa, chuva)
- ✅ Sistema de notificações com slide-in
- ✅ **Save/Load** automático em disco (`savegame.dat`)
- ✅ Tela de pausa com opções
- ✅ Tela de créditos com scroll
- ✅ Tela de final com farol ativado e navio no horizonte

---

## 🐛 Problemas comuns no macOS

**"dyld: Library not loaded: libraylib.dylib"**
```bash
brew reinstall raylib
```

**"make: cc: No such file or directory"**
```bash
xcode-select --install
```

**Tela não abre / erro OpenGL no Apple Silicon**
```bash
# Verifique o brew está no PATH:
eval "$(/opt/homebrew/bin/brew shellenv)"
make run
```
