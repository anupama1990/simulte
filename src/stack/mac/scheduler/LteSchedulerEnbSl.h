//
//                           SimuLTE
//
// This file is part of a software released under the license included in file
// "license.pdf". This license can be also found at http://www.ltesimulator.com/
// The above file and the present reference are part of the software itself,
// and cannot be removed from it.
//

#ifndef __ARTERY_LTESCHEDULERENBSL_H_
#define __ARTERY_LTESCHEDULERENBSL_H_

#include "stack/mac/scheduler/LcgScheduler.h"
#include "stack/mac/layer/LteMacEnb.h"
#include "stack/mac/packet/LteSidelinkGrant.h"
#include "common/LteCommon.h"
#include "stack/mac/allocator/LteAllocationModule.h"

/**
 * @class LteSchedulerEnbSl
 */

class LteMacEnbD2D;

class LteSchedulerEnbSl: public LcgScheduler
{
protected:
    // Reference to the LTE Binder
    LteBinder *binder_;

    // System allocator, carries out the block-allocation functions.
    LteAllocationModule *allocator_;
    LcgScheduler* lcgScheduler_;
    // Schedule List
    LteMacScheduleList scheduleList_;

    // Scheduled Bytes List
    LteMacScheduleList scheduledBytesList_;
public:
    LteMacEnbD2D* macd2d_;
    /**
     * Default constructor.
     */
    LteSchedulerEnbSl(LteMacEnbD2D * mac);

    /**
     * Destructor.
     */
    virtual ~LteSchedulerEnbSl();


    /* Executes the LCG scheduling algorithm
     * @param availableBytes
     * @return # of scheduled sdus per cid
     */

    LteSidelinkGrant* slGrant;
    LteSchedulingGrant* lteGrant;

    virtual LteMacScheduleList* schedule(LteSidelinkGrant* slgrant);
};
#endif
