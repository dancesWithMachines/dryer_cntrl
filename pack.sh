#!/bin/bash

# Set paths
UF2_FILE="./build/rp2040.rp2040.rpipico/dryer_cntrl.ino.uf2"
SOURCE_FILE="dryer_cntrl.ino"

# Check if UF2 file exists
if [ ! -f "$UF2_FILE" ]; then
  echo "Error: Binary file not found: $UF2_FILE"
  exit 1
fi

# Extract VERSION and DEBUG values from source file
VERSION=$(grep '#define VERSION' "$SOURCE_FILE" | cut -d'"' -f2)
DEBUG=$(grep '#define DEBUG' "$SOURCE_FILE" | awk '{print $3}')

# Extract base name from source file (without extension)
BASENAME=$(basename "$SOURCE_FILE" .ino)

# Construct archive name
ARCHIVE_NAME="${BASENAME}_${VERSION}"
if [ "$DEBUG" == "true" ]; then
  ARCHIVE_NAME+="_d"
fi
ARCHIVE_NAME+=".zip"

# Collect files to archive
FILES_TO_ZIP=()
for f in ./LICENSE*; do
  [ -e "$f" ] && FILES_TO_ZIP+=("$f")
done
FILES_TO_ZIP+=("$UF2_FILE")
[ -f "./README.md" ] && FILES_TO_ZIP+=("./README.md")

# Create zip archive
zip "$ARCHIVE_NAME" "${FILES_TO_ZIP[@]}"

echo "Created archive: $ARCHIVE_NAME"
