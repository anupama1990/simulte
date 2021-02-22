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

#ifndef __ARTERY_NONIPCONNECTIONSTABLE_H_
#define __ARTERY_NONIPCONNECTIONSTABLE_H_



/**
 * TODO - Generated class
 */
/// This is the maximum number of allowed connections * 2
#define TABLE_SIZE 2048

#include "common/LteCommon.h"
#include <omnetpp.h>

using namespace omnetpp;
/**
 * @class NonIpConnectionsTable
 * @brief Hash table to keep track of connections
 *
 * This is an hash table used by the RRC layer
 * to assign CIDs to different connections.
 * The table is in the format:
 *  __________________________________________________________
 * | srcAddr | dstAddr | srcPort | dstPort | Direction | LCID |
 *
 * A tuple (plus direction) is used to check if connection was already
 * established and return the proper LCID, otherwise a
 * new entry is added to the table
 */
class NonIpConnectionsTable
{
  public:
    NonIpConnectionsTable();
    virtual ~NonIpConnectionsTable();

    /**
     * find_entry() checks if an entry is in the
     * table and returns a proper number.
     *
     * @param srcAddr part of tuple
     * @param dstAddr part of tuple
     * @return value of LCID field in hash table:
     *             - 0xFFFF if no entry was found
     *             - LCID if it was found
     */
    LogicalCid find_entry(long srcAddr, long dstAddr);

    /**
     * find_entry() checks if an entry is in the
     * table and returns a proper number.
     *
     * @param srcAddr part of 2-tuple
     * @param dstAddr part of 2-tuple
     * @param dir flow direction (DL/UL/D2D)
     * @return value of LCID field in hash table:
     *             - 0xFFFF if no entry was found
     *             - LCID if it was found
     */
    LogicalCid find_entry(long srcAddr, long dstAddr, uint16_t dir);

    /**
     * create_entry() adds a new entry to the table
     *
     * @param srcAddr part of tuple
     * @param dstAddr part of tuple
     * @param LCID connection id to insert
     */
    void create_entry(long srcAddr, long dstAddr, LogicalCid lcid);

    /**
     * create_entry() adds a new entry to the table
     *
     * @param srcAddr part of tuple
     * @param dstAddr part of tuple
     * @param dir flow direction (DL/UL/D2D)
     * @param LCID connection id to insert
     */
    void create_entry(long srcAddr, long dstAddr,
         uint16_t dir, LogicalCid lcid);

  private:
    /**
     * hash_func() calculates the hash function used
     * by this structure. At the moment it's simply an OR
     * operation between all fields of the tuple
     *
     * @param srcAddr part of tuple
     * @param dstAddr part of tuple
     */
    unsigned int hash_func(long srcAddr, long dstAddr);

    /**
     * hash_func() calculates the hash function used
     * by this structure. At the moment it's simply an OR
     * operation between all fields of the tuple
     *
     * @param srcAddr part of tuple
     * @param dstAddr part of tuple
     * @param dir flow direction (DL/UL/D2D)
     */
    unsigned int hash_func(long srcAddr, long dstAddr, uint16_t dir);

    /*
     * Data Structures
     */

    /**
     * \struct entry
     * \brief hash table entry
     *
     * This structure contains an entry of the
     * connections hash table. It contains
     * all fields of the tuple and the
     * associated LCID (Logical Connection ID).
     */
    struct entry_
    {
        long srcAddr_;
        long dstAddr_;
        uint16_t dir_;
        LogicalCid lcid_;
    };
    /// Hash table of size TABLE_SIZE
    entry_ NonIpHt_[TABLE_SIZE];
};


#endif
