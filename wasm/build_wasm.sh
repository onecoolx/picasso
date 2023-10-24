#!/bin/sh

emmake make -f Makefile.em

emcc --no-entry -sEXPORTED_FUNCTIONS=@export.list libpicasso.a -o picasso.wasm

emmake make -f Makefile.em clean

