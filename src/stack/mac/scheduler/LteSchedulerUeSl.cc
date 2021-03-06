//
//                           SimuLTE
//
// This file is part of a software released under the license included in file
// "license.pdf". This license can be also found at http://www.ltesimulator.com/
// The above file and the present reference are part of the software itself,
// and cannot be removed from it.
//

#include "stack/mac/scheduler/LteSchedulerUeSl.h"
#include "stack/mac/buffer/LteMacBuffer.h"
#include "stack/mac/layer/LteMacUeD2D.h"
#include "stack/mac/amc/UserTxParams.h"
#include "stack/mac/packet/LteSchedulingGrant.h"
#include "stack/mac/packet/LteMacPdu.h"
#include "stack/mac/scheduler/LcgScheduler.h"
#include "stack/mac/configuration/SidelinkConfiguration.h"
LteSchedulerUeSl::LteSchedulerUeSl(LteMacUeD2D* mac)
:LcgScheduler(mac)
{
	macd2d_=mac;

	lcgScheduler_ = new LcgScheduler(mac_);

}

LteSchedulerUeSl::~LteSchedulerUeSl()
{
	// TODO Auto-generated destructor stub
	delete lteGrant;

	delete lcgScheduler_;
}


LteMacScheduleList* LteSchedulerUeSl::schedule(LteSidelinkGrant* slgrant)
{

	/* clean up old schedule decisions
     for each cid, this map will store the the amount of sent data (in SDUs)
	 */
	scheduleList_.clear();

	/*
	 * Clean up scheduling support status map
	 */
	statusMap_.clear();

	//Obtain information from Sidelink Grant
	// get the grant

	Direction grantDir = slgrant->getDirection();
	EV<<"Direction: "<<slgrant->getDirection()<<endl;
	// get the nodeId of the mac owner node
	MacNodeId nodeId = mac_->getMacNodeId();
	unsigned int codewords = slgrant->getCodewords();
	EV<<"LteSchedulerUeSl::schedule codewords: "<<codewords<<endl;
	EV << NOW << " LteSchedulerUeSl::schedule - Scheduling node " << nodeId << endl;

	// TODO get the amount of granted data per codeword
	//unsigned int availableBytes = grant->getGrantedBytes();

	unsigned int availableBlocks = slgrant->getTotalGrantedBlocks();
	// TODO check if HARQ ACK messages should be subtracted from available bytes
	EV<<"LteSchedulerUeSl::schedule Available blocks: "<<availableBlocks<<endl;
	for (Codeword cw = 0; cw < codewords; ++cw)
	{
		unsigned int availableBytes = slgrant->getGrantedCwBytes(cw);

		EV << NOW << " LteSchedulerUeSl::schedule - Node " << mac_->getMacNodeId() << " available data from grant are "
				<< " blocks " << availableBlocks << " [" << availableBytes << " - Bytes]  on codeword " << cw << endl;

		// per codeword LCP scheduler invocation

		// invoke the schedule() method of the attached LCP scheduler in order to schedule
		// the connections provided
		std::map<MacCid, unsigned int>& sdus = lcgScheduler_->schedule(availableBytes, grantDir);
		EV<<"LteSchedulerUeSl::schedule sdu size: "<<sdus.size()<<endl;
		// get the amount of bytes scheduled for each connection
		std::map<MacCid, unsigned int>& bytes = lcgScheduler_->getScheduledBytesList();
		EV<<"LteSchedulerUeSl::schedule bytes size: "<<bytes.size()<<endl;
		// TODO check if this jump is ok
		if (sdus.empty())
			continue;

		std::map<MacCid, unsigned int>::const_iterator it = sdus.begin(), et = sdus.end();
		for (; it != et; ++it)
		{
			// set schedule list entry
			std::pair<MacCid, Codeword> schedulePair(it->first, cw);
			scheduleList_[schedulePair] = it->second;
		}

		std::map<MacCid, unsigned int>::const_iterator bit = bytes.begin(), bet = bytes.end();
		for (; bit != bet; ++bit)
		{
			// set schedule list entry
			std::pair<MacCid, Codeword> schedulePair(bit->first, cw);
			scheduledBytesList_[schedulePair] = bit->second;
		}

		MacCid highestBackloggedFlow = 0;
		MacCid highestBackloggedPriority = 0;
		MacCid lowestBackloggedFlow = 0;
		MacCid lowestBackloggedPriority = 0;
		bool backlog = false;

		// get the highest backlogged flow id and priority
		backlog = mac_->getHighestBackloggedFlow(highestBackloggedFlow, highestBackloggedPriority);

		if (backlog) // at least one backlogged flow exists
		{
			// get the lowest backlogged flow id and priority
			mac_->getLowestBackloggedFlow(lowestBackloggedFlow, lowestBackloggedPriority);
		}

		// TODO make use of above values
	}

	return &scheduleList_;
}
