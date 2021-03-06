# OPENLIBM

openlibm_SRCS := $(wildcard $(SOURCE_ROOT)/openlibm/src/*.c)
openlibm_OBJECT_FILES := $(addprefix $(BUILD_DIR)/openlibm/, $(notdir $(openlibm_SRCS:.c=.o)))

openlibm_HEADERS := cdefs-compat.h complex.h math.h openlibm_fenv_arm.h openlibm_fenv.h

# ARCHIVE RULES

$(BUILD_DIR)/openlibm/libm.a: $(openlibm_OBJECT_FILES)
	mkdir -p $(@D)
	rm -f $@
	$(AR) $(AR_FLAGS) $@ $+

# BUILD RULES

$(BUILD_DIR)/openlibm/%.o: $(SOURCE_ROOT)/openlibm/src/%.c
	mkdir -p $(@D)
	$(CC) $(CC_FLAGS) -I$(SOURCE_ROOT)/openlibm/include -I$(SOURCE_ROOT)/openlibm/src -c -o $@ $<

# DIST RULES

DIST_OPENLIBM := $(BIOSPHERE_HOME)/lib/libm.a $(BIOSPHERE_HOME)/openlibm_flag \
	$(addprefix $(BIOSPHERE_HOME)/include/,$(openlibm_HEADERS))

# this is terrible.
# force newlib to install before us
$(DIST_OPENLIBM): $(BIOSPHERE_HOME)/lib/libc.a $(BUILD_DIR)/openlibm/libm.a
	install -d $(@D)
	install $(BUILD_DIR)/openlibm/libm.a $(BIOSPHERE_HOME)/lib/libm.a
	install $(addprefix $(SOURCE_ROOT)/openlibm/include/,$(openlibm_HEADERS)) $(BIOSPHERE_HOME)/include/
	touch $(BIOSPHERE_HOME)/openlibm_flag
	touch $(BIOSPHERE_HOME)/lib/libm.a

$(BIOSPHERE_HOME)/include/%: $(SOURCE_ROOT)/openlibm/include/%
	install -d $(@D)
	install $< $@

.PHONY: dist_openlibm
dist_openlibm: $(DIST_OPENLIBM)

# CLEAN RULES

.PHONY: clean_openlibm

clean_openlibm:
	rm -rf $(BUILD_DIR)/openlibm
