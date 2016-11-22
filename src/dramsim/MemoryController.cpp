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



//MemoryController.cpp
//
//Class file for memory controller object
//

#include <cassert>
#include <cmath>

#include "MemoryController.h"
#include "MemorySystem.h"
#include "AddressMapping.h"

#define SEQUENTIAL(rank,bank) (rank*NUM_BANKS)+bank

/* Power computations are localized to MemoryController.cpp */
extern unsigned IDD0;
extern unsigned IDD1;
extern unsigned IDD2P;
extern unsigned IDD2Q;
extern unsigned IDD2N;
extern unsigned IDD3Pf;
extern unsigned IDD3Ps;
extern unsigned IDD3N;
extern unsigned IDD4W;
extern unsigned IDD4R;
extern unsigned IDD5;
extern unsigned IDD6;
extern unsigned IDD6L;
extern unsigned IDD7;
extern float Vdd;

using namespace DRAMSim;

MemoryController::MemoryController(MemorySystem *parent, CSVWriter &csvOut_, ostream &dramsim_log_) :
		csvOut(csvOut_),
		dramsim_log(dramsim_log_),
		bankStates(NUM_RANKS, vector<BankState>(NUM_BANKS, dramsim_log)),
		commandQueue(bankStates, dramsim_log_),
		poppedBusPacket(NULL),
		totalTransactions(0),
		totalRowBufferHits(0),
		totalRowBufferMisses(0),
		refreshRank(0)
{
	//get handle on parent
	parentMemorySystem = parent;


	//bus related fields
	outgoingCmdPacket = NULL;
	outgoingDataPacket = NULL;
	dataCyclesLeft = 0;
	cmdCyclesLeft = 0;

	//set here to avoid compile errors
	currentClockCycle = 0;

	//reserve memory for vectors
	transactionQueue.reserve(TRANS_QUEUE_DEPTH);
	powerDown = vector<bool>(NUM_RANKS,false);
	grandTotalBankReads = vector<uint64_t>(NUM_RANKS * NUM_BANKS, 0);
	grandTotalBankWrites = vector<uint64_t>(NUM_RANKS * NUM_BANKS, 0);
	totalReadsPerBank = vector<uint64_t>(NUM_RANKS * NUM_BANKS, 0);
	totalWritesPerBank = vector<uint64_t>(NUM_RANKS * NUM_BANKS, 0);
	rowBufferHitsPerBank = vector<uint64_t>(NUM_RANKS * NUM_BANKS, 0);
	rowBufferMissesPerBank = vector<uint64_t>(NUM_RANKS * NUM_BANKS, 0);
	totalEpochLatency = vector<uint64_t> (NUM_RANKS * NUM_BANKS, 0);

	writeDataCountdown.reserve(NUM_RANKS);
	writeDataToSend.reserve(NUM_RANKS);
	refreshCountdown.reserve(NUM_RANKS);

	//Power related packets
	backgroundEnergy = vector <uint64_t> (NUM_RANKS, 0);
	burstEnergy = vector <uint64_t> (NUM_RANKS, 0);
	actpreEnergy = vector <uint64_t> (NUM_RANKS, 0);
	refreshEnergy = vector <uint64_t> (NUM_RANKS, 0);
	cyclesAllBanksIdle = vector <uint64_t> (NUM_RANKS, 0);

	//staggers when each rank is due for a refresh
	for (size_t i=0;i<NUM_RANKS;i++)
	{
		refreshCountdown.push_back((int)((REFRESH_PERIOD/tCK)/NUM_RANKS)*(i+1));
	}
}

//get a bus packet from either data or cmd bus
void MemoryController::receiveFromBus(BusPacket *bpacket)
{
	if (bpacket->busPacketType != DATA)
	{
		ERROR("== Error - Memory Controller received a non-DATA bus packet from rank");
		bpacket->print();
		exit(0);
	}

	if (DEBUG_BUS)
	{
		PRINTN(" -- MC Receiving From Data Bus : ");
		bpacket->print();
	}

	//add to return read data queue
	returnTransaction.push_back(new Transaction(RETURN_DATA, bpacket->physicalAddress, bpacket->data));
	totalReadsPerBank[SEQUENTIAL(bpacket->rank,bpacket->bank)]++;

	// this delete statement saves a mindboggling amount of memory
	delete(bpacket);
}

//sends read data back to the CPU
void MemoryController::returnReadData(const Transaction *trans)
{
	if (parentMemorySystem->ReturnReadData!=NULL)
	{
		(*parentMemorySystem->ReturnReadData)(parentMemorySystem->systemID, trans->address, trans->interthreadPenalty);
	}
}

//gives the memory controller a handle on the rank objects
void MemoryController::attachRanks(vector<Rank *> *ranks)
{
	this->ranks = ranks;
}

//memory controller update
void MemoryController::update()
{

	//PRINT(" ------------------------- [" << currentClockCycle << "] -------------------------");

	//update bank states
	for (size_t i=0; i<NUM_RANKS; i++)
	{
		for (size_t j=0; j<NUM_BANKS; j++)
		{
			if (bankStates[i][j].stateChangeCountdown > 0)
			{
				//decrement counters
				bankStates[i][j].stateChangeCountdown--;

				//if counter has reached 0, change state
				if (bankStates[i][j].stateChangeCountdown == 0)
				{
					switch (bankStates[i][j].lastCommand)
					{
						//only these commands have an implicit state change
					case WRITE_P:
					case READ_P:
						bankStates[i][j].currentBankState = Precharging;
						bankStates[i][j].lastCommand = PRECHARGE;
						bankStates[i][j].stateChangeCountdown = tRP;
						break;

					case REFRESH:
					case PRECHARGE:
						bankStates[i][j].currentBankState = Idle;
						break;
					default:
						break;
					}
				}
			}
		}

		if (powerDown[i])
			cyclesAllBanksIdle[i]++;
	}


	//check for outgoing command packets and handle countdowns
	if (outgoingCmdPacket != NULL)
	{
		cmdCyclesLeft--;
		if (cmdCyclesLeft == 0) //packet is ready to be received by rank
		{
			(*ranks)[outgoingCmdPacket->rank]->receiveFromBus(outgoingCmdPacket);
			outgoingCmdPacket = NULL;
		}
	}

	//check for outgoing data packets and handle countdowns
	if (outgoingDataPacket != NULL)
	{
		dataCyclesLeft--;
		if (dataCyclesLeft == 0)
		{
			//inform upper levels that a write is done
			if (parentMemorySystem->WriteDataDone!=NULL)
			{
				(*parentMemorySystem->WriteDataDone)(parentMemorySystem->systemID, outgoingDataPacket->physicalAddress, outgoingDataPacket->transaction->interthreadPenalty);
				delete outgoingDataPacket->transaction; /* Since this is a write and there is not a pending writes queue this transaction must be freed here */
			}

			(*ranks)[outgoingDataPacket->rank]->receiveFromBus(outgoingDataPacket);
			outgoingDataPacket=NULL;
		}
	}


	//if any outstanding write data needs to be sent
	//and the appropriate amount of time has passed (WL)
	//then send data on bus
	//
	//write data held in fifo vector along with countdowns
	if (writeDataCountdown.size() > 0)
	{
		for (size_t i=0;i<writeDataCountdown.size();i++)
		{
			writeDataCountdown[i]--;
		}

		if (writeDataCountdown[0]==0)
		{
			unsigned rank = writeDataToSend[0]->rank;
			unsigned bank = writeDataToSend[0]->bank;

			//send to bus and print debug stuff
			if (DEBUG_BUS)
			{
				PRINTN(" -- MC Issuing On Data Bus    : ");
				writeDataToSend[0]->print();
			}

			// queue up the packet to be sent
			if (outgoingDataPacket != NULL)
			{
				ERROR("== Error - Data Bus Collision");
				exit(-1);
			}

			outgoingDataPacket = writeDataToSend[0];
			dataCyclesLeft = BL/2;

			totalTransactions++;
			totalWritesPerBank[SEQUENTIAL(rank, bank)]++;

			/* If this transaction remains as the occupant of the bank, remove it */
			assert(outgoingDataPacket->transaction);
			if (bankStates[rank][bank].transaction == outgoingDataPacket->transaction)
				bankStates[rank][bank].transaction = NULL;

			writeDataCountdown.erase(writeDataCountdown.begin());
			writeDataToSend.erase(writeDataToSend.begin());
		}
	}

	//if its time for a refresh issue a refresh
	// else pop from command queue if it's not empty
	if (refreshCountdown[refreshRank]==0)
	{
		commandQueue.needRefresh(refreshRank);
		(*ranks)[refreshRank]->refreshWaiting = true;
		refreshCountdown[refreshRank] =	 REFRESH_PERIOD/tCK;
		refreshRank++;
		if (refreshRank == NUM_RANKS)
		{
			refreshRank = 0;
		}
	}
	//if a rank is powered down, make sure we power it up in time for a refresh
	else if (powerDown[refreshRank] && refreshCountdown[refreshRank] <= tXP)
	{
		(*ranks)[refreshRank]->refreshWaiting = true;
	}

	//pass a pointer to a poppedBusPacket

	//function returns true if there is something valid in poppedBusPacket
	if (commandQueue.pop(&poppedBusPacket))
	{
		if (poppedBusPacket->busPacketType == WRITE || poppedBusPacket->busPacketType == WRITE_P)
		{
			BusPacket *data = new BusPacket(DATA, poppedBusPacket->physicalAddress, poppedBusPacket->column,
					poppedBusPacket->row, poppedBusPacket->rank, poppedBusPacket->bank,
					poppedBusPacket->data, dramsim_log);
			data->transaction = poppedBusPacket->transaction;

			writeDataToSend.push_back(data);
			writeDataCountdown.push_back(WL);
		}

		//
		//update each bank's state based on the command that was just popped out of the command queue
		//
		//for readability's sake
		unsigned rank = poppedBusPacket->rank;
		unsigned bank = poppedBusPacket->bank;
		unsigned row = poppedBusPacket->row;
		switch (poppedBusPacket->busPacketType)
		{
			case READ_P:
			case READ:
			{
				int core;
				int thread;

				assert(poppedBusPacket->transaction);

				core = poppedBusPacket->transaction->core;
				thread = poppedBusPacket->transaction->thread;
				bankStates[rank][bank].read_history.push_front(std::make_pair(core, thread));
				commandQueue.recordAccess(poppedBusPacket->busPacketType, rank, bank, row, core, thread);

				//add energy to account for total
				if (DEBUG_POWER)
				{
					PRINT(" ++ Adding Read energy to total energy");
				}
				burstEnergy[rank] += (IDD4R - IDD3N) * BL/2 * NUM_DEVICES;

				if (poppedBusPacket->busPacketType == READ_P)
				{
					//Don't bother setting next read or write times because the bank is no longer active
					//bankStates[rank][bank].currentBankState = Idle;
					bankStates[rank][bank].nextActivate = max(currentClockCycle + READ_AUTOPRE_DELAY,
							bankStates[rank][bank].nextActivate);
					bankStates[rank][bank].lastCommand = READ_P;
					bankStates[rank][bank].stateChangeCountdown = READ_TO_PRE_DELAY;
				}
				else if (poppedBusPacket->busPacketType == READ)
				{
					bankStates[rank][bank].nextPrecharge = max(currentClockCycle + READ_TO_PRE_DELAY,
							bankStates[rank][bank].nextPrecharge);
					bankStates[rank][bank].lastCommand = READ;
				}

				/* Set the transaction currently occupying the bank */
				assert(poppedBusPacket->transaction);
				bankStates[rank][bank].transaction = poppedBusPacket->transaction;

				for (size_t i=0;i<NUM_RANKS;i++)
				{
					for (size_t j=0;j<NUM_BANKS;j++)
					{
						if (i!=poppedBusPacket->rank)
						{
							//check to make sure it is active before trying to set (save's time?)
							if (bankStates[i][j].currentBankState == RowActive)
							{
								bankStates[i][j].nextRead = max(currentClockCycle + BL/2 + tRTRS, bankStates[i][j].nextRead);
								bankStates[i][j].nextWrite = max(currentClockCycle + READ_TO_WRITE_DELAY,
										bankStates[i][j].nextWrite);
							}
						}
						else
						{
							bankStates[i][j].nextRead = max(currentClockCycle + max(tCCD, BL/2), bankStates[i][j].nextRead);
							bankStates[i][j].nextWrite = max(currentClockCycle + READ_TO_WRITE_DELAY,
									bankStates[i][j].nextWrite);
						}
					}
				}

				if (poppedBusPacket->busPacketType == READ_P)
				{
					//set read and write to nextActivate so the state table will prevent a read or write
					//  being issued (in cq.isIssuable())before the bank state has been changed because of the
					//  auto-precharge associated with this command
					bankStates[rank][bank].nextRead = bankStates[rank][bank].nextActivate;
					bankStates[rank][bank].nextWrite = bankStates[rank][bank].nextActivate;
				}

				if (rowBufferPolicy == OpenPage)
				{
					/* Update row buffer hit/miss stats */
					if (poppedBusPacket->rowBufferHit == 1)
					{
						rowBufferHitsPerBank[SEQUENTIAL(rank, bank)]++;
						totalRowBufferHits++;
					}
					else if (poppedBusPacket->rowBufferHit == 0)
					{
						rowBufferMissesPerBank[SEQUENTIAL(rank, bank)]++;
						totalRowBufferMisses++;
					}
					else
					{
						ERROR("Invalid value for row buffer hit/miss status");
						exit(-1);
					}
				}

				break;
			}

			case WRITE_P:
			case WRITE:
			{
				int core;
				int thread;

				assert(poppedBusPacket->transaction);

				core = poppedBusPacket->transaction->core;
				thread = poppedBusPacket->transaction->thread;
				bankStates[rank][bank].write_history.push_front(std::make_pair(core, thread));
				commandQueue.recordAccess(poppedBusPacket->busPacketType, rank, bank, row, core, thread);

				if (poppedBusPacket->busPacketType == WRITE_P)
				{
					bankStates[rank][bank].nextActivate = max(currentClockCycle + WRITE_AUTOPRE_DELAY,
							bankStates[rank][bank].nextActivate);
					bankStates[rank][bank].lastCommand = WRITE_P;
					bankStates[rank][bank].stateChangeCountdown = WRITE_TO_PRE_DELAY;
				}
				else if (poppedBusPacket->busPacketType == WRITE)
				{
					bankStates[rank][bank].nextPrecharge = max(currentClockCycle + WRITE_TO_PRE_DELAY,
							bankStates[rank][bank].nextPrecharge);
					bankStates[rank][bank].lastCommand = WRITE;
				}

				/* Set the transaction currently occupying the bank */
				assert(poppedBusPacket->transaction);
				bankStates[rank][bank].transaction = poppedBusPacket->transaction;

				//add energy to account for total
				if (DEBUG_POWER)
				{
					PRINT(" ++ Adding Write energy to total energy");
				}
				burstEnergy[rank] += (IDD4W - IDD3N) * BL/2 * NUM_DEVICES;

				for (size_t i=0;i<NUM_RANKS;i++)
				{
					for (size_t j=0;j<NUM_BANKS;j++)
					{
						if (i!=poppedBusPacket->rank)
						{
							if (bankStates[i][j].currentBankState == RowActive)
							{
								bankStates[i][j].nextWrite = max(currentClockCycle + BL/2 + tRTRS, bankStates[i][j].nextWrite);
								bankStates[i][j].nextRead = max(currentClockCycle + WRITE_TO_READ_DELAY_R,
										bankStates[i][j].nextRead);
							}
						}
						else
						{
							bankStates[i][j].nextWrite = max(currentClockCycle + max(BL/2, tCCD), bankStates[i][j].nextWrite);
							bankStates[i][j].nextRead = max(currentClockCycle + WRITE_TO_READ_DELAY_B,
									bankStates[i][j].nextRead);
						}
					}
				}

				//set read and write to nextActivate so the state table will prevent a read or write
				//  being issued (in cq.isIssuable())before the bank state has been changed because of the
				//  auto-precharge associated with this command
				if (poppedBusPacket->busPacketType == WRITE_P)
				{
					bankStates[rank][bank].nextRead = bankStates[rank][bank].nextActivate;
					bankStates[rank][bank].nextWrite = bankStates[rank][bank].nextActivate;
				}

				if (rowBufferPolicy == OpenPage)
				{
					/* Update row buffer hit/miss stats */
					if (poppedBusPacket->rowBufferHit == 1)
					{
						rowBufferHitsPerBank[SEQUENTIAL(rank, bank)]++;
						totalRowBufferHits++;
					}
					else if (poppedBusPacket->rowBufferHit == 0)
					{
						rowBufferMissesPerBank[SEQUENTIAL(rank, bank)]++;
						totalRowBufferMisses++;
					}
					else
					{
						ERROR("Invalid value for row buffer hit/miss status");
						exit(-1);
					}
				}

				break;
			}

			case ACTIVATE:
			{
				int core;
				int thread;

				assert(poppedBusPacket->transaction);

				core = poppedBusPacket->transaction->core;
				thread = poppedBusPacket->transaction->thread;
				bankStates[rank][bank].activate_history.push_front(std::make_pair(core, thread));
				commandQueue.recordAccess(poppedBusPacket->busPacketType, rank, bank, row, core, thread);

				//add energy to account for total
				if (DEBUG_POWER)
				{
					PRINT(" ++ Adding Activate and Precharge energy to total energy");
				}
				actpreEnergy[rank] += ((IDD0 * tRC) - ((IDD3N * tRAS) + (IDD2N * (tRC - tRAS)))) * NUM_DEVICES;

				bankStates[rank][bank].currentBankState = RowActive;
				bankStates[rank][bank].lastCommand = ACTIVATE;
				bankStates[rank][bank].openRowAddress = poppedBusPacket->row;
				bankStates[rank][bank].nextActivate = max(currentClockCycle + tRC, bankStates[rank][bank].nextActivate);
				bankStates[rank][bank].nextPrecharge = max(currentClockCycle + tRAS, bankStates[rank][bank].nextPrecharge);

				//if we are using posted-CAS, the next column access can be sooner than normal operation

				bankStates[rank][bank].nextRead = max(currentClockCycle + (tRCD-AL), bankStates[rank][bank].nextRead);
				bankStates[rank][bank].nextWrite = max(currentClockCycle + (tRCD-AL), bankStates[rank][bank].nextWrite);

				for (size_t i=0;i<NUM_BANKS;i++)
				{
					if (i!=poppedBusPacket->bank)
					{
						bankStates[rank][i].nextActivate = max(currentClockCycle + tRRD, bankStates[rank][i].nextActivate);
					}
				}

				break;
			}

			case PRECHARGE:
				bankStates[rank][bank].currentBankState = Precharging;
				bankStates[rank][bank].lastCommand = PRECHARGE;
				bankStates[rank][bank].stateChangeCountdown = tRP;
				bankStates[rank][bank].nextActivate = max(currentClockCycle + tRP, bankStates[rank][bank].nextActivate);
				break;

			case REFRESH:
				//add energy to account for total
				if (DEBUG_POWER)
				{
					PRINT(" ++ Adding Refresh energy to total energy");
				}
				refreshEnergy[rank] += (IDD5 - IDD3N) * tRFC * NUM_DEVICES;

				for (size_t i=0;i<NUM_BANKS;i++)
				{
					bankStates[rank][i].nextActivate = currentClockCycle + tRFC;
					bankStates[rank][i].currentBankState = Refreshing;
					bankStates[rank][i].lastCommand = REFRESH;
					bankStates[rank][i].stateChangeCountdown = tRFC;
				}

				break;

			default:
				ERROR("== Error - Popped a command we shouldn't have of type : " << poppedBusPacket->busPacketType);
				exit(0);
		}

		//issue on bus and print debug
		if (DEBUG_BUS)
		{
			PRINTN(" -- MC Issuing On Command Bus : ");
			poppedBusPacket->print();
		}

		//check for collision on bus
		if (outgoingCmdPacket != NULL)
		{
			ERROR("== Error - Command Bus Collision");
			exit(-1);
		}
		outgoingCmdPacket = poppedBusPacket;
		cmdCyclesLeft = tCMD;
	}

	for (size_t i=0;i<transactionQueue.size();i++)
	{
		//pop off top transaction from queue
		//
		//	assuming simple scheduling at the moment
		//	will eventually add policies here
		Transaction *transaction = transactionQueue[i];

		//map address to rank,bank,row,col
		unsigned newTransactionChan, newTransactionRank, newTransactionBank, newTransactionRow, newTransactionColumn;

		// pass these in as references so they get set by the addressMapping function
		addressMapping(transaction->address, newTransactionChan, newTransactionRank, newTransactionBank, newTransactionRow, newTransactionColumn);

		//if we have room, break up the transaction into the appropriate commands
		//and add them to the command queue
		if (commandQueue.hasRoomFor(2, newTransactionRank, newTransactionBank))
		{
			if (DEBUG_ADDR_MAP)
			{
				PRINTN("== New Transaction - Mapping Address [0x" << hex << transaction->address << dec << "]");
				if (transaction->transactionType == DATA_READ)
				{
					PRINT(" (Read)");
				}
				else
				{
					PRINT(" (Write)");
				}
				PRINT("  Rank : " << newTransactionRank);
				PRINT("  Bank : " << newTransactionBank);
				PRINT("  Row  : " << newTransactionRow);
				PRINT("  Col  : " << newTransactionColumn);
			}

			//now that we know there is room in the command queue, we can remove from the transaction queue
			transactionQueue.erase(transactionQueue.begin()+i);

			//create activate command to the row we just translated
			BusPacket *ACTcommand = new BusPacket(ACTIVATE, transaction->address,
					newTransactionColumn, newTransactionRow, newTransactionRank,
					newTransactionBank, 0, dramsim_log);
			ACTcommand->transaction = transaction;

			//create read or write command and enqueue it
			BusPacketType bpType = transaction->getBusPacketType();
			BusPacket *command = new BusPacket(bpType, transaction->address,
					newTransactionColumn, newTransactionRow, newTransactionRank,
					newTransactionBank, transaction->data, dramsim_log);
			command->transaction = transaction;

			commandQueue.enqueue(ACTcommand);
			commandQueue.enqueue(command);

			// If we have a read, save the transaction so when the data comes back
			// in a bus packet, we can staple it back into a transaction and return it
			if (transaction->transactionType == DATA_READ)
				pendingReadTransactions.push_back(transaction);

			/* Update penalty counter for the transactions of other threads, since they are waiting due to this access */
			for (size_t j = i; j < transactionQueue.size(); j++)
			{
				Transaction *waiting_transaction = transactionQueue[i];

				/* Only count interthread penalty */
				if (waiting_transaction->core == transaction->core &&
						waiting_transaction->thread == transaction->thread)
					continue;

				/* Cycles waiting */
				waiting_transaction->interthreadPenalty++;
			}

			/* only allow one transaction to be scheduled per cycle -- this should
			 * be a reasonable assumption considering how much logic would be
			 * required to schedule multiple entries per cycle (parallel data
			 * lines, switching logic, decision logic)
			 */
			break;
		}

		else // no room in command queue
		{
			vector<BusPacket *> &queue = commandQueue.getCommandQueue(newTransactionRank, newTransactionBank);
			if (queue[0]->transaction->core != transaction->core || queue[0]->transaction->thread != transaction->thread)
				transaction->interthreadPenalty++;
		}
	}


	//calculate power
	//  this is done on a per-rank basis, since power characterization is done per device (not per bank)
	for (size_t i=0;i<NUM_RANKS;i++)
	{
		if (USE_LOW_POWER)
		{
			//if there are no commands in the queue and that particular rank is not waiting for a refresh...
			if (commandQueue.isEmpty(i) && !(*ranks)[i]->refreshWaiting)
			{
				//check to make sure all banks are idle
				bool allIdle = true;
				for (size_t j=0;j<NUM_BANKS;j++)
				{
					if (bankStates[i][j].currentBankState != Idle)
					{
						allIdle = false;
						break;
					}
				}

				//if they ARE all idle, put in power down mode and set appropriate fields
				if (allIdle)
				{
					powerDown[i] = true;
					(*ranks)[i]->powerDown();
					for (size_t j=0;j<NUM_BANKS;j++)
					{
						bankStates[i][j].currentBankState = PowerDown;
						bankStates[i][j].nextPowerUp = currentClockCycle + tCKE;
					}
				}
			}
			//if there IS something in the queue or there IS a refresh waiting (and we can power up), do it
			else if (currentClockCycle >= bankStates[i][0].nextPowerUp && powerDown[i]) //use 0 since theyre all the same
			{
				powerDown[i] = false;
				(*ranks)[i]->powerUp();
				for (size_t j=0;j<NUM_BANKS;j++)
				{
					bankStates[i][j].currentBankState = Idle;
					bankStates[i][j].nextActivate = currentClockCycle + tXP;
				}
			}
		}

		//check for open bank
		bool bankOpen = false;
		for (size_t j=0;j<NUM_BANKS;j++)
		{
			if (bankStates[i][j].currentBankState == Refreshing ||
			        bankStates[i][j].currentBankState == RowActive)
			{
				bankOpen = true;
				break;
			}
		}

		//background power is dependent on whether or not a bank is open or not
		if (bankOpen)
		{
			if (DEBUG_POWER)
			{
				PRINT(" ++ Adding IDD3N to total energy [from rank "<< i <<"]");
			}
			backgroundEnergy[i] += IDD3N * NUM_DEVICES;
		}
		else
		{
			//if we're in power-down mode, use the correct current
			if (powerDown[i])
			{
				if (DEBUG_POWER)
				{
					PRINT(" ++ Adding IDD2P to total energy [from rank " << i << "]");
				}
				backgroundEnergy[i] += IDD2P * NUM_DEVICES;
			}
			else
			{
				if (DEBUG_POWER)
				{
					PRINT(" ++ Adding IDD2N to total energy [from rank " << i << "]");
				}
				backgroundEnergy[i] += IDD2N * NUM_DEVICES;
			}
		}
	}

	//check for outstanding data to return to the CPU
	if (returnTransaction.size()>0)
	{
		if (DEBUG_BUS)
		{
			PRINTN(" -- MC Issuing to CPU bus : " << *returnTransaction[0]);
		}
		totalTransactions++;

		bool foundMatch=false;
		//find the pending read transaction to calculate latency
		for (size_t i=0;i<pendingReadTransactions.size();i++)
		{
			if (pendingReadTransactions[i]->address == returnTransaction[0]->address)
			{
				//if(currentClockCycle - pendingReadTransactions[i]->timeAdded > 2000)
				//	{
				//		pendingReadTransactions[i]->print();
				//		exit(0);
				//	}
				unsigned chan,rank,bank,row,col;
				addressMapping(returnTransaction[0]->address,chan,rank,bank,row,col);
				insertHistogram(currentClockCycle-pendingReadTransactions[i]->timeAdded,rank,bank);
				//return latency
				returnReadData(pendingReadTransactions[i]);

				/* This transaction in no longer occupying a bank, so if it is set as the bank occupant, remove it */
				if (bankStates[rank][bank].transaction == pendingReadTransactions[i])
					bankStates[rank][bank].transaction = NULL;

				delete pendingReadTransactions[i];
				pendingReadTransactions.erase(pendingReadTransactions.begin()+i);
				foundMatch=true;
				break;
			}
		}
		if (!foundMatch)
		{
			ERROR("Can't find a matching transaction for 0x"<<hex<<returnTransaction[0]->address<<dec);
			abort();
		}
		delete returnTransaction[0];
		returnTransaction.erase(returnTransaction.begin());
	}

	//decrement refresh counters
	for (size_t i=0;i<NUM_RANKS;i++)
	{
		refreshCountdown[i]--;
	}

	//
	//print debug
	//
	if (DEBUG_TRANS_Q)
	{
		PRINT("== Printing transaction queue");
		for (size_t i=0;i<transactionQueue.size();i++)
		{
			PRINTN("  " << i << "] "<< *transactionQueue[i]);
		}
	}

	if (DEBUG_BANKSTATE)
	{
		//TODO: move this to BankState.cpp
		PRINT("== Printing bank states (According to MC)");
		for (size_t i=0;i<NUM_RANKS;i++)
		{
			for (size_t j=0;j<NUM_BANKS;j++)
			{
				if (bankStates[i][j].currentBankState == RowActive)
				{
					PRINTN("[" << bankStates[i][j].openRowAddress << "] ");
				}
				else if (bankStates[i][j].currentBankState == Idle)
				{
					PRINTN("[idle] ");
				}
				else if (bankStates[i][j].currentBankState == Precharging)
				{
					PRINTN("[pre] ");
				}
				else if (bankStates[i][j].currentBankState == Refreshing)
				{
					PRINTN("[ref] ");
				}
				else if (bankStates[i][j].currentBankState == PowerDown)
				{
					PRINTN("[lowp] ");
				}
			}
			PRINT(""); // effectively just cout<<endl;
		}
	}

	if (DEBUG_CMD_Q)
	{
		commandQueue.print();
	}

	commandQueue.step();

	estimateBandwidthRequirements();
}

/* Estimates bandwidth requirements per core */
void MemoryController::estimateBandwidthRequirements()
{
	for (size_t r = 0; r < NUM_RANKS; r++)
	{
		map<int,bool> counted;
		for (size_t b = 0; b < NUM_BANKS; b++)
		{
			/* Bandwidth Consumed by Core */
			if (bankStates[r][b].transaction)
			{
				int core = bankStates[r][b].transaction->core;
				int thread = bankStates[r][b].transaction->thread;
				assert(core >= 0 && thread >= 0);
				bwc[core] += 1;
				counted[core] = true;
			}

			/* Bandwidth Needed By Core */
			if (queuingStructure == PerRankPerBank)
			{
				auto queue = commandQueue.queues[r][b];
				for (auto it = queue.begin(); it != queue.end(); it++)
				{
					BusPacket *bp = *it;
					BusPacketType type = bp->busPacketType;
					int core;
					int thread;

					if (type == READ || type == WRITE || type == READ_P || type == WRITE_P)
					{
						assert(bp->transaction);
						core = bp->transaction->core;
						thread = bp->transaction->thread;
						assert(core >= 0 && thread >= 0);
						if (!counted[core])
						{
							counted[core] = true;
							bwn[core] += 1;
						}
					}
				}
			}
		}

		/* Bandwidth Needed by Core */
		if (queuingStructure == PerRank)
		{
			map<int, map<int, bool>> counted; /* bank -> core -> true/false */
			auto queue = commandQueue.queues[r][0];
			for (auto it = queue.begin(); it != queue.end(); it++)
			{
				BusPacket *bp = *it;
				BusPacketType type = bp->busPacketType;
				int core;
				int thread;
				int bank = bp->bank;
				int rank = bp->rank;
				assert(bank >= 0);
				assert(rank >= 0);

				if (type == READ || type == WRITE || type == READ_P || type == WRITE_P)
				{
					assert(bp->transaction);
					core = bp->transaction->core;
					thread = bp->transaction->thread;
					assert(core >= 0 && thread >= 0);
					assert((unsigned) rank == r);
					if (!counted[bank][core])
					{
						counted[bank][core] = true;
						bwn[core] += 1;
					}
				}
			}
		}
	}

	/* Bandwidth Needed by Others */
	uint64_t total_bwn = 0;
	for (auto it = bwn.begin(); it != bwn.end(); it++)
	{
		int core = it->first;
		uint64_t core_bwn = it->second;
		assert(core >= 0 && core_bwn >= 0);
		total_bwn += core_bwn;
	}

	for (auto it = bwn.begin(); it != bwn.end(); it++)
	{
		int core = it->first;
		uint64_t core_bwn = it->second;
		assert(core >= 0 && core_bwn >= 0);
		bwno[core] = total_bwn - core_bwn;
	}
}


bool MemoryController::WillAcceptTransaction()
{
	return transactionQueue.size() < TRANS_QUEUE_DEPTH;
}

//allows outside source to make request of memory system
bool MemoryController::addTransaction(Transaction *trans)
{
	if (WillAcceptTransaction())
	{
		trans->timeAdded = currentClockCycle;
		transactionQueue.push_back(trans);
		return true;
	}
	else
	{
		return false;
	}
}


void MemoryController::resetStats()
{
	for (size_t i=0; i<NUM_RANKS; i++)
	{
		for (size_t j=0; j<NUM_BANKS; j++)
		{
			//XXX: this means the bank list won't be printed for partial epochs
			grandTotalBankReads[SEQUENTIAL(i,j)] += totalReadsPerBank[SEQUENTIAL(i,j)];
			grandTotalBankWrites[SEQUENTIAL(i,j)] += totalWritesPerBank[SEQUENTIAL(i,j)];
			totalReadsPerBank[SEQUENTIAL(i,j)] = 0;
			totalWritesPerBank[SEQUENTIAL(i,j)] = 0;
			totalEpochLatency[SEQUENTIAL(i,j)] = 0;
			rowBufferHitsPerBank[SEQUENTIAL(i,j)] = 0;
			rowBufferMissesPerBank[SEQUENTIAL(i,j)] = 0;
		}
		burstEnergy[i] = 0;
		actpreEnergy[i] = 0;
		refreshEnergy[i] = 0;
		backgroundEnergy[i] = 0;
		cyclesAllBanksIdle[i] = 0;
	}
}


void MemoryController::printLatencyHistogram(ostream &out)
{
	map<unsigned,unsigned>::iterator it;
	out << endl << "!!HISTOGRAM_DATA" << endl;
	for (it = latencies.begin(); it != latencies.end(); it++)
		out << it->first << "," << it->second << endl;
}


void MemoryController::printStatsLog(bool finalStats)
{
	unsigned bytesPerTransaction = JEDEC_DATA_BUS_BITS * BL / 8;
	uint64_t cyclesElapsed = (currentClockCycle % EPOCH_LENGTH == 0) ? EPOCH_LENGTH : currentClockCycle % EPOCH_LENGTH;
	uint64_t transactionsEpoch = 0;
	double secondsThisEpoch = cyclesElapsed * tCK * 1E-9;
	double totalBandwidth = 0.0;

	/* Per rank */
	auto totalReadsPerRank = vector<uint64_t>(NUM_RANKS, 0);
	auto totalWritesPerRank = vector<uint64_t>(NUM_RANKS, 0);
	auto backgroundPower = vector<double>(NUM_RANKS, 0.0);
	auto burstPower = vector<double>(NUM_RANKS, 0.0);
	auto refreshPower = vector<double>(NUM_RANKS, 0.0);
	auto actprePower = vector<double>(NUM_RANKS, 0.0);
	auto averagePower = vector<double>(NUM_RANKS, 0.0);

	/* Per bank */
	auto averageLatency = vector<double>(NUM_RANKS * NUM_BANKS, 0.0);
	auto bandwidth = vector<double>(NUM_RANKS * NUM_BANKS, 0.0);

	#ifdef LOG_OUTPUT
		dramsim_log.precision(3);
		dramsim_log.setf(ios::fixed, ios::floatfield);
	#else
		cout.precision(3);
		cout.setf(ios::fixed, ios::floatfield);
	#endif

	for (unsigned r = 0; r < NUM_RANKS; r++)
	{
		for (unsigned b = 0; b < NUM_BANKS; b++)
		{
			totalReadsPerRank[r] += totalReadsPerBank[SEQUENTIAL(r, b)];
			totalWritesPerRank[r] += totalWritesPerBank[SEQUENTIAL(r, b)];
		}
		transactionsEpoch += totalReadsPerRank[r] + totalWritesPerRank[r];

		backgroundPower[r] = ((double)backgroundEnergy[r] / (double)(cyclesElapsed)) * Vdd / 1000.0;
		actprePower[r] = ((double)actpreEnergy[r] / (double)(cyclesElapsed)) * Vdd / 1000.0;
		burstPower[r] = ((double)burstEnergy[r] / (double)(cyclesElapsed)) * Vdd / 1000.0;
		refreshPower[r] = ((double) refreshEnergy[r] / (double)(cyclesElapsed)) * Vdd / 1000.0;
		averagePower[r] = backgroundPower[r] + actprePower[r] + burstPower[r] + refreshPower[r];
	}
	totalBandwidth += transactionsEpoch * bytesPerTransaction / (1024.0 * 1024.0 * 1024.0) / secondsThisEpoch;

	PRINT( " =======================================================" );
	PRINT( " ============== Printing Statistics [id:" << parentMemorySystem->systemID << "]==============" );
	PRINTN( "   Total Return Transactions : " << totalTransactions );
	PRINT( " (" << totalTransactions * bytesPerTransaction << " bytes) aggregate average bandwidth " << totalBandwidth << "GB/s");

	for (size_t r = 0; r < NUM_RANKS; r++)
	{
		PRINT( "      -Rank   " << r << " : ");
		PRINTN( "        -Reads  : " << totalReadsPerRank[r]);
		PRINT( " (" << totalReadsPerRank[r] * bytesPerTransaction << " bytes)");
		PRINTN( "        -Writes : " << totalWritesPerRank[r]);
		PRINT( " (" << totalWritesPerRank[r] * bytesPerTransaction << " bytes)");

		for (size_t j = 0; j < NUM_BANKS; j++)
			PRINT( "        -Bandwidth / Latency  (Bank " << j << "): " <<
					bandwidth[SEQUENTIAL(r, j)] << " GB/s\t\t" <<
					averageLatency[SEQUENTIAL(r, j)] << " ns");

		PRINT( " == Power Data for Rank        " << r );
		PRINT( "   Average Power (watts)     : " << averagePower[r] );
		PRINT( "     -Background (watts)     : " << backgroundPower[r] );
		PRINT( "     -Act/Pre    (watts)     : " << actprePower[r] );
		PRINT( "     -Burst      (watts)     : " << burstPower[r]);
		PRINT( "     -Refresh    (watts)     : " << refreshPower[r] );
	}

	// only print the latency histogram at the end of the simulation since it clogs the output too much to print every epoch
	if (finalStats)
	{
		PRINT( " ---  Latency list (" << latencies.size() << ")");
		PRINT( "       [lat] : #");

		map<unsigned, unsigned>::iterator it; //
		for (it = latencies.begin(); it != latencies.end(); it++)
			PRINT( "       [" << it->first << "-" << it->first + (HISTOGRAM_BIN_SIZE - 1) << "] : " << it->second );
		PRINT( " --- Grand Total Bank usage list");
		for (size_t i = 0; i < NUM_RANKS; i++)
		{
			PRINT("Rank " << i << ":");
			for (size_t j = 0; j < NUM_BANKS; j++)
				PRINT( "  b" << j << ": " << grandTotalBankReads[SEQUENTIAL(i, j)] + grandTotalBankWrites[SEQUENTIAL(i, j)]);
		}
	}

	PRINT(endl << " == Pending Transactions : " << pendingReadTransactions.size() << " (" << currentClockCycle << ")==");
	for (size_t i = 0; i < pendingReadTransactions.size(); i++)
		PRINT(i << "] I've been waiting for " << currentClockCycle - pendingReadTransactions[i]->timeAdded << endl);

	#ifdef LOG_OUTPUT
	   dramsim_log.flush();
	#endif
}


uint64_t MemoryController::getBWC(int core)
{
	assert(core >= 0);
	return bwc[core];
}


uint64_t MemoryController::getBWN(int core)
{
	assert(core >= 0);
	return bwn[core];
}


uint64_t MemoryController::getBWNO(int core)
{
	assert(core >= 0);
	return bwno[core];
}


void MemoryController::setBWC(int core, uint64_t value)
{
	assert(core >= 0);
	assert(value >= 0);
	bwc[core] = value;
}


void MemoryController::setBWN(int core, uint64_t value)
{
	assert(core >= 0);
	assert(value >= 0);
	bwn[core] = value;
}


void MemoryController::setBWNO(int core, uint64_t value)
{
	assert(core >= 0);
	assert(value >= 0);
	bwno[core] = value;
}


//prints statistics at the end of an epoch or simulation
void MemoryController::printStats(bool finalStats)
{
	if (!VIS_FILE_OUTPUT)
		return;

	unsigned myChannel = parentMemorySystem->systemID;

	/* If we are not at the end of the epoch, make sure to adjust for the actual number of cycles elapsed */
	uint64_t cyclesElapsed = (currentClockCycle % EPOCH_LENGTH == 0) ? EPOCH_LENGTH : currentClockCycle % EPOCH_LENGTH;

	unsigned bytesPerTransaction = (JEDEC_DATA_BUS_BITS * BL) / 8;
	double secondsThisEpoch = cyclesElapsed * tCK * 1E-9;

	/* Per rank */
	auto backgroundPower = vector<double>(NUM_RANKS, 0.0);
	auto burstPower = vector<double>(NUM_RANKS, 0.0);
	auto refreshPower = vector<double>(NUM_RANKS, 0.0);
	auto actprePower = vector<double>(NUM_RANKS, 0.0);

	/* Per bank */
	auto averageLatency = vector<double>(NUM_RANKS * NUM_BANKS, 0.0);
	auto bandwidth = vector<double>(NUM_RANKS * NUM_BANKS, 0.0);

	double totalAggregateBandwidth = 0.0;
	for (unsigned r = 0; r < NUM_RANKS; r++)
	{
		double totalRankBandwidth = 0.0;
		for (size_t b = 0; b < NUM_BANKS; b++)
		{
			uint64_t transactions = totalReadsPerBank[SEQUENTIAL(r, b)] + totalWritesPerBank[SEQUENTIAL(r, b)];
			double rowBufferHit = rowBufferPolicy == OpenPage && transactions > 0 ?
					(double) rowBufferHitsPerBank[SEQUENTIAL(r, b)] / transactions :
					NAN;

			bandwidth[SEQUENTIAL(r, b)] = transactions * bytesPerTransaction / (1024.0 * 1024.0 * 1024.0) / secondsThisEpoch; /* GB/s */
			averageLatency[SEQUENTIAL(r, b)] = ((double)totalEpochLatency[SEQUENTIAL(r, b)] / (double)(totalReadsPerBank[SEQUENTIAL(r, b)])) * tCK; /* ns */

			totalRankBandwidth += bandwidth[SEQUENTIAL(r, b)];
			totalAggregateBandwidth += bandwidth[SEQUENTIAL(r, b)];

			csvOut.header("Completed_Reads" + squared(myChannel) + squared(r) + squared(b))
					.field(totalReadsPerBank[SEQUENTIAL(r, b)]);
			csvOut.header("Completed_Writes" + squared(myChannel) + squared(r) + squared(b))
					.field(totalWritesPerBank[SEQUENTIAL(r, b)]);
			csvOut.header("Bandwidth" + squared(myChannel) + squared(r) + squared(b))
					.field(bandwidth[SEQUENTIAL(r,b)]);
			csvOut.header("Average_Latency" + squared(myChannel) + squared(r) + squared(b))
					.field(averageLatency[SEQUENTIAL(r, b)]);
			csvOut.header("Row_Buffer_Hit" + squared(myChannel) + squared(r) + squared(b))
					.field(rowBufferHit);
		}

		/* Factor of 1000 at the end is to account for the fact that totalEnergy is accumulated in mJ since IDD values are given in mA */
		backgroundPower[r] = ((double)backgroundEnergy[r] / (double)(cyclesElapsed)) * Vdd / 1000.0;
		actprePower[r] = ((double)actpreEnergy[r] / (double)(cyclesElapsed)) * Vdd / 1000.0;
		burstPower[r] = ((double)burstEnergy[r] / (double)(cyclesElapsed)) * Vdd / 1000.0;
		refreshPower[r] = ((double) refreshEnergy[r] / (double)(cyclesElapsed)) * Vdd / 1000.0;

		csvOut.header("Rank_Aggregate_Bandwidth" + squared(myChannel) + squared(r))
				.field(totalRankBandwidth);
		csvOut.header("Rank_Average_Bandwidth" + squared(myChannel) + squared(r))
				.field(totalRankBandwidth / NUM_RANKS);

		csvOut.header("Background_Power" + squared(myChannel) + squared(r))
				.field(backgroundPower[r]);
		csvOut.header("ACT_PRE_Power" + squared(myChannel) + squared(r))
				.field(actprePower[r]);
		csvOut.header("Burst_Power" + squared(myChannel) + squared(r))
				.field(burstPower[r]);
		csvOut.header("Refresh_Power" + squared(myChannel) + squared(r))
				.field(refreshPower[r]);
		csvOut.header("All_Banks_Idle" + squared(myChannel) + squared(r))
				.field((double) cyclesAllBanksIdle[r] / cyclesElapsed);
	}

	csvOut.header("Aggregate_Bandwidth" + squared(myChannel))
			.field(totalAggregateBandwidth);
	csvOut.header("Average_Bandwidth" + squared(myChannel))
			.field(totalAggregateBandwidth / (NUM_RANKS * NUM_BANKS));

	csvOut.header("Reads_Accessing" + squared(myChannel))
			.field(pendingReadTransactions.size());
	csvOut.header("Queued_Requests" + squared(myChannel))
			.field(transactionQueue.size());
	csvOut.header("Cumulative_Completed_Requests" + squared(myChannel))
			.field(totalTransactions);

	/* Only print the latency histogram at the end of the simulation since it clogs the output too much to print every epoch */
	// if (finalStats)
	// 	printLatencyHistogram(csvOut.getOutputStream());

	resetStats();
}


MemoryController::~MemoryController()
{
	for (size_t i=0; i<pendingReadTransactions.size(); i++)
	{
		delete pendingReadTransactions[i];
	}
	for (size_t i=0; i<returnTransaction.size(); i++)
	{
		delete returnTransaction[i];
	}

	assert(rowBufferPolicy == ClosePage || totalTransactions == totalRowBufferHits + totalRowBufferMisses);
}


//inserts a latency into the latency histogram
void MemoryController::insertHistogram(unsigned latencyValue, unsigned rank, unsigned bank)
{
	totalEpochLatency[SEQUENTIAL(rank,bank)] += latencyValue;
	//poor man's way to bin things.
	latencies[(latencyValue/HISTOGRAM_BIN_SIZE)*HISTOGRAM_BIN_SIZE]++;
}
