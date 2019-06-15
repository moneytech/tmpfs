MYFS_SRC_FILES := src/myfs.c
TMPFS_SRC_FILES := src/tmpfs.c
OUTPUT_DIR := output
CFLAGS := -D_FILE_OFFSET_BITS=64 -l fuse

MOUNTPOINT := /tmp/mnt

myfs: $(MYFS_SRC_FILES)
	clang $(CFLAGS) $^ -o $(OUTPUT_DIR)/$@

tmpfs: $(TMPFS_SRC_FILES)
	clang $(CFLAGS) $^ -o $(OUTPUT_DIR)/$@

run:
	umount $(MOUNTPOINT)
	$(OUTPUT_DIR)/tmpfs $(MOUNTPOINT) -o allow_other
