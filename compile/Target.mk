ifndef BIOSPHERE_ROOT
    $(error BIOSPHERE_ROOT must be set)
endif

ifeq ($(SOURCE_ROOT),)
    SOURCE_ROOT	:=	$(CURDIR)
endif

ifeq ($(TARGET_MODE),)
	TARGET_MODE	:=	NRO
endif

ifeq ($(BUILD_DIR),)
	BUILD_DIR	:=	build
endif

include $(BIOSPHERE_ROOT)/compile/Base.mk

ifeq ($(TARGET_MODE),NRO)

all: $(TARGET).nro

$(TARGET).nro : $(TARGET).nrs

clean_target:
	@rm -rf $(SOURCE_ROOT)/$(BUILD_DIR) $(TARGET).nrs $(TARGET).nro

else ifeq ($(TARGET_MODE),NSO)

all: $(TARGET).nso

$(TARGET).nso : $(TARGET).nss

clean_target:
	@rm -rf $(SOURCE_ROOT)/$(BUILD_DIR) $(TARGET).nss $(TARGET).nso

else ifeq ($(TARGET_MODE),LIBNRO)

all: $(TARGET).lib.nro

$(TARGET).lib.nro : $(TARGET).lib.nrs

clean_target:
	@rm -rf $(SOURCE_ROOT)/$(BUILD_DIR) $(TARGET).lib.nrs $(TARGET).lib.nro

else ifeq ($(TARGET_MODE),STATICLIB)

all: $(TARGET).a

clean_target:
	@rm -rf $(SOURCE_ROOT)/$(BUILD_DIR) $(TARGET).a

else

$(error Invalid target mode was specified.)

endif

.PHONY: all clean_target