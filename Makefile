all: IdeaH

builtin.o: builtin.c builtin.h
	gcc -c builtin.c

parser.o: parser.c parser.h builtin.h
	gcc -c parser.c

IdeaH.o: IdeaH.c IdeaH.h parser.h
	gcc -c IdeaH.c

IdeaH: builtin.o parser.o IdeaH.o
	gcc builtin.o parser.o IdeaH.o -o IdeaH

clean:
	rm *.o IdeaH

cleanbkp:
	rm *~
