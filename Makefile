# define targets
target = main
targetA = mainA

# define Headers and Flags
HEADER = src/common.h
FLAGS = -o
IMPLE = src/implementation_functions.c

# define make mode
test: $(target) $(HEADER)

main: src/main.c
	gcc $(FLAGS) $@ $^ $(IMPLE)

# define clean
clean:
	rm -rf $(target) $(targetA) *.log *.gch