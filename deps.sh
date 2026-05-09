#!/usr/bin/env bash
set -euo pipefail

mkdir -p ext
cd ext
if [ ! -d voicevox_core ]; then
    git clone https://github.com/VOICEVOX/voicevox_core.git
fi

cd voicevox_core/crates
cargo build --release --package downloader
echo y | ../target/release/download --only onnxruntime --output ../download/
cargo build --release --package voicevox_core_c_api --features load-onnxruntime

cd ../../..
mkdir -p build
find ./ext -type f -name "libonnxruntime*.dylib" -exec cp {} ./build \;
find ./ext -type f -name "libvoicevox_core.dylib" -exec cp {} ./build \;
find ./ext -type f -name "voicevox_core.h" -exec sed -i '' \
    -e 's|^#define VOICEVOX_LINK_ONNXRUNTIME|// #define VOICEVOX_LINK_ONNXRUNTIME|' \
    -e 's|^//[[:space:]]*#define VOICEVOX_LOAD_ONNXRUNTIME|#define VOICEVOX_LOAD_ONNXRUNTIME|' \
    {} +
