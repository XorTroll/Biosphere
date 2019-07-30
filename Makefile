SOURCE_ROOT := .
BUILD_DIR := $(SOURCE_ROOT)/build/

BIOSPHERE_HOME := $(realpath .)/Biosphere

ifeq ($(shell id -u), 0)
   $(error "This script must not be run as root")
endif

include Compile.mk

# for newlib
export LD
export CC
export CXX
export AS
export AR
export LD_FOR_TARGET = $(LD)
export CC_FOR_TARGET = $(CC)
export AS_FOR_TARGET = $(AS) -arch=aarch64 -mattr=+neon
export AR_FOR_TARGET = $(AR)
export RANLIB_FOR_TARGET = $(RANLIB)
export CFLAGS_FOR_TARGET = $(CC_FLAGS) -Wno-unused-command-line-argument -Wno-error-implicit-function-declaration

include mk/newlib.mk
include mk/compiler-rt.mk
include mk/liblzma.mk
include mk/pthread.mk
include mk/openlibm.mk
include mk/libcxx.mk

DIST := $(DIST_NEWLIB) $(DIST_PTHREAD) $(DIST_COMPILER_RT) $(DIST_LIBLZMA) $(DIST_OPENLIBM) $(DIST_LIBCXX) $(DIST_LIBCXXABI) $(DIST_LIBUNWIND)
SUPPORT_HEADERS := endian.h machine/_align.h sys/_iovec.h sys/socket.h sys/_sockaddr_storage.h sys/sockio.h netinet/in.h netinet/tcp.h netdb.h arpa/inet.h net/if.h sys/features.h nl_types.h lz4.h netinet6/in6.h features.h sha256.h expected.hpp dlfcn.h poll.h

.PHONY: default precompile libbio flora fauna clean
.DEFAULT_GOAL := default

default: precompile $(DIST) libbio flora fauna
	@echo " - Biosphere finished compiling. Run 'SetBiosphereRoot.sh' script in order to set 'BIOSPHERE_ROOT' variable to that directory (needed for development)"

precompile:
	@echo " - Starting Biosphere build..."
	@mkdir -p $(BIOSPHERE_HOME)
	@mkdir -p $(BIOSPHERE_HOME)/include
	@mkdir -p $(BIOSPHERE_HOME)/include/sys
	@mkdir -p $(BIOSPHERE_HOME)/include/netinet
	@mkdir -p $(BIOSPHERE_HOME)/include/arpa
	@mkdir -p $(BIOSPHERE_HOME)/include/netinet6
	@mkdir -p $(BIOSPHERE_HOME)/include/machine
	@mkdir -p $(BIOSPHERE_HOME)/include/net
	@cp -r $(CURDIR)/utils/ $(BIOSPHERE_HOME)/
	@cp -r $(CURDIR)/compile/ $(BIOSPHERE_HOME)/
	@mv $(BIOSPHERE_HOME)/utils/SetBiosphereRoot.sh $(BIOSPHERE_HOME)/SetBiosphereRoot.sh
	@$(foreach header,$(SUPPORT_HEADERS),cp $(CURDIR)/include/$(header) $(BIOSPHERE_HOME)/include/$(header);)
	@echo " - Compiling basic standard libraries... (newlib, libcxx, libcxxabi, libpthread, libunwind...)"

# Custom targets for flora, fauna and libbio

flora:
	@echo " - Compiling flora logging tool (console-side)..."
	@$(MAKE) -C flora/
	@mkdir -p $(BIOSPHERE_HOME)/utils/dev/flora/0100000000000721/flags
	@touch $(BIOSPHERE_HOME)/utils/dev/flora/0100000000000721/flags/boot2.flag
	@cp -r flora/flora.nsp $(BIOSPHERE_HOME)/utils/dev/flora/0100000000000721/exefs.nsp

fauna:
	@echo " - Compiling fauna logging tool (PC-side)..."
	@mkdir -p $(BIOSPHERE_HOME)/utils/dev/fauna
	@cp -r fauna/fauna/bin/Release/net461/fauna.exe $(BIOSPHERE_HOME)/utils/dev/fauna/fauna.exe

libbio:
	@echo " - Compiling Biosphere homebrew libraries (libbio)..."
	@$(MAKE) -C libbio/

clean:
	@echo " - Cleaning Biosphere build..."
	@rm -rf $(BIOSPHERE_HOME) $(BUILD_DIR)