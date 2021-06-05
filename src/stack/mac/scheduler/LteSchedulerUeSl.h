//
//                           SimuLTE
//
// This file is part of a software released under the license included in file
// "license.pdf". This license can be also found at http://www.ltesimulator.com/
// The above file and the present reference are part of the software itself,
// and cannot be removed from it.
//

#ifndef __ARTERY_LTESCHEDULERUESL_H_
#define __ARTERY_LTESCHEDULERUESL_H_

#include "stack/mac/scheduler/LcgScheduler.h"
#include "stack/mac/layer/LteMacUeD2D.h"
#include "stack/mac/packet/LteSidelinkGrant.h"
#include "common/LteCommon.h"
#include "stack/mac/allocator/LteAllocationModule.h"

/**
 * @class LteSchedulerUeSl
 */

class LteMacUeD2D;

class LteSchedulerUeSl: public LcgScheduler
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
    LteMacUeD2D* macd2d_;
    /**
     * Default constructor.
     */
    LteSchedulerUeSl(LteMacUeD2D * mac);

    /**
     * Destructor.
     */
    virtual ~LteSchedulerUeSl();


    /* Executes the LCG scheduling algorithm
     * @param availableBytes
     * @return # of scheduled sdus per cid
     */

    LteSidelinkGrant* slGrant;
    LteSchedulingGrant* lteGrant;

    virtual LteMacScheduleList* schedule(LteSidelinkGrant* slgrant);
};
#endif
