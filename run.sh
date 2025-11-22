#!/bin/bash
export LD_PRELOAD=/usr/lib/libgtk4-layer-shell.so
LOCK_FILE="/tmp/hypr-audio-control.lock"

if [ -f "$LOCK_FILE" ]; then
    PID=$(cat "$LOCK_FILE")
    if kill -0 "$PID" 2>/dev/null; then
        echo "Stopping existing instance (PID: $PID)..."
        kill "$PID"
        rm -f "$LOCK_FILE"
        exit 0
    else
        echo "Found stale lock file. Removing..."
        rm -f "$LOCK_FILE"
    fi
fi

# Cleanup function to remove lock file on exit
cleanup() {
    rm -f "$LOCK_FILE"
}
trap cleanup EXIT

if command -v uv &> /dev/null; then
    uv run main.py &
else
    python3 main.py &
fi

PID=$!
echo "$PID" > "$LOCK_FILE"
wait "$PID"
