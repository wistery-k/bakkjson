CXX = g++
CXXFLAGS = -std=c++0x -O2 -Wall

.PHONY: version clean

a: a.cpp

version:
	$(CXX) --version

clean:
	$(RM) a