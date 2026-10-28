#!/bin/bash

# Resulting payload layout (read from left to right):
# +------------------+------------------+--------------+---------+
# | Config shim      | UEFI FD          | Config file  | Padding |
# +------------------+------------------+--------------+---------+

set -euo pipefail

if [[ $# -ne 6 ]]; then
  echo "Usage: $0 <firmware> <config-shim> <config> <output> <firmware-size> <payload-size>" >&2
  exit 2
fi

FIRMWARE=$1
CONFIG_SHIM=$2
CONFIG=$3
OUTPUT=$4

if [[ ! $5 =~ ^(0[xX][[:xdigit:]]+|[0-9]+)$ || ! $6 =~ ^(0[xX][[:xdigit:]]+|[0-9]+)$ ]]; then
  echo "Firmware and payload sizes must be decimal or hexadecimal integers" >&2
  exit 2
fi

FIRMWARE_SIZE=$(( $5 ))
PAYLOAD_SIZE=$(( $6 ))

for FILE in "$FIRMWARE" "$CONFIG_SHIM" "$CONFIG"; do
  if [[ ! -f $FILE || ! -r $FILE ]]; then
    echo "Input file is missing or unreadable: $FILE" >&2
    exit 1
  fi
done

ACTUAL_FIRMWARE_SIZE=$(wc -c < "$FIRMWARE")
CONFIG_SHIM_SIZE=$(wc -c < "$CONFIG_SHIM")
CONFIG_SIZE=$(wc -c < "$CONFIG")

if (( ACTUAL_FIRMWARE_SIZE == 0 )); then
  echo "Firmware file must not be empty" >&2
  exit 1
fi
if (( ACTUAL_FIRMWARE_SIZE != FIRMWARE_SIZE )); then
  printf 'Firmware size 0x%x does not match expected size 0x%x\n' \
    "$ACTUAL_FIRMWARE_SIZE" "$FIRMWARE_SIZE" >&2
  exit 1
fi
if (( CONFIG_SHIM_SIZE != 4096 )); then
  echo "Config shim must be 4096 bytes; got $CONFIG_SHIM_SIZE" >&2
  exit 1
fi
if (( CONFIG_SIZE > 65535 )); then
  echo "Config file must not exceed 65535 bytes; got $CONFIG_SIZE" >&2
  exit 1
fi
if ! iconv -f UTF-8 -t UTF-8 "$CONFIG" > /dev/null; then
  echo "Config file must be UTF-8 plain text" >&2
  exit 1
fi
if [[ $(LC_ALL=C tr -d '\000' < "$CONFIG" | wc -c) -ne $CONFIG_SIZE ]]; then
  echo "Config file must not contain NUL bytes" >&2
  exit 1
fi

CONTENT_SIZE=$(( CONFIG_SHIM_SIZE + ACTUAL_FIRMWARE_SIZE + CONFIG_SIZE ))
if (( CONTENT_SIZE > PAYLOAD_SIZE )); then
  printf 'Payload size 0x%x exceeds loader capacity 0x%x\n' \
    "$CONTENT_SIZE" "$PAYLOAD_SIZE" >&2
  exit 1
fi

mkdir -p "$(dirname "$OUTPUT")"
TEMP_OUTPUT="${OUTPUT}.tmp.$$"
trap 'rm -f "$TEMP_OUTPUT"' EXIT

cat "$CONFIG_SHIM" "$FIRMWARE" "$CONFIG" > "$TEMP_OUTPUT"
head -c "$(( PAYLOAD_SIZE - CONTENT_SIZE ))" /dev/zero >> "$TEMP_OUTPUT"
mv "$TEMP_OUTPUT" "$OUTPUT"
trap - EXIT
