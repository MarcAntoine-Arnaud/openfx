CXXFLAGS = -I../../include -g
OPTIMIZER = -g

ifeq ($(shell getconf LONG_BIT),64)
	CXXFLAGS = -I../../include -g -fPIC
endif

basic.dso : basic.o
	$(CXX) -shared basic.o -o basic.dso
	strip -s basic.dso

