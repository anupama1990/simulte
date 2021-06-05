//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#ifndef __ARTERY_LTESIDELINKGRANT_H_
#define __ARTERY_LTESIDELINKGRANT_H_

#include <omnetpp.h>
#include "stack/mac/packet/LteSchedulingGrant_m.h"
#include "LteSchedulingGrant.h"
using namespace omnetpp;

/**
 * TODO - Generated class
 */
class LteSidelinkGrant : public LteSidelinkGrant_Base
{
protected:
	simtime_t startTime;
	std::vector<double> possibleRRIs;
	bool retransmission;
	bool firstTransmission;
	unsigned int timeGapTransRetrans;
	unsigned int spsPriority;
	unsigned int numSubchannels;
	double maximumLatency;
	unsigned int startingSubchannel;
	unsigned int mcs;
	unsigned int retransSubchannel; // It is possible the retransmission has different resources assigned to it.
	unsigned int resourceReselectionCounter;
	int transmitBlockSize;
	unsigned int periodCounter;
	unsigned int expirationCounter;
	int packetId;
	int CAMId;
	MacNodeId destId;
	bool periodic;
	unsigned int period;
	unsigned int expiration;
	// blocks granted on all Remotes, all Bands
	unsigned int totalGrantedBlocks;
	// number of codewords
	unsigned int codewords;
	// granted bytes per codeword
	std::vector<unsigned int> grantedCwBytes;
	RbMap grantedBlocks;
	Direction direction_;
	const UserTxParams* userTxParams;

public:

	LteSidelinkGrant(const char *name = NULL, int kind = 0) :
		LteSidelinkGrant_Base()
	{
		numSubchannels = 0;
		spsPriority = 0;
		maximumLatency = 0.0;
		timeGapTransRetrans = 0;
		startingSubchannel = 0;
		mcs = 0;
		retransSubchannel = 0;
		resourceReselectionCounter = 0;
		firstTransmission = true;
		startTime = simTime();
		packetId = 0;
		CAMId = 0;


		periodic=true;
		period=0;
		expiration=0;
		totalGrantedBlocks=0;
		codewords=0;
		grantedCwBytes.resize(MAX_CODEWORDS);
		userTxParams = nullptr;

	}


	~LteSidelinkGrant()
	{
		if (userTxParams != NULL)
		{
			delete userTxParams;
			userTxParams = NULL;
		}
	}
	LteSidelinkGrant(const LteSidelinkGrant& other) :
		LteSidelinkGrant_Base()
	{
		operator=(other);
	}

	LteSidelinkGrant& operator=(const LteSidelinkGrant& other)
	{
		numSubchannels = other.numSubchannels;
		spsPriority = other.spsPriority;
		startTime = other.startTime;
		maximumLatency = other.maximumLatency;
		timeGapTransRetrans = other.timeGapTransRetrans;
		startingSubchannel = other.startingSubchannel;
		mcs = other.mcs;
		retransSubchannel = other.retransSubchannel;
		resourceReselectionCounter = other.resourceReselectionCounter;
		possibleRRIs = other.possibleRRIs;
		destId = other.destId;
		periodCounter = other.periodCounter;
		expirationCounter = other.expirationCounter;
		packetId = other.packetId;
		CAMId  = other.CAMId;

		periodic=other.periodic;
		period=other.period;
		expiration=other.expiration;
		totalGrantedBlocks=other.totalGrantedBlocks;
		codewords=other.codewords;
		totalGrantedBlocks=other.totalGrantedBlocks;
		grantedCwBytes = other.grantedCwBytes;
		grantedBlocks = other.grantedBlocks;
		direction_ = other.direction_;

		LteSidelinkGrant_Base::operator=(other);
		return *this;
	}

	virtual LteSidelinkGrant *dup() const override
	{
		return new LteSidelinkGrant(*this);
	}
	void setUserTxParams(const UserTxParams* arg)
	{
		if(userTxParams){
			delete userTxParams;
		}
		userTxParams = arg;
	}

	const UserTxParams* getUserTxParams() const
	{
		return userTxParams;
	}
	void setStartTime(simtime_t start)
	{
		startTime = start;
	}
	simtime_t getStartTime() const
	{
		return startTime;
	}
	void setSpsPriority(unsigned int priority)
	{
		spsPriority = priority;
	}
	unsigned int getSpsPriority() const
	{
		return spsPriority;
	}
	void setNumberSubchannels(unsigned int subchannels)
	{
		numSubchannels = subchannels;
	}
	unsigned int getNumSubchannels() const
	{
		return numSubchannels;
	}
	void setMaximumLatency(unsigned int maxLatency)
	{
		maximumLatency = maxLatency;
	}
	unsigned int getMaximumLatency() const
	{
		return maximumLatency;
	}
	void setTimeGapTransRetrans(unsigned int timeGapTransRetrans)
	{
		this->timeGapTransRetrans = timeGapTransRetrans;
	}
	unsigned int getTimeGapTransRetrans() const
	{
		return timeGapTransRetrans;
	}
	void setStartingSubchannel(unsigned int subchannelIndex)
	{
		this->startingSubchannel = subchannelIndex;
	}
	unsigned int getStartingSubchannel() const
	{
		return startingSubchannel;
	}
	void setMcs(unsigned int mcs)
	{
		this->mcs = mcs;
	}
	unsigned int getMcs() const
	{
		return mcs;
	}

	void setTransmitBlockSize(int tbSize)
	{

		this->transmitBlockSize = tbSize;
	}
	int getTransmitBlockSize() const
	{
		return transmitBlockSize;
	}

	void setRetransSubchannel(unsigned int retransSubchannel)
	{
		this->retransSubchannel = retransSubchannel;
	}
	unsigned int getRetransSubchannel() const
	{
		return retransSubchannel;
	}
	void setResourceReselectionCounter(unsigned int resourceReselectionCounter)
	{
		this->resourceReselectionCounter = resourceReselectionCounter;
	}
	unsigned int getResourceReselectionCounter() const
	{
		return resourceReselectionCounter;
	}
	void setRetransmission(bool retransmission)
	{
		this->retransmission = retransmission;
	}
	bool getRetransmission() const
	{
		return retransmission;
	}
	std::vector<double> getPossibleRRIs() const
	{
		return possibleRRIs;
	}
	void setPossibleRRIs(std::vector<double> RRIs)
	{
		this->possibleRRIs = RRIs;
	}
	bool getFirstTransmission() const
	{
		return firstTransmission;
	}
	void setFirstTransmission(bool firstTransmission)
	{
		this->firstTransmission = firstTransmission;
	}

	MacNodeId getDestId() const {
		return destId;
	}

	void setDestId(MacNodeId destId) {
		this->destId = destId;
	}

	unsigned int getExpirationCounter() const{
		return expirationCounter;
	}

	void setExpirationCounter(unsigned int expirationCounter) {
		this->expirationCounter = expirationCounter;
	}

	unsigned int getPeriodCounter() const{
		return periodCounter;
	}

	void setPeriodCounter(unsigned int periodCounter) {
		this->periodCounter = periodCounter;
	}
	void setPacketId(int pid)
	{
		this->packetId = pid;
	}
	int getPacketId() const
	{
		return packetId;
	}

	int getCamId()  const{
		return CAMId;
	}

	void setCamId(int camId) {
		CAMId = camId;
	}

	const RbMap& getGrantedBlocks() const
	{
		return grantedBlocks;
	}

	void setGrantedBlocks(const RbMap& rbMap)
	{
		grantedBlocks = rbMap;
	}
	void setDirection(Direction dir)
	{
		direction_ = dir;
	}
	Direction getDirection() const
	{
		return direction_;
	}
	virtual void setGrantedCwBytesArraySize(size_t size)
	{
		grantedCwBytes.resize(size);
	}
	virtual size_t getGrantedCwBytesArraySize() const
	{
		return grantedCwBytes.size();
	}
	virtual unsigned int getGrantedCwBytes(size_t k) const
	{
		return grantedCwBytes.at(k);
	}
	virtual void setGrantedCwBytes(size_t k, unsigned int grantedCwBytes_var)
	{
		grantedCwBytes[k] = grantedCwBytes_var;
	}

	virtual void insertGrantedCwBytes(unsigned int grantedCwBytes)  {
		throw cRuntimeError("insertGrantedCwBytes not implemented");
	}
	virtual void insertGrantedCwBytes(size_t k, unsigned int grantedCwBytes)  {
		throw cRuntimeError("insertGrantedCwBytes not implemented");
	}
	virtual void eraseGrantedCwBytes(size_t k) override {
		throw cRuntimeError("eraseGrantedCwBytes not implemented");
	}
	//Register_Class{LteSidelinkGrant};

	unsigned int getCodewords() const {
		return codewords;
	}

	void setCodewords(unsigned int codewords) {
		this->codewords = codewords;
	}

	unsigned int getExpiration() const {
		return expiration;
	}

	void setExpiration(unsigned int expiration) {
		this->expiration = expiration;
	}

	unsigned int getPeriod() const {
		return period;
	}

	void setPeriod(unsigned int period) {
		this->period = period;
	}

	bool isPeriodic() const {
		return periodic;
	}

	void setPeriodic(bool periodic) {
		this->periodic = periodic;
	}
	bool getPeriodic() const{
		return periodic;
	}
	unsigned int getTotalGrantedBlocks() const {
		return totalGrantedBlocks;
	}

	void setTotalGrantedBlocks(unsigned int totalGrantedBlocks) {
		this->totalGrantedBlocks = totalGrantedBlocks;
	}
};


//Register_Class{LteSidelinkGrant};


#endif
