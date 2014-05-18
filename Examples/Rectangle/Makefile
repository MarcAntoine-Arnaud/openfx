CXXFLAGS = -I../../include
OPTIMIZER = -O3

ifeq ($(shell getconf LONG_BIT),64)
	CXXFLAGS = -I../../include -g -fPIC
endif

rectangle.dso : rectangle.o
	$(CXX) -shared rectangle.o -o rectangle.dso
	strip -s rectangle.dso

