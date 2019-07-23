

INCLUDES	:=	$(foreach dir,$(INCLUDE),$(wildcard $(dir)/*.h))
SOURCES_CPP	:=	$(foreach dir,$(SOURCE),$(wildcard $(dir)/*.cpp))
SOURCES_CC	:=	$(foreach dir,$(SOURCE),$(wildcard $(dir)/*.cc))
SOURCES_C	:=	$(foreach dir,$(SOURCE),$(wildcard $(dir)/*.c))
SOURCES_ASM	:=	$(foreach dir,$(SOURCE),$(wildcard $(dir)/*.s))

OBJECTS	:=	$(SOURCES_CPP:.cpp=.o) $(SOURCES_CC:.cc=.o) $(SOURCES_C:.c=.o) $(SOURCES_ASM:.s=.o)

INCLUDE_DIRS	:=	$(foreach dir,$(INCLUDE),-I$(SOURCE_ROOT)/$(dir))
FULL_OBJECTS	:=	$(foreach obj,$(OBJECTS),$(SOURCE_ROOT)/$(BUILD_DIR)/$(obj))