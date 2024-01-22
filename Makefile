all: a3w23

a3w23: a3w23.o
	g++ a3w23.cpp -o a3w23

clean:
	rm -f a3w23 a3w23 *.o 

tar:
	tar -cvf abbas-a3.tar *.cpp Makefile a3-ex1.dat report.pdf