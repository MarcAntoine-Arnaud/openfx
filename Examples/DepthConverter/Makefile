CXXFLAGS = -I../../include
OPTIMIZER = -O3

ifeq ($(shell getconf LONG_BIT),64)
	CXXFLAGS = -I../../include -g -fPIC
endif

depthConverter.dso : depthConverter.o
	$(CXX) -shared depthConverter.o -o depthConverter.dso
	strip -s depthConverter.dso

