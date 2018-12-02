/*
 * Mapper.cpp
 *
 *  Created on: 2018-11-16
 *      Author: chentuju
 */

#include "Mapper.h"

Mapper::Mapper() {
}

Mapper::~Mapper() {

	for(unsigned int i=0; i<m_circuitList.size(); i++)
		if(m_circuitList[i])
			delete m_circuitList[i];

}

void Mapper::initMapper(const char *logicalRAMTxt, const char * logicBlockCountTxt) {

	std::ifstream file;
	char s[32];

	// parse logic_block_count.txt
	file.open(logicBlockCountTxt, std::ifstream::in);
	while(file >> s)
		if(strcmp(s, "0") == 0)
			break;
	do {
		int logicCount;
		file >> s;
		logicCount = atoi(s);
		Circuit *circuit = new Circuit();
		circuit->setCircuitInfo(0, m_circuitList.size());
		circuit->setCircuitInfo(1, logicCount);
		m_circuitList.push_back(circuit);
	} while(file >> s);
	file.close();

	// parse logical_RAM.txt
	file.open(logicalRAMTxt, std::ifstream::in);
	while(file >> s)
		if(strcmp(s, "0") == 0)
			break;
	do {
		int circuitId;
		int RAMId;
		char mode[32];
		int depth;
		int width;

		circuitId = atoi(s);
		file >> s;
		RAMId = atoi(s);
		file >> s;
		memset(mode, '\0', sizeof(mode));
		strncpy(mode, s, sizeof(mode));
		file >> s;
		depth = atoi(s);
		file >> s;
		width = atoi(s);

		if(circuitId < 0 || circuitId >= (int)m_circuitList.size()) {
			std::cout << "[ERROR Mapper::init] Circuit ID "
					<< circuitId << " is not valid" << std::endl;
			exit(-1);
		}
		m_circuitList[circuitId]->insertLogicalRAM(RAMId, mode, depth, width);
	} while(file >> s);
	file.close();
}

void Mapper::mapBRAM(long int size1, int maxWidth1, int ratio1,
		long int size2, int maxWidth2, int ratio2, bool useSRAMNotMJT) {

	struct timeval t1;
	struct timeval t2;

	gettimeofday(&t1, NULL);
	for(unsigned int i=0; i<m_circuitList.size(); i++)
		if(size2 == 0)
			m_circuitList[i]->mapSingleBRAM(size1, maxWidth1, ratio1, useSRAMNotMJT);
		else
			m_circuitList[i]->mapDualBRAM(size1, maxWidth1, ratio1,
					size2, maxWidth2, ratio2, useSRAMNotMJT);
	gettimeofday(&t2, NULL);

	double t1ms = t1.tv_sec * 1000.0 + t1.tv_usec / 1000.0;
	double t2ms = t2.tv_sec * 1000.0 + t2.tv_usec / 1000.0;
	std::cout << "RAM Mapper took " << (t2ms-t1ms) << " ms to finish" << std::endl;
}

void Mapper::genFile(const char* mappingFileTxt) {

	std::ofstream file;
	file.open(mappingFileTxt, std::ofstream::out);
	for(unsigned int i=0; i<m_circuitList.size(); i++)
		m_circuitList[i]->genFile(file);
	file.close();
}

void Mapper::printCircuitList(bool printLogicalRAM, bool printPhysicalRAM) {

	for(unsigned int i=0; i<m_circuitList.size(); i++) {
		m_circuitList[i]->printCircuit(printLogicalRAM, printPhysicalRAM);
		std::cout << std::endl;
	}
}

void Mapper::printArea() {

	long double geoAvgArea = 1.0;
	for(unsigned int i=0; i<m_circuitList.size(); i++)
		geoAvgArea *= (m_circuitList[i]->printAreaUsage() / 100000000.0);
	geoAvgArea = pow(geoAvgArea, 1.0/m_circuitList.size()) * 100000000.0;
	std::cout << "Geometric Average Area: " << std::scientific
			<< geoAvgArea << std::endl;
}
