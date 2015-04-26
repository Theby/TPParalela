FLAGS = -std=c++11 -Wall -msse4 -O2
DEPS = minHeap.hpp
OBJ = minHeap.o simsort.o

%.o: %.cpp $(DEPS)
	g++ $(FLAGS) -c -o $@ $<

simsort: $(OBJ)
	g++ $(FLAGS) -o $@ $^

clean:
	rm -f Simsort *.o
.PHONY: all clean