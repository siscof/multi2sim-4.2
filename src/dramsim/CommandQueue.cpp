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








//CommandQueue.cpp
//
//Class file for command queue object
//

#include "BusPacket.h"
#include "CommandQueue.h"
#include "MemoryController.h"
#include <assert.h>

using namespace DRAMSim;


CommandQueue::CommandQueue(vector< vector<BankState> > &states, ostream &dramsim_log_) :
		dramsim_log(dramsim_log_),
		bankStates(states),
		nextBank(0),
		nextRank(0),
		nextBankPRE(0),
		nextRankPRE(0),
		refreshRank(0),
		refreshWaiting(false),
		sendAct(true),
		rankAccesses(
				NUM_RANKS,
				boost::circular_buffer <std::tuple <int, int, int>> (
					max(NUM_BANKS, TOTAL_ROW_ACCESSES)
				)),
		rankBankAccesses(
				NUM_RANKS,
				std::vector <boost::circular_buffer <std::tuple <BusPacketType, int, int>>> (
					NUM_BANKS,
					boost::circular_buffer <std::tuple <BusPacketType, int, int>>
					(
						max(NUM_BANKS, TOTAL_ROW_ACCESSES)
					)
				)),
		rankHistory(NUM_RANKS, RankHistory(4096))
{
	//set here to avoid compile errors
	currentClockCycle = 0;

	//use numBankQueus below to create queue structure
	size_t numBankQueues;
	if (queuingStructure==PerRank)
	{
		numBankQueues = 1;
	}
	else if (queuingStructure==PerRankPerBank)
	{
		numBankQueues = NUM_BANKS;
	}
	else
	{
		ERROR("== Error - Unknown queuing structure");
		exit(0);
	}

	//vector of counters used to ensure rows don't stay open too long
	rowAccessCounters = vector< vector<unsigned> >(NUM_RANKS, vector<unsigned>(NUM_BANKS,0));

	//create queue based on the structure we want
	BusPacket1D actualQueue;
	BusPacket2D perBankQueue = BusPacket2D();
	queues = BusPacket3D();
	for (size_t rank=0; rank<NUM_RANKS; rank++)
	{
		//this loop will run only once for per-rank and NUM_BANKS times for per-rank-per-bank
		for (size_t bank=0; bank<numBankQueues; bank++)
		{
			actualQueue	= BusPacket1D();
			perBankQueue.push_back(actualQueue);
		}
		queues.push_back(perBankQueue);
	}


	//FOUR-bank activation window
	//	this will count the number of activations within a given window
	//	(decrementing counter)
	//
	//countdown vector will have decrementing counters starting at tFAW
	//  when the 0th element reaches 0, remove it
	tFAWCountdown.reserve(NUM_RANKS);
	for (size_t i=0;i<NUM_RANKS;i++)
	{
		//init the empty vectors here so we don't seg fault later
		tFAWCountdown.push_back(vector<unsigned>());
	}
}
CommandQueue::~CommandQueue()
{
	//ERROR("COMMAND QUEUE destructor");
	size_t bankMax = NUM_RANKS;
	if (queuingStructure == PerRank) {
		bankMax = 1;
	}
	for (size_t r=0; r< NUM_RANKS; r++)
	{
		for (size_t b=0; b<bankMax; b++)
		{
			for (size_t i=0; i<queues[r][b].size(); i++)
			{
				delete(queues[r][b][i]);
			}
			queues[r][b].clear();
		}
	}
}
//Adds a command to appropriate queue
void CommandQueue::enqueue(BusPacket *newBusPacket)
{
	unsigned rank = newBusPacket->rank;
	unsigned bank = newBusPacket->bank;
	if (queuingStructure==PerRank)
	{
		queues[rank][0].push_back(newBusPacket);
		if (queues[rank][0].size()>CMD_QUEUE_DEPTH)
		{
			ERROR("== Error - Enqueued more than allowed in command queue");
			ERROR("						Need to call .hasRoomFor(int numberToEnqueue, unsigned rank, unsigned bank) first");
			exit(0);
		}
	}
	else if (queuingStructure==PerRankPerBank)
	{
		queues[rank][bank].push_back(newBusPacket);
		if (queues[rank][bank].size()>CMD_QUEUE_DEPTH)
		{
			ERROR("== Error - Enqueued more than allowed in command queue");
			ERROR("						Need to call .hasRoomFor(int numberToEnqueue, unsigned rank, unsigned bank) first");
			exit(0);
		}
	}
	else
	{
		ERROR("== Error - Unknown queuing structure");
		exit(0);
	}
}

//Removes the next item from the command queue based on the system's
//command scheduling policy
bool CommandQueue::pop(BusPacket **busPacket)
{
	*busPacket = nullptr;

	//this can be done here because pop() is called every clock cycle by the parent MemoryController
	//	figures out the sliding window requirement for tFAW
	//
	//deal with tFAW book-keeping
	//	each rank has it's own counter since the restriction is on a device level
	for (size_t i=0;i<NUM_RANKS;i++)
	{
		//decrement all the counters we have going
		for (size_t j=0;j<tFAWCountdown[i].size();j++)
		{
			tFAWCountdown[i][j]--;
		}

		//the head will always be the smallest counter, so check if it has reached 0
		if (tFAWCountdown[i].size()>0 && tFAWCountdown[i][0]==0)
		{
			tFAWCountdown[i].erase(tFAWCountdown[i].begin());
		}
	}

	/* Now we need to find a packet to issue. When the code picks a packet, it will set
		 *busPacket = [some eligible packet]

		 First the code looks if any refreshes need to go
		 Then it looks for data packets
		 Otherwise, it starts looking for rows to close (in open page)
	*/

	if (rowBufferPolicy == ClosePage)
	{
		bool sendingREF = false;
		//if the memory controller set the flags signaling that we need to issue a refresh
		if (refreshWaiting)
		{
			bool foundActiveOrTooEarly = false;
			//look for an open bank
			for (size_t b=0;b<NUM_BANKS;b++)
			{
				vector<BusPacket *> &queue = getCommandQueue(refreshRank,b);
				//checks to make sure that all banks are idle
				if (bankStates[refreshRank][b].currentBankState == RowActive)
				{
					foundActiveOrTooEarly = true;
					//if the bank is open, make sure there is nothing else
					// going there before we close it
					for (size_t j=0;j<queue.size();j++)
					{
						BusPacket *packet = queue[j];
						if (packet->row == bankStates[refreshRank][b].openRowAddress &&
								packet->bank == b)
						{
							if (packet->busPacketType != ACTIVATE && isIssuable(packet))
							{
								*busPacket = packet;
								queue.erase(queue.begin() + j);
								sendingREF = true;
							}
							break;
						}
					}

					break;
				}
				//	NOTE: checks nextActivate time for each bank to make sure tRP is being
				//				satisfied.	the next ACT and next REF can be issued at the same
				//				point in the future, so just use nextActivate field instead of
				//				creating a nextRefresh field
				else if (bankStates[refreshRank][b].nextActivate > currentClockCycle)
				{
					foundActiveOrTooEarly = true;
					break;
				}
			}

			//if there are no open banks and timing has been met, send out the refresh
			//	reset flags and rank pointer
			if (!foundActiveOrTooEarly && bankStates[refreshRank][0].currentBankState != PowerDown)
			{
				*busPacket = new BusPacket(REFRESH, 0, 0, 0, refreshRank, 0, 0, dramsim_log);
				refreshRank = -1;
				refreshWaiting = false;
				sendingREF = true;
			}
		} // refreshWaiting

		//if we're not sending a REF, proceed as normal
		if (!sendingREF)
		{
			bool foundIssuable = false;
			unsigned startingRank = nextRank;
			unsigned startingBank = nextBank;
			do
			{
				vector<BusPacket *> &queue = getCommandQueue(nextRank, nextBank);
				//make sure there is something in this queue first
				//	also make sure a rank isn't waiting for a refresh
				//	if a rank is waiting for a refesh, don't issue anything to it until the
				//		refresh logic above has sent one out (ie, letting banks close)
				if (!queue.empty() && !((nextRank == refreshRank) && refreshWaiting))
				{
					/* Per rank round robin command queues */
					if (queuingStructure == PerRank)
					{
						//search from beginning to find first issuable bus packet
						for (size_t i=0;i<queue.size();i++)
						{
							if (isIssuable(queue[i]))
							{
								//check to make sure we aren't removing a read/write that is paired with an activate
								if (i>0 && queue[i-1]->busPacketType==ACTIVATE &&
										queue[i-1]->physicalAddress == queue[i]->physicalAddress)
									continue;

								*busPacket = queue[i];
								queue.erase(queue.begin()+i);
								foundIssuable = true;
								break;
							}
						}

						/* Count the cicles waiting due interthread interference */
						if (!queue.empty())
							countInterthreadPenalty(*busPacket, queue.front());
					}

					/* Per rank and per bank round robin command queues */
					else
					{
						if (isIssuable(queue[0]))
						{
							//no need to search because if the front can't be sent,
							// then no chance something behind it can go instead
							*busPacket = queue[0];
							queue.erase(queue.begin());
							foundIssuable = true;
						}

						/* Count the cicles waiting due interthread interference */
						if (!queue.empty())
							countInterthreadPenalty(*busPacket, queue.front());
					}
				}

				//if we found something, break out of do-while
				if (foundIssuable) break;

				/* Rank round robin */
				if (queuingStructure == PerRank)
				{
					nextRank = (nextRank + 1) % NUM_RANKS;
					if (startingRank == nextRank)
						break;
				}

				/* Rank bank round robin */
				else
				{
					nextRankAndBank(nextRank, nextBank);
					if (startingRank == nextRank && startingBank == nextBank)
						break;
				}
			}
			while (true);

			//if we couldn't find anything to send, return false
			if (!foundIssuable) return false;
		}
	}

	else if (rowBufferPolicy == OpenPage)
	{
		bool sendingREForPRE = false;
		if (refreshWaiting)
		{
			bool sendREF = true;
			//make sure all banks idle and timing met for a REF
			for (size_t b=0;b<NUM_BANKS;b++)
			{
				//if a bank is active we can't send a REF yet
				if (bankStates[refreshRank][b].currentBankState == RowActive)
				{
					sendREF = false;
					bool closeRow = true;
					//search for commands going to an open row
					vector <BusPacket *> &refreshQueue = getCommandQueue(refreshRank,b);

					for (size_t j=0;j<refreshQueue.size();j++)
					{
						BusPacket *packet = refreshQueue[j];
						//if a command in the queue is going to the same row . . .
						if (bankStates[refreshRank][b].openRowAddress == packet->row &&
								b == packet->bank)
						{
							// . . . and is not an activate . . .
							if (packet->busPacketType != ACTIVATE)
							{
								closeRow = false;
								// . . . and can be issued . . .
								if (isIssuable(packet))
								{
									/* Row buffer miss */
									assert(packet->busPacketType == READ || packet->busPacketType == WRITE);
									packet->rowBufferHit = 0;

									//send it out
									*busPacket = packet;
									refreshQueue.erase(refreshQueue.begin()+j);
									sendingREForPRE = true;
								}
								break;
							}
							else //command is an activate
							{
								//if we've encountered another act, no other command will be of interest
								break;
							}
						}
					}

					//if the bank is open and we are allowed to close it, then send a PRE
					if (closeRow && currentClockCycle >= bankStates[refreshRank][b].nextPrecharge)
					{
						rowAccessCounters[refreshRank][b]=0;
						*busPacket = new BusPacket(PRECHARGE, 0, 0, 0, refreshRank, b, 0, dramsim_log);
						sendingREForPRE = true;
					}
					break;
				}
				//	NOTE: the next ACT and next REF can be issued at the same
				//				point in the future, so just use nextActivate field instead of
				//				creating a nextRefresh field
				else if (bankStates[refreshRank][b].nextActivate > currentClockCycle) //and this bank doesn't have an open row
				{
					sendREF = false;
					break;
				}
			}

			//if there are no open banks and timing has been met, send out the refresh
			//	reset flags and rank pointer
			if (sendREF && bankStates[refreshRank][0].currentBankState != PowerDown)
			{
				*busPacket = new BusPacket(REFRESH, 0, 0, 0, refreshRank, 0, 0, dramsim_log);
				refreshRank = -1;
				refreshWaiting = false;
				sendingREForPRE = true;
			}
		}

		if (!sendingREForPRE)
		{
			unsigned startingRank = nextRank;
			unsigned startingBank = nextBank;
			bool foundIssuable = false;
			do // round robin over queues
			{
				vector<BusPacket *> &queue = getCommandQueue(nextRank,nextBank);
				//make sure there is something there first
				if (!queue.empty() && !((nextRank == refreshRank) && refreshWaiting))
				{
					bool interthreadRowBufferMiss = false;
					//search from the beginning to find first issuable bus packet
					for (size_t i=0;i<queue.size();i++)
					{
						BusPacket *packet = queue[i];
						if (isIssuable(packet))
						{
							//check for dependencies
							bool dependencyFound = false;
							for (size_t j=0;j<i;j++)
							{
								BusPacket *prevPacket = queue[j];
								if (prevPacket->busPacketType != ACTIVATE &&
										prevPacket->bank == packet->bank &&
										prevPacket->row == packet->row)
								{
									dependencyFound = true;
									break;
								}
							}
							if (dependencyFound) continue;

							*busPacket = packet;

							//if the bus packet before is an activate, that is the act that was
							//	paired with the column access we are removing, so we have to remove
							//	that activate as well (check i>0 because if i==0 then theres nothing before it)
							if (i>0 && queue[i-1]->busPacketType == ACTIVATE)
							{
								rowAccessCounters[(*busPacket)->rank][(*busPacket)->bank]++;
								// i is being returned, but i-1 is being thrown away, so must delete it here
								delete (queue[i-1]);

								// remove both i-1 (the activate) and i (we've saved the pointer in *busPacket)
								queue.erase(queue.begin()+i-1,queue.begin()+i+1);

								/* Row buffer hit */
								assert((*busPacket)->busPacketType == READ || (*busPacket)->busPacketType == WRITE);
								packet->rowBufferHit = 1;
							}
							else // there's no activate before this packet
							{
								if ((*busPacket)->busPacketType != ACTIVATE)
								{
									int core;
									int thread;
									int rank;
									int bank;
									unsigned int row;

									assert((*busPacket)->transaction);

									core = (*busPacket)->transaction->core;
									thread = (*busPacket)->transaction->thread;
									rank = (*busPacket)->rank;
									bank = (*busPacket)->bank;
									row = (*busPacket)->row;

									/* Row buffer miss */
									assert((*busPacket)->busPacketType == READ || (*busPacket)->busPacketType == WRITE);
									packet->rowBufferHit = 0;

									/* Is this an interthread row buffer miss? */
									interthreadRowBufferMiss = isInterthreadRowBufferMiss(rank, bank, row, core, thread);
								}
								//so just remove the bus packet
								queue.erase(queue.begin()+i);
							}

							foundIssuable = true;
							break;
						}
					}

					/* Count the cicles waiting due interthread interference */
					for (size_t i = 0; i < queue.size(); i++)
					{
						countInterthreadPenalty(*busPacket, queue[i]);

						/* If there is an interthread row buffer miss, the packets of the affected thread
						 * wait tRCD extra cycles, due to the activation not being omited */
						if (interthreadRowBufferMiss &&
								(queue[i]->busPacketType == READ || queue[i]->busPacketType == WRITE) &&
								queue[i]->transaction->core == (*busPacket)->transaction->core &&
								queue[i]->transaction->thread == (*busPacket)->transaction->thread)
						{
							(*busPacket)->transaction->interthreadPenalty += tRCD; /* Add the time spent reactivating the row */
						}
					}
				}

				//if we found something, break out of do-while
				if (foundIssuable) break;

				//rank round robin
				if (queuingStructure == PerRank)
				{
					nextRank = (nextRank + 1) % NUM_RANKS;
					if (startingRank == nextRank)
					{
						break;
					}
				}
				else
				{
					nextRankAndBank(nextRank, nextBank);
					if (startingRank == nextRank && startingBank == nextBank)
					{
						break;
					}
				}
			}
			while (true);

			//if nothing was issuable, see if we can issue a PRE to an open bank
			//	that has no other commands waiting
			if (!foundIssuable)
			{
				//search for banks to close
				bool sendingPRE = false;
				unsigned startingRank = nextRankPRE;
				unsigned startingBank = nextBankPRE;

				do // round robin over all ranks and banks
				{
					vector <BusPacket *> &queue = getCommandQueue(nextRankPRE, nextBankPRE);
					bool found = false;
					//check if bank is open
					if (bankStates[nextRankPRE][nextBankPRE].currentBankState == RowActive)
					{
						for (size_t i=0;i<queue.size();i++)
						{
							//if there is something going to that bank and row, then we don't want to send a PRE
							if (queue[i]->bank == nextBankPRE &&
									queue[i]->row == bankStates[nextRankPRE][nextBankPRE].openRowAddress)
							{
								found = true;
								break;
							}
						}

						//if nothing found going to that bank and row or too many accesses have happend, close it
						if (!found || rowAccessCounters[nextRankPRE][nextBankPRE] == TOTAL_ROW_ACCESSES)
						{
							if (currentClockCycle >= bankStates[nextRankPRE][nextBankPRE].nextPrecharge)
							{
								sendingPRE = true;
								rowAccessCounters[nextRankPRE][nextBankPRE] = 0;
								*busPacket = new BusPacket(PRECHARGE, 0, 0, 0, nextRankPRE, nextBankPRE, 0, dramsim_log);
								break;
							}
						}
					}
					nextRankAndBank(nextRankPRE, nextBankPRE);
				}
				while (!(startingRank == nextRankPRE && startingBank == nextBankPRE));

				//if no PREs could be sent, just return false
				if (!sendingPRE) return false;
			}
		}
	}

	//sendAct is flag used for posted-cas
	//  posted-cas is enabled when AL>0
	//  when sendAct is true, when don't want to increment our indexes
	//  so we send the column access that is paid with this act
	if (AL>0 && sendAct)
	{
		sendAct = false;
	}
	else
	{
		sendAct = true;
		nextRankAndBank(nextRank, nextBank);
	}

	//if its an activate, add a tfaw counter
	if ((*busPacket)->busPacketType==ACTIVATE)
	{
		tFAWCountdown[(*busPacket)->rank].push_back(tFAW);
	}

	return true;
}

//check if a rank/bank queue has room for a certain number of bus packets
bool CommandQueue::hasRoomFor(unsigned numberToEnqueue, unsigned rank, unsigned bank)
{
	vector<BusPacket *> &queue = getCommandQueue(rank, bank);
	return (CMD_QUEUE_DEPTH - queue.size() >= numberToEnqueue);
}

//prints the contents of the command queue
void CommandQueue::print()
{
	if (queuingStructure==PerRank)
	{
		PRINT(endl << "== Printing Per Rank Queue" );
		for (size_t i=0;i<NUM_RANKS;i++)
		{
			PRINT(" = Rank " << i << "  size : " << queues[i][0].size() );
			for (size_t j=0;j<queues[i][0].size();j++)
			{
				PRINTN("    "<< j << "]");
				queues[i][0][j]->print();
			}
		}
	}
	else if (queuingStructure==PerRankPerBank)
	{
		PRINT("\n== Printing Per Rank, Per Bank Queue" );

		for (size_t i=0;i<NUM_RANKS;i++)
		{
			PRINT(" = Rank " << i );
			for (size_t j=0;j<NUM_BANKS;j++)
			{
				PRINT("    Bank "<< j << "   size : " << queues[i][j].size() );

				for (size_t k=0;k<queues[i][j].size();k++)
				{
					PRINTN("       " << k << "]");
					queues[i][j][k]->print();
				}
			}
		}
	}
}

/**
 * return a reference to the queue for a given rank, bank. Since we
 * don't always have a per bank queuing structure, sometimes the bank
 * argument is ignored (and the 0th index is returned
 */
vector<BusPacket *> &CommandQueue::getCommandQueue(unsigned rank, unsigned bank)
{
	if (queuingStructure == PerRankPerBank)
	{
		return queues[rank][bank];
	}
	else if (queuingStructure == PerRank)
	{
		return queues[rank][0];
	}
	else
	{
		ERROR("Unknown queue structure");
		abort();
	}

}

//checks if busPacket is allowed to be issued
bool CommandQueue::isIssuable(BusPacket *busPacket)
{
	switch (busPacket->busPacketType)
	{
	case REFRESH:

		break;
	case ACTIVATE:
		if ((bankStates[busPacket->rank][busPacket->bank].currentBankState == Idle ||
		        bankStates[busPacket->rank][busPacket->bank].currentBankState == Refreshing) &&
		        currentClockCycle >= bankStates[busPacket->rank][busPacket->bank].nextActivate &&
		        tFAWCountdown[busPacket->rank].size() < 4)
		{
			return true;
		}
		else
		{
			return false;
		}
		break;
	case WRITE:
	case WRITE_P:
		if (bankStates[busPacket->rank][busPacket->bank].currentBankState == RowActive &&
		        currentClockCycle >= bankStates[busPacket->rank][busPacket->bank].nextWrite &&
		        busPacket->row == bankStates[busPacket->rank][busPacket->bank].openRowAddress &&
		        rowAccessCounters[busPacket->rank][busPacket->bank] < TOTAL_ROW_ACCESSES)
		{
			return true;
		}
		else
		{
			return false;
		}
		break;
	case READ_P:
	case READ:
		if (bankStates[busPacket->rank][busPacket->bank].currentBankState == RowActive &&
		        currentClockCycle >= bankStates[busPacket->rank][busPacket->bank].nextRead &&
		        busPacket->row == bankStates[busPacket->rank][busPacket->bank].openRowAddress &&
		        rowAccessCounters[busPacket->rank][busPacket->bank] < TOTAL_ROW_ACCESSES)
		{
			return true;
		}
		else
		{
			return false;
		}
		break;
	case PRECHARGE:
		if (bankStates[busPacket->rank][busPacket->bank].currentBankState == RowActive &&
		        currentClockCycle >= bankStates[busPacket->rank][busPacket->bank].nextPrecharge)
		{
			return true;
		}
		else
		{
			return false;
		}
		break;
	default:
		ERROR("== Error - Trying to issue a crazy bus packet type : ");
		busPacket->print();
		exit(0);
	}
	return false;
}

//figures out if a rank's queue is empty
bool CommandQueue::isEmpty(unsigned rank)
{
	if (queuingStructure == PerRank)
	{
		return queues[rank][0].empty();
	}
	else if (queuingStructure == PerRankPerBank)
	{
		for (size_t i=0;i<NUM_BANKS;i++)
		{
			if (!queues[rank][i].empty()) return false;
		}
		return true;
	}
	else
	{
		DEBUG("Invalid Queueing Stucture");
		abort();
	}
}

//tells the command queue that a particular rank is in need of a refresh
void CommandQueue::needRefresh(unsigned rank)
{
	refreshWaiting = true;
	refreshRank = rank;
}

void CommandQueue::nextRankAndBank(unsigned &rank, unsigned &bank)
{
	if (schedulingPolicy == RankThenBankRoundRobin)
	{
		rank++;
		if (rank == NUM_RANKS)
		{
			rank = 0;
			bank++;
			if (bank == NUM_BANKS)
			{
				bank = 0;
			}
		}
	}
	//bank-then-rank round robin
	else if (schedulingPolicy == BankThenRankRoundRobin)
	{
		bank++;
		if (bank == NUM_BANKS)
		{
			bank = 0;
			rank++;
			if (rank == NUM_RANKS)
			{
				rank = 0;
			}
		}
	}
	else
	{
		ERROR("== Error - Unknown scheduling policy");
		exit(0);
	}

}

void CommandQueue::update()
{
	//do nothing since pop() is effectively update(),
	//needed for SimulatorObject
	//TODO: make CommandQueue not a SimulatorObject
}


// Count interthread interference
void CommandQueue::countInterthreadPenalty(BusPacket *issued, BusPacket *queued)
{
	int bussy_core;
	int bussy_thread;
	int core = queued->transaction->core;
	int thread = queued->transaction->thread;
	unsigned int row = queued->row;
	unsigned int rank = queued->rank;
	unsigned int bank = queued->bank;
	Transaction *transaction = queued->transaction;

	/* A packet has been issued before us, if it belongs to other thread count this lost cycle as interthread penalty */
	if (issued &&
			issued->busPacketType != PRECHARGE &&
			issued->busPacketType != REFRESH &&
			issued->transaction->core == core &&
			issued->transaction->thread == thread)
	{
		transaction->cycleLost(currentClockCycle);
		return;
	}

	/* No packet has been issued this cycle, or the packet issued was from the same thread we are.
	 * Maybe we haven't been issued due other thread packet using shared resources, account for that. */
	switch (queued->busPacketType)
	{
		case REFRESH:
		case PRECHARGE:
			break;

		case ACTIVATE:
		{
			/* If there is an active row and we haven't activated it, or not enough time has passed
			 * to activate the row and it was activated by another thread, this counts as interthread penalty. */
			if (bankStates[rank][bank].currentBankState == RowActive ||
					(currentClockCycle < bankStates[rank][bank].nextActivate &&
					!bankStates[rank][bank].activate_history.empty()))
			{
				auto pair = bankStates[rank][bank].activate_history.front();
				bussy_core = pair.first;
				bussy_thread = pair.second;
				if (bussy_core != core || bussy_thread != thread)
					transaction->cycleLost(currentClockCycle);
			}

			/* We cannot activate because four banks have been accessed and tFAW has not passed.
			 * If some of the activations done in this tFAW interval are from other threads then count this cycle as interthread penalty. */
			else if (tFAWCountdown[rank].size() >= 4)
			{
					/* Which banks in the rank have been recently accessed and by whom */
					int size = min(rankAccesses[rank].size(), tFAWCountdown[rank].size());
					for (int i = 0; i < size; i++)
					{
						auto tuple = rankAccesses[rank].at(i);
						bussy_core = std::get <1> (tuple);
						bussy_thread = std::get <2> (tuple);
						/* One of the most recent accesses if from another thread so account penalty */
						if (bussy_core != core || bussy_thread != thread)
						{
							transaction->cycleLost(currentClockCycle);
							break;
						}
					}
			}
			break;
		}

		case WRITE:
		case WRITE_P:
		{
			/* Row is not active. No penalty. */
			if (bankStates[rank][bank].currentBankState != RowActive)
				break;

			/* If the last write is from other thread, account penalty */
			else if (currentClockCycle < bankStates[rank][bank].nextWrite && !bankStates[rank][bank].write_history.empty())
			{
				auto pair = bankStates[rank][bank].write_history.front();
				bussy_core = pair.first;
				bussy_thread = pair.second;
				if (bussy_core != core || bussy_thread != thread)
					transaction->cycleLost(currentClockCycle);
			}

			/* There is an active row, but it isn't our row. If it was activated by other thread account penalty. */
			else if (row != bankStates[rank][bank].openRowAddress && !bankStates[rank][bank].activate_history.empty())
			{
				auto pair = bankStates[rank][bank].activate_history.front();
				bussy_core = pair.first;
				bussy_thread = pair.second;
				if (bussy_core != core || bussy_thread != thread)
					transaction->cycleLost(currentClockCycle);
			}

			/* Maximum number of access per row reached. If other thread has accessed this row account penalty. */
			else if (rowAccessCounters[rank][bank] >= TOTAL_ROW_ACCESSES)
			{
				int size = min(rankBankAccesses[rank][bank].size(), (size_t) TOTAL_ROW_ACCESSES);
				for (int i = 0; i < size; i++)
				{
					auto tuple = rankBankAccesses[rank][bank].at(i);
					BusPacketType accessType = std::get <0> (tuple);
					if (accessType == READ || accessType == READ_P || accessType == WRITE || accessType == WRITE_P)
					{
						bussy_core = std::get <1> (tuple);
						bussy_thread = std::get <2> (tuple);
						if (bussy_core != core || bussy_thread != thread)
							transaction->cycleLost(currentClockCycle);
					}
				}
			}
			break;
		}

		case READ_P:
		case READ:
		{
			/* Row is not active. No penalty. */
			if (bankStates[rank][bank].currentBankState != RowActive)
				break;

			/* If the last read is from other thread, account penalty */
			else if (currentClockCycle < bankStates[rank][bank].nextRead && !bankStates[rank][bank].read_history.empty())
			{
				auto pair = bankStates[rank][bank].read_history.front();
				bussy_core = pair.first;
				bussy_thread = pair.second;
				if (bussy_core != core || bussy_thread != thread)
					transaction->cycleLost(currentClockCycle);
			}

			/* There is an active row, but it isn't our row. If it was activated by other thread account penalty. */
			else if (row != bankStates[rank][bank].openRowAddress && !bankStates[rank][bank].activate_history.empty())
			{
				auto pair = bankStates[rank][bank].activate_history.front();
				bussy_core = pair.first;
				bussy_thread = pair.second;
				if (bussy_core != core || bussy_thread != thread)
					transaction->cycleLost(currentClockCycle);
			}

			/* Maximum number of access per row reached. If other thread has accessed this row account penalty. */
			else if (rowAccessCounters[rank][bank] >= TOTAL_ROW_ACCESSES)
			{
				int size = min(rankBankAccesses[rank][bank].size(), (size_t) TOTAL_ROW_ACCESSES);
				for (int i = 0; i < size; i++)
				{
					auto tuple = rankBankAccesses[rank][bank].at(i);
					BusPacketType accessType = std::get <0> (tuple);
					if (accessType == READ || accessType == READ_P || accessType == WRITE || accessType == WRITE_P)
					{
						bussy_core = std::get <1> (tuple);
						bussy_thread = std::get <2> (tuple);
						if (bussy_core != core || bussy_thread != thread)
							transaction->cycleLost(currentClockCycle);
					}
				}
			}
			break;
		}

		default:
			ERROR("== Error - Trying to count  interthread penalty cycles for crazy bus packet type : ");
			queued->print();
			exit(0);
	}
}


void CommandQueue::recordAccess(BusPacketType type, int rank, int bank, int row, int core, int thread)
{
	rankAccesses[rank].push_front(make_tuple(bank, core, thread));
	rankBankAccesses[rank][bank].push_front(make_tuple(type, core, thread));

	rankHistory[rank].insert(currentClockCycle, type, bank, row, core, thread);
}


bool CommandQueue::isInterthreadRowBufferMiss(int rank, int bank, unsigned int row, int core, int thread)
{
	auto weak = rankHistory[rank].bankAccessor[bank];
	auto shared = weak.lock();
	bool interference = false;
	if(weak.expired())
	{
		return false;
	}

	while (!weak.expired())
	{
		shared = weak.lock();
		/* Last activate command to this bank */
		if (shared->type == ACTIVATE)
			break;
		weak = shared->prevSameBank;
	}

	/* There should be a previous activation */
	assert(!weak.expired());
	assert(shared->type == ACTIVATE);

	/* Since this packet is a read or a write, the last activation must be to this row and must be from this thread */
	assert(shared->row == row);
	assert(shared->core == core);
	assert(shared->thread == thread);

	/* We are interested in the previous activation done by this thread. If it was to the same row
	 * and other thread has done any activation inbetween, then this miss is because interthread interference. */
	weak = shared->prevSameBank;
	while (!weak.expired())
	{
		shared = weak.lock();
		if (shared->type == ACTIVATE)
		{
			/* The previous activation done by this thread */
			if (shared->core == core && shared->thread == thread)
			{
				/* Other thread has activated one or more rows, thus preventing a row buffer hit */
				if (shared->row == row && interference)
					return true;

				/* We have found the previous activation, nothing more to do */
				break;
			}

			/* Activations done by other threads to the same bank inbetween the two last activations
			 * done by this thread. */
			else
				interference = true;
		}
		weak = shared->prevSameBank;
	}

	return false;
}
