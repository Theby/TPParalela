FLAGS = -Wall -O2 -std=c++11 -multi
DEPS = inputBuffer.hpp outputBuffer.hpp productor.hpp reconocedor.hpp escritor.hpp 
OBJ = inputBuffer.o outputBuffer.o productor.o reconocedor.o escritor.o exp.o

%.o: %.cpp $(DEPS)
	u++ $(FLAGS) -c -o $@ $<

exp: $(OBJ)
	u++ $(FLAGS) -o $@ $^

clean:
	rm -f exp *.o
.PHONY: all clean