
export BIOSPHERE_ROOT := $(CURDIR)/../Biosphere

export INCLUDE	:=	include \
					include/bio \
					include/bio/svc \
					include/bio/os \
					include/bio/ipc \
					include/bio/log \
					include/bio/sm \
					include/bio/fsp \
					include/bio/fs \
					include/bio/err \
					include/bio/hid \
					include/bio/input \
					include/bio/ro \
					include/bio/ld \
					include/bio/fatal \
					include/bio/flora

export SOURCE	:=	source \
					source/bio \
					source/bio/crt0 \
					source/bio/svc \
					source/bio/os \
					source/bio/ipc \
					source/bio/log \
					source/bio/sm \
					source/bio/fsp \
					source/bio/fs \
					source/bio/err \
					source/bio/hid \
					source/bio/input \
					source/bio/ro \
					source/bio/ld \
					source/bio/fatal \
					source/bio/flora

export TARGET_MODE	:=	STATICLIB

all: precompile nso nro libnro install

precompile: # Makefile system adds this folders by default, so will error if they are not present, and since we will need to create them later...
	@mkdir -p $(BIOSPHERE_ROOT)/libbio
	@mkdir -p $(BIOSPHERE_ROOT)/libbio/lib

nso:
	$(MAKE) -f $(CURDIR)/compile/NSO.mk

nro:
	$(MAKE) -f $(CURDIR)/compile/NRO.mk

libnro:
	$(MAKE) -f $(CURDIR)/compile/libNRO.mk

install:
	cp -a $(CURDIR)/include $(BIOSPHERE_ROOT)/libbio/include

clean:
	@rm -rf $(CURDIR)/build.nso
	@rm -rf $(CURDIR)/build.nro
	@rm -rf $(CURDIR)/build.libnro
	@rm -rf $(BIOSPHERE_ROOT)/libbio