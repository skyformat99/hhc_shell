CC              := gcc
CLANG           := clang
MEMCHECK        := valgrind
CFLAGS          := -g -Wall -Wextra -fPIC 
CPPFLAGS        := -I src/
LDFLAGS         :=
LDLIBS          := 
TARGET          := config
OBJDIR          := obj
SRC             := $(wildcard src/*.c)
MEMCHECK_FLAGS  := --leak-check=full --show-leak-kinds=all

.PHONY: clean clobber

$(TARGET): $(SRC)
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -o $(OBJDIR)/$(TARGET) $(SRC) $(CPPFLAGS) $(LDFLAGS) $(LDLIBS) 

clean:
	@rm -rf *.o $(OBJDIR)/$(TARGET) 
