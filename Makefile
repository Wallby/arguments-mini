arguments-mini: arguments_mini.o
	if exist libarguments-mini.lib del libarguments-mini.lib
	ar rcs libarguments-mini.lib arguments_mini.o

arguments_mini.o: arguments_mini.cpp
	g++ -c arguments_mini.cpp

clean:
	if exist libarguments-mini.lib del libarguments-mini.lib
	if exist arguments_mini.o del arguments_mini.o
# if linux leftover.. also remove linux leftover..
	if exist libarguments-mini.a del libarguments-mini.a