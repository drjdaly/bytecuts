CXX = g++
CXXFLAGS = -g -std=c++14 -pedantic -fpermissive -fopenmp -O3 $(INCLUDE) 

all: main validate

main: Classify.cpp InputReader.o OutputWriter.o MapExtensions.o ByteCuts.o ByteCutsNode.o TreeBuilder.o
	$(CXX) $(CXXFLAGS) -o main Classify.cpp *.o
	
validate: Validate.cpp InputReader.o OutputWriter.o
	$(CXX) $(CXXFLAGS) -o validate Validate.cpp *.o

Classify.o:	Classify.cpp IO/InputReader.h IO/OutputWriter.h Utilities/MapExtensions.h ByteCuts/ByteCuts.h
	$(CXX) $(CXXFLAGS) -c Classify.cpp


clean:
	rm *.o main validate

# IO 

InputReader.o: IO/InputReader.cpp IO/InputReader.h Common.h
	$(CXX) $(CXXFLAGS) -c IO/InputReader.cpp

OutputWriter.o: IO/OutputWriter.cpp IO/OutputWriter.h Common.h
	$(CXX) $(CXXFLAGS) -c IO/OutputWriter.cpp

# Utilities

MapExtensions.o: Utilities/MapExtensions.cpp Utilities/MapExtensions.h
	$(CXX) $(CXXFLAGS) -c Utilities/MapExtensions.cpp
	
# Classifiers

ByteCuts.o: ByteCuts/ByteCuts.cpp ByteCuts/ByteCuts.h ByteCuts/ByteCutsNode.h ByteCuts/TreeBuilder.h Common.h
	$(CXX) $(CXXFLAGS) -c ByteCuts/ByteCuts.cpp

ByteCutsNode.o: ByteCuts/ByteCutsNode.cpp ByteCuts/ByteCutsNode.h Common.h
	$(CXX) $(CXXFLAGS) -c ByteCuts/ByteCutsNode.cpp
	
TreeBuilder.o: ByteCuts/TreeBuilder.cpp ByteCuts/ByteCutsNode.h ByteCuts/TreeBuilder.h Common.h
	$(CXX) $(CXXFLAGS) -c ByteCuts/TreeBuilder.cpp
