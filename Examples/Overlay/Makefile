CXXFLAGS = -I../../include 
OPTIMIZER = -O3

ifeq ($(shell getconf LONG_BIT),64)
	CXXFLAGS = -I../../include -g -fPIC
endif

overlay.dso : overlay.o
	$(CXX) -shared overlay.o -o overlay.dso
	strip -s overlay.dso

