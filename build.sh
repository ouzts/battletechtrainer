#!/bin/bash
mkdir build
cd build
emcmake cmake ..
make -j8
cp battletech_trainer.js ..
cp battletech_trainer.wasm ..
