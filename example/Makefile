# callback to __tracer
CXX = g++-4.5 -fplugin=../branchcov.so -fplugin-arg-branchcov-calltype=callback -fplugin-arg-branchcov-tracefile=branchcov 
# printf:
# CXX = g++-4.5 -fplugin=../branchcov.so -fplugin-arg-branchcov-calltype=callback -fplugin-arg-branchcov-tracefile=branchcov

CXXFLAGS =-Wall -g
INCLUDES =
SOURCES = example.cpp

OBJECTS = $(SOURCES:.cpp=.o)
EXECUTABLE = example 

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(INCLUDES) $(OBJECTS)
	$(CXX) -o $@ $(OBJECTS)


clean:
	rm *.o $(EXECUTABLE)

.cpp.o:
	$(CXX) $(CXXFLAGS) -c -o $@ $<

