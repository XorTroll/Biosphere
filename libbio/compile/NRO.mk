TARGET := $(BIOSPHERE_ROOT)/libbio/lib/libbio-NRO

BUILD_DIR	:=	build.nro
SOURCE	+=	source/bio/crt0/crt0.common source/bio/crt0/crt0.nro

include $(BIOSPHERE_ROOT)/compile/Target.mk