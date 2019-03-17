#ifndef PLUME_H
#define PLUME_H

#include "ns3/application.h"
#include "ns3/ptr.h"
#include "ns3/address/h"

#include "plume-simulator/message.h"
#include "plume-simulator/dag.h"
#include "plume-simulator/block.h"

using namespace ns3;

class Plume : public Application
{
public:
    Plume(void);
    static TypeId GetTypeId(void);
    virtual ~Plume(void);

    void HandleRecv(Ptr<Socket> scoket);
    void HandleAccept(Ptr<Socket> socket, const Address &from);
    void SendMessage(enum Messages recvType, enum Messages respType, rapidjson::Document &d, Ptr<Socket> socket);
    void SendMessage(enum Messages recvType, enum Messages respType, rapidjson::Document &d, Address &address);
    void BroadcasrNewBlock(const Block &block);

    Ptr<Socket>   m_socket;
    Address       m_local;
    TypeId        m_tid;
    int           m_numOfPeers;
    DAG           m_dag;

    std::vector<Ipv4Address>               m_peersAddresses;
    std::map<IpvsAddress, Ptr<Socket>>     m_peersSockets;

    const int m_plumePort;
};

#endif