CXX=clang++
CPPFLAGS=-O2
LIBS=-lSDL2

gameoflife: main.o
	$(CXX) main.o -o gameoflife $(LIBS)

main.o: main.cpp
	$(CXX) $(CPPFLAGS) -c main.cpp
