# BUILD RULES

include $(BIOSPHERE_ROOT)/compile/Input.mk
include $(BIOSPHERE_ROOT)/compile/Config.mk

# Rule for building C++ files (*.cpp)
%.o: %.cpp
	@mkdir -p $(dir $(SOURCE_ROOT)/$(BUILD_DIR)/$*.o)
	@echo $(notdir $<)
	$(CXX) $(CXX_FLAGS) $(BIOSPHERE_WARNINGS) -MMD -MP -MF $(SOURCE_ROOT)/$(BUILD_DIR)/$*.d -c -o $(SOURCE_ROOT)/$(BUILD_DIR)/$*.o $(SOURCE_ROOT)/$*.cpp

# Rule for building C++ files (*.cc)
%.o: %.cc
	@mkdir -p $(dir $(SOURCE_ROOT)/$(BUILD_DIR)/$*.o)
	@echo $(notdir $<)
	$(CXX) $(CXX_FLAGS) $(BIOSPHERE_WARNINGS) -MMD -MP -MF $(SOURCE_ROOT)/$(BUILD_DIR)/$*.d -c -o $(SOURCE_ROOT)/$(BUILD_DIR)/$*.o $(SOURCE_ROOT)/$*.cc

# Rule for building C files
%.o: %.c
	@mkdir -p $(dir $(SOURCE_ROOT)/$(BUILD_DIR)/$*.o)
	@echo $(notdir $<)
	$(CC) $(CC_FLAGS) $(BIOSPHERE_WARNINGS) -MMD -MP -MF $(SOURCE_ROOT)/$(BUILD_DIR)/$*.d -c -o $(SOURCE_ROOT)/$(BUILD_DIR)/$*.o $(SOURCE_ROOT)/$*.c

# Rule for building assembly files
%.o: %.s
	@mkdir -p $(dir $(SOURCE_ROOT)/$(BUILD_DIR)/$*.o)
	$(AS) $(AS_FLAGS) $(SOURCE_ROOT)/$*.s -filetype=obj -o $(SOURCE_ROOT)/$(BUILD_DIR)/$*.o
	@touch $(SOURCE_ROOT)/$(BUILD_DIR)/$*.d

%.nss: $(OBJECTS)
	@echo linking $(notdir $@)
	@$(LD) $(LD_FLAGS) -o $@ $(FULL_OBJECTS) $(BIOSPHERE_NSO_LDFLAGS)

%.nrs: $(OBJECTS)
	@echo linking $(notdir $@)
	@$(LD) $(LD_FLAGS) -o $@ $(FULL_OBJECTS) $(BIOSPHERE_NRO_LDFLAGS)

%.lib.nrs: $(OBJECTS)
	@echo linking $(notdir $@)
	@$(LD) $(LD_FLAGS) -o $@ $(FULL_OBJECTS) $(BIOSPHERE_LIBNRO_LDFLAGS)

%.a: $(OBJECTS)
	@echo building $(notdir $@)
	@rm -f $@
	@mkdir -p $(dir $@)
	$(AR) $(AR_FLAGS) $@ $(FULL_OBJECTS)

%.nro: %.nrs
	@$(BIOSPHERE_ROOT)/utils/ld/linkle nro $(BIOSPHERE_NRO_LINKLE_ARGS) $< $@
	@echo built ... $(notdir $@)

%.lib.nro: %.lib.nrs
	@$(BIOSPHERE_ROOT)/utils/ld/linkle nro $< $@
	@echo built ... $(notdir $@)

%.nso: %.nss
	@$(BIOSPHERE_ROOT)/utils/ld/linkle nso $< $@
	@echo built ... $(notdir $@)

%.nsp: %.nso
	@mkdir $@
	@echo generating ... $(notdir $@)
	@cp $(BIOSPHERE_ROOT)/libbio/lib/libbio-sdk.nso  $@/sdk
	@mv $< $@/main
	@touch rtld.stub