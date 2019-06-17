MYFS_SRC_FILES := src/myfs.c
TMPFS_SRC_FILES := src/tmpfs.c
OUTPUT_DIR := output
CFLAGS := -D_FILE_OFFSET_BITS=64 -l fuse -ggdb

MOUNTPOINT := /tmp/mnt

tmpfs: $(TMPFS_SRC_FILES)
	clang $(CFLAGS) $^ -o $(OUTPUT_DIR)/$@

myfs: $(MYFS_SRC_FILES)
	clang $(CFLAGS) $^ -o $(OUTPUT_DIR)/$@

run:
	umount $(MOUNTPOINT) || /bin/true
	$(OUTPUT_DIR)/tmpfs $(MOUNTPOINT) -d -o allow_other
