main.o	: main.cpp
		g++ -c main.cpp -Wall -g -fno-inline-functions
		g++ -o main.exe main.o -g

clean	:
		rm main.o main.exe