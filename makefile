all: mandel

multiwave.o: multiwave.h multiwave.cpp
	$(CXX) multiwave.cpp -c `byteimage-config --cflags` -Wno-unused-result -O3

main.o: multiwave.h main.cpp
	$(CXX) main.cpp -c `byteimage-config --cflags` -Wno-unused-result -O3

mandel: multiwave.o main.o
	$(CXX) multiwave.o main.o -o $@ `byteimage-config --libs` -lgmp -lgmpxx

clean:
	rm -f *~ *.o mandel

run: mandel
	./mandel

