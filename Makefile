.PHONY: all clang lint clean doc

CC              := gcc
CLANG           := clang
VALGRIND        := valgrind

OBJDIR          := obj
DOCDIR          := docs

SRC             := $(wildcard src/*.c)

TARGET          := hhc_shell 

VALGRIND_FLAGS  := --leak-check=full --show-leak-kinds=all

CFLAGS_BASE     := -g -Wall -Wextra -fPIC -std=gnu11
CFLAGS_PRD      := -Os
CFLAGS_DBG      := -O0 -g -DCONFIG_CLI_DEBUG
CFLAGS          := $(CFLAGS_BASE) $(CFLAGS_PRD)

LDLIBS          := -lrt -lreadline -lhistory

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

doc:
	$(MAKE) -C $(DOCDIR) html

clean:
	@rm -rf *.o $(OBJDIR)/$(TARGET) 
	$(MAKE) -C $(DOCDIR) clean

mk_obj_dirs:
	@mkdir -p $(OBJDIR)
