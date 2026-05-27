#!/bin/bash

echo ""
echo "╔══════════════════════════════════════════╗"
echo "║   Configurando VS Code para macOS...     ║"
echo "╚══════════════════════════════════════════╝"
echo ""

# ── 1. Instalar Raylib se não tiver ──────────────────────────────────
if ! command -v brew &>/dev/null; then
    echo "❌ Homebrew não encontrado."
    echo "   Instale com:"
    echo '   /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"'
    exit 1
fi

if ! brew list raylib &>/dev/null; then
    echo "→ Instalando Raylib..."
    brew install raylib
fi

# ── 2. Detectar caminho do Raylib ─────────────────────────────────────
RAYLIB_H=$(brew --prefix raylib)/include
RAYLIB_LIB=$(brew --prefix raylib)/lib
BREW_PREFIX=$(brew --prefix)

echo "✓ Raylib encontrado em: $RAYLIB_H"

# ── 3. Detectar arquitetura ──────────────────────────────────────────
ARCH=$(uname -m)
if [[ "$ARCH" == "arm64" ]]; then
    INTELLISENSE_MODE="macos-clang-arm64"
    echo "✓ Arquitetura: Apple Silicon (arm64)"
else
    INTELLISENSE_MODE="macos-clang-x64"
    echo "✓ Arquitetura: Intel (x86_64)"
fi

# ── 4. Gerar .vscode/c_cpp_properties.json ───────────────────────────
mkdir -p .vscode
cat > .vscode/c_cpp_properties.json << EOF
{
    "configurations": [
        {
            "name": "macOS",
            "includePath": [
                "\${workspaceFolder}/src",
                "$RAYLIB_H",
                "$BREW_PREFIX/include"
            ],
            "defines": ["__APPLE__"],
            "compilerPath": "/usr/bin/cc",
            "cStandard": "c11",
            "intelliSenseMode": "$INTELLISENSE_MODE"
        }
    ],
    "version": 4
}
EOF
echo "✓ .vscode/c_cpp_properties.json gerado"

# ── 5. Gerar .vscode/settings.json ───────────────────────────────────
cat > .vscode/settings.json << EOF
{
    "files.associations": { "*.c": "c", "*.h": "c" },
    "C_Cpp.default.cStandard": "c11",
    "C_Cpp.default.compilerPath": "/usr/bin/cc",
    "C_Cpp.default.includePath": [
        "\${workspaceFolder}/src",
        "$RAYLIB_H",
        "$BREW_PREFIX/include"
    ],
    "C_Cpp.default.intelliSenseMode": "$INTELLISENSE_MODE",
    "editor.tabSize": 4,
    "terminal.integrated.cwd": "\${workspaceFolder}"
}
EOF
echo "✓ .vscode/settings.json gerado"

# ── 6. Compilar o jogo ────────────────────────────────────────────────
echo ""
echo "→ Compilando..."
make clean && make

echo ""
echo "╔══════════════════════════════════════════╗"
echo "║  ✓ Pronto! Reabra o VS Code na pasta.   ║"
echo "║                                          ║"
echo "║  Para jogar:  ./ilha_dos_paradoxos       ║"
echo "╚══════════════════════════════════════════╝"
echo ""
