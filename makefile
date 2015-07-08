all: mandel palettemaker test

CFLAGS = `byteimage-config --cflags` -Wno-unused-result -O3

multiwave.o: multiwave.h multiwave.cpp
	$(CXX) multiwave.cpp -c $(CFLAGS)

main.o: complex.h grid.h multiwave.h main.cpp
	$(CXX) main.cpp -c $(CFLAGS)

mandel: multiwave.o main.o
	$(CXX) multiwave.o main.o -o $@ `byteimage-config --libs` -lgmp -lgmpxx

editor.o: multiwave.h editor.h editor.cpp
	$(CXX) editor.cpp -c $(CFLAGS)

palettemaker: multiwave.h editor.h editor.o palettemaker.cpp
	$(CXX) multiwave.o editor.o palettemaker.cpp -o $@ $(CFLAGS) `byteimage-config --libs`

fractal.o: complex.h grid.h multiwave.h fractal.h fractal.cpp
	$(CXX) fractal.cpp -c $(CFLAGS)

test: multiwave.o fractal.o
	$(CXX) multiwave.o fractal.o -o $@ `byteimage-config --libs` -lgmp -lgmpxx

clean:
	rm -f *~ *.o mandel palettemaker test

run: mandel
	./mandel

