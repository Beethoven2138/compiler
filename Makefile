GCCPARAMS = -Iinclude -g

OBJECTS=obj/main.o \
	obj/file.o \
	obj/lex.o \
	obj/parse.o \
	obj/asm.o \

obj/%.o: src/%.c
	mkdir -p $(@D)
	gcc $(GCCPARAMS) -c -o $@ $<

all: compiler

compiler: $(OBJECTS)
	gcc $(OBJECTS) -o compiler $(GCCPARAMS)

clean:
	rm -rf compiler obj
