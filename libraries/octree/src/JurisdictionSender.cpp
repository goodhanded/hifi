//
//  JurisdictionSender.cpp
//  shared
//
//  Created by Brad Hefta-Gaub on 8/12/13.
//  Copyright (c) 2013 High Fidelity, Inc. All rights reserved.
//
//  Threaded or non-threaded jurisdiction Sender for the Application
//

#include <PerfStat.h>

#include <OctalCode.h>
#include <SharedUtil.h>
#include <PacketHeaders.h>
#include "JurisdictionSender.h"


JurisdictionSender::JurisdictionSender(JurisdictionMap* map, NodeType_t type) :
    ReceivedPacketProcessor(),
    _jurisdictionMap(map),
    _nodeType(type),
    _packetSender(JurisdictionSender::DEFAULT_PACKETS_PER_SECOND)
{
}

JurisdictionSender::~JurisdictionSender() {
}


void JurisdictionSender::processPacket(const SharedNodePointer& sendingNode, const QByteArray& packet) {
    if (packetTypeForPacket(packet) == PacketTypeJurisdictionRequest) {
        if (sendingNode) {
            lockRequestingNodes();
            _nodesRequestingJurisdictions.push(sendingNode->getUUID());
            unlockRequestingNodes();
        }
    }
}

bool JurisdictionSender::process() {
    bool continueProcessing = isStillRunning();

    // call our ReceivedPacketProcessor base class process so we'll get any pending packets
    if (continueProcessing && (continueProcessing = ReceivedPacketProcessor::process())) {
        // add our packet to our own queue, then let the PacketSender class do the rest of the work.
        static unsigned char buffer[MAX_PACKET_SIZE];
        unsigned char* bufferOut = &buffer[0];
        ssize_t sizeOut = 0;

        if (_jurisdictionMap) {
            sizeOut = _jurisdictionMap->packIntoMessage(bufferOut, MAX_PACKET_SIZE);
        } else {
            sizeOut = JurisdictionMap::packEmptyJurisdictionIntoMessage(getNodeType(), bufferOut, MAX_PACKET_SIZE);
        }
        int nodeCount = 0;

        lockRequestingNodes();
        while (!_nodesRequestingJurisdictions.empty()) {

            QUuid nodeUUID = _nodesRequestingJurisdictions.front();
            _nodesRequestingJurisdictions.pop();
            SharedNodePointer node = NodeList::getInstance()->nodeWithUUID(nodeUUID);

            if (node && node->getActiveSocket()) {
                _packetSender.queuePacketForSending(node, QByteArray(reinterpret_cast<char *>(bufferOut), sizeOut));
                nodeCount++;
            }
        }
        unlockRequestingNodes();

        // set our packets per second to be the number of nodes
        _packetSender.setPacketsPerSecond(nodeCount);

        continueProcessing = _packetSender.process();
    }
    return continueProcessing;
}
