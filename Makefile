.PHONY: all clang lint clean 

CC              := gcc
CLANG           := clang
VALGRIND        := valgrind

OBJDIR          := obj

SRC             := $(wildcard src/*.c)

TARGET          := pd_shell 

VALGRIND_FLAGS  := --leak-check=full --show-leak-kinds=all

CFLAGS_BASE     := -g -Wall -Wextra -fPIC `pkg-config --libs --cflags glib-2.0 libnm` -I src/
CFLAGS_PRD      := -Os
CFLAGS_DBG      := -O0 -g -DPD_SHELL_DEBUG
CFLAGS          := $(CFLAGS_BASE) $(CFLAGS_PRD)

LDLIBS          := -lrt

all: mk_obj_dirs $(TARGET)

debug: CFLAGS := $(CFLAGS_BASE) $(CFLAGS_DBG)
debug: all

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(OBJDIR)/$(TARGET) $(SRC) $(LDFLAGS) $(LDLIBS) 

check: $(SRC)
	$(MAKE) clang
	$(MAKE) clean
	$(MAKE) lint

clang: $(SRC)
	$(MAKE) CC=$(CLANG) all

lint: $(SRC)
	scan-build $(MAKE) CC=$(CLANG) all

testm: debug
	@echo
	@echo "RUNNING VALGRIND"
	@echo "----------------"
	$(VALGRIND) $(VALGRIND_FLAGS) $(OBJDIR)/$(TARGET)

clean:
	@rm -rf *.o $(OBJDIR)/$(TARGET) 

mk_obj_dirs:
	@mkdir -p $(OBJDIR)
