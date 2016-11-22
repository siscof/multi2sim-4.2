/*********************************************************************************
*  Copyright (c) 2010-2011, Elliott Cooper-Balis
*                             Paul Rosenfeld
*                             Bruce Jacob
*                             University of Maryland
*                             dramninjas [at] gmail [dot] com
*  All rights reserved.
*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions are met:
*
*     * Redistributions of source code must retain the above copyright notice,
*        this list of conditions and the following disclaimer.
*
*     * Redistributions in binary form must reproduce the above copyright notice,
*        this list of conditions and the following disclaimer in the documentation
*        and/or other materials provided with the distribution.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
*  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
*  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
*  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
*  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
*  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
*  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
*  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
*  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
*  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*********************************************************************************/


#ifndef MEMORYCONTROLLER_H
#define MEMORYCONTROLLER_H

//MemoryController.h
//
//Header file for memory controller object
//

#include "SimulatorObject.h"
#include "Transaction.h"
#include "SystemConfiguration.h"
#include "CommandQueue.h"
#include "BusPacket.h"
#include "BankState.h"
#include "Rank.h"
#include "CSVWriter.h"
#include <map>

using namespace std;

namespace DRAMSim
{
class MemorySystem;
class MemoryController : public SimulatorObject
{

public:
	//functions
	MemoryController(MemorySystem* ms, CSVWriter &csvOut_, ostream &dramsim_log_);
	virtual ~MemoryController();

	bool addTransaction(Transaction *trans);
	bool WillAcceptTransaction();
	void returnReadData(const Transaction *trans);
	void receiveFromBus(BusPacket *bpacket);
	void attachRanks(vector<Rank *> *ranks);
	void update();
	void printStats(bool finalStats = false);
	void resetStats();

	uint64_t getBWC(int core);
	uint64_t getBWN(int core);
	uint64_t getBWNO(int core);
	void setBWC(int core, uint64_t value);
	void setBWN(int core, uint64_t value);
	void setBWNO(int core, uint64_t value);

	//fields
	vector<Transaction *> transactionQueue;

private:
	//functions
	void insertHistogram(unsigned latencyValue, unsigned rank, unsigned bank);
	void printLatencyHistogram(ostream &out);
	void printStatsLog(bool finalStats);
	void estimateBandwidthRequirements();

	//output file
	CSVWriter &csvOut;
	ostream &dramsim_log;

	//fields
	MemorySystem *parentMemorySystem;
	vector< vector <BankState> > bankStates;
	CommandQueue commandQueue;
	BusPacket *poppedBusPacket;
	vector<unsigned>refreshCountdown;
	vector<BusPacket *> writeDataToSend;
	vector<unsigned> writeDataCountdown;
	vector<Transaction *> returnTransaction;
	vector<Transaction *> pendingReadTransactions;
	map<unsigned,unsigned> latencies; // latencyValue -> latencyCount
	vector<bool> powerDown;

	vector<Rank *> *ranks;

	// these packets are counting down waiting to be transmitted on the "bus"
	BusPacket *outgoingCmdPacket;
	unsigned cmdCyclesLeft;
	BusPacket *outgoingDataPacket;
	unsigned dataCyclesLeft;

	uint64_t totalTransactions;
	uint64_t totalRowBufferHits;
	uint64_t totalRowBufferMisses;

	vector<uint64_t> grandTotalBankReads; //Accumulated. Never cleared.
	vector<uint64_t> grandTotalBankWrites; //Accumulated. Never cleared.

	vector<uint64_t> cyclesAllBanksIdle; //Cleared
	vector<uint64_t> totalReadsPerBank; //Cleared
	vector<uint64_t> totalWritesPerBank; //Cleared
	vector<uint64_t> totalEpochLatency; //Cleared

	vector<uint64_t> rowBufferHitsPerBank; //Cleared
	vector<uint64_t> rowBufferMissesPerBank; //Cleared

	/* Bandwidth Consumed by Core */
	map<int,uint64_t> bwc; //Cleared
	/* Bandwidth Needed by Core */
	map<int,uint64_t> bwn; //Cleared
	/* Bandwidth Needed by Others */
	map<int,uint64_t> bwno; //Cleared

	unsigned channelBitWidth;
	unsigned rankBitWidth;
	unsigned bankBitWidth;
	unsigned rowBitWidth;
	unsigned colBitWidth;
	unsigned byteOffsetWidth;


	unsigned refreshRank;

public:
	// energy values are per rank -- SST uses these directly, so make these public
	vector< uint64_t > backgroundEnergy;
	vector< uint64_t > burstEnergy;
	vector< uint64_t > actpreEnergy;
	vector< uint64_t > refreshEnergy;

};
}

#endif

