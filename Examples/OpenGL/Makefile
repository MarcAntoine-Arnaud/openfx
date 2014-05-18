CXXFLAGS = -I../../include 
OPTIMIZER = -O3

ifeq ($(shell getconf LONG_BIT),64)
	CXXFLAGS = -I../../include -g -fPIC
endif

opengl.dso : opengl.o
	$(CXX) -shared opengl.o -o opengl.dso
	strip -s opengl.dso

