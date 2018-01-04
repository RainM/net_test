SRC=$(wildcard *.cpp)
OBJ=$(SRC:.cpp=.cpp.o)

CXX=g++
CXXFLAGS=-std=c++11 -O0 -g -D__GNU_VISIBLE -D_GNU_SOURCE -Wall -pedantic

ifneq ($(RELEASE),)
CXXFLAGS+=-O3
endif

RM?=rm -rf

NET_TEST_OBJ=net_test.cpp.o client.cpp.o
MIRROR_OBJ=mirror.cpp.o server.cpp.o

HDR_HISTOGRAM_LIB=lib/hdr_histogram/build/src/libhdr_histogram_static.a
CXXFLAGS+=-Ilib/hdr_histogram/src

PORT=12346

%.cpp.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

all: net_test mirror

net_test: $(NET_TEST_OBJ) $(HDR_HISTOGRAM_LIB)
	$(CXX) $(CXXFLAGS) $(LINKFLAGS) $^ -o $@

mirror: $(MIRROR_OBJ)
	$(CXX) $(CXXFLAGS) $(LINKFLAGS) $^ -o $@

clean:
	rm -rf *.o server mirror

$(HDR_HISTOGRAM_LIB):
	-mkdir lib/hdr_histogram/build/
	(cd lib/hdr_histogram/build ; cmake -DCMAKE_BUILD_TYPE=Release ..)
	make -C lib/hdr_histogram/build

download-modules:
	git submodule init
	git submodule update

start-mirror: net_test mirror
	scp ./mirror "$(HOST):/tmp/"
	-ssh "$(HOST)" /tmp/mirror $(PORT) 
