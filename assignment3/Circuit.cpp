/*
 * Circuit.cpp
 *
 *  Created on: 2018-11-16
 *      Author: chentuju
 */

#include "Circuit.h"

Circuit::Circuit() {

	m_id = -1;
	m_logicCount = 0;
	m_decodeLUT = 0;
	m_muxLUT = 0;
	m_decodeMuxCount = 0;
	m_addSpareLogic = 0;
	m_addSpareUsed = 0;

	m_usedLUTRAM = 0;
	m_sizeBRAM1 = 1024;
	m_maxWidthBRAM1 = 64;
	m_ratioBRAM1 = 50;
	m_usedBRAM1 = 0;
	m_sizeBRAM2 = 1024;
	m_maxWidthBRAM2 = 64;
	m_ratioBRAM2 = 50;
	m_usedBRAM2 = 0;

	m_finalUsedLUTRAM = 0;
	m_finalUsedBRAM = 0;
	m_finalUsedBRAM2 = 0;
	m_finalUsedRegularLB = 0;
	m_finalRequiredLBTile = 0;
	m_useSRAMNotMJT = true;
	m_finalArea = 0;
}

Circuit::~Circuit() {

	for(unsigned int i=0; i<m_logicalRAMList.size(); i++)
		if(m_logicalRAMList[i])
			delete m_logicalRAMList[i];
}

void Circuit::setCircuitInfo(int index, int num) {

	switch(index) {
	case 0:
		m_id = num;
		break;
	case 1:
		m_logicCount = num;
		break;
	default:
		std::cout << "[ERROR Circuit::setCircuitInfo] Index "
		<< index << " is not valid" << std::endl;
		exit(-1);
	}
}

void Circuit::insertLogicalRAM(int id, const char *mode, int depth, int width) {

	LogicalRAM *logicalRAM = new LogicalRAM();
	logicalRAM->initLogicalRAM(id, this, mode, depth, width);
	m_logicalRAMList.push_back(logicalRAM);
}

void Circuit::mapSingleBRAM(long int size, int maxWidth, int ratio, bool useSRAMNotMJT) {

	m_sizeBRAM1 = size;
	m_maxWidthBRAM1 = maxWidth;
	m_ratioBRAM1 = ratio;
	m_useSRAMNotMJT = useSRAMNotMJT;

	m_usedLUTRAM = 0;
	m_usedBRAM1 = 0;
	m_decodeMuxCount = 0;

	// sort logical RAMs with descending RAM size to achieve minimum area
	sort(m_logicalRAMList.begin(), m_logicalRAMList.end(), Circuit::logicalRAMCompare);

	// loop through all logical RAMs and map each of them
	for(unsigned int i=0; i<m_logicalRAMList.size(); i++)
		m_logicalRAMList[i]->mapSingleBRAM(size, maxWidth, ratio, useSRAMNotMJT);

	sanityCheck();
	computeArea();
}

void Circuit::mapDualBRAM(long int size1, int maxWidth1, int ratio1,
		long int size2, int maxWidth2, int ratio2, bool useSRAMNotMJT) {

	m_sizeBRAM1 = size1;
	m_maxWidthBRAM1 = maxWidth1;
	m_ratioBRAM1 = ratio1;
	m_sizeBRAM2 = size2;
	m_maxWidthBRAM2 = maxWidth2;
	m_ratioBRAM2 = ratio2;
	m_useSRAMNotMJT = useSRAMNotMJT;

	m_usedLUTRAM = 0;
	m_usedBRAM1 = 0;
	m_decodeMuxCount = 0;

	// sort logical RAMs with descending RAM size to achieve minimum area
	sort(m_logicalRAMList.begin(), m_logicalRAMList.end(), Circuit::logicalRAMCompare);

	// loop through all logical RAMs and map each of them
	for(unsigned int i=0; i<m_logicalRAMList.size(); i++)
		m_logicalRAMList[i]->mapDualBRAM(size1, maxWidth1, ratio1,
				size2, maxWidth2, ratio2, useSRAMNotMJT);

	sanityCheck();
	computeArea();
}

void Circuit::updateUsedRAMs(int mode, int numUsedRAMs) {

	switch(mode) {
	case LogicalRAM::LUTRAM:
		if(getUsedLUTRAM() + numUsedRAMs <= getAvailLUTRAM()) {
			while(numUsedRAMs > 0 && m_addSpareUsed < (int)(m_addSpareLogic/2)) {
				m_addSpareUsed++;
				numUsedRAMs--;
			}
			if(numUsedRAMs > 0) {
				if(getUsedLUTRAM() + numUsedRAMs <= getAvailLUTRAM())
					m_usedLUTRAM += numUsedRAMs;
				else {
					std::cout << "[ERROR Circuit::updateUsedRAMs] Number of RAMs used "
							<< numUsedRAMs << " (after allocated to spare logic) "
							<< "has exceeded available LUTRAMs" << std::endl;
					exit(-1);
				}
			}
			break;
		}
		else {
			std::cout << "[ERROR Circuit::updateUsedRAMs] Number of RAMs used "
					<< numUsedRAMs << " has exceeded available LUTRAMs" << std::endl;
			exit(-1);
		}
	case LogicalRAM::BRAM1:
		if(m_usedBRAM1 + numUsedRAMs <= getAvailBRAM()) {
			m_usedBRAM1 += numUsedRAMs;
			break;
		}
		else {
			std::cout << "[ERROR Circuit::updateUsedRAMs] Number of RAMs used "
					<< numUsedRAMs << " has exceeded available BRAMs" << std::endl;
			exit(-1);
		}
	case LogicalRAM::BRAM2:
		if(m_usedBRAM2 + numUsedRAMs <= getAvailBRAM2()) {
			m_usedBRAM2 += numUsedRAMs;
			break;
		}
		else {
			std::cout << "[ERROR Circuit::updateUsedRAMs] Number of RAMs used "
					<< numUsedRAMs << " has exceeded available BRAM2s" << std::endl;
			exit(-1);
		}
	default:
		std::cout << "[ERROR Circuit::updateUsedRAMs] Input mode "
			<< mode << " is invalid" << std::endl;
		exit(-1);
	}
}

void Circuit::updateLogicCount(int inputDecodeLogic, int outputMuxLogic) {

	m_decodeLUT += inputDecodeLogic;
	m_muxLUT += outputMuxLogic;
	m_decodeMuxCount = (int)ceil((m_decodeLUT + m_muxLUT) / 10.0);
}

void Circuit::updateAddSpareLogic(int addSpareLogic) {

	m_addSpareLogic += addSpareLogic;
}

void Circuit::sanityCheck() {

	if(getUsedLUTRAM() > getAvailLUTRAM()) {
		std::cout << "[ERROR Circuit::sanityCheck] Number of used LUTRAMs "
				<< getUsedLUTRAM() << " has exceeded available number "
				<< getAvailLUTRAM() << std::endl;
		exit(-1);
	}
	if(m_addSpareUsed > m_addSpareLogic / 2) {
		std::cout << "[ERROR Circuit::sanityCheck] Number of used spare LUTRAMs "
				<< m_addSpareUsed << " has exceeded half of available number "
				<< m_addSpareLogic << std::endl;
		exit(-1);
	}
	if(getUsedBRAM() > getAvailBRAM()) {
		std::cout << "[ERROR Circuit::sanityCheck] Number of used BRAM1 "
				<< getUsedBRAM() << " has exceeded available number "
				<< getAvailBRAM() << std::endl;
		exit(-1);
	}
	if(getUsedBRAM2() > getAvailBRAM2()) {
		std::cout << "[ERROR Circuit::sanityCheck] Number of used BRAM2 "
				<< getUsedBRAM2() << " has exceeded available number "
				<< getAvailBRAM2() << std::endl;
		exit(-1);
	}
}

void Circuit::computeArea() {

	m_finalUsedLUTRAM = m_usedLUTRAM + m_addSpareUsed;
	m_finalUsedBRAM = m_usedBRAM1;
	m_finalUsedBRAM2 = m_usedBRAM2;
	m_finalUsedRegularLB = m_logicCount + m_decodeMuxCount;

	m_finalRequiredLBTile = std::max(m_finalUsedLUTRAM*2, m_finalUsedLUTRAM+m_finalUsedRegularLB);
	m_finalRequiredLBTile = std::max(m_finalRequiredLBTile, m_finalUsedBRAM*m_ratioBRAM1);
	m_finalRequiredLBTile = std::max(m_finalRequiredLBTile, m_finalUsedBRAM2*m_ratioBRAM2);

	m_finalArea = m_finalRequiredLBTile * 37500 +
			calculateSRAMArea(m_sizeBRAM1, m_maxWidthBRAM1) * (m_finalRequiredLBTile/m_ratioBRAM1) +
			calculateSRAMArea(m_sizeBRAM2, m_maxWidthBRAM2) * (m_finalRequiredLBTile/m_ratioBRAM2);
}

void Circuit::genFile(std::ofstream &file) {

	for(unsigned int i=0; i<m_logicalRAMList.size(); i++)
		m_logicalRAMList[i]->genFile(file);
}

int Circuit::getCircuitId() {

	return m_id;
}

int Circuit::getAvailLUTRAM() {

	return (m_logicCount + m_decodeMuxCount + (int)(m_addSpareLogic/2));
}

int Circuit::getAvailBRAM() {

	return getTotalLB() / m_ratioBRAM1;
}

int Circuit::getAvailBRAM2() {

	return getTotalLB() / m_ratioBRAM2;
}

int Circuit::getUsedLUTRAM() {

	return m_usedLUTRAM + m_addSpareUsed;
}

int Circuit::getUsedBRAM() {

	return m_usedBRAM1;
}

int Circuit::getUsedBRAM2() {

	return m_usedBRAM2;
}

int Circuit::getTotalLB() {

	return m_logicCount + m_decodeMuxCount + m_addSpareLogic + m_usedLUTRAM;
}

unsigned long long Circuit::getTotalArea() {

	return m_finalArea;
}

bool Circuit::logicalRAMCompare(LogicalRAM* &a, LogicalRAM* &b) {

	return a->getRAMSize() > b->getRAMSize();
}

unsigned long long Circuit::calculateSRAMArea(unsigned long bits, unsigned int maxWidth) {

	unsigned long long area;
	if(m_useSRAMNotMJT)
		area = 9000 + 5 * bits + 90 * sqrt(bits) + 1200 * maxWidth;
	else
		area = 9000 + 1.25 * bits + 90 * sqrt(bits) + 1200 * maxWidth;
	return area;
}

void Circuit::printCircuit(bool printLogicalRAM, bool printPhysicalRAM) {

	std::cout << "(Circuit ID: " << std::setw(3) << m_id
			<< "; Logic: " << std::setw(6) << m_logicCount
			<< "; DecodeMux: " << std::setw(4) << m_decodeMuxCount
			<< "; SpareLogic: " << std::setw(4) << m_addSpareLogic
			<< "; SpareUsed: " << std::setw(4) << m_addSpareUsed
			<< "; Avail LUTRAM: " << std::setw(5) << getAvailLUTRAM()
			<< "; Avail BRAM1: " << std::setw(5) << getAvailBRAM()
			<< "; Avail BRAM2: " << std::setw(3) << getAvailBRAM2()
			<< "; Used LUTRAM: " << std::setw(5) << m_usedLUTRAM
			<< "; Used BRAM1: " << std::setw(5) << m_usedBRAM1
			<< "; Used BRAM2: " << std::setw(3) << m_usedBRAM2
			<< ")" << std::endl;
	if(printLogicalRAM || printPhysicalRAM) {
		for(unsigned int i=0; i<m_logicalRAMList.size(); i++) {
			if(printLogicalRAM) {
				m_logicalRAMList[i]->printLogicalRAM();
				std::cout << std::endl;
			}
			if(printPhysicalRAM) {
				m_logicalRAMList[i]->printPhysicalRAM();
				std::cout << std::endl;
			}
		}
	}
}

unsigned long long Circuit::printAreaUsage() {

	std::cout << "(Circuit ID: " << std::setw(3) << m_id
			<< "; # LUTRAM: " << std::setw(4) << m_finalUsedLUTRAM
			<< "; # BRAM1: " << std::setw(4) << m_finalUsedBRAM
			<< "; # BRAM2: " << std::setw(4) << m_finalUsedBRAM2
			<< "; # Regular LB : " << std::setw(5) << m_finalUsedRegularLB
			<< "; # Required Tiles: " << std::setw(5) << m_finalRequiredLBTile
			<< "; Total FPGA Area: "<< std::scientific << (double)m_finalArea
			<< std::endl;
	return m_finalArea;
}

