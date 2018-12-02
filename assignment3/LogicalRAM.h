/*
 * LogicalRAM.h
 *
 *  Created on: 2018-11-16
 *      Author: chentuju
 */

#ifndef LOGICALRAM_H_
#define LOGICALRAM_H_

#include <iostream>
#include <fstream>
#include <cstring>
#include <iomanip>
#include "Circuit.h"

class Circuit;

class LogicalRAM {
public:
	LogicalRAM();
	~LogicalRAM();

	// functional methods
	void initLogicalRAM(int id, Circuit *circuit, const char *mode, int depth, int width);
	int getRAMMode();
	int getRAMSize();
	int getPhysicalRAMMode();
	void mapSingleBRAM(long int size, int maxWidth, int ratio, bool useSRAMNotMJT);
	void mapDualBRAM(long int size1, int maxWidth1, int ratio1,
			long int size2, int maxWidth2, int ratio2, bool useSRAMNotMJT);

	// helper methods
	bool mapAsLUTRAM(bool allocate);
	bool mapAsBRAM1(bool allocate);
	bool mapAsBRAM2(bool allocate);
	void mapWithRealloc();
	unsigned long long calculateSRAMArea(unsigned long bits, unsigned int maxWidth);
	void updateNumDecoderMux();
	void sanityCheck();
	void genFile(std::ofstream &file);

	// debugging methods
	void printLogicalRAM();
	void printPhysicalRAM();

	// enum for logical/physical RAM mode
	enum {
		ROM,
		SinglePort,
		SimpleDualPort,
		TrueDualPort,
		LUTRAM,
		BRAM1,
		BRAM2
	};

private:
	// logical RAM data members
	int m_id;
	Circuit *m_circuit;
	int m_mode;
	int m_depth;
	int m_width;
	int m_size;

	// physical RAM data members
	int m_physicalMode;
	int m_physicalDepth;
	int m_physicalWidth;
	int m_physicalNumRow;
	int m_physicalNumCol;
	int m_physicalNumDecoder;
	int m_physicalNumMux;
	int m_addSpareLBAlloc;

	// single/dual BRAM mapping
	long int m_sizeBRAM1;
	int m_maxWidthBRAM1;
	int m_minDepthBRAM1;
	int m_ratioBRAM1;
	long int m_sizeBRAM2;
	int m_maxWidthBRAM2;
	int m_minDepthBRAM2;
	int m_ratioBRAM2;
	bool m_useSRAMNotMJT;
};

#endif /* LOGICALRAM_H_ */
