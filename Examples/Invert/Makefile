CXXFLAGS = -I../../include
OPTIMIZER = -O3

ifeq ($(shell getconf LONG_BIT),64)
	CXXFLAGS = -I../../include -g -fPIC
endif

invert.dso : invert.o
	$(CXX) -shared invert.o -o invert.dso
	strip -s invert.dso

