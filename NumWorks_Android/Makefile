Q ?= @
CC = arm-none-eabi-gcc
BUILD_DIR = output
NWLINK = npx --yes -- nwlink@0.0.19
TARGET = $(BUILD_DIR)/NumWorks_Android.nwa

src = $(wildcard src/*.c)
obj = $(addprefix $(BUILD_DIR)/,$(addsuffix .o,$(basename $(src))))

CFLAGS = -std=c99 -Iinclude
CFLAGS += $(shell $(NWLINK) eadk-cflags)
CFLAGS += -Os -Wall -ffunction-sections -fdata-sections
CFLAGS += -fno-builtin -fno-tree-loop-distribute-patterns
LDFLAGS = -Wl,--relocatable -nostartfiles --specs=nano.specs
LDFLAGS += -Wl,-e,main -Wl,-u,eadk_app_name -Wl,-u,eadk_app_icon -Wl,-u,eadk_api_level
LDFLAGS += -Wl,--gc-sections

.PHONY: build run clean assets
build: $(TARGET)

assets:
	python tools/prepare_assets.py

run: $(TARGET)
	$(NWLINK) install-nwa $<

$(TARGET): $(obj) $(BUILD_DIR)/icon.o
	@echo "LD      $@"
	$(Q) $(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	@echo "CC      $<"
	$(Q) mkdir -p $(dir $@)
	$(Q) $(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/icon.o: assets/icon.png | $(BUILD_DIR)
	@echo "ICON    $<"
	$(Q) $(NWLINK) png-icon-o $< $@

.PRECIOUS: $(BUILD_DIR)/icon.o
$(BUILD_DIR):
	$(Q) mkdir -p $@

clean:
	rm -rf $(BUILD_DIR)
