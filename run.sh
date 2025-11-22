#!/bin/bash
export LD_PRELOAD=/usr/lib/libgtk4-layer-shell.so
if command -v uv &> /dev/null; then
    uv run main.py
else
    python3 main.py
fi
