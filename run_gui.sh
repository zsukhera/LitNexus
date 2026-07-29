#!/usr/bin/env bash
# run_gui.sh  —  Launch LitNexus GUI from WSL or native Linux
set -e
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# ── Virtual-environment auto-detection ───────────────────────────────────────
activate_venv() {
    for venv_dir in "$SCRIPT_DIR/.venv" "$SCRIPT_DIR/venv" "$SCRIPT_DIR/../.venv" "$SCRIPT_DIR/../venv"; do
        if [ -f "$venv_dir/bin/activate" ]; then
            source "$venv_dir/bin/activate"
            echo "[run_gui] Activated virtual environment: $venv_dir"
            return 0
        fi
    done
    echo "[run_gui] No virtual environment found — using system Python."
}
activate_venv
# ── WSL display setup ────────────────────────────────────────────────────────
if grep -qiE "(microsoft|wsl)" /proc/version 2>/dev/null; then
    if [ -z "$DISPLAY" ]; then
        if [ -z "$WAYLAND_DISPLAY" ]; then
            WIN_IP=$(cat /etc/resolv.conf 2>/dev/null | grep nameserver | awk '{print $2}' | head -1)
            if [ -n "$WIN_IP" ]; then
                export DISPLAY="${WIN_IP}:0.0"
                echo "[run_gui] WSL: set DISPLAY=${DISPLAY}"
            else
                export DISPLAY=":0"
                echo "[run_gui] WSL: defaulting to DISPLAY=:0"
            fi
        fi
    fi
fi
# ── Qt platform hint for WSL ─────────────────────────────────────────────────
if [ -z "$QT_QPA_PLATFORM" ] && grep -qiE "(microsoft|wsl)" /proc/version 2>/dev/null; then
    if [ -n "$WAYLAND_DISPLAY" ]; then
        export QT_QPA_PLATFORM=wayland
    else
        export QT_QPA_PLATFORM=xcb
    fi
fi
# ── Locate the directory that contains the gui/ package ──────────────────────
find_gui_parent() {
    if [ -d "$SCRIPT_DIR/python/gui" ] && [ -f "$SCRIPT_DIR/python/gui/app.py" ]; then
        echo "$SCRIPT_DIR/python"; return
    fi
    if [ -d "$SCRIPT_DIR/gui" ] && [ -f "$SCRIPT_DIR/gui/app.py" ]; then
        echo "$SCRIPT_DIR"; return
    fi
    if [ -f "$SCRIPT_DIR/app.py" ]; then
        echo "$SCRIPT_DIR/.."; return
    fi
    echo ""
}
GUI_PARENT="$(find_gui_parent)"
if [ -z "$GUI_PARENT" ]; then
    echo "[run_gui] ERROR: Cannot find the gui/ package."
    exit 1
fi
# ── Launch ───────────────────────────────────────────────────────────────────
echo "[run_gui] Starting LitNexus..."
cd "$GUI_PARENT"
exec python -m gui "$@"
