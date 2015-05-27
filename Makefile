FLAGS = -Wall -O2
DEPS = 
OBJ = hotpotato.o

%.o: %.cpp $(DEPS)
	mpic++ $(FLAGS) -c -o $@ $<

hotpotato: $(OBJ)
	mpic++ $(FLAGS) -o $@ $^

clean:
	rm -f Hotpotato *.o
.PHONY: all clean