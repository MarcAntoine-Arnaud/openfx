CXXFLAGS = -I../../include
OPTIMIZER = -g

ifeq ($(shell getconf LONG_BIT),64)
	CXXFLAGS = -I../../include -g -fPIC
endif

testProperties.bundle : testProperties.o ofxLog.o
	$(CXX) -shared testProperties.o ofxLog.o -o testProperties.dso