all: mandel

mandel: main.cpp
	$(CXX) main.cpp -o $@ `byteimage-config --cflags --libs` -lgmp -lgmpxx -O3

clean:
	rm -f *~ *.o mandel

run: mandel
	./mandel

