# Hypr Audio Control

A simple audio control widget for Hyprland using GTK4 and gtk4-layer-shell.

This project offers two implementations:
1.  **C Version (Recommended)**: A lightweight, native implementation.
2.  **Python Version (Legacy)**: The original Python implementation.

## Features

- Volume control with visual slider
- Display all available audio output devices
- Switch between audio devices with a single click
- Real-time synchronization with system audio state via WirePlumber/PipeWire
- Clean, modern interface that integrates seamlessly with Hyprland
- **Keyboard Shortcuts**:
    - **q** or **Ctrl+q**: Exit the application
    - **1-9**: Set volume to 10%-90% (1=10%, 2=20%, ..., 9=90%)
    - **0**: Set volume to 100%
    - **-**: Decrease volume by 5%
    - **=**: Increase volume by 5%
    - **`** (Grave): Mute (0%)

## C Version (Recommended)

### Requirements

- GCC
- Make
- GTK4 (`libgtk-4-dev`)
- gtk4-layer-shell (`libgtk4-layer-shell-dev`)
- WirePlumber (`wireplumber`)

### Build and Install

1.  Clone the repository:
    ```bash
    git clone <repository-url>
    cd hypr-audio-control
    ```

2.  Build the application:
    ```bash
    make
    ```

3.  Run locally:
    ```bash
    ./hypr-audio-control
    ```

### Arch Linux Package (C)

To build and install the C version as a system package on Arch Linux:

```bash
makepkg -si
```

This uses the `PKGBUILD` in the root directory.

## Python Version (Legacy)

The Python source code is located in the `py-src` directory.

### Requirements

- Python 3.13 or higher
- GTK4
- gtk4-layer-shell
- WirePlumber/PipeWire (wpctl)

### Run Locally

```bash
uv run main.py
```

*Note: You may need to use the `run.sh` script if you encounter layer shell linking errors.*

### Arch Linux Package (Python)

To build the Python version package:

1.  Rename `PKGBUILD_PY` to `PKGBUILD` (backup the C `PKGBUILD` first).
2.  Run `makepkg -si`.

## Project Structure

```
hypr-audio-control/
├── src/              # C source code
│   ├── main.c
│   ├── audio.c
│   └── audio.h
├── py-src/           # Python source code
│   ├── ui.py
│   └── audio.py
├── style/            # Shared CSS styles
│   └── style.css
├── Makefile          # Build script for C version
├── PKGBUILD          # Arch Linux package script for C version
├── PKGBUILD_PY       # Arch Linux package script for Python version
└── main.py           # Entry point for Python version
```

## License

This project is licensed under the MIT License. See LICENSE file for details.
