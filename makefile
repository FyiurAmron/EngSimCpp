CC = g++
#FLAGS = -c -O3 -Wall -Wextra
FLAGS = -c -O3 -Wall -Wno-unused-variable -Wno-unused-local-typedefs
DEPFLAGS = -MT $@ -MMD -MP -MF $(BUILDDIR)/$*.d
#LIBS = -Llib -lopenblas.dll -lcomdlg32 -lgdi32 -lgdiplus
LIBS = -Llib -lcbia.lib.lapack.dyn.rel.x86.12 -lcomdlg32 -lgdi32 -lgdiplus
INCLUDES = -Iinclude

SOURCEDIR = src
BUILDDIR = build

EXECUTABLENAME = EngSim
EXECUTABLE = bin/$(EXECUTABLENAME)
SOURCES = $(wildcard $(SOURCEDIR)/*.cpp)
OBJECTS = $(patsubst $(SOURCEDIR)/%.cpp,$(BUILDDIR)/%.o,$(SOURCES))
DEPS := $(OBJECTS:.o=.d)

.PHONY: all buildrun run clean printmakevars dir

all: dir $(EXECUTABLE)

printmakevars:
	$(foreach v, $(.VARIABLES), $(info $(v) = $($(v))))

dir:
	mkdir -p $(BUILDDIR)

clean:
	rm -f $(BUILDDIR)/*.o $(EXECUTABLE) $(EXECUTABLE).exe

buildrun: all run

run:
	$(EXECUTABLE)

$(EXECUTABLE) : $(OBJECTS)
	$(CC) $^ -o $@ $(LIBS)

#.c.o
-include $(DEPS)

$(OBJECTS) : $(BUILDDIR)/%.o : $(SOURCEDIR)/%.cpp
	$(CC) $(INCLUDES) $(FLAGS) $(DEPFLAGS) $< -o $@

