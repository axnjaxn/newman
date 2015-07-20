all: newman

CFLAGS = `byteimage-config --cflags` -Wno-unused-result -O3

mandelbrot.o: mandelbrot.h mandelbrot.cpp
	$(CXX) mandelbrot.cpp -c $(CFLAGS)

multiwave.o: multiwave.h multiwave.cpp
	$(CXX) multiwave.cpp -c $(CFLAGS)

editor.o: multiwave.h editor.h editor.cpp
	$(CXX) editor.cpp -c $(CFLAGS)

viewer.o: complex.h grid.h mandelbrot.h multiwave.h viewer.h viewer.cpp
	$(CXX) viewer.cpp -c $(CFLAGS)

display.o: viewer.h display.h display.cpp
	$(CXX) display.cpp -c $(CFLAGS)

newman: mandelbrot.o multiwave.o editor.o viewer.o display.o
	$(CXX) mandelbrot.o multiwave.o editor.o viewer.o display.o -o $@ `byteimage-config --libs` -lgmp -lgmpxx

clean:
	rm -f *~ *.o newman

run: newman
	./newman

