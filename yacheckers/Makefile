CXXFLAGS = `fltk-config --use-images --cxxflags`
LIBS = `fltk-config --use-images --ldflags`
CPPFILES = src/checkers.cc \
	src/rcheckers.cc\
	src/echeckers.cc\
	src/flcheckers.cc

#	src/pdn.cc\


OBJ   = $(CPPFILES:.cc=.o)

all: flchekers


flchekers: $(OBJ) 
	$(CXX) -o $@ $^ $(LIBS) $(CXXFLAGS) 

