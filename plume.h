#ifndef PLUME_H
#define PLUME_H

#include "ns3/application.h"
#include "ns3/ptr.h"
#include "ns3/address.h"

#include <map>
#include <vector>
#include <string>
#include <time.h>

#include "plume-simulator/message.h"
#include "plume-simulator/dag.h"
#include "plume-simulator/block.h"

#include "./rapidjson/include/rapidjson/document.h"
#include "./rapidjson/include/rapidjson/stringbuffer.h"
#include "./rapidjson/include/rapidjson/writer.h"


using namespace ns3;

class Plume : public Application
{
public:
    Plume(void);
    virtual ~Plume(void);
    static TypeId GetTypeId(void);
    void DoDispose(void);

    std::vector<Ipv4Address> GetPeersAddresses();
    std::vector<Ptr<Socket>> GetPeersSocket();

    void HandleAccept(Ptr<Socket> socket, const Address &from);
    void HandleRead(Ptr<Socket> socket);
    void BroadcastNewBlock(const Block &block,Ipv4Address from,bool flag);
    Block CreateNewBlock(void);
    std::vector<std::string> FindAllTips(void);
    std::vector<std::string> AddBlockToLocal(const Block &block);
    void GetOldBlocks(std::vector<std::string> parents);
    void SendBlocks(std::vector<Block> blocks,Ipv4Address dst);

    uint32_t      m_nodeID;
    int           m_seq;
    Ptr<Socket>   m_socket;
    //Address       m_local;
    //TypeId        m_tid;
    int           m_numOfPeers;
    //TODO:local dag表示
    DAG           m_dag;

    std::vector<Ipv4Address>               m_peersAddresses;
    std::map<Ipv4Address, Ptr<Socket>>     m_peersSockets;
    std::map<Address, std::string>         m_bufferedData;
    std::map<std::string, Block*>          m_localBlocks;
    std::map<Block*, BlockHelper*>         m_blockHelpers;

    //const int m_plumePort;
};

#endif
