#!/usr/bin/env bash
# Build script para Vercel: instala Emscripten y compila el juego a WebAssembly.
# Salida final en ./dist (configurado como outputDirectory en vercel.json).
set -euo pipefail

EMSDK_VERSION="${EMSDK_VERSION:-3.1.64}"
ROOT_DIR="$(pwd)"
EMSDK_DIR="$ROOT_DIR/.emsdk"
BUILD_DIR="$ROOT_DIR/build-web"
DIST_DIR="$ROOT_DIR/dist"

echo ">>> Cat Game - Vercel Build"
echo ">>> Working dir: $ROOT_DIR"

# 1. Instalar Emscripten (clonar emsdk si no existe en cache)
if [ ! -d "$EMSDK_DIR" ]; then
    echo ">>> Cloning emsdk..."
    git clone --depth 1 https://github.com/emscripten-core/emsdk.git "$EMSDK_DIR"
fi

cd "$EMSDK_DIR"
./emsdk install "$EMSDK_VERSION"
./emsdk activate "$EMSDK_VERSION"
# shellcheck disable=SC1091
source ./emsdk_env.sh
cd "$ROOT_DIR"

echo ">>> emcc version:"
emcc --version | head -1

# 2. Configurar y compilar con CMake + Emscripten
echo ">>> Configuring with emcmake..."
emcmake cmake -S . -B "$BUILD_DIR" \
    -DCMAKE_BUILD_TYPE=Release \
    -DPLATFORM=Web

echo ">>> Building..."
cmake --build "$BUILD_DIR" -j"$(nproc 2>/dev/null || echo 2)"

# 3. Copiar artefactos a dist/
echo ">>> Preparing dist..."
rm -rf "$DIST_DIR"
mkdir -p "$DIST_DIR"
cp "$BUILD_DIR/index.html" "$DIST_DIR/"
cp "$BUILD_DIR/index.js"   "$DIST_DIR/"
cp "$BUILD_DIR/index.wasm" "$DIST_DIR/"
# El .data file es opcional (solo si hay assets empaquetados)
if [ -f "$BUILD_DIR/index.data" ]; then
    cp "$BUILD_DIR/index.data" "$DIST_DIR/"
fi

echo ">>> Build complete. Artifacts in $DIST_DIR:"
ls -lh "$DIST_DIR"
