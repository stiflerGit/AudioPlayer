SRCDIR = src
INCDIR = inc
OBJDIR = obj
TESTDIR = test

TARGET = player
#------------------------------------------------
CFLAGS = -Wall -g
CPPFLAGS = -I./$(INCDIR)
LDLIBS = -lrt -lfftw3f -lm
LDTEST = -lcriterion
LDFLAGS = -pthread
GLIBS = `allegro-config --libs`
MAIN = main.o
#------------------------------------------------

SOURCES := $(shell find $(SRCDIR) -name '*.c')
OBJECTS := $(patsubst $(SRCDIR)/%,$(OBJDIR)/%,$(SOURCES:.c=.o))
TEST_SOURCES := $(shell find $(TESTDIR) -name '*.c')
TEST_OBJECTS := $(patsubst $(TEST_SOURCES)/%,$(OBJDIR)/%,$(TEST_SOURCES:.c=.o))
DEP := $(filter-out obj/main.o,$(OBJECTS))

$(TARGET): $(OBJECTS)
	$(CC) -o $@ $^ $(CPPFLAGS) $(LDFLAGS) $(LDLIBS) $(GLIBS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	mkdir -p $(dir $@)
	$(CC) -o $@ -c $^ $(CPPFLAGS) $(LDFLAGS) $(LDLIBS) $(GLIBS)

test: $(DEP) $(TEST_OBJECTS)
	$(CC) -o $(TARGET)_test $^ $(CPPFLAGS) $(LDFLAGS) $(LDTEST) $(LDLIBS) $(GLIBS)

clean:
	@rm -rf $(OBJDIR)
	@rm -f $(TARGET) $(TARGET)_test
	@rm -f $(TEST_OBJECTS)

.PHONY: clean
