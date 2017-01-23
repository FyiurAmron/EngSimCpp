CC = g++
#FLAGS = -c -O3 -Wall -Wextra
FLAGS = -c -O3 -Wall -Wno-unused-variable -Wno-unused-local-typedefs
LIBS = -lcomdlg32 -lgdi32 -lgdiplus -Llib -llapack_win32_MT
INCLUDES = -Iinclude

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
	$(CC) $^ -o $@ $(LIBS)

$(OBJECTS): $(BUILDDIR)/%.o : $(SOURCEDIR)/%.cpp
	$(CC) $(INCLUDES) $(FLAGS) $< -o $@

clean:
	rm -f $(BUILDDIR)/*.o $(EXECUTABLE)
