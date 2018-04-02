SRC = $(wildcard src/*.c)
OBJ = $(SRC:src/%.c=obj/%.o)
CFLAGS = -fsanitize=address -include src/precompiled.h -c
LDFLAGS = -fsanitize=address -o

all: dirs xorator

dirs:
	@mkdir -p obj bin

xorator: precompiled.h.pch $(OBJ)
	gcc $(LDFLAGS) bin/$@ $(OBJ)
	#------------------------------------------

$(OBJ): obj/%.o: src/%.c
	check/checkpatch.pl --no-tree --fix -f $^
	#------------------------------------------
	cppcheck $^
	#------------------------------------------
	gcc $(CFLAGS) $^ -o $@
	#------------------------------------------

precompiled.h.pch: src/precompiled.h
	gcc $< -o $@
	#------------------------------------------