# llvm programs

# On MacOS, brew refuses to install clang5/llvm5 in a global place. As a result,
# they have to muck around with changing the path, which sucks.
# Let's make their lives easier by asking brew where LLVM_CONFIG is.
ifeq ($(shell uname -s),Darwin)
    ifeq ($(shell brew --prefix llvm),)
        $(error need llvm installed via brew)
    else
        LLVM_CONFIG := $(shell brew --prefix llvm)/bin/llvm-config
    endif
else
    LLVM_CONFIG := llvm-config$(LLVM_POSTFIX)
endif

LLVM_BINDIR := $(shell $(LLVM_CONFIG) --bindir)
ifeq ($(LLVM_BINDIR),)
  $(error llvm-config needs to be installed)
endif

LD := $(LLVM_BINDIR)/ld.lld
CC := $(LLVM_BINDIR)/clang
CXX := $(LLVM_BINDIR)/clang++
AS := $(LLVM_BINDIR)/llvm-mc
AR := $(LLVM_BINDIR)/llvm-ar
RANLIB := $(LLVM_BINDIR)/llvm-ranlib

SYS_INCLUDES := -isystem $(BIOSPHERE_ROOT)/include/ -isystem $(BIOSPHERE_ROOT)/libbio/include/ -isystem $(BIOSPHERE_ROOT)/include/c++/v1/

PKG_CONFIG_SYSROOT_DIR=$(BIOSPHERE_ROOT)

# linker flags for building main binary
#   -Bsymbolic: bind symbols locally
#   --shared: build a shared object
LD_FLAGS := -Bsymbolic \
	--shared \
	--gc-sections \
	--eh-frame-hdr \
	--no-undefined \
	-T $(BIOSPHERE_ROOT)/utils/ld/link.ld \
	-L $(BIOSPHERE_ROOT)/lib/ \
	-L $(BIOSPHERE_ROOT)/libbio/lib/

# linker flags for building shared libraries
#   --shared: build a shared object
#   -Bdynamic: link against shared libraries
LD_SHARED_LIBRARY_FLAGS := --shared \
	--gc-sections \
	--eh-frame-hdr \
	-T $(BIOSPHERE_ROOT)/utils/ld/link.ld \
	-L $(BIOSPHERE_ROOT)/lib/ \
	-L $(BIOSPHERE_ROOT)/libbio/lib/ \
	-Bdynamic

CC_FLAGS := -g -fPIE -fexceptions -fuse-ld=lld -O2 -mtune=cortex-a53 -target aarch64-none-linux-gnu -nostdlib -nostdlibinc $(DEFINES) $(SYS_INCLUDES) $(INCLUDE_DIRS) -D__SWITCH__=1 -Wno-unused-command-line-argument
CXX_FLAGS := $(CPP_INCLUDES) $(CC_FLAGS) -std=c++17 -stdlib=libc++ -nodefaultlibs -nostdinc++
AR_FLAGS := rcs
AS_FLAGS := -arch=aarch64 -triple aarch64-none-switch

# these are libraries that libbio depends on, and that must be statically linked.
# any other libraries may be dynamically linked.
#   -Bstatic: do not link against shared libraries
#   -Bdynamic: link against shared libraries

BIOSPHERE_BASE_LIBS	:=	-lc -lm -lclang_rt.builtins-aarch64 -lpthread -llzma -lc++ -lc++abi -lunwind

BIOSPHERE_EXECUTABLE_LDFLAGS := -Bstatic \
	$(BIOSPHERE_BASE_LIBS) \
	-Bdynamic

BIOSPHERE_NRO_LDFLAGS := -lbio-NRO $(BIOSPHERE_EXECUTABLE_LDFLAGS)
BIOSPHERE_NSO_LDFLAGS := -lbio-NSO $(BIOSPHERE_EXECUTABLE_LDFLAGS)
BIOSPHERE_LIBNRO_LDFLAGS := -lbio-libNRO -lc -lclang_rt.builtins-aarch64 -lc++ -lc++abi -lunwind

ifneq ($(NACP_JSON),)
BIOSPHERE_NRO_LINKLE_ARGS += --nacp-path "$(NACP_JSON)"
endif

ifneq ($(ROMFS_DIR),)
BIOSPHERE_NRO_LINKLE_ARGS += --romfs-path "$(ROMFS_DIR)"
endif

ifneq ($(ICON_JPG),)
BIOSPHERE_NRO_LINKLE_ARGS += --icon-path "$(ICON_JPG)"
endif