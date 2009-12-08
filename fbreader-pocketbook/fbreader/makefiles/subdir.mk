include $(ROOTDIR)/makefiles/config.mk

INCLUDE = $(ZINCLUDE) $(EXTERNALINCLUDE)

HEADERS = $(wildcard *.h)
SOURCES = $(wildcard *.cpp)
OBJECTS = $(patsubst %.cpp, %.o, $(SOURCES))

.SUFFIXES: .cpp .o .h

.cpp.o:
	@echo -n "Compiling $@ ..."
	@$(CC) -MMD -c $(CFLAGS) $(INCLUDE) $<
	@echo " OK"

all: $(OBJECTS)

clean:
	@$(RM) *.o *.s *.ld *.d

-include *.d
