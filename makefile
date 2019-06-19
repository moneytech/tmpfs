TMPFS_SRC_FILES := src/*.c
OUTPUT_DIR := output
CFLAGS := $(shell pkg-config fuse --cflags --libs) -ggdb -Iinc

MOUNTPOINT := /tmp/mnt

tmpfs: $(TMPFS_SRC_FILES)
	mkdir -p $(OUTPUT_DIR)
	clang $(CFLAGS) $^ -o $(OUTPUT_DIR)/$@

run:
	umount $(MOUNTPOINT) || /bin/true
	$(OUTPUT_DIR)/tmpfs $(MOUNTPOINT) -d -o allow_other
