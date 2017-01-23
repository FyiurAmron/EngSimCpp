CC = g++
#FLAGS = -c -O3 -Wall -Wextra
FLAGS = -c -O3 -Wall -Wno-unused-variable -Wno-unused-local-typedefs
#LIBS = -Llib -lopenblas.dll -lcomdlg32 -lgdi32 -lgdiplus
LIBS = -Llib -lcbia.lib.lapack.dyn.rel.x86.12 -lcomdlg32 -lgdi32 -lgdiplus
INCLUDES = -Iinclude

SOURCEDIR = src
BUILDDIR = build

EXECUTABLENAME = EngSim
EXECUTABLE = bin/$(EXECUTABLENAME)
SOURCES = $(wildcard $(SOURCEDIR)/*.cpp)
OBJECTS = $(patsubst $(SOURCEDIR)/%.cpp,$(BUILDDIR)/%.o,$(SOURCES))

all: dir $(EXECUTABLE)

.PHONY: clean printmakevars

printmakevars:
	$(foreach v, $(.VARIABLES), $(info $(v) = $($(v))))

dir:
	mkdir -p $(BUILDDIR)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $^ -o $@ $(LIBS)

$(OBJECTS): $(BUILDDIR)/%.o : $(SOURCEDIR)/%.cpp
	$(CC) $(INCLUDES) $(FLAGS) $< -o $@

clean:
	rm -f $(BUILDDIR)/*.o $(EXECUTABLE)
