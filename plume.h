#ifndef PLUME_H
#define PLUME_H

#include "ns3/application.h"
#include "ns3/ptr.h"
#include "ns3/address.h"
#include "ns3/socket.h"

#include <map>
#include <vector>
#include <string>
#include <time.h>

#include "message.h"
#include "block.h"
#include "blockhelper.h"

#include "./rapidjson/document.h"
#include "./rapidjson/stringbuffer.h"
#include "./rapidjson/writer.h"

namespace ns3 {

class Plume : public Application
{
public:
    Plume(void);
    virtual ~Plume(void);
    static TypeId GetTypeId(void);
    virtual void DoDispose(void);
    virtual void StartApplication(void);
    virtual void StopApplication(void);

    std::vector<Ipv4Address> GetPeersAddresses();
    //std::vector<Ptr<Socket>> GetPeersSocket();

    void HandleAccept(Ptr<Socket> socket, const Address &from);
    void HandleRead(Ptr<Socket> socket);
    void BroadcastNewBlock(const Block &block,Ipv4Address from,bool flag);
    Block CreateNewBlock(void);
    std::vector<std::string> FindAllTips(void);
    std::vector<std::string> AddBlockToLocal(Block &block);
    void SendBlocksReq(std::vector<std::string> parents,Ipv4Address from);
    void SendBlock(const Block& block,Ipv4Address dst);
    void GetNewBlock(void);

    uint32_t      m_nodeID;
    int           m_seq;
    Ptr<Socket>   m_socket;
    //Address       m_local;
    //TypeId        m_tid;
    int           m_numOfPeers;

    std::vector<Ipv4Address>               m_peersAddresses;
    std::map<Ipv4Address, Ptr<Socket>>     m_peersSockets;
    std::map<Address, std::string>         m_bufferedData;
    std::map<std::string, Block*>          m_localBlocks;
    std::map<Block*, BlockHelper*>         m_blockHelpers;

    //const int m_plumePort;
};

}
#endif
