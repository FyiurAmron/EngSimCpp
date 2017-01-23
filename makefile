# define a sample name for the application's name 
target := EngSim 
 
# define build options 
# compile options 
CXXFLAGS :=
# link options 
LDFLAGS := -L../../../library
# link libraries 
LDLIBS :=
#-lTCP -lboost_thread etc.
 
# construct list of .cpp and their corresponding .o and .d files 
sources  := $(wildcard *.cpp) 
includes := -I../../../library
objects  := $(sources:.cpp=.o) 
dep_file := Makefile.dep
 
# file disambiguity is achieved via the '.PHONY' directive 
.PHONY : all clean 


# main goal for 'make' is the first target, here 'all' 
# 'all' is always assumed to be a target, and not a file 
# usage: 'make all' or simply 'make', since this is the first target 
#
all : $(target) 
 

# rule for 'target' 
# the automatic variable '$<' expands to the first prerequisite (objects) 
# the automatic variable '$@' expands to the target's name 
# the actual action done is the linking of all .o files into the executable 
#
$(target) : $(objects)
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@ 


# even if there exists a file 'clean', 'make clean' will execute its commands 
# and won't ever assume 'clean' is an up-to-date file 
# usage: 'make clean' 
#
clean : 
	$(RM) $(target) $(dep_file) $(objects)
 

# rule for creating .o files
#
.cpp.o :
	$(CXX) $(CXXFLAGS) $(includes) -c $< -o $@ 
 

# rule to make dependencies within one file
#
depend $(dep_file):
	@echo Makefile - creating dependencies for: $(sources)
	@$(RM) $(dep_file)
	@$(CXX) -E -MM $(includes) $(sources) >> $(dep_file)


# include dependency file only when 'make clean' is not specified 
# this is done to prevent recreating the dependency file when the clean target is specified 
# if no dependencies exist, on the other hand, no warning will be issued:'-' key 
#
ifeq (,$(findstring clean,$(MAKECMDGOALS)))
-include $(dep_file)
endif