#!/bin/sh

emmake make -f Makefile.em

emcc --no-entry -s"ALLOW_MEMORY_GROWTH=1" -s"EXPORTED_FUNCTIONS=@export.list" -s"EXPORTED_RUNTIME_METHODS=@runtime.list" libpicasso.a -o picasso.wasm

emmake make -f Makefile.em clean

