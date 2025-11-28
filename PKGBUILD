# Maintainer: User <user@localhost>
pkgname=hypr-audio-control
pkgver=0.2.0
pkgrel=3
pkgdesc="Simple Audio Control Widget for Hyprland (C Version)"
arch=('any')
url="https://github.com/hongyanca/hypr-audio-control"
license=('MIT')
depends=('gtk4' 'gtk4-layer-shell' 'wireplumber')
makedepends=('gcc' 'make' 'pkg-config')
source=()
md5sums=()
options=('!debug')

build() {
  cd "$startdir"
  make
}

package() {
  # Install directory
  install -d "$pkgdir/usr/share/hypr-audio-control"

  # Copy application files
  install -m755 "$startdir/hypr-audio-control" "$pkgdir/usr/share/hypr-audio-control/"
  install -d "$pkgdir/usr/share/hypr-audio-control/style"
  install -m644 "$startdir/style/style.css" "$pkgdir/usr/share/hypr-audio-control/style/"

  # Create wrapper script
  install -d "$pkgdir/usr/bin"
  cat > "$pkgdir/usr/bin/hypr-audio-control" <<EOF
#!/bin/bash
cd /usr/share/hypr-audio-control

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

./hypr-audio-control &

PID=\$!
echo "\$PID" > "\$LOCK_FILE"
wait "\$PID"
EOF

  chmod 755 "$pkgdir/usr/bin/hypr-audio-control"

  # Install License and Documentation
  if [ -f "$startdir/LICENSE" ]; then
    install -Dm644 "$startdir/LICENSE" "$pkgdir/usr/share/licenses/$pkgname/LICENSE"
  fi

  if [ -f "$startdir/README.md" ]; then
    install -Dm644 "$startdir/README.md" "$pkgdir/usr/share/doc/$pkgname/README.md"
  fi
}
