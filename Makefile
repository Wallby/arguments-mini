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

arguments_mini.o: arguments_mini.cpp
	g++ -c arguments_mini.cpp

clean:
	$(call RM,arguments_mini.o)
	$(call RM,libarguments-mini.a)
	$(call RM,libarguments-mini.lib)