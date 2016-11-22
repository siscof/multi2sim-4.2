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






#ifndef CMDQUEUE_H
#define CMDQUEUE_H

//CommandQueue.h
//
//Header
//

#include <tuple>
#include <map>
#include <memory>
#include <boost/circular_buffer.hpp>

#include "BusPacket.h"
#include "BankState.h"
#include "Transaction.h"
#include "SystemConfiguration.h"
#include "SimulatorObject.h"

using namespace std;

namespace DRAMSim
{


struct RankHistory
{
	typedef std::tuple <int, BusPacketType> BankTypeTuple;

	struct RankHistoryItem
	{
		long long when;
		BusPacketType type;
		unsigned int bank;
		unsigned int row;
		int core;
		int thread;

		/* We use weak pointers because due using a circular buffer,
		 * the next element could have been destroyed by the time we try to access it. */
		std::weak_ptr<RankHistoryItem> prevSameType;
		std::weak_ptr<RankHistoryItem> prevSameBank;
		std::weak_ptr<RankHistoryItem> prevSameBankType;
		std::weak_ptr<RankHistoryItem> prev;

		/* Constructor */
		RankHistoryItem(long long when, BusPacketType type, unsigned int bank, unsigned int row, int core, int thread) :
				when(when),
				type(type),
				bank(bank),
				row(row),
				core(core),
				thread(thread) {}
	};

	/* We use shared pointers in the circular buffer for being able to use weak pointers in the direct access linked lists */
	boost::circular_buffer <std::shared_ptr <RankHistoryItem>> history;

	/* Direct accessors to bank, packet type and (bank, packet type) */
	std::map <int,           std::weak_ptr <RankHistoryItem>> bankAccessor;
	std::map <BusPacketType, std::weak_ptr <RankHistoryItem>> typeAccessor;
	std::map <BankTypeTuple, std::weak_ptr <RankHistoryItem>> bankTypeAccessor;

	/* Constructor */
	RankHistory(int historyDepth) : history(historyDepth) {}


	void insert(long long when, BusPacketType type, int bank, unsigned int row, int core, int thread)
	{
		/* Create the new item as shared pointer */
		auto newItem = std::make_shared <RankHistoryItem> (when, type, bank, row, core, thread);

		auto bankTypeTuple = std::make_tuple(bank, type);

		/* Set prev */
		if (history.size())
			newItem->prev = history.front();

		/* Set prev same type */
		if (typeAccessor.count(type))
			newItem->prevSameType = typeAccessor[type];
		typeAccessor[type] = newItem;

		/* Set prev same bank */
		if (bankAccessor.count(bank))
			newItem->prevSameBank = bankAccessor[bank];
		bankAccessor[bank] = newItem;

		/* Set prev same bank and type */
		if (bankTypeAccessor.count(bankTypeTuple))
			newItem->prevSameBankType = bankTypeAccessor[bankTypeTuple];
		bankTypeAccessor[bankTypeTuple] = newItem;

		/* Insert element in the circular buffer */
		history.push_front(newItem);
	}
};


class CommandQueue : public SimulatorObject
{
	CommandQueue();
	ostream &dramsim_log;

	public:

	//typedefs
	typedef vector<BusPacket *> BusPacket1D;
	typedef vector<BusPacket1D> BusPacket2D;
	typedef vector<BusPacket2D> BusPacket3D;

	//functions
	CommandQueue(vector< vector<BankState> > &states, ostream &dramsim_log);
	virtual ~CommandQueue();

	void enqueue(BusPacket *newBusPacket);
	bool pop(BusPacket **busPacket);
	bool hasRoomFor(unsigned numberToEnqueue, unsigned rank, unsigned bank);
	bool isIssuable(BusPacket *busPacket);
	bool isEmpty(unsigned rank);
	void needRefresh(unsigned rank);
	void print();
	void update(); //SimulatorObject requirement
	vector<BusPacket *> &getCommandQueue(unsigned rank, unsigned bank);
	void recordAccess(BusPacketType type, int rank, int bank, int row, int core, int thread);

	//fields
	BusPacket3D queues; // 3D array of BusPacket pointers
	vector< vector<BankState> > &bankStates;

	private:

	void nextRankAndBank(unsigned &rank, unsigned &bank);
	void countInterthreadPenalty(BusPacket *issued, BusPacket *queued);
	bool isInterthreadRowBufferMiss(int rank, int bank, unsigned int row, int core, int thread);

	//fields
	unsigned nextBank;
	unsigned nextRank;

	unsigned nextBankPRE;
	unsigned nextRankPRE;

	unsigned refreshRank;
	bool refreshWaiting;
	bool sendAct;

	vector< vector<unsigned> > tFAWCountdown;
	vector< vector<unsigned> > rowAccessCounters;

	/* Access history */
	std::vector <boost::circular_buffer <std::tuple <int, int, int>>> rankAccesses;
	std::vector <std::vector <boost::circular_buffer <std::tuple <BusPacketType, int, int>>>> rankBankAccesses;

	std::vector <RankHistory> rankHistory;
};
}

#endif
