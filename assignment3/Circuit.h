/*
 * Circuit.h
 *
 *  Created on: 2018-11-16
 *      Author: chentuju
 */

#ifndef CIRCUIT_H_
#define CIRCUIT_H_

#include <iostream>
#include <fstream>
#include <iomanip>
#include <cmath>
#include <algorithm>
#include <vector>
#include <set>
#include "LogicalRAM.h"

class LogicalRAM;

class Circuit {
public:
	Circuit();
	~Circuit();

	// functional methods
	void setCircuitInfo(int index, int num);
	void insertLogicalRAM(int id, const char *mode, int depth, int width);
	void mapSingleBRAM(long int size, int maxWidth, int ratio, bool useSRAMNotMJT);
	void mapDualBRAM(long int size1, int maxWidth1, int ratio1,
			long int size2, int maxWidth2, int ratio2, bool useSRAMNotMJT);
	void updateUsedRAMs(int mode, int numUsedRAMs);
	void updateLogicCount(int inputDecodeLogic, int outputMuxLogic);
	void updateAddSpareLogic(int addSpareLogic);
	void sanityCheck();
	void computeArea();
	void genFile(std::ofstream &file);

	// helper methods
	int getCircuitId();
	int getAvailLUTRAM();
	int getAvailBRAM();
	int getAvailBRAM2();
	int getUsedLUTRAM();
	int getUsedBRAM();
	int getUsedBRAM2();
	int getTotalLB();
	unsigned long long getTotalArea();
	static bool logicalRAMCompare(LogicalRAM* &a, LogicalRAM* &b);
	unsigned long long calculateSRAMArea(unsigned long bits, unsigned int maxWidth);

	// debugging methods
	void printCircuit(bool printLogicalRAM, bool printPhysicalRAM);
	unsigned long long printAreaUsage();

private:
	// basic circuit info
	int m_id;
	int m_logicCount;
	int m_decodeLUT;
	int m_muxLUT;
	int m_decodeMuxCount;
	int m_addSpareLogic;
	int m_addSpareUsed;

	// RAM info
	std::vector<LogicalRAM*> m_logicalRAMList;
	int m_usedLUTRAM;
	long m_sizeBRAM1;
	int m_maxWidthBRAM1;
	int m_ratioBRAM1;
	int m_usedBRAM1;
	long m_sizeBRAM2;
	int m_maxWidthBRAM2;
	int m_ratioBRAM2;
	int m_usedBRAM2;

	// Area info
	int m_finalUsedLUTRAM;
	int m_finalUsedBRAM;
	int m_finalUsedBRAM2;
	int m_finalUsedRegularLB;
	int m_finalRequiredLBTile;
	bool m_useSRAMNotMJT;
	unsigned long long m_finalArea;
};

#endif /* CIRCUIT_H_ */
