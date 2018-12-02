/*
 * main.cpp
 *
 *  Created on: 2018-11-16
 *      Author: chentuju
 */


#include <iostream>
#include <cstring>
#include <iomanip>
#include "Mapper.h"

int main(int argc, char **argv) {

	bool printArea = false;
	bool useSRAMNotMJT = true;
	long int size1 = 0;
	long int size2 = 0;
	int maxWidth1 = 0;
	int maxWidth2 = 0;
	int ratio1 = 0;
	int ratio2 = 0;

	if(argc < 7) {
		std::cout << "[ERROR main] Too few input arguments" << std::endl;
		exit(-1);
	}
	else if(argc < 10) {
		size1 = atol(argv[4]);
		maxWidth1 = atoi(argv[5]);
		ratio1 = atoi(argv[6]);
		if(argc >= 8) {
			if(strcmp(argv[7], "-t") == 0)
				printArea = true;
			else if(strcmp(argv[7], "-mjt") == 0)
				useSRAMNotMJT = false;
		}
		if(argc == 9) {
			if(strcmp(argv[8], "-t") == 0)
				printArea = true;
			else if(strcmp(argv[8], "-mjt") == 0)
				useSRAMNotMJT = false;
		}
	}
	else if(argc < 13) {
		size1 = atol(argv[4]);
		maxWidth1 = atoi(argv[5]);
		ratio1 = atoi(argv[6]);
		size2 = atol(argv[7]);
		maxWidth2 = atoi(argv[8]);
		ratio2 = atoi(argv[9]);
		if(argc >= 11) {
			if(strcmp(argv[10], "-t") == 0)
				printArea = true;
			else if(strcmp(argv[10], "-mjt") == 0)
				useSRAMNotMJT = false;
		}
		if(argc == 12) {
			if(strcmp(argv[11], "-t") == 0)
				printArea = true;
			else if(strcmp(argv[11], "-mjt") == 0)
				useSRAMNotMJT = false;
		}
	}
	else {
		std::cout << "[ERROR main] Too many input arguments" << std::endl;
		exit(-1);
	}

	Mapper mapper;
	mapper.initMapper(argv[1], argv[2]);
	mapper.mapBRAM(size1, maxWidth1, ratio1, size2, maxWidth2, ratio2, useSRAMNotMJT);
	mapper.genFile(argv[3]);
	if(printArea)
		mapper.printArea();

	return 0;
}
