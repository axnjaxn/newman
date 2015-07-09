all: mandel palettemaker

CFLAGS = `byteimage-config --cflags` -Wno-unused-result -O3

multiwave.o: multiwave.h multiwave.cpp
	$(CXX) multiwave.cpp -c $(CFLAGS)

editor.o: multiwave.h editor.h editor.cpp
	$(CXX) editor.cpp -c $(CFLAGS)

palettemaker: multiwave.h editor.h editor.o palettemaker.cpp
	$(CXX) multiwave.o editor.o palettemaker.cpp -o $@ $(CFLAGS) `byteimage-config --libs`

fractal.o: complex.h grid.h multiwave.h fractal.h fractal.cpp
	$(CXX) fractal.cpp -c $(CFLAGS)

display.o: fractal.h display.h display.cpp
	$(CXX) display.cpp -c $(CFLAGS)

mandel: multiwave.o fractal.o display.o
	$(CXX) multiwave.o fractal.o display.o -o $@ `byteimage-config --libs` -lgmp -lgmpxx

clean:
	rm -f *~ *.o mandel palettemaker

run: mandel
	./mandel

