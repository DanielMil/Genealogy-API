CC=gcc
CFLAGS= -std=c11 -Iinclude

#UNAME Shell Variable
UNAME_S := $(shell uname -s)

# OS X Dependant Variables
ifeq ($(UNAME_S), Darwin)
    LIB := ../sharedLib.dylib
# GNU/Linux Depedant Variables
else ifeq ($(UNAME_S), Linux)
    LIB := ../sharedLib.so
endif

$(LIB): GEDCOMutilities.o GEDCOMparser.o LinkedListAPI.o
	$(CC) -shared -o $(LIB) GEDCOMparser.o GEDCOMutilities.o LinkedListAPI.o

GEDCOMutilities.o: $/src/GEDCOMutilities.c include/GEDCOMutilities.h
	$(CC) $(CFLAGS) -c -fpic $/src/GEDCOMutilities.c 

GEDCOMparser.o: $/src/GEDCOMparser.c include/GEDCOMparser.h
	$(CC) $(CFLAGS) -c -fpic $/src/GEDCOMparser.c

LinkedListAPI.o: $/src/LinkedListAPI.c 
	$(CC) $(CFLAGS) -c -fpic $/src/LinkedListAPI.c

clean:
	rm $(LIB) *.o
