# gcc should point to gcc-4.5
CC = gcc
PLUGIN_SOURCE_FILES = plugin.c trace.c
PLUGIN_OBJECT_FILES = $(patsubst %.c,%.o,$(PLUGIN_SOURCE_FILES))

PLUGINS_DIR := $(shell $(CC) -print-file-name=plugin)
CFLAGS += -I$(PLUGINS_DIR)/include -fPIC -shared 

branchcov.so: $(PLUGIN_OBJECT_FILES)
	$(CC) -shared  $^ -o $@

clean:
	rm *.o *.so
