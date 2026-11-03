#!/bin/bash
set -Eeuo pipefail

SOURCE_DIR=/mnt/c/Users/ETHAN/Documents/Citron-OS
WORK_DIR=/root/citron-remaster
BASE_NAME=debian-live-13.7.0-amd64-xfce.iso
BASE_URL=https://cdimage.debian.org/debian-cd/current-live/amd64/iso-hybrid
BASE_ISO=/root/debian-live-base.iso
OUTPUT_DIR=/mnt/c/Users/ETHAN/Documents/Citron-OS
VERSION=1.1
OUTPUT_ISO=$OUTPUT_DIR/Citron-OS-${VERSION}-amd64.iso

if [[ ! -f "$SOURCE_DIR/citron-shell.c" ]]; then
  echo "Citron native shell source not found at $SOURCE_DIR" >&2
  exit 1
fi

mkdir -p "$WORK_DIR"
if [[ ! -s "$BASE_ISO" ]]; then
  wget -O "$BASE_ISO" "$BASE_URL/$BASE_NAME"
fi
wget -qO "$WORK_DIR/SHA256SUMS" "$BASE_URL/SHA256SUMS"
EXPECTED=$(awk -v name="$BASE_NAME" '$2 == name {print $1}' "$WORK_DIR/SHA256SUMS")
ACTUAL=$(sha256sum "$BASE_ISO" | awk '{print $1}')
if [[ -z "$EXPECTED" || "$EXPECTED" != "$ACTUAL" ]]; then
  echo "The downloaded Debian Live image failed its SHA-256 check" >&2
  exit 1
fi
echo "Verified Debian Live SHA-256: $ACTUAL"

rm -rf "$WORK_DIR/tree" "$WORK_DIR/rootfs" "$WORK_DIR/overlay" "$WORK_DIR/citron.squashfs" "$WORK_DIR/plymouth-theme-package"
mkdir -p "$WORK_DIR/tree" "$WORK_DIR/overlay/usr/local/bin" \
  "$WORK_DIR/overlay/etc/xdg/autostart" "$WORK_DIR/overlay/etc/plymouth" \
  "$WORK_DIR/overlay/usr/share/plymouth/themes"
xorriso -osirrox on -indev "$BASE_ISO" -extract / "$WORK_DIR/tree"

if [[ ! -f "$WORK_DIR/tree/live/filesystem.squashfs" ]]; then
  echo "Could not find the live root filesystem in the Debian image" >&2
  exit 1
fi

gcc "$SOURCE_DIR/citron-shell.c" -O2 -Wall -Wextra -o "$WORK_DIR/citron-shell" \
  $(pkg-config --cflags --libs gtk+-3.0) -lm
install -m 0755 "$WORK_DIR/citron-shell" "$WORK_DIR/overlay/usr/local/bin/citron-shell"

(cd "$WORK_DIR" && apt-get download plymouth-themes >/dev/null)
dpkg-deb -x "$WORK_DIR"/plymouth-themes_*.deb "$WORK_DIR/plymouth-theme-package"
cp -a "$WORK_DIR/plymouth-theme-package/usr/share/plymouth/themes/script" \
  "$WORK_DIR/overlay/usr/share/plymouth/themes/citron"
gcc "$SOURCE_DIR/citron-splash.c" -O2 -Wall -Wextra -o "$WORK_DIR/citron-splash" \
  $(pkg-config --cflags --libs cairo) -lm
(cd "$WORK_DIR" && ./citron-splash --mark && ./citron-splash --loading-assets)
install -m 0644 "$SOURCE_DIR/citron-plymouth.script" \
  "$WORK_DIR/overlay/usr/share/plymouth/themes/citron/citron.script"
install -m 0644 "$WORK_DIR"/citron-mark.png "$WORK_DIR"/progress-track.png \
  "$WORK_DIR"/progress-fill.png "$WORK_DIR"/progress-dot.png \
  "$WORK_DIR/overlay/usr/share/plymouth/themes/citron/"
cat > "$WORK_DIR/overlay/usr/share/plymouth/themes/citron/citron.plymouth" <<'PLYMOUTHTHEME'
[Plymouth Theme]
Name=Citron OS
Description=Citron OS loading animation
ModuleName=script

[script]
ImageDir=/usr/share/plymouth/themes/citron
ScriptFile=/usr/share/plymouth/themes/citron/citron.script
PLYMOUTHTHEME
"$WORK_DIR/citron-splash" "$WORK_DIR/tree/boot/grub/splash.png" 800 600
"$WORK_DIR/citron-splash" "$WORK_DIR/tree/isolinux/splash.png" 640 480
cat > "$WORK_DIR/overlay/etc/plymouth/plymouthd.conf" <<'PLYMOUTH'
[Daemon]
Theme=citron
ShowDelay=0
PLYMOUTH

cat > "$WORK_DIR/overlay/etc/os-release" <<OSRELEASE
PRETTY_NAME="Citron OS ${VERSION} (Debian GNU/Linux 13.7)"
NAME="Citron OS"
VERSION_ID="${VERSION}"
ID=citron
ID_LIKE=debian
OSRELEASE

cat > "$WORK_DIR/overlay/usr/local/bin/start-citron-os" <<'START'
#!/bin/sh
xfce4-panel --quit >/dev/null 2>&1 || true
xset s off >/dev/null 2>&1 || true
xset -dpms >/dev/null 2>&1 || true
PROFILE="$HOME/.config/citron-os/profile.ini"
if [ -r "$PROFILE" ]; then
  LANGUAGE_CHOICE=$(sed -n 's/^language=//p' "$PROFILE" | head -n 1)
  KEYBOARD_CHOICE=$(sed -n 's/^keyboard=//p' "$PROFILE" | head -n 1)
  case "$LANGUAGE_CHOICE" in en) export LANG=en_US.UTF-8 LANGUAGE=en ;; fr) export LANG=fr_FR.UTF-8 LANGUAGE=fr ;; esac
  case "$KEYBOARD_CHOICE" in us|fr) setxkbmap "$KEYBOARD_CHOICE" >/dev/null 2>&1 || true ;; esac
fi
sleep 2
while true; do
  /usr/local/bin/citron-shell
  sleep 1
done
START
chmod 0755 "$WORK_DIR/overlay/usr/local/bin/start-citron-os"

cat > "$WORK_DIR/overlay/etc/xdg/autostart/citron-os.desktop" <<'DESKTOP'
[Desktop Entry]
Type=Application
Name=Citron OS
Comment=Bureau Citron natif GTK
Exec=/usr/local/bin/start-citron-os
X-GNOME-Autostart-enabled=true
DESKTOP

# Fetch GNOME applications and their dependency closure against the exact
# package database shipped in the live root filesystem, then layer the .debs.
mkdir -p "$WORK_DIR/package-cache/archives/partial" "$WORK_DIR/overlay"
unsquashfs -cat "$WORK_DIR/tree/live/filesystem.squashfs" var/lib/dpkg/status > "$WORK_DIR/live-dpkg-status"
apt-get -o Dir::State::status="$WORK_DIR/live-dpkg-status" \
  -o Dir::Cache::archives="$WORK_DIR/package-cache/archives" \
  --download-only --no-install-recommends -y install \
  gnome-control-center gnome-terminal gnome-calendar gnome-clocks gnome-firmware
shopt -s nullglob
for deb in "$WORK_DIR/package-cache/archives"/*.deb; do
  dpkg-deb -x "$deb" "$WORK_DIR/overlay"
done
unsquashfs -f -d "$WORK_DIR/overlay" "$WORK_DIR/tree/live/filesystem.squashfs" usr/share/glib-2.0/schemas >/dev/null
if command -v glib-compile-schemas >/dev/null 2>&1; then
  find "$WORK_DIR/overlay/usr/share/glib-2.0/schemas" -type f -name '*.xml' -print0 2>/dev/null \
    | xargs -0 -r -I{} dirname '{}' | sort -u \
    | while IFS= read -r schema_dir; do glib-compile-schemas "$schema_dir"; done
fi

mksquashfs "$WORK_DIR/overlay" "$WORK_DIR/citron.squashfs" -noappend -comp xz -b 1M
cp "$WORK_DIR/citron.squashfs" "$WORK_DIR/tree/live/citron.squashfs"
printf 'filesystem.squashfs\ncitron.squashfs\n' > "$WORK_DIR/tree/live/filesystem.module"
for cfg in "$WORK_DIR/tree"/boot/grub/*.cfg "$WORK_DIR/tree"/isolinux/*.cfg; do
  [[ -f "$cfg" ]] || continue
  sed -i "s/Debian GNU\\/Linux Live/Citron OS ${VERSION}/g; s/Debian GNU\\/Linux live/Citron OS ${VERSION}/g" "$cfg"
  # Ask live-boot to mount a USB partition labelled "persistence" when one exists.
  sed -i 's/boot=live/boot=live persistence/g' "$cfg"
done
sed -i "s/Live Boot Menu with GRUB/Citron OS ${VERSION}/g" "$WORK_DIR/tree/boot/grub/theme.cfg"
sed -i "s/menu title Boot menu/menu title Citron OS ${VERSION}/g" "$WORK_DIR/tree/isolinux/stdmenu.cfg"

rm -f "$OUTPUT_ISO"
xorriso -indev "$BASE_ISO" -outdev "$OUTPUT_ISO" \
  -update_r "$WORK_DIR/tree" / \
  -boot_image any replay \
  -volid CITRON_OS_1_1 \
  -commit

sha256sum "$OUTPUT_ISO" > "$OUTPUT_ISO.sha256"
echo "Created $OUTPUT_ISO"
