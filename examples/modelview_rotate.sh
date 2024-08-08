#!/bin/sh

set -e

ANIMFOLDER=modelview_rotate
if [ $# -ne 3 ]; then
  echo "You must pass the obj, tex.ppm, and zofs"
  exit 1
fi

# Build the file first
echo "Building"
CWD=$(pwd)
cd ..
make examples/modelview.exe
cd "$CWD"

# Pi times two is about 6.3 so we rotate up to 6.2
echo "Running"
mkdir -p "$ANIMFOLDER"
frame=0
for x in $(seq 0 0.1 6.2); do
  ff=$(printf "%03d" $frame)
  ./modelview.exe "$1" "$2" 0 0 "$3" "$x"
  mv modelview.ppm "$ANIMFOLDER/$ff.ppm"
  frame=$((frame + 1))
done

echo "Converting animation"

cd "$ANIMFOLDER"
convert -delay 3 -loop 0 *.ppm anim.gif
