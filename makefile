all: mandel

#TODO: O3
CFLAGS = `byteimage-config --cflags` -Wno-unused-result -g

multiwave.o: multiwave.h multiwave.cpp
	$(CXX) multiwave.cpp -c $(CFLAGS)

editor.o: multiwave.h editor.h editor.cpp
	$(CXX) editor.cpp -c $(CFLAGS)

fractal.o: complex.h grid.h multiwave.h fractal.h fractal.cpp
	$(CXX) fractal.cpp -c $(CFLAGS)

display.o: fractal.h display.h display.cpp
	$(CXX) display.cpp -c $(CFLAGS)

mandel: multiwave.o editor.o fractal.o display.o
	$(CXX) multiwave.o editor.o fractal.o display.o -o $@ `byteimage-config --libs` -lgmp -lgmpxx

clean:
	rm -f *~ *.o mandel

run: mandel
	./mandel

