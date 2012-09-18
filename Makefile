CXX = g++
CXXFLAGS = -std=c++0x -O2 -Wall

.PHONY: version clean

a: bakkjson.o a.o
	$(CXX) $(CXXFLAGS) bakkjson.o a.o -o a

version:
	$(CXX) --version

clean:
	$(RM) a *.o