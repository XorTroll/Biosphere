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

.SUFFIXES: # disable built-in rules
.SECONDARY: # don't delete intermediate files

.PHONY: default

default:

include mk/newlib.mk
include mk/compiler-rt.mk
include mk/liblzma.mk
include mk/pthread.mk
include mk/openlibm.mk
include mk/libcxx.mk

DIST := $(DIST_NEWLIB) $(DIST_PTHREAD) $(DIST_COMPILER_RT) $(DIST_LIBLZMA) $(DIST_OPENLIBM) $(DIST_LIBCXX) $(DIST_LIBCXXABI) $(DIST_LIBUNWIND)
SUPPORT_HEADERS := endian.h machine/_align.h sys/_iovec.h sys/socket.h sys/_sockaddr_storage.h sys/sockio.h netinet/in.h netinet/tcp.h netdb.h arpa/inet.h net/if.h sys/features.h nl_types.h lz4.h netinet6/in6.h features.h sha256.h expected.hpp dlfcn.h poll.h

dist: $(DIST)
.PHONY: dist

precompile:
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
	@$(foreach header,$(SUPPORT_HEADERS),cp $(CURDIR)/include/$(header) $(BIOSPHERE_HOME)/include/$(header);)

# libbio has a different compiling system so is handled separately, and after all the basic libs

default: precompile $(DIST)
	@$(MAKE) -C libbio/

clean_real:
	rm -rf $(BIOSPHERE_HOME) $(BUILD_DIR)

clean:
	rm -rf $(BIOSPHERE_HOME)