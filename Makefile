NAME	:= ollvm

LLVM_V	:= 20

define log_info
	echo -e "[\033[0;33m*\033[0m] $(1)"
endef

define log_success
	echo -e "[\033[0;32m+\033[0m] Done"
endef

define compile_pass
	podman run --rm -v $(PWD):/usr/local/src llvm-dev sh -c "clang++ -std=c++20 -fPIC -shared passes/$(1)/src/*.cc -o bin/$(NAME).so \`llvm-config --cxxflags --ldflags --libs core support\`"
endef

ListFunctionNames: clean
	@ $(call log_info,Compiling...)
	@ $(call compile_pass,ListFunctionNames)
	@ $(call log_success)

ListFunctions: clean
	@ $(call log_info,Compiling...)
	@ $(call compile_pass,ListFunctions)
	@ $(call log_success)

ListBasicBlocks: clean
	@ $(call log_info,Compiling...)
	@ $(call compile_pass,ListBasicBlocks)
	@ $(call log_success)

ListInstructions: clean
	@ $(call log_info,Compiling...)
	@ $(call compile_pass,ListInstructions)
	@ $(call log_success)

test:
	@ $(call log_info,Testing...)
	@ clang -fpass-plugin=bin/$(NAME).so test/test.cc -o test/test
	@ $(call log_success)

pod-build:
	@ $(call log_info, Building Podman image...)
	@ podman build --build-arg LLVM_VERSION_TAG=llvmorg-$(LLVM_V) --quiet -t llvm-dev . --format docker
	@ $(call log_success)

pod-clean:
	@ $(call log_info, Deleting Podman image...)
	@ podman image rm llvm-dev
	@ $(call log_success)

clean:
	@ $(call log_info,Cleaning build artifacts)
	@ rm -f bin/*.so test/test
	@ $(call log_success)

.PHONY: test clean pod-build pod-clean
