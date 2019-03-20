#ifndef PLUME_H
#define PLUME_H

#include "ns3/application.h"
#include "ns3/ptr.h"
#include "ns3/address.h"

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

    std::vector<Ipv4Address> GetPeersAddresses();
    std::vector<Ptr<Socket>> GetPeersSocket();
    void HandleAccept(Ptr<Socket> socket, const Address &from);
    void HandleRead(Ptr<Socket> socket);
    void SendMessage(enum Messages recvType, enum Messages respType, rapidjson::Document &d, Ptr<Socket> socket);
    void BroadcastNewBlock(const Block &block,Ipv4Address from);
    void BroadcastNewBlock(const Block &block)

    uint32_t      m_nodeID;
    Ptr<Socket>   m_socket;
    //Address       m_local;
    //TypeId        m_tid;
    int           m_numOfPeers;
    //TODO:local dag表示
    DAG           m_dag;

    std::vector<Ipv4Address>               m_peersAddresses;
    std::map<Ipv4Address, Ptr<Socket>>     m_peersSockets;
    std::map<Address, std::string>         m_bufferedData;
    std::map<string, Block*>               m_localBlocks;

    //const int m_plumePort;
};

#endif