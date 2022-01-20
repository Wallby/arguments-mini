arguments-mini: arguments_mini.o
	rm -f libarguments-mini.a
	ar rcs libarguments-mini.a arguments_mini.o

arguments_mini.o: arguments_mini.cpp
	g++ -c arguments_mini.cpp

clean:
	rm -f libarguments-mini.a arguments_mini.o
