CXXFLAGS = -I../../include 
OPTIMIZER = -O3

ifeq ($(shell getconf LONG_BIT),64)
	CXXFLAGS = -I../../include -g -fPIC
endif

custom.dso : custom.o
	$(CXX) -shared custom.o -o custom.dso
	strip -s custom.dso

