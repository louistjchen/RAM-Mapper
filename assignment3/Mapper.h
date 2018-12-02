/*
 * Mapper.h
 *
 *  Created on: 2018-11-16
 *      Author: chentuju
 */

#ifndef MAPPER_H_
#define MAPPER_H_

#include <iostream>
#include <fstream>
#include <cstring>
#include <sys/time.h>
#include <vector>
#include "Circuit.h"
#include "LogicalRAM.h"

class Mapper {
public:
	Mapper();
	~Mapper();

	// main methods
	void initMapper(const char* logicalRAMTxt, const char* logicBlockCountTxt);
	void mapBRAM(long int size1, int maxWidth1, int ratio1,
			long int size2, int maxWidth2, int ratio2, bool useSRAMNotMJT);
	void genFile(const char* mappingFileTxt);

	// debugging methods
	void printCircuitList(bool printLogicalRAM, bool printPhysicalRAM);
	void printArea();

private:
	std::vector<Circuit*> m_circuitList;
};

#endif /* MAPPER_H_ */
