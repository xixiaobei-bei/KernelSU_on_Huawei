
#only on CLANG builds
ifeq ($(cc-name),clang)

HOSTCXXFLAGS += -Wall -std=c++11 -stdlib=libc++ -fno-exceptions -fno-rtti
HOSTCXXFLAGS += -I$(CLANG_PREBUILTS_PATH)include/
export HOSTCXXFLAGS
HOSTLDFLAGS += -stdlib=libc++ -Wl,-rpath,$(CLANG_PREBUILTS_PATH)lib64/
HOST_LOADLIBES += -lLLVM-9svn

llvm-ircopy-cxxobjs := llvm-ircopy.o

hostprogs-y := llvm-ircopy
always := $(hostprogs-y)

endif

