ifndef OS # linux
LIBRARY_EXTENSION=.a
RM=rm -f $(1)
else ifeq ($(OS), Windows_NT) # windows
LIBRARY_EXTENSION=.lib
RM=if exist $(1) del $(1)
else
$(error os not supported)
endif

libarguments-mini$(LIBRARY_EXTENSION): arguments_mini.o
	$(call RM,libarguments-mini$(LIBRARY_EXTENSION))
	ar rcs libarguments-mini$(LIBRARY_EXTENSION) arguments_mini.o

arguments_mini.o: arguments_mini.cpp arguments_mini.h
	g++ -c arguments_mini.cpp

test$(EXECUTABLE_EXTENSION): test.o libarguments-mini$(LIBRARY_EXTENSION)
	gcc -Wl,--wrap=malloc,--wrap=free -o test$(EXECUTABLE_EXTENSION) test.o -L./ -larguments-mini

test.o: test.c
	gcc -c test.c -I./

#******************************************************************************

release: test$(EXECUTABLE_EXTENSION)
	./test$(EXECUTABLE_EXTENSION)

clean:
	$(call RM,arguments_mini.o)
	$(call RM,libarguments-mini.a)
	$(call RM,libarguments-mini.lib)
	
	$(call RM,test.o)
	$(call RM,test)
	$(call RM,test.exe)