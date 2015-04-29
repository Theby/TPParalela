FLAGS = -std=c++0x -msse3 -Wall -O2 -fopenmp
DEPS = minHeap.hpp
OBJ = minHeap.o simsort.o

%.o: %.cpp $(DEPS)
	g++ $(FLAGS) -c -o $@ $<

simsort: $(OBJ)
	g++ $(FLAGS) -o $@ $^

clean:
	rm -f Simsort *.o
.PHONY: all clean