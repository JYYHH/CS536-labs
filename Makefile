# define targets
target = main
targetA = mainA
targetB = mainB
targetC = mainC

# define Headers and Flags
HEADER = src/common.h
FLAGS = -o
IMPLE = src/implementation_functions.c

# define make mode
test: $(target) $(HEADER)
A: $(targetA) $(HEADER)
B: $(targetB) $(HEADER)
C: $(targetC) $(HEADER)

# define the building of targets
main: src/main.c
	gcc $(FLAGS) $@ $^ $(IMPLE)
mainA: src/mainA.c
	gcc $(FLAGS) $@ $^ $(IMPLE)
mainB: src/mainB.c
	gcc $(FLAGS) $@ $^ $(IMPLE)
mainC: src/mainC.c
	gcc $(FLAGS) $@ $^ $(IMPLE)

# define clean
clean:
	rm -rf $(target) $(targetA) $(targetB) $(targetC) *.log *.gch