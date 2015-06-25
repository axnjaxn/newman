all: mandel

mandel: main.cpp
	$(CXX) main.cpp -o $@ `byteimage-config --cflags --libs` -lgmp -lgmpxx -Wno-unused-result -O3

clean:
	rm -f *~ *.o mandel

run: mandel
	./mandel

