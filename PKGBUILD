# Maintainer: User <user@localhost>
pkgname=hypr-audio-control
pkgver=0.1.1
pkgrel=1
pkgdesc="Simple Audio Control Widget for Hyprland"
arch=('any')
url="https://github.com/example/hypr-audio-control"
license=('MIT')
depends=('python' 'python-gobject' 'gtk4' 'gtk4-layer-shell' 'wireplumber')
makedepends=()
source=()
md5sums=()

package() {
  # Install directory
  install -d "$pkgdir/usr/share/hypr-audio-control"
  
  # Copy application files from the build directory
  # Using "$startdir" allows building directly from the source tree without a source array
  cp -r "$startdir/audiocontrol" "$startdir/main.py" "$startdir/style.css" "$pkgdir/usr/share/hypr-audio-control/"
  
  # Create wrapper script
  install -d "$pkgdir/usr/bin"
  cat > "$pkgdir/usr/bin/hypr-audio-control" <<EOF
#!/bin/bash
# Navigate to the installation directory so relative paths (like style.css) work
cd /usr/share/hypr-audio-control

# Preload gtk4-layer-shell to fix linking issues
export LD_PRELOAD=/usr/lib/libgtk4-layer-shell.so

LOCK_FILE="/tmp/hypr-audio-control.lock"

if [ -f "\$LOCK_FILE" ]; then
    PID=\$(cat "\$LOCK_FILE")
    if kill -0 "\$PID" 2>/dev/null; then
        echo "Stopping existing instance (PID: \$PID)..."
        kill "\$PID"
        rm -f "\$LOCK_FILE"
        exit 0
    else
        echo "Found stale lock file. Removing..."
        rm -f "\$LOCK_FILE"
    fi
fi

# Cleanup function to remove lock file on exit
cleanup() {
    rm -f "\$LOCK_FILE"
}
trap cleanup EXIT

python3 main.py &

PID=\$!
echo "\$PID" > "\$LOCK_FILE"
wait "\$PID"
EOF
  
  # Make wrapper executable
  chmod 755 "$pkgdir/usr/bin/hypr-audio-control"
  
  # Install License and Documentation
  if [ -f "$startdir/LICENSE" ]; then
    install -Dm644 "$startdir/LICENSE" "$pkgdir/usr/share/licenses/$pkgname/LICENSE"
  fi
  
  if [ -f "$startdir/README.md" ]; then
    install -Dm644 "$startdir/README.md" "$pkgdir/usr/share/doc/$pkgname/README.md"
  fi
}
