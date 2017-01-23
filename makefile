CC = g++
FLAGS = -O3 -Wall -Wextra -c
LIBS = gdiplus

SOURCEDIR = src
BUILDDIR = build

EXECUTABLENAME = EngSim
EXECUTABLE = $(EXECUTABLENAME)
SOURCES = $(wildcard $(SOURCEDIR)/*.cpp)
OBJECTS = $(patsubst $(SOURCEDIR)/%.cpp,$(BUILDDIR)/%.o,$(SOURCES))

all: dir $(EXECUTABLE)

.PHONY: clean printmakevars

printmakevars:
	$(foreach v, $(.VARIABLES), $(info $(v) = $($(v))))

dir:
	mkdir -p $(BUILDDIR)

$(EXECUTABLE): $(OBJECTS)
	$(CC) -l$(LIBS) $^ -o $@

$(OBJECTS): $(BUILDDIR)/%.o : $(SOURCEDIR)/%.cpp
	$(CC) $(FLAGS) $< -o $@

clean:
	rm -f $(BUILDDIR)/*.o $(EXECUTABLE)
