#!/bin/sh
# Run FreeCAD's command-line interpreter on a script: scripts/freecad.sh cad/build_case.py
#
# Set FREECAD to your FreeCAD AppImage (or a freecadcmd binary), e.g. in ~/.bashrc:
#   export FREECAD="$HOME/Desktop/AppImage/FreeCAD_1.1.4-Linux-x86_64-py311.AppImage"
# Without it, freecadcmd must be on PATH (e.g. `sudo apt install freecad`).
set -e
if [ -z "$FREECAD" ]; then
    exec freecadcmd "$@"
fi
case "$FREECAD" in
    # An AppImage contains several programs; the first argument picks one.
    *.AppImage) exec "$FREECAD" freecadcmd "$@" ;;
    *) exec "$FREECAD" "$@" ;;
esac
