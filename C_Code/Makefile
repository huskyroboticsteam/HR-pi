CC       := gcc
CFLAGS   := -Wall -Wextra -O2 -MMD -MP
BUILD_DIR:= build

# --- Configure these -----------------------------------------------

# Directories (relative to this Makefile) to skip entirely.
EXCLUDE_DIRS := archive Spectrometer Webcam

# File listing source paths that should each become their own binary,
# one path per line, relative to this Makefile. Each of these is
# compiled with -DIS_MAIN so the file knows to expose a main().
MAIN_LIST_FILE := mains.txt

# -----------------------------------------------------------------------

# Read MAIN_LIST_FILE into a Make list, one path per line. Blank
# lines and lines starting with '#' (comments) are dropped. If the
# file doesn't exist yet, this yields an empty list rather than
# erroring out.
ifneq ($(wildcard $(MAIN_LIST_FILE)),)
MAIN_SRCS := $(shell grep -v '^[[:space:]]*#' $(MAIN_LIST_FILE) | grep -v '^[[:space:]]*$$')
else
MAIN_SRCS :=
endif

# Build a `find` prune expression from EXCLUDE_DIRS. The trailing
# "-false" makes this safe even if EXCLUDE_DIRS is empty.
PRUNE_EXPR := $(foreach d,$(EXCLUDE_DIRS),-path './$(d)' -o )

ALL_SRCS := $(shell find . \( $(PRUNE_EXPR) -false \) -prune -o -name '*.c' -print | sed 's|^\./||')

COMMON_SRCS := $(filter-out $(MAIN_SRCS),$(ALL_SRCS))

COMMON_OBJS := $(patsubst %.c,$(BUILD_DIR)/%.o,$(COMMON_SRCS))
MAIN_OBJS   := $(patsubst %.c,$(BUILD_DIR)/%.o,$(MAIN_SRCS))
DEPS        := $(COMMON_OBJS:.o=.d) $(MAIN_OBJS:.o=.d)

BINARIES := $(notdir $(basename $(MAIN_SRCS)))

LIBCOMMON := $(BUILD_DIR)/libcommon.a

.PHONY: all clean
all: $(BINARIES)

# Ordinary (non-main) object files
$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# "Main" object files get the extra macro. This is a static pattern
# rule, so it takes precedence over the generic rule above for the
# specific files listed in MAIN_OBJS.
$(MAIN_OBJS): $(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -DIS_MAIN -c $< -o $@

# Archive all the non-main objects. Linking against this archive
# (rather than listing every .o by hand) is what makes each binary
# automatically pull in only the object files it actually needs.
$(LIBCOMMON): $(COMMON_OBJS)
	ar rcs $@ $^

# Generate one link rule per main file.
define MAIN_TEMPLATE
$(notdir $(basename $(1))): $(BUILD_DIR)/$(basename $(1)).o $(LIBCOMMON)
	$(CC) $(CFLAGS) $$^ -o $$@
endef
$(foreach src,$(MAIN_SRCS),$(eval $(call MAIN_TEMPLATE,$(src))))

clean:
	rm -rf $(BUILD_DIR) $(BINARIES)

-include $(DEPS)