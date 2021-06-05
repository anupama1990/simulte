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

#ifndef EXTERN_SIMULTE_SRC_COMMON_FLOWCONTROLINFONONIP_H_
#define EXTERN_SIMULTE_SRC_COMMON_FLOWCONTROLINFONONIP_H_


#include "common/LteControlInfo_m.h"
class FlowControlInfoNonIp : public FlowControlInfoNonIp_Base{
public:
    FlowControlInfoNonIp();
    virtual ~FlowControlInfoNonIp();

    /*
     * Operator = : packet copy
     * @param other source packet
     * @return reference to this packet
     */
    FlowControlInfoNonIp& operator=(const FlowControlInfoNonIp& other);

    /**
     * Copy constructor: packet copy
     * @param other source packet
     */
    FlowControlInfoNonIp(const FlowControlInfoNonIp& other) :
        FlowControlInfoNonIp_Base()
    {
        operator=(other);
    }

    /**
     * dup() : packet duplicate
     * @return pointer to duplicate packet
     */
    virtual FlowControlInfoNonIp *dup() const
    {
        return new FlowControlInfoNonIp(*this);
    }
    std::map<MacNodeId,inet::Coord> BroadcastUeInfo;

    std::map<MacNodeId, inet::Coord> getBroadcastUeInfo() const {
        return BroadcastUeInfo;
    }

    void setBroadcastUeInfo(std::map<MacNodeId, inet::Coord> broadcastUeInfo) {
        BroadcastUeInfo = broadcastUeInfo;
    }

};


Register_Class(FlowControlInfoNonIp);
#endif /* EXTERN_SIMULTE_SRC_COMMON_FLOWCONTROLINFONONIP_H_ */
