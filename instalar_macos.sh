#!/bin/bash
set -e

echo ""
echo "╔══════════════════════════════════════════╗"
echo "║     Ilha dos Paradoxos — Setup macOS     ║"
echo "╚══════════════════════════════════════════╝"
echo ""

# ── 1. Xcode Command Line Tools ─────────────────────────────────────
if ! xcode-select -p &>/dev/null; then
    echo "→ Instalando Xcode Command Line Tools..."
    xcode-select --install
    echo "  Aguarde a instalação e rode o script novamente."
    exit 1
else
    echo "✓ Xcode Command Line Tools já instalado"
fi

# ── 2. Homebrew ──────────────────────────────────────────────────────
if ! command -v brew &>/dev/null; then
    echo "→ Instalando Homebrew..."
    /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
    # Apple Silicon: adiciona brew ao PATH da sessão atual
    if [[ -f /opt/homebrew/bin/brew ]]; then
        eval "$(/opt/homebrew/bin/brew shellenv)"
    fi
else
    echo "✓ Homebrew já instalado"
fi

# ── 3. Raylib ────────────────────────────────────────────────────────
if brew list raylib &>/dev/null; then
    echo "✓ Raylib já instalado"
else
    echo "→ Instalando Raylib via Homebrew..."
    brew install raylib
fi

# ── 4. Compilar ──────────────────────────────────────────────────────
echo ""
echo "→ Compilando o jogo..."
make clean
make

echo ""
echo "╔══════════════════════════════════════════╗"
echo "║          Tudo pronto! Para jogar:        ║"
echo "║                                          ║"
echo "║          ./ilha_dos_paradoxos            ║"
echo "║                                          ║"
echo "║    (ou use make run)                     ║"
echo "╚══════════════════════════════════════════╝"
echo ""
