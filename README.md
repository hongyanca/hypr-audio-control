# Hypr Audio Control

A simple audio control widget for Hyprland using GTK4 and gtk4-layer-shell.

## Features

- Volume control with visual slider
- Display all available audio output devices
- Switch between audio devices with a single click
- Real-time synchronization with system audio state via WirePlumber/PipeWire
- Clean, modern interface that integrates seamlessly with Hyprland

## Requirements

- Python 3.13 or higher
- GTK4
- gtk4-layer-shell
- WirePlumber/PipeWire (wpctl)
- Hyprland or another Wayland compositor with layer-shell support

## Installation

1. Clone the repository:
```bash
git clone <repository-url>
cd hypr-audio-control
```

2. Install dependencies using uv:
```bash
uv sync
```

Alternatively, install system dependencies:
```bash
# Arch Linux
sudo pacman -S python-gobject gtk4 gtk4-layer-shell wireplumber

# Other distributions: install equivalent packages
```

### Arch Linux Package

To build and install as a system package on Arch Linux:

```bash
makepkg -si
```

This will install the application to `/usr/share/hypr-audio-control` and provide the `hypr-audio-control` command.

Or build and install manually:

```bash
makepkg -s
sudo pacman -U ./hypr-audio-control-*.pkg.tar.zst
```

## Usage

Run the application:
```bash
uv run main.py
```

Or if using system Python:
```bash
python main.py
```

The widget will appear in the top-right corner of your screen.

### Controls

#### Mouse Controls
- **Volume Slider**: Drag to adjust the volume of the active audio device
- **Device List**: Click on any device name to set it as the default output device
- **Sound Preferences**: Opens system sound preferences (placeholder)

#### Keyboard Shortcuts
- **q** or **Ctrl+q**: Exit the application
- **1-9**: Set volume to 10%-90% (1=10%, 2=20%, ..., 9=90%)
- **0**: Set volume to 100%
- **-**: Decrease volume by 5%
- **=**: Increase volume by 5%

## Project Structure

```
hypr-audio-control/
├── audiocontrol/
│   ├── __init__.py
│   ├── audio.py      # Audio device management and wpctl integration
│   └── ui.py         # GTK4 user interface
├── main.py           # Application entry point
├── style.css         # UI styling
└── pyproject.toml    # Project configuration
```

## Configuration

The widget anchors to the top-right corner by default. To change the position, modify the `setup_layer_shell` method in `audiocontrol/ui.py`.

## License

This project is licensed under the MIT License. See LICENSE file for details.

## Contributing

Contributions are welcome. Please open an issue or submit a pull request.

## Troubleshooting

### GTK4 Layer Shell Initialization Error

If you encounter an error like:
```
Failed to initialize layer surface, GTK4 Layer Shell may have been linked after libwayland.
```

Please use the provided `run.sh` script to start the application:
```bash
./run.sh
```

This script preloads the `libgtk4-layer-shell.so` library to resolve the linking order issue.
