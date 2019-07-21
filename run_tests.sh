#!/bin/bash

OUTPUT_DIR=output
MOUNTPOINT=/tmp/mnt

umount $MOUNTPOINT
mkdir -p $MOUNTPOINT
$OUTPUT_DIR/tmpfs $MOUNTPOINT -o allow_other -s
python3 -m pytest tests/
#umount $MOUNTPOINT
