//
//  Assignment.cpp
//  hifi
//
//  Created by Stephen Birarda on 8/22/13.
//  Copyright (c) 2013 HighFidelity, Inc. All rights reserved.
//

#include "PacketHeaders.h"
#include "SharedUtil.h"
#include "UUID.h"

#include <QtCore/QDataStream>

#include "Assignment.h"

Assignment::Type Assignment::typeForNodeType(NodeType_t nodeType) {
    switch (nodeType) {
        case NodeType::AudioMixer:
            return Assignment::AudioMixerType;
        case NodeType::AvatarMixer:
            return Assignment::AvatarMixerType;
        case NodeType::Agent:
            return Assignment::AgentType;
        case NodeType::VoxelServer:
            return Assignment::VoxelServerType;
        case NodeType::ParticleServer:
            return Assignment::ParticleServerType;
        case NodeType::MetavoxelServer:
            return Assignment::MetavoxelServerType;
        default:
            return Assignment::AllTypes;
    }
}

#ifdef WIN32
//warning C4351: new behavior: elements of array 'Assignment::_payload' will be default initialized 
// We're disabling this warning because the new behavior which is to initialize the array with 0 is acceptable to us.
#pragma warning(disable:4351) 
#endif

Assignment::Assignment() :
    _uuid(),
    _command(Assignment::RequestCommand),
    _type(Assignment::AllTypes),
    _pool(),
    _location(Assignment::LocalLocation),
    _payload()
{
    
}

Assignment::Assignment(Assignment::Command command, Assignment::Type type, const QString& pool, Assignment::Location location) :
    _uuid(),
    _command(command),
    _type(type),
    _pool(pool),
    _location(location),
    _payload()
{
    if (_command == Assignment::CreateCommand) {
        // this is a newly created assignment, generate a random UUID
        _uuid = QUuid::createUuid();
    }
}

Assignment::Assignment(const QByteArray& packet) :
    _pool(),
    _location(GlobalLocation),
    _payload()
{
    PacketType packetType = packetTypeForPacket(packet);
    
    if (packetType == PacketTypeRequestAssignment) {
        _command = Assignment::RequestCommand;
    } else if (packetType == PacketTypeCreateAssignment) {
        _command = Assignment::CreateCommand;
    }
    
    QDataStream packetStream(packet);
    packetStream.skipRawData(numBytesForPacketHeader(packet));
    
    packetStream >> *this;
}

#ifdef WIN32
#pragma warning(default:4351) 
#endif


Assignment::Assignment(const Assignment& otherAssignment) {
    
    _uuid = otherAssignment._uuid;
    
    _command = otherAssignment._command;
    _type = otherAssignment._type;
    _location = otherAssignment._location;
    _pool = otherAssignment._pool;
    _payload = otherAssignment._payload;
}

Assignment& Assignment::operator=(const Assignment& rhsAssignment) {
    Assignment temp(rhsAssignment);
    swap(temp);
    return *this;
}

void Assignment::swap(Assignment& otherAssignment) {
    using std::swap;
    
    swap(_uuid, otherAssignment._uuid);
    swap(_command, otherAssignment._command);
    swap(_type, otherAssignment._type);
    swap(_location, otherAssignment._location);
    swap(_pool, otherAssignment._pool);
    swap(_payload, otherAssignment._payload);
}

const char* Assignment::getTypeName() const {
    switch (_type) {
        case Assignment::AudioMixerType:
            return "audio-mixer";
        case Assignment::AvatarMixerType:
            return "avatar-mixer";
        case Assignment::AgentType:
            return "agent";
        case Assignment::VoxelServerType:
            return "voxel-server";
        case Assignment::ParticleServerType:
            return "particle-server";
        case Assignment::MetavoxelServerType:
            return "metavoxel-server";
        default:
            return "unknown";
    }
}

QDebug operator<<(QDebug debug, const Assignment &assignment) {
    debug.nospace() << "UUID: " << qPrintable(assignment.getUUID().toString()) <<
        ", Type: " << assignment.getType();
    
    if (!assignment.getPool().isEmpty()) {
        debug << ", Pool: " << assignment.getPool();
    }

    return debug.space();
}

QDataStream& operator<<(QDataStream &out, const Assignment& assignment) {
    out << (quint8) assignment._type << assignment._uuid << assignment._pool << assignment._payload;
    
    return out;
}

QDataStream& operator>>(QDataStream &in, Assignment& assignment) {
    quint8 packedType;
    in >> packedType;
    assignment._type = (Assignment::Type) packedType;
    
    in >> assignment._uuid >> assignment._pool >> assignment._payload;
    
    return in;
}
