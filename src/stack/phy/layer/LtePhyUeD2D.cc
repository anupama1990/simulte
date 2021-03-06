//
//                           SimuLTE
//
// This file is part of a software released under the license included in file
// "license.pdf". This license can be also found at http://www.ltesimulator.com/
// The above file and the present reference are part of the software itself,
// and cannot be removed from it.
//

#include <assert.h>
#include "stack/phy/layer/LtePhyUeD2D.h"
#include "stack/phy/packet/LteFeedbackPkt.h"
#include "stack/d2dModeSelection/D2DModeSelectionBase.h"
#include "stack/mac/packet/SPSResourcePoolMode3.h"
#include "stack/phy/packet/SPSResourcePool.h"

Define_Module(LtePhyUeD2D);
using namespace inet;

LtePhyUeD2D::LtePhyUeD2D()
{
	handoverStarter_ = nullptr;
	handoverTrigger_ = nullptr;
}

LtePhyUeD2D::~LtePhyUeD2D()
{
	SPSResourcePool* csrpkt = new SPSResourcePool("CSRs");
	delete csrpkt;
}

void LtePhyUeD2D::initialize(int stage)
{
	LtePhyUe::initialize(stage);
	if (stage == INITSTAGE_LOCAL)
	{
		averageCqiD2D_ = registerSignal("averageCqiD2D");
		d2dTxPower_ = par("d2dTxPower");
		d2dMulticastEnableCaptureEffect_ = par("d2dMulticastCaptureEffect");
		d2dDecodingTimer_ = nullptr;
	}
	frameSent = false;
}

void LtePhyUeD2D::handleSelfMessage(cMessage *msg)
{
	if (msg->isName("d2dDecodingTimer"))
	{
		// select one frame from the buffer. Implements the capture effect
		LteAirFrame* frame = extractAirFrame();
		UserControlInfo* lteInfo = check_and_cast<UserControlInfo*>(frame->removeControlInfo());

		// decode the selected frame
		decodeAirFrame(frame, lteInfo);

		// clear buffer
		while (!d2dReceivedFrames_.empty())
		{
			LteAirFrame* frame = d2dReceivedFrames_.back();
			d2dReceivedFrames_.pop_back();
			delete frame;
		}

		delete msg;
		d2dDecodingTimer_ = nullptr;
	}
	else if (msg->isName("doModeSwitchAtHandover"))
	{
		// call mode selection module to check if DM connections are possible
		cModule* enb = getSimulation()->getModule(binder_->getOmnetId(masterId_));
		D2DModeSelectionBase *d2dModeSelection = check_and_cast<D2DModeSelectionBase*>(enb->getSubmodule("lteNic")->getSubmodule("d2dModeSelection"));
		d2dModeSelection->doModeSwitchAtHandover(nodeId_, true);

		delete msg;
	}
	else
		LtePhyUe::handleSelfMessage(msg);
}

// TODO: ***reorganize*** method
void LtePhyUeD2D::handleAirFrame(cMessage* msg)
{
	if(strcmp(msg->getName(), "RRCStateChange")== 0)
	{
		//throw cRuntimeError("RRC Enb");
		delete msg;
		return;
	}

	UserControlInfo* lteInfo = check_and_cast<UserControlInfo*>(msg->removeControlInfo());
	receivedPacket = false;
	EV<<"LtePhyUeD2D::handleAirFrame Received: "<<msg->getName()<<endl;
	if (useBattery_)
	{
		//TODO BatteryAccess::drawCurrent(rxAmount_, 0);
	}

	connectedNodeId_ = masterId_;
	LteAirFrame* frame = check_and_cast<LteAirFrame*>(msg);


	EV << "LtePhyUeD2D: received new LteAirFrame with ID " << lteInfo->getFrameType() << " from channel" << "source Id: "<<lteInfo->getSourceId()<<
			"Destination Id: "<<lteInfo->getDestId()<<"Node Id: "<<nodeId_<<endl;

	int sourceId = binder_->getOmnetId(lteInfo->getSourceId());
	if(sourceId == 0 )
	{

		// source has left the simulation
		delete msg;
		return;
	}

	//Update coordinates of this user
	if (lteInfo->getFrameType() == HANDOVERPKT)
	{
		EV<<"Check for HANDOVERPKT "<<endl;
		// check if handover is already in process
		if (handoverTrigger_ != NULL && handoverTrigger_->isScheduled())
		{
			delete lteInfo;
			delete frame;
			return;
		}

		handoverHandler(frame, lteInfo);
		return;
	}

	if (lteInfo->getFrameType() == SCIPKT || lteInfo->getFrameType() ==DATAPKT)
	{

		//Process SCI
		frame->setControlInfo(lteInfo);

		if(lteInfo->getGrantStartTime()==NOW)
		{
			halfDuplexError=true;
			receivedPacket=false;
			lteInfo->setDeciderResult(false);
		}
		else
		{
			halfDuplexError= false;
			receivedPacket=true;
			lteInfo->setDeciderResult(true);

		}

		//Statistics collection
		if (lteInfo->getFrameType()==DATAPKT)
		{
			numAirFrameReceived_ = numAirFrameReceived_+1;
			//emit(numberReceivedPackets,numAirFrameReceived_);
		}


		return;
	}


	if(lteInfo->getFrameType()==CSRPKT)
	{

		SPSResourcePoolMode3* csr  = check_and_cast<SPSResourcePoolMode3*>(frame->decapsulate());

		csr->setControlInfo(lteInfo);
		EV<<"CSRPKT LtePhyUeD2D: "<<csr<<endl;
		send(csr, upperGateOut_);

		return;

	}


	// Check if the frame is for us ( MacNodeId matches or - if this is a multicast communication - enrolled in multicast group)
	/*if (lteInfo->getDestId() != nodeId_ && !(binder_->isInMulticastGroup(nodeId_, lteInfo->getMulticastGroupId())))
	    {
	        EV << "ERROR: Frame is not for us. Delete it LtePhyUeD2D." << endl;
	        EV << "Packet Type: " << phyFrameTypeToA((LtePhyFrameType)lteInfo->getFrameType()) << endl;
	        EV << "Frame MacNodeId: " << lteInfo->getDestId() << endl;
	        EV << "Local MacNodeId: " << nodeId_ << endl;
	        delete lteInfo;
	        delete frame;
	        return;
	    }*/

	if (binder_->isInMulticastGroup(nodeId_,lteInfo->getMulticastGroupId()))
	{
		// HACK: if this is a multicast connection, change the destId of the airframe so that upper layers can handle it
		lteInfo->setDestId(nodeId_);
	}

	// send H-ARQ feedback up
	if (lteInfo->getFrameType() == HARQPKT || lteInfo->getFrameType() == GRANTPKT || lteInfo->getFrameType() == RACPKT || lteInfo->getFrameType() == D2DMODESWITCHPKT)
	{
		handleControlMsg(frame, lteInfo);
		return;
	}

	// if the packet is a D2D multicast one, store it and decode it at the end of the TTI
	if (d2dMulticastEnableCaptureEffect_ && binder_->isInMulticastGroup(nodeId_,lteInfo->getMulticastGroupId()))
	{
		// if not already started, auto-send a message to signal the presence of data to be decoded
		if (d2dDecodingTimer_ == NULL)
		{
			d2dDecodingTimer_ = new cMessage("d2dDecodingTimer");
			d2dDecodingTimer_->setSchedulingPriority(10);          // last thing to be performed in this TTI
			scheduleAt(NOW, d2dDecodingTimer_);
		}

		// store frame, together with related control info
		frame->setControlInfo(lteInfo);
		storeAirFrame(frame);            // implements the capture effect

		return;                          // exit the function, decoding will be done later
	}

	if ((lteInfo->getUserTxParams()) != NULL)
	{
		int cw = lteInfo->getCw();
		if (lteInfo->getUserTxParams()->readCqiVector().size() == 1)
			cw = 0;
		double cqi = lteInfo->getUserTxParams()->readCqiVector()[cw];
		//if (lteInfo->getDirection() == DL)

	}

	/*
	    RemoteSet r = lteInfo->getUserTxParams()->readAntennaSet();
	    if (r.size() > 1)
	    {
	        // DAS
	        for (RemoteSet::iterator it = r.begin(); it != r.end(); it++)
	        {
	            EV << "LtePhyUeD2D: Receiving Packet from antenna " << (*it) << "\n";
	 * On UE set the sender position
	 * and tx power to the sender das antenna
	            //            cc->updateHostPosition(myHostRef,das_->getAntennaCoord(*it));
	            // Set position of sender
	            //            Move m;
	            //            m.setStart(das_->getAntennaCoord(*it));
	            RemoteUnitPhyData data;
	            data.txPower=lteInfo->getTxPower();
	            data.m=getRadioPosition();
	            frame->addRemoteUnitPhyDataVector(data);
	        }
	        // apply analog models For DAS
	        result=channelModel_->isCorruptedDas(frame,lteInfo);
	    }
	    else
	    {
	        //RELAY and NORMAL
	        result = channelModel_->isCorrupted(frame,lteInfo);
	    }
	 */

}

void LtePhyUeD2D::triggerHandover()
{
	// stop active D2D flows (go back to Infrastructure mode)
	// currently, DM is possible only for UEs served by the same cell

	// trigger D2D mode switch
	/*cModule* enb = getSimulation()->getModule(binder_->getOmnetId(masterId_));
	D2DModeSelectionBase *d2dModeSelection = check_and_cast<D2DModeSelectionBase*>(enb->getSubmodule("lteNic")->getSubmodule("d2dModeSelection"));
	d2dModeSelection->doModeSwitchAtHandover(nodeId_, false);*/

	LtePhyUe::triggerHandover();
}

void LtePhyUeD2D::doHandover()
{
	// amc calls
	LteAmc *oldAmc = getAmcModule(masterId_);
	LteAmc *newAmc = getAmcModule(candidateMasterId_);
	assert(newAmc != nullptr);
	oldAmc->detachUser(nodeId_, D2D);
	newAmc->attachUser(nodeId_, D2D);

	LtePhyUe::doHandover();

	// send a self-message to schedule the possible mode switch at the end of the TTI (after all UEs have performed the handover)
	cMessage* msg = new cMessage("doModeSwitchAtHandover");
	msg->setSchedulingPriority(10);
	scheduleAt(NOW, msg);
}

void LtePhyUeD2D::handleUpperMessage(cMessage* msg)
{
	//    if (useBattery_) {
	//    TODO     BatteryAccess::drawCurrent(txAmount_, 1);
	//    }
	if(strcmp(msg->getName(), "LteMacPdu")== 0)
	{
		UserControlInfo* lteInfo = new UserControlInfo();
		EV<<"Broadcasting sidelink control information (SCI)"<<endl;
		//sendBroadcast(sciframe);

		//Prepare data frame for broadcast
		frame = new LteAirFrame("airframePdu");
		frame->encapsulate(check_and_cast<cPacket*>(msg));
		LteRealisticChannelModel* chan = check_and_cast< LteRealisticChannelModel*>(getParentModule()->getSubmodule("channelModel"));

		EV<<"Computing RSRP and RSSI vectors: "<<endl;
		EV<<"Number of UEs: "<<binder_->getUeList()->size()<<endl;


		// initialize frame fields

		frame->setSchedulingPriority(airFramePriority_);
		frame->setDuration(TTI);
		// set current position
		lteInfo->setCoord(getRadioPosition());
		lteInfo->setTxPower(txPower_);
		lteInfo->setD2dTxPower(d2dTxPower_);
		frame->setControlInfo(lteInfo);

		sendBroadcast(frame);
		frameSent=true;
		return;
	}
	if(strcmp(msg->getName(), "RRCStateChange")== 0)
	{
		RRCStateChange* rrcstate = check_and_cast<RRCStateChange*> (msg);
		rrcCurrentState = rrcstate->getState();
		EV<<"RRC state PHY: "<<rrcCurrentState<<endl;
		return;
	}

	if(strcmp(msg->getName(), "LteMode3Grant")== 0)
	{
		enb = new LtePhyEnbD2D();
		enb->handleUpperMessage(msg);
	}


	if(strcmp(msg->getName(), "LteMode4Grant")== 0)
	{
		EV<<"LtePhyUeD2D::handleUpperMessage received sidelink grant"<<endl;
		Packet* pkt = check_and_cast<Packet*>(msg);
		SidelinkResourceAllocation* sra = check_and_cast<SidelinkResourceAllocation*>(getParentModule()->getSubmodule("mode4"));
		sciframe = sra->createSCIMessage(msg,pkt);
		EV<<"LtePhyUeD2D::handleUpperMessage received sidelink grant"<<endl;
		sra->handleUpperMessage(pkt);

		//SCI
		sciframe->setSchedulingPriority(airFramePriority_);
		sciframe->setDuration(TTI);

		return;
	}
	if (strcmp(msg->getName(), "CSRsPrevious")==0)
	{
		SPSResourcePool* grantedblocksprevious = check_and_cast<SPSResourcePool *>(msg);
		SidelinkResourceAllocation* sra = check_and_cast<SidelinkResourceAllocation*>(getParentModule()->getSubmodule("mode4"));
		sra->setAllocatedBlocksPrevious(grantedblocksprevious->getAllocatedBlocksScIandDataPrevious());
		EV<<"Allocated blocks previous: "<<grantedblocksprevious->getAllocatedBlocksScIandDataPrevious()<<endl;
		return;
	}


	auto pkt = check_and_cast<inet::Packet *>(msg);
	auto lteInfo = pkt->removeTag<UserControlInfo>();
	if(lteInfo->getIpBased()==true)
	{
		EV<<"Check for IPBased: "<<lteInfo->getIpBased()<< "Transmitting packet ID: "<<lteInfo->getPktId()<<endl;
		numAirFrameAlertTransmitted_ = numAirFrameAlertTransmitted_ +1;
		emit(numberAlertTransmittedPackets,numAirFrameAlertTransmitted_ );  //Number of packets sent across the radio interface
		emit(AlertTrPktId,lteInfo->getPktId());

	}

	if (lteInfo->getIpBased()==false)
	{

	}

	if ((lteInfo->getFrameType()==MODE3GRANTREQUEST))
	{
		frame = new LteAirFrame("Mode3GrantRequest");
		lteInfo->setSourceId(nodeId_);
		lteInfo->setDestId(candidateMasterId_);
		frame->encapsulate(check_and_cast<cPacket*>(msg));
		frame->setDuration(TTI);
		frame->setControlInfo(lteInfo.get());

		OmnetId destOmnetId = binder_->getOmnetId(candidateMasterId_);

		EV<<"Sending mode 3 grant request to eNodeB"<<endl;
		sendUnicast(frame);
		return;
	}

	if ((lteInfo->getFrameType()==DATAARRIVAL))
	{
		frame = new LteAirFrame("Data Arrival");
		lteInfo->setSourceId(nodeId_);
		lteInfo->setDestId(candidateMasterId_);
		frame->encapsulate(check_and_cast<cPacket*>(msg));
		frame->setDuration(TTI);
		frame->setControlInfo(lteInfo.get());

		OmnetId destOmnetId = binder_->getOmnetId(candidateMasterId_);

		EV<<"Inform eNodeB about newDataPkt"<<endl;
		sendUnicast(frame);

		return;
	}

	if (lteInfo->getFrameType() == HARQPKT || lteInfo->getFrameType() == GRANTPKT || lteInfo->getFrameType() == RACPKT)
	{
		frame = new LteAirFrame("harqFeedback-grant");

	}



	//Decrement the re-selection counter on successful data frame transmission in mode 4

	if (frameSent==true && rrcCurrentState=="RRC_IDLE")
	{
		SidelinkResourceAllocation* sra = check_and_cast<SidelinkResourceAllocation*>(getParentModule()->getSubmodule("mode4"));
		cResel = sra->getReselectionCounter();
		cResel = cResel-1;
		sra->setReselectionCounter(cResel);
		EV<<"Successful transmission of TB, cResel: "<<cResel<<endl;
		frameSent=false;

		if (cResel <0)
		{
			cResel=0;
			sra->setReselectionCounter(cResel);
		}
		numAirFrameTransmitted_ = numAirFrameTransmitted_+1;
	}


}

void LtePhyUeD2D::storeAirFrame(LteAirFrame* newFrame)
{
	// implements the capture effect
	// store the frame received from the nearest transmitter
	UserControlInfo* newInfo = check_and_cast<UserControlInfo*>(newFrame->getControlInfo());
	Coord myCoord = getCoord();
	double distance = 0.0;
	double rsrpMean = 0.0;
	std::vector<double> rsrpVector;
	bool useRsrp = false;

	if (strcmp(par("d2dMulticastCaptureEffectFactor"),"RSRP") == 0)
	{
		useRsrp = true;

		double sum = 0.0;
		unsigned int allocatedRbs = 0;
		rsrpVector = channelModel_->getRSRP_D2D(newFrame, newInfo, nodeId_, myCoord);

		// get the average RSRP on the RBs allocated for the transmission
		RbMap rbmap = newInfo->getGrantedBlocks();
		RbMap::iterator it;
		std::map<Band, unsigned int>::iterator jt;
		//for each Remote unit used to transmit the packet
		for (it = rbmap.begin(); it != rbmap.end(); ++it)
		{
			//for each logical band used to transmit the packet
			for (jt = it->second.begin(); jt != it->second.end(); ++jt)
			{
				Band band = jt->first;
				if (jt->second == 0) // this Rb is not allocated
					continue;

				sum += rsrpVector.at(band);
				allocatedRbs++;
			}
		}
		rsrpMean = sum / allocatedRbs;
		EV << "LtePhyUeD2D::storeAirFrame - Average RSRP from node " << newInfo->getSourceId() << ": " << rsrpMean << endl;
	}
	else  // distance
	{
		Coord newSenderCoord = newInfo->getCoord();
		distance = myCoord.distance(newSenderCoord);
		EV << NOW << " LtePhyUeD2D::storeAirFrame - Distance from node " << newInfo->getSourceId() << ": " << distance << endl;
	}

	if (!d2dReceivedFrames_.empty())
	{
		LteAirFrame* prevFrame = d2dReceivedFrames_.front();
		if (!useRsrp && distance < nearestDistance_)
		{
			EV << "[ < nearestDistance: " << nearestDistance_ << "]" << endl;

			// remove the previous frame
			d2dReceivedFrames_.pop_back();
			delete prevFrame;

			nearestDistance_ = distance;
			d2dReceivedFrames_.push_back(newFrame);
		}
		else if (rsrpMean > bestRsrpMean_)
		{
			EV << "[ > bestRsrp: " << bestRsrpMean_ << "]" << endl;

			// remove the previous frame
			d2dReceivedFrames_.pop_back();
			delete prevFrame;

			bestRsrpMean_ = rsrpMean;
			bestRsrpVector_ = rsrpVector;
			d2dReceivedFrames_.push_back(newFrame);
		}
		else
		{
			// this frame will not be decoded
			delete newFrame;
		}
	}
	else
	{
		if (!useRsrp)
		{
			nearestDistance_ = distance;
			d2dReceivedFrames_.push_back(newFrame);
		}
		else
		{
			bestRsrpMean_ = rsrpMean;
			bestRsrpVector_ = rsrpVector;
			d2dReceivedFrames_.push_back(newFrame);
		}
	}
}

LteAirFrame* LtePhyUeD2D::extractAirFrame()
{
	// implements the capture effect
	// the vector is storing the frame received from the strongest/nearest transmitter

	return d2dReceivedFrames_.front();
}

void LtePhyUeD2D::decodeAirFrame(LteAirFrame* frame, UserControlInfo* lteInfo)
{
	EV << NOW << " LtePhyUeD2D::decodeAirFrame - Start decoding..." << endl;

	// apply decider to received packet
	bool result = true;

	RemoteSet r = lteInfo->getUserTxParams()->readAntennaSet();
	if (r.size() > 1)
	{
		// DAS
		for (RemoteSet::iterator it = r.begin(); it != r.end(); it++)
		{
			EV << "LtePhyUeD2D::decodeAirFrame: Receiving Packet from antenna " << (*it) << endl;

			/*
			 * On UE set the sender position
			 * and tx power to the sender das antenna
			 */

			//            cc->updateHostPosition(myHostRef,das_->getAntennaCoord(*it));
			// Set position of sender
			//            Move m;
			//            m.setStart(das_->getAntennaCoord(*it));
			RemoteUnitPhyData data;
			data.txPower=lteInfo->getTxPower();
			data.m=getRadioPosition();
			frame->addRemoteUnitPhyDataVector(data);
		}
		// apply analog models For DAS
		result=channelModel_->isCorruptedDas(frame,lteInfo);
	}
	else
	{
		//RELAY and NORMAL
		if (lteInfo->getDirection() == D2D_MULTI)
			result = channelModel_->isCorrupted_D2D(frame,lteInfo,bestRsrpVector_);
		else
			result = channelModel_->isCorrupted(frame,lteInfo);
	}

	// update statistics
	if (result)
		numAirFrameReceived_++;
	else
		numAirFrameNotReceived_++;

	EV << "Handled LteAirframe with ID " << frame->getId() << " with result "
			<< ( result ? "RECEIVED" : "NOT RECEIVED" ) << endl;

	auto pkt = check_and_cast<inet::Packet *>(frame->decapsulate());

	// Note: no need to delete the frame itself - will be deleted later when the buffer of
	// received frames is cleared

	// attach the decider result to the packet as control info
	lteInfo->setDeciderResult(result);
	*(pkt->addTagIfAbsent<UserControlInfo>()) = *lteInfo;
	delete lteInfo;

	// send decapsulated message along with result control info to upperGateOut_
	send(pkt, upperGateOut_);

	if (getEnvir()->isGUI())
		updateDisplayString();
}


void LtePhyUeD2D::sendFeedback(LteFeedbackDoubleVector fbDl, LteFeedbackDoubleVector fbUl, FeedbackRequest req)
{
	Enter_Method("SendFeedback");
	EV << "LtePhyUeD2D: feedback from Feedback Generator" << endl;

	//Create a feedback packet
	auto fbPkt = makeShared<LteFeedbackPkt>();
	//Set the feedback
	fbPkt->setLteFeedbackDoubleVectorDl(fbDl);
	fbPkt->setLteFeedbackDoubleVectorDl(fbUl);
	fbPkt->setSourceNodeId(nodeId_);

	auto pkt = new Packet("feedback_pkt");
	pkt->insertAtFront(fbPkt);

	UserControlInfo* uinfo = new UserControlInfo();
	uinfo->setSourceId(nodeId_);
	uinfo->setDestId(masterId_);
	uinfo->setFrameType(FEEDBACKPKT);
	uinfo->setIsCorruptible(false);
	// create LteAirFrame and encapsulate a feedback packet
	LteAirFrame* frame = new LteAirFrame("feedback_pkt");
	frame->encapsulate(check_and_cast<cPacket*>(pkt));
	uinfo->feedbackReq = req;
	uinfo->setDirection(UL);
	simtime_t signalLength = TTI;
	uinfo->setTxPower(txPower_);
	uinfo->setD2dTxPower(d2dTxPower_);
	// initialize frame fields

	frame->setSchedulingPriority(airFramePriority_);
	frame->setDuration(signalLength);
	uinfo->setCoord(getRadioPosition());
	frame->setControlInfo(uinfo);

	lastFeedback_ = NOW;
	EV << "LtePhy: " << nodeTypeToA(nodeType_) << " with id "
			<< nodeId_ << " sending feedback to the air channel" << endl;
	sendUnicast(frame);
}

void LtePhyUeD2D::finish()
{
	if (getSimulation()->getSimulationStage() != CTX_FINISH)
	{
		// do this only at deletion of the module during the simulation

		if (rrcCurrentState=="RRC_CONN")
			// amc calls
		{
			LteAmc *amc = getAmcModule(masterId_);

			if (amc != NULL)
				amc->detachUser(nodeId_, D2D);
			LtePhyUe::finish();
		}

	}
}
