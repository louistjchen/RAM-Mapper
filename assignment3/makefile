# This is the makefile for ECE1756 Assignment 3 RAM Mapper
# Developer: 	Louis Chen
# Date:		Nov 22, 2018

all:
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -std=c++0x -o main.o main.cpp
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -std=c++0x -o Mapper.o Mapper.cpp
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -std=c++0x -o Circuit.o Circuit.cpp
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -std=c++0x -o LogicalRAM.o LogicalRAM.cpp
	g++ -o RAM-Mapper main.o Mapper.o Circuit.o LogicalRAM.o

clean:
	rm -f main.o Mapper.o Circuit.o LogicalRAM.o RAM-Mapper
