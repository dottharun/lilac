#!/usr/bin/env bash

rm -rf emsdk/
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk

./emsdk install latest
./emsdk activate latest
cd ..

source ./emsdk/emsdk_env.sh

# Echo the path to ensure it’s set correctly
echo "Emscripten is set up. Path to emcc: $(which emcc)"
