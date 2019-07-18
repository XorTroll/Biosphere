TARGET := $(BIOSPHERE_ROOT)/libbio/lib/libbio-NSO

BUILD_DIR	:=	build.nso
SOURCE	+=	source/bio/crt0/crt0.common source/bio/crt0/crt0.nso

include $(BIOSPHERE_ROOT)/compile/Target.mk