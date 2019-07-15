# COMPILER_RT

$(LIB_DEP_COMPILER_RT_BUILTINS): $(BUILD_DIR)/compiler-rt/Makefile
	$(MAKE) -C $(BUILD_DIR)/compiler-rt/ all install
	touch $@

$(BUILD_DIR)/compiler-rt/Makefile: $(DIST_NEWLIB) $(DIST_TRANSISTOR_SUPPORT) mk/compiler-rt.mk
	mkdir -p $(@D)
	cd $(@D); cmake -G "Unix Makefiles" $(realpath $(SOURCE_ROOT))/compiler-rt \
		-DCOMPILER_RT_BAREMETAL_BUILD=ON \
		-DCOMPILER_RT_BUILD_BUILTINS=ON \
		-DCOMPILER_RT_BUILD_SANITIZERS=OFF \
		-DCOMPILER_RT_BUILD_XRAY=OFF \
		-DCOMPILER_RT_BUILD_LIBFUZZER=OFF \
		-DCOMPILER_RT_BUILD_PROFILE=OFF \
		-DCMAKE_C_COMPILER="$(CC)" \
		-DCMAKE_C_FLAGS="$(CC_FLAGS)" \
		-DCMAKE_C_COMPILER_TARGET="aarch64-none-linux-gnu" \
		-DCMAKE_CXX_COMPILER="$(CXX)" \
		-DCMAKE_CXX_FLAGS="$(CC_FLAGS)" \
		-DCMAKE_CXX_COMPILER_TARGET="aarch64-none-linux-gnu" \
		-DCMAKE_AR="$(AR)" \
		-DCMAKE_EXE_LINKER_FLAGS="-fuse-ld=lld" \
		-DCOMPILER_RT_DEFAULT_TARGET_ONLY=ON \
		-DLLVM_CONFIG_PATH=$(LLVM_CONFIG) \
		-DCMAKE_SYSTEM_NAME=Linux \
		-DCMAKE_INSTALL_PREFIX=$(BIOSPHERE_HOME) \
		-DCOMPILER_RT_OS_DIR=.

# DIST RULES

DIST_COMPILER_RT := $(LIB_DEP_COMPILER_RT_BUILTINS)

.PHONY: dist_compiler-rt
dist_compiler-rt: $(DIST_COMPILER_RT)

# CLEAN RULES

.PHONY: clean_compiler-rt

clean_compiler-rt:
	rm -rf build/compiler-rt
