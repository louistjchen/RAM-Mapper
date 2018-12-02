/*
 * LogicalRAM.cpp
 *
 *  Created on: 2018-11-16
 *      Author: chentuju
 */

#include "LogicalRAM.h"

LogicalRAM::LogicalRAM() {

	m_id = -1;
	m_circuit = NULL;
	m_mode = -1;
	m_depth = -1;
	m_width = -1;
	m_size = -1;

	m_physicalMode = -1;
	m_physicalDepth = -1;
	m_physicalWidth = -1;
	m_physicalNumRow = -1;
	m_physicalNumCol = -1;
	m_physicalNumDecoder = -1;
	m_physicalNumMux = -1;
	m_addSpareLBAlloc = 0;

	m_sizeBRAM1 = 1024;
	m_maxWidthBRAM1 = 64;
	m_minDepthBRAM1 = 16;
	m_ratioBRAM1 = 50;
	m_sizeBRAM2 = 0;
	m_maxWidthBRAM2 = 0;
	m_minDepthBRAM2 = 0;
	m_ratioBRAM2 = 0;
	m_useSRAMNotMJT = true;
}

LogicalRAM::~LogicalRAM() {
}

void LogicalRAM::initLogicalRAM(int id, Circuit *circuit, const char *mode, int depth, int width) {

	m_id = id;
	m_circuit = circuit;
	if(strcmp(mode, "ROM") == 0)
		m_mode = LogicalRAM::ROM;
	else if(strcmp(mode, "SinglePort") == 0)
		m_mode = LogicalRAM::SinglePort;
	else if(strcmp(mode, "SimpleDualPort") == 0)
		m_mode = LogicalRAM::SimpleDualPort;
	else if(strcmp(mode, "TrueDualPort") == 0)
		m_mode = LogicalRAM::TrueDualPort;
	else {
		std::cout << "[ERROR LogicalRAM::initLogicalRAM] Input mode \""
				<< mode << "\" is not valid" << std::endl;
		exit(-1);
	}
	m_depth = depth;
	m_width = width;
	m_size = m_depth * m_width;
}

int LogicalRAM::getRAMMode() {

	return m_mode;
}

int LogicalRAM::getRAMSize() {

	return m_size;
}

int LogicalRAM::getPhysicalRAMMode() {

	return m_physicalMode;
}

void LogicalRAM::mapSingleBRAM(long int size, int maxWidth, int ratio, bool useSRAMNotMJT) {

	m_sizeBRAM1 = size;
	m_maxWidthBRAM1 = maxWidth;
	m_minDepthBRAM1 = size / maxWidth;
	m_ratioBRAM1 = ratio;
	m_useSRAMNotMJT = useSRAMNotMJT;

	int numLUTRAM = (int)ceil(m_size/640.0);
	int numBRAM = (int)ceil(m_size/(1.0*m_sizeBRAM1));
	long long areaLUTRAM = numLUTRAM * 40000;
	long long areaBRAM = calculateSRAMArea(m_sizeBRAM1, m_maxWidthBRAM1) * numBRAM;

	// use LUTRAM to allocate areaLUTRAM <= areBRAM
	if(areaLUTRAM <= areaBRAM && m_mode != LogicalRAM::TrueDualPort) {
		if(!mapAsLUTRAM(false))
			if(!mapAsLUTRAM(true)) {
				if(!mapAsBRAM1(true)) {
					std::cout << "[ERROR LogicalRAM:mapSingleBRAM] Cannot map circuit "
							<< m_circuit->getCircuitId() << " logical RAM "
							<< m_id << " with BRAM" << std::endl;
					exit(-1);
				}
			}
	}
	// use BRAM to allocate otherwise
	else {
		if(!mapAsBRAM1(true)) {
			std::cout << "[ERROR LogicalRAM:mapSingleBRAM] Cannot map circuit "
					<< m_circuit->getCircuitId() << " logical RAM "
					<< m_id << " with BRAM" << std::endl;
			exit(-1);
		}
	}

	sanityCheck();
}

void LogicalRAM::mapDualBRAM(long int size1, int maxWidth1, int ratio1,
		long int size2, int maxWidth2, int ratio2, bool useSRAMNotMJT) {

	m_sizeBRAM1 = size1;
	m_maxWidthBRAM1 = maxWidth1;
	m_minDepthBRAM1 = size1 / maxWidth1;
	m_ratioBRAM1 = ratio1;

	m_sizeBRAM2 = size2;
	m_maxWidthBRAM2 = maxWidth2;
	m_minDepthBRAM2 = size2 / maxWidth2;
	m_ratioBRAM2 = ratio2;

	m_useSRAMNotMJT = useSRAMNotMJT;

	int numLUTRAM = (int)ceil(m_size/640.0);
	int numBRAM = (int)ceil(m_size/(1.0*m_sizeBRAM1));
	int numBRAM2 = (int)ceil(m_size/(1.0*m_sizeBRAM2));
	long long areaLUTRAM = numLUTRAM * 40000;
	long long areaBRAM = calculateSRAMArea(m_sizeBRAM1, m_maxWidthBRAM1) * numBRAM;
	long long areaBRAM2 = calculateSRAMArea(m_sizeBRAM2, m_maxWidthBRAM2) * numBRAM2;

	// Non TrueDualPort
	if(m_mode <= LogicalRAM::SimpleDualPort) {
		// LUTRAM < BRAM < BRAM2
		if(areaLUTRAM <= areaBRAM && areaBRAM <= areaBRAM2) {
			if(!mapAsLUTRAM(false))
				if(!mapAsBRAM1(false))
					if(!mapAsBRAM2(false))
						mapWithRealloc();
		}
		// LUTRAM < BRAM2 < BRAM
		else if(areaLUTRAM <= areaBRAM2 && areaBRAM2 <= areaBRAM) {
			if(!mapAsLUTRAM(false))
				if(!mapAsBRAM2(false))
					if(!mapAsBRAM1(false))
						mapWithRealloc();
		}
		// BRAM < LUTRAM < BRAM2
		else if(areaBRAM <= areaLUTRAM && areaLUTRAM <= areaBRAM2) {
			if(!mapAsBRAM1(false))
				if(!mapAsLUTRAM(false))
					if(!mapAsBRAM2(false))
						mapWithRealloc();
		}
		// BRAM < BRAM2 < LUTRAM
		else if(areaBRAM <= areaBRAM2 && areaBRAM2 <= areaLUTRAM) {
			if(!mapAsBRAM1(false))
				if(!mapAsBRAM2(false))
					if(!mapAsLUTRAM(false))
						mapWithRealloc();
		}
		// BRAM2 < LUTRAM < BRAM
		else if(areaBRAM2 <= areaLUTRAM && areaLUTRAM <= areaBRAM) {
			if(!mapAsBRAM2(false))
				if(!mapAsLUTRAM(false))
					if(!mapAsBRAM1(false))
						mapWithRealloc();
		}
		// BRAM2 < BRAM < LUTRAM
		else {
			if(!mapAsBRAM2(false))
				if(!mapAsBRAM1(false))
					if(!mapAsLUTRAM(false))
						mapWithRealloc();
		}
	}
	else {
		// BRAM < BRAM2
		if(areaBRAM <= areaBRAM2) {
			if(!mapAsBRAM1(false))
				if(!mapAsBRAM2(false))
					mapWithRealloc();
		}
		// BRAM2 < BRAM
		else {
			if(!mapAsBRAM2(false))
				if(!mapAsBRAM1(false))
					mapWithRealloc();
		}
	}

	sanityCheck();
}

bool LogicalRAM::mapAsLUTRAM(bool allocate) {

	// return false immediately if exceeds 16 rows using
	// largest available physical depth - 64
	if((int)ceil(m_depth/64.0) > 16)
		return false;

	m_physicalMode = LogicalRAM::LUTRAM;

	if(m_depth <= 32) {
		m_physicalDepth = 32;
		m_physicalWidth = 640/m_physicalDepth;
		m_physicalNumRow = 1;
		m_physicalNumCol = (int)ceil(m_width/(m_physicalWidth*1.0));
	}
	else if(m_depth <= 64) {
		m_physicalDepth = 64;
		m_physicalWidth = 640/m_physicalDepth;
		m_physicalNumRow = 1;
		m_physicalNumCol = (int)ceil(m_width/(m_physicalWidth*1.0));
	}
	else {
		int mod = -1;
		int depth = 64;
		for(int i=depth; i>=32; i/=2) {
			int quotient = (int)ceil(m_depth/(i*1.0));
			int remainder = m_depth % i;
			if(remainder > mod && quotient <= 16) {
				mod = remainder;
				depth = i;
			}
		}
		m_physicalDepth = depth;
		m_physicalWidth = 640/m_physicalDepth;
		m_physicalNumRow = (int)ceil(m_depth/(m_physicalDepth*1.0));
		m_physicalNumCol = (int)ceil(m_width/(m_physicalWidth*1.0));
	}

	int availLUTRAM = m_circuit->getAvailLUTRAM() - m_circuit->getUsedLUTRAM();
	if(!allocate) {
		if(m_physicalNumRow * m_physicalNumCol > availLUTRAM)
			return false;
	}
	else {
		// allocate enough logic blocks s.t. there are enough LUTRAMs
		int numLUTRAMsNeeded = m_physicalNumRow * m_physicalNumCol;
		int numAddSpareLogicNeeded = numLUTRAMsNeeded * 2;
		if(numAddSpareLogicNeeded < 0)
			numAddSpareLogicNeeded = 0;
		m_circuit->updateAddSpareLogic(numAddSpareLogicNeeded);
		m_addSpareLBAlloc = numAddSpareLogicNeeded;

		// if number of LUTRAMs needed required is greater than available LUTRAMs
		availLUTRAM = m_circuit->getAvailLUTRAM() - m_circuit->getUsedLUTRAM();
		if(m_physicalNumRow * m_physicalNumCol > availLUTRAM) {
			std::cout << "[ERROR LogicalRAM::mapAsLUTRAMWithRealloc] Allocation of "
					<< numAddSpareLogicNeeded << " spare LBs not enough" << std::endl;
			m_circuit->updateAddSpareLogic(-1*numAddSpareLogicNeeded);
			exit(-1);
		}
	}

	// update # of rows/columns of physical RAMs
	updateNumDecoderMux();
	// update circuit used RAM
	m_circuit->updateUsedRAMs(LogicalRAM::LUTRAM, m_physicalNumRow*m_physicalNumCol);
	// update circuit logic count
	m_circuit->updateLogicCount(m_physicalNumDecoder, m_physicalNumMux);
	return true;
}

bool LogicalRAM::mapAsBRAM1(bool allocate) {

	// return false immediately if exceeds 16 rows using
	// largest available physical depth - m_sizeBRAM1
	if((int)ceil(m_depth/(1.0*m_sizeBRAM1)) > 16)
		return false;

	m_physicalMode = LogicalRAM::BRAM1;

	bool set = false;
	for(int depth=m_minDepthBRAM1; depth<=m_sizeBRAM1; depth*=2) {
		if(m_depth <= m_minDepthBRAM1 && m_mode != LogicalRAM::TrueDualPort) {
			m_physicalDepth = depth;
			m_physicalWidth = m_sizeBRAM1/m_physicalDepth;
			m_physicalNumRow = 1;
			m_physicalNumCol = (int)ceil(m_width/(m_physicalWidth*1.0));
			set = true;
			break;
		}
		else if(depth != m_minDepthBRAM1 && m_depth <= depth) {
			m_physicalDepth = depth;
			m_physicalWidth = m_sizeBRAM1/m_physicalDepth;
			m_physicalNumRow = 1;
			m_physicalNumCol = (int)ceil(m_width/(m_physicalWidth*1.0));
			set = true;
			break;
		}
	}
	if(!set) {
		int mod = -1;
		int depth = m_sizeBRAM1;
		int stop = (m_mode == LogicalRAM::TrueDualPort) ?
				m_minDepthBRAM1 * 2 : m_minDepthBRAM1;
		for(int i=depth; i>=stop; i/=2) {
			int quotient = (int)ceil(m_depth/(i*1.0));
			int remainder = m_depth % i;
			if(remainder > mod && quotient <= 16) {
				mod = remainder;
				depth = i;
			}
		}
		m_physicalDepth = depth;
		m_physicalWidth = m_sizeBRAM1/m_physicalDepth;
		m_physicalNumRow = (int)ceil(m_depth/(m_physicalDepth*1.0));
		m_physicalNumCol = (int)ceil(m_width/(m_physicalWidth*1.0));
		set = true;
	}

	int availBRAM = m_circuit->getAvailBRAM() - m_circuit->getUsedBRAM();
	if(!allocate) {
		if(m_physicalNumRow * m_physicalNumCol > availBRAM)
			return false;
	}
	else {
		// allocate enough logic blocks if insufficient
		int numBRAMsNeeded = m_physicalNumRow * m_physicalNumCol;
		numBRAMsNeeded -= (m_circuit->getAvailBRAM() - m_circuit->getUsedBRAM());
		int numAddSpareLogicNeeded = m_ratioBRAM1 * numBRAMsNeeded -
				(m_circuit->getTotalLB() % m_ratioBRAM1);
		if(numAddSpareLogicNeeded < 0)
			numAddSpareLogicNeeded = 0;
		m_circuit->updateAddSpareLogic(numAddSpareLogicNeeded);
		m_addSpareLBAlloc = numAddSpareLogicNeeded;

		// if number of BRAMs needed required is greater than available BRAM1
		availBRAM = m_circuit->getAvailBRAM() - m_circuit->getUsedBRAM();
		if(m_physicalNumRow * m_physicalNumCol > availBRAM) {
			std::cout << "[ERROR LogicalRAM::mapAsBRAM1] Allocation of "
					<< numAddSpareLogicNeeded << " spare LBs not enough" << std::endl;
			m_circuit->updateAddSpareLogic(-1*numAddSpareLogicNeeded);
			exit(-1);
		}
	}

	// update # of rows/columns of physical RAMs
	updateNumDecoderMux();
	// update circuit used RAM
	m_circuit->updateUsedRAMs(LogicalRAM::BRAM1, m_physicalNumRow*m_physicalNumCol);
	// update circuit logic count
	m_circuit->updateLogicCount(m_physicalNumDecoder, m_physicalNumMux);
	return true;
}

bool LogicalRAM::mapAsBRAM2(bool allocate) {

	// return false immediately if exceeds 16 rows using
	// largest available physical depth - m_sizeBRAM1
	if((int)ceil(m_depth/(1.0*m_sizeBRAM2)) > 16)
		return false;

	m_physicalMode = LogicalRAM::BRAM2;

	bool set = false;
	for(int depth=m_minDepthBRAM2; depth<=m_sizeBRAM2; depth*=2) {
		if(m_depth <= m_minDepthBRAM2 && m_mode != LogicalRAM::TrueDualPort) {
			m_physicalDepth = depth;
			m_physicalWidth = m_sizeBRAM2/m_physicalDepth;
			m_physicalNumRow = 1;
			m_physicalNumCol = (int)ceil(m_width/(m_physicalWidth*1.0));
			set = true;
			break;
		}
		else if(depth != m_minDepthBRAM2 && m_depth <= depth) {
			m_physicalDepth = depth;
			m_physicalWidth = m_sizeBRAM2/m_physicalDepth;
			m_physicalNumRow = 1;
			m_physicalNumCol = (int)ceil(m_width/(m_physicalWidth*1.0));
			set = true;
			break;
		}
	}
	if(!set) {
		int mod = -1;
		int depth = m_sizeBRAM2;
		int stop = (m_mode == LogicalRAM::TrueDualPort) ?
				m_minDepthBRAM2 * 2 : m_minDepthBRAM2;
		for(int i=depth; i>=stop; i/=2) {
			int quotient = (int)ceil(m_depth/(i*1.0));
			int remainder = m_depth % i;
			if(remainder > mod && quotient <= 16) {
				mod = remainder;
				depth = i;
			}
		}
		m_physicalDepth = depth;
		m_physicalWidth = m_sizeBRAM2/m_physicalDepth;
		m_physicalNumRow = (int)ceil(m_depth/(m_physicalDepth*1.0));
		m_physicalNumCol = (int)ceil(m_width/(m_physicalWidth*1.0));
		set = true;
	}

	int availBRAM = m_circuit->getAvailBRAM2() - m_circuit->getUsedBRAM2();
	if(!allocate)
		if(m_physicalNumRow * m_physicalNumCol > availBRAM)
			return false;

	// allocate enough logic blocks if insufficient
	int numBRAMsNeeded = m_physicalNumRow * m_physicalNumCol;
	numBRAMsNeeded -= (m_circuit->getAvailBRAM2() - m_circuit->getUsedBRAM2());
	int numAddSpareLogicNeeded = m_ratioBRAM2 * numBRAMsNeeded -
			(m_circuit->getTotalLB() % m_ratioBRAM2);
	if(numAddSpareLogicNeeded < 0)
		numAddSpareLogicNeeded = 0;
	m_circuit->updateAddSpareLogic(numAddSpareLogicNeeded);
	m_addSpareLBAlloc = numAddSpareLogicNeeded;

	// if number of BRAM2s needed required is greater than available BRAM2
	availBRAM = m_circuit->getAvailBRAM2() - m_circuit->getUsedBRAM2();
	if(m_physicalNumRow * m_physicalNumCol > availBRAM) {
		std::cout << "[ERROR LogicalRAM::mapAsBRAM2] Allocation of "
				<< numAddSpareLogicNeeded << " spare LBs not enough" << std::endl;
		m_circuit->updateAddSpareLogic(-1*numAddSpareLogicNeeded);
		exit(-1);
	}

	// update # of rows/columns of physical RAMs
	updateNumDecoderMux();
	// update circuit used RAM2
	m_circuit->updateUsedRAMs(LogicalRAM::BRAM2, m_physicalNumRow*m_physicalNumCol);
	// update circuit logic count
	m_circuit->updateLogicCount(m_physicalNumDecoder, m_physicalNumMux);
	return true;
}

void LogicalRAM::mapWithRealloc() {

	int numLUTRAM = (int)ceil(m_size/640.0);
	int numBRAM = (int)ceil(m_size/(1.0*m_sizeBRAM1));
	int numBRAM2 = (int)ceil(m_size/(1.0*m_sizeBRAM2));
	long long areaLUTRAM = numLUTRAM * 75000;
	long long areaBRAM = calculateSRAMArea(m_sizeBRAM1, m_maxWidthBRAM1) * numBRAM;
	long long areaBRAM2 = calculateSRAMArea(m_sizeBRAM2, m_maxWidthBRAM2) * numBRAM2;
	areaBRAM += numBRAM * m_ratioBRAM1 * 37500;
	areaBRAM2 += numBRAM2 * m_ratioBRAM2 * 37500;

	// Non TrueDualPort
	if(m_mode <= LogicalRAM::SimpleDualPort) {
		// LUTRAM < BRAM < BRAM2
		if(areaLUTRAM <= areaBRAM && areaBRAM <= areaBRAM2) {
			if(!mapAsLUTRAM(true))
				if(!mapAsBRAM1(true))
					if(!mapAsBRAM2(true)) {
						std::cout << "[ERROR LogicalRAM:mapWithRealloc] Cannot map circuit "
								<< m_circuit->getCircuitId() << " logical RAM "
								<< m_id << std::endl;
						exit(-1);
					}
		}
		// LUTRAM < BRAM2 < BRAM
		else if(areaLUTRAM <= areaBRAM2 && areaBRAM2 <= areaBRAM) {
			if(!mapAsLUTRAM(true))
				if(!mapAsBRAM2(true))
					if(!mapAsBRAM1(true)) {
						std::cout << "[ERROR LogicalRAM:mapWithRealloc] Cannot map circuit "
								<< m_circuit->getCircuitId() << " logical RAM "
								<< m_id << std::endl;
						exit(-1);
					}
		}
		// BRAM < LUTRAM < BRAM2
		else if(areaBRAM <= areaLUTRAM && areaLUTRAM <= areaBRAM2) {
			if(!mapAsBRAM1(true))
				if(!mapAsLUTRAM(true))
					if(!mapAsBRAM2(true)) {
						std::cout << "[ERROR LogicalRAM:mapWithRealloc] Cannot map circuit "
								<< m_circuit->getCircuitId() << " logical RAM "
								<< m_id << std::endl;
						exit(-1);
					}
		}
		// BRAM < BRAM2 < LUTRAM
		else if(areaBRAM <= areaBRAM2 && areaBRAM2 <= areaLUTRAM) {
			if(!mapAsBRAM1(true))
				if(!mapAsBRAM2(true))
					if(!mapAsLUTRAM(true)) {
						std::cout << "[ERROR LogicalRAM:mapWithRealloc] Cannot map circuit "
								<< m_circuit->getCircuitId() << " logical RAM "
								<< m_id << std::endl;
						exit(-1);
					}
		}
		// BRAM2 < LUTRAM < BRAM
		else if(areaBRAM2 <= areaLUTRAM && areaLUTRAM <= areaBRAM) {
			if(!mapAsBRAM2(true))
				if(!mapAsLUTRAM(true))
					if(!mapAsBRAM1(true)) {
						std::cout << "[ERROR LogicalRAM:mapWithRealloc] Cannot map circuit "
								<< m_circuit->getCircuitId() << " logical RAM "
								<< m_id << std::endl;
						exit(-1);
					}
		}
		// BRAM2 < BRAM < LUTRAM
		else {
			if(!mapAsBRAM2(true))
				if(!mapAsBRAM1(true))
					if(!mapAsLUTRAM(true)) {
						std::cout << "[ERROR LogicalRAM:mapWithRealloc] Cannot map circuit "
								<< m_circuit->getCircuitId() << " logical RAM "
								<< m_id << std::endl;
						exit(-1);
					}
		}
	}
	else {
		// BRAM < BRAM2
		if(areaBRAM <= areaBRAM2) {
			if(!mapAsBRAM1(true))
				if(!mapAsBRAM2(true)) {
					std::cout << "[ERROR LogicalRAM:mapWithRealloc] Cannot map circuit "
							<< m_circuit->getCircuitId() << " logical RAM "
							<< m_id << " with BRAM" << std::endl;
					exit(-1);
				}
		}
		// BRAM2 < BRAM
		else {
			if(!mapAsBRAM2(true))
				if(!mapAsBRAM1(true)) {
					std::cout << "[ERROR LogicalRAM:mapWithRealloc] Cannot map circuit "
							<< m_circuit->getCircuitId() << " logical RAM "
							<< m_id << " with BRAM2" << std::endl;
					exit(-1);
				}
		}
	}
}

unsigned long long LogicalRAM::calculateSRAMArea(unsigned long bits, unsigned int maxWidth) {

	unsigned long long area;
	if(m_useSRAMNotMJT)
		area = 9000 + 5 * bits + 90 * sqrt(bits) + 1200 * maxWidth;
	else
		area = 9000 + 1.25 * bits + 90 * sqrt(bits) + 1200 * maxWidth;
	return area;
}

void LogicalRAM::updateNumDecoderMux() {

	if(m_physicalNumRow <= 2)
		m_physicalNumDecoder = m_physicalNumRow - 1;
	else if(m_physicalNumRow <= 16)
		m_physicalNumDecoder = m_physicalNumRow;
	else {
		printLogicalRAM();
		printPhysicalRAM();
		m_circuit->printCircuit(false, false);
		std::cout << "[ERROR LogicalRAM::updateNumDecoderMux] Number of rows of physical RAMs "
				<< m_physicalNumRow << " is invalid" << std::endl;
		exit(-1);
	}

	if(m_physicalNumRow == 1)
		m_physicalNumMux = 0;
	else if(m_physicalNumRow >= 2 && m_physicalNumRow <= 4)
		m_physicalNumMux = m_width;
	else if(m_physicalNumRow >= 5 && m_physicalNumRow <= 7)
		m_physicalNumMux = 2 * m_width;
	else if(m_physicalNumRow >= 8 && m_physicalNumRow <= 10)
		m_physicalNumMux = 3 * m_width;
	else if(m_physicalNumRow >= 11 && m_physicalNumRow <= 13)
		m_physicalNumMux = 4 * m_width;
	else if(m_physicalNumRow >= 14 && m_physicalNumRow <= 16)
		m_physicalNumMux = 5 * m_width;

	else {
		std::cout << "[ERROR LogicalRAM::updateNumDecoderMux] Number of rows of physical RAMs "
				<< m_physicalNumRow << " is invalid" << std::endl;
		exit(-1);
	}

	if(m_mode == LogicalRAM::TrueDualPort) {
		m_physicalNumDecoder *= 2;
		m_physicalNumMux *= 2;
	}
}

void LogicalRAM::sanityCheck() {

	// check if physical RAM size is enough
	int physicalRAMSize = m_physicalNumRow * m_physicalNumCol *
			m_physicalDepth * m_physicalWidth;
	if(physicalRAMSize < m_size) {
		std::cout << "[ERROR LogicalRAM::sanityCheck] Physical RAM size "
				<< physicalRAMSize << " is smaller than logical RAM size "
				<< m_size << std::endl;
		exit(-1);
	}

	// check if # of rows has exceeded 16
	if(m_physicalNumRow > 16) {
		std::cout << "[ERROR LogicalRAM::sanityCheck] Number of rows "
				<< m_physicalNumRow << " has exceeded 16" << std::endl;
		exit(-1);
	}

	// check if TrueDualPort has been mapped with LUTRAM
	if(m_mode == LogicalRAM::TrueDualPort && m_physicalMode == LogicalRAM::LUTRAM) {
		std::cout << "[ERROR LogicalRAM::sanityCheck] TrueDualPort RAM has been "
				<< "mapped with LUTRAM"<< std::endl;
		exit(-1);
	}

	// check if TrueDualPort has invalid RAM width size
	if(m_mode == LogicalRAM::TrueDualPort && m_physicalMode == LogicalRAM::BRAM1)
		if(m_physicalWidth > m_maxWidthBRAM1 / 2) {
			std::cout << "[ERROR LogicalRAM::sanityCheck] TrueDualPort RAM has been "
					<< "mapped into BRAM1 with invalid width size "
					<< m_physicalWidth << " greater than "
					<< m_maxWidthBRAM1/2 << std::endl;
			exit(-1);
		}
	if(m_mode == LogicalRAM::TrueDualPort && m_physicalMode == LogicalRAM::BRAM2)
		if(m_physicalWidth > m_maxWidthBRAM2 / 2) {
			std::cout << "[ERROR LogicalRAM::sanityCheck] TrueDualPort RAM has been "
					<< "mapped into BRAM2 with invalid width size "
					<< m_physicalWidth << " greater than "
					<< m_maxWidthBRAM2/2 << std::endl;
			exit(-1);
		}
}

void LogicalRAM::genFile(std::ofstream &file) {

	// circuit ID and RAM ID
	file << m_circuit->getCircuitId() << " " << m_id << " ";
	// number of additional LUTs used
	int numLUT = m_physicalNumDecoder + m_physicalNumMux;
	file << numLUT << " ";
	// logical width and depth
	file << "LW " << m_width << " LD " << m_depth << " ";
	// physical RAM ID (set to logical RAM ID)
	file << "ID " << m_id << " ";
	// number of rows/columns (series/parallel)
	file << "S " << m_physicalNumRow << " P " << m_physicalNumCol << " ";
	// physical RAM type (BRAM = 1, LUTRAM = 2)
	switch(m_physicalMode) {
	case LogicalRAM::LUTRAM:
		file << "Type 1 ";
		break;
	case LogicalRAM::BRAM1:
		file << "Type 2 ";
		break;
	case LogicalRAM::BRAM2:
		file << "Type 3 ";
		break;
	default:
		std::cout << "[ERROR LogicalRAM::genFile] Physical RAM mode "
				<< m_physicalMode << " is invalid" << std::endl;
		exit(-1);
	}
	// logical RAM type (ROM, SinglePort, SimpleDualPort, TrueDualPort)
	file << "Mode ";
	switch(m_mode) {
	case LogicalRAM::ROM:
		file << "ROM ";
		break;
	case LogicalRAM::SinglePort:
		file << "SinglePort ";
		break;
	case LogicalRAM::SimpleDualPort:
		file << "SimpleDualPort ";
		break;
	case LogicalRAM::TrueDualPort:
		file << "TrueDualPort ";
		break;
	default:
		std::cout << "[ERROR LogicalRAM::genFile] Logical RAM mode "
				<< m_mode << " is invalid" << std::endl;
		exit(-1);
	}
	// physical RAM width/depth
	file << "W " << m_physicalWidth << " D " << m_physicalDepth << std::endl;
}

void LogicalRAM::printLogicalRAM() {

	std::cout << "(Logical RAM ID: " << std::setw(5) << m_id
			<< "; Associated Circuit ID: " << std::setw(3) << m_circuit->getCircuitId()
			<< "; Mode: ";
	if(m_mode == LogicalRAM::ROM)
		std::cout << "           ROM";
	else if(m_mode == LogicalRAM::SinglePort)
		std::cout << "    SinglePort";
	else if(m_mode == LogicalRAM::SimpleDualPort)
		std::cout << "SimpleDualPort";
	else if(m_mode == LogicalRAM::TrueDualPort)
		std::cout << "  TrueDualPort";
	else {
		std::cout << "[ERROR LogicalRAM::printLogicalRAM] Logical RAM Mode "
				<< m_mode << " is not valid" << std::endl;
		exit(-1);
	}
	std::cout << "; Depth: " << std::setw(4) << m_depth
			<< "; Width: " << std::setw(4) << m_width
			<< "; Size: " << std::setw(4) << m_size
			<< ")" << std::endl;
}

void LogicalRAM::printPhysicalRAM() {

	std::cout << "(Physical RAM Mode: ";
	if(m_physicalMode == LogicalRAM::LUTRAM)
		std::cout << "LULRAM";
	else if(m_physicalMode == LogicalRAM::BRAM1)
		std::cout << " BRAM1";
	else if(m_physicalMode == LogicalRAM::BRAM2)
		std::cout << " BRAM2";
	else {
		std::cout << "[ERROR LogicalRAM::printPhysicalRAM] Physical RAM Mode "
				<< m_physicalMode << " is not valid" << std::endl;
		exit(-1);
	}
	std::cout << "; # RAMs: " << std::setw(4) << m_physicalNumRow * m_physicalNumCol
			<< "; Depth: " << std::setw(6) << m_physicalDepth
			<< "; Width: " << std::setw(3) << m_physicalWidth
			<< "; Rol: " << std::setw(3) << m_physicalNumRow
			<< "; Col: " << std::setw(3) << m_physicalNumCol
			<< "; Decoder: " << std::setw(2) << m_physicalNumDecoder
			<< "; Mux: " << std::setw(2) << m_physicalNumMux
			<< "; AddSpareLogic: " << std::setw(2) << m_addSpareLBAlloc
			<< ")" << std::endl;
}
