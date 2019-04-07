#include "ns3/address.h"
#include "ns3/address-utils.h"
#include "ns3/log.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/node.h"
#include "ns3/socket.h"
#include "ns3/udp-socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/tcp-socket-factory.h"
#include "ns3/uinteger.h"
#include "ns3/double.h"

#include "plume.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("plume");

NS_OBJECT_ENSURE_REGISTERED (Plume);

TypeId Plume::GetTypeId(void) {
static TypeId tid = TypeId("ns3::Plume")
        .SetParent<Application>()
        .SetGroupName("Applications")
        .AddConstructor<Plume>();
    return tid;
}
Plume::Plume(void) {

}

Plume::~Plume(void) {

}

void Plume::DoDispose(void) {
    m_socket = 0;
    Application::DoDispose();
}

void Plume::StartApplication(void) {
    InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(),8787);
    if (!m_socket) {
      m_socket = Socket::CreateSocket(GetNode(),TcpSocketFactory::GetTypeId());
      m_socket->Bind(local);
      m_socket->Listen();
      m_socket->SetAttribute("RcvBufSize",UintegerValue(1024*1024));
      m_socket->SetAcceptCallback(
          MakeNullCallback<bool, Ptr<Socket>, const Address &>(),
          MakeCallback(&Plume::HandleAccept,this)
          );
      m_socket->SetRecvCallback(MakeCallback(&Plume::HandleRead,this));
    }
    for (std::vector<Ipv4Address>::const_iterator i=m_peersAddresses.begin();i!=m_peersAddresses.end();i++) {
      m_peersSockets[*i] = Socket::CreateSocket(GetNode(),TcpSocketFactory::GetTypeId());
      m_peersSockets[*i]->Connect(InetSocketAddress(*i,8787));
    }
}

void Plume::StopApplication(void) {
  NS_LOG_FUNCTION(this);
  for (std::vector<Ipv4Address>::const_iterator i=m_peersAddresses.begin();i!=m_peersAddresses.end();++i) {
    m_peersSockets[*i]->Close();
  }
  if (m_socket) {
    m_socket->Close();
  }
}

std::vector<Ipv4Address> Plume::GetPeersAddresses(void) {
    return m_peersAddresses;
}

/*
std::vector<Ptr<Socket>> Plume::GetPeersSocket(void) {
    return m_peersSockets;
}
*/

void Plume::HandleAccept(Ptr<Socket> socket, const Address &from) {
    NS_LOG_FUNCTION(this<<socket<<from);
    socket->SetRecvCallback(MakeCallback(&Plume::HandleRead,this));
}

void Plume::HandleRead(Ptr<Socket> socket) {
    NS_LOG_FUNCTION(this<<socket);

    Ptr<Packet> packet;
    Address from;
    //double newBlockReceiveTime = Simulator::Now().GetSeconds();

    while (packet = socket->RecvFrom(from)) {
        if (packet->GetSize() == 0) {
            break;
        }

        if (InetSocketAddress::IsMatchingType(from)) {
            std::string delimiter = "#";
            std::string parsedPacket;
            size_t pos = 0;
            char *packetInfo = new char[packet->GetSize()+1];
            std::ostringstream totalStream;
            packet->CopyData(reinterpret_cast<uint8_t*>(packetInfo),packet->GetSize());
            packetInfo[packet->GetSize()] = '\0';
            totalStream << m_bufferedData[from] << packetInfo;
            std::string receivedData(totalStream.str());

            while ((pos = receivedData.find(delimiter)) != std::string::npos) {
                parsedPacket = receivedData.substr(0, pos);
                rapidjson::Document d;
                d.Parse(parsedPacket.c_str());
                if (!d.IsObject()) {
                    receivedData.erase(0,pos+delimiter.length());
                    continue;
                }

                rapidjson::StringBuffer buffer;
                rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
                d.Accept(writer);
                NS_LOG_INFO(GetNode()->GetId()<<" get message from "<<from<<buffer.GetString());

                switch (d["message"].GetInt())
                {
                    case BLOCK:
                        {
                            Block newBlock;
                            newBlock.m_hash = d["hash"].GetString();
                            if (m_localBlocks.find(newBlock.m_hash)!=m_localBlocks.end()) {
                               break;
                            }
                            newBlock.m_seq = d["seq"].GetInt();
                            newBlock.m_timestamp = d["timestamp"].GetDouble();
                            //TODO timeout
                            for (uint8_t i = 0;i < d["parents"].Size();i++) {
                                newBlock.m_parent.push_back(d["parents"][i].GetString());
                            }
                            std::vector<std::string> unfinedParents = AddBlockToLocal(newBlock);
                            Ipv4Address from_ipv4 = InetSocketAddress::ConvertFrom(from).GetIpv4();
                            BroadcastNewBlock(newBlock,from_ipv4,true);
                            if (unfinedParents.size()>0) {
                                SendBlocksReq(unfinedParents,from_ipv4);
                            }
                            // must have break!!!
                            break;
                        }
                    case GET_BLOCKS_REQ:
                        {
                            Ipv4Address from_ipv4 = InetSocketAddress::ConvertFrom(from).GetIpv4();
                            for (uint8_t i = 0;i < d["parents"].Size();i++) {
                                std::string parent = d["parents"][i].GetString();
                                Block block = m_localBlocks[parent];
                                SendBlock(block,from_ipv4);
                            }
                            break;
                        }
                    case GET_BLOCKS_RESP:
                        {
                            Block newBlock;
                            newBlock.m_seq = d["seq"].GetInt();
                            newBlock.m_hash = d["hash"].GetString();
                            newBlock.m_timestamp = d["timestamp"].GetDouble();
                            //TODO timeout
                            for (uint8_t i = 0; i < d["parents"].Size(); i++) {
                                newBlock.m_parent.push_back(d["parents"][i].GetString());
                            }
                            AddBlockToLocal(newBlock);
                            break;
                        }
                }

                receivedData.erase(0,pos+delimiter.length());
            }

            m_bufferedData[from] = receivedData;
            delete []packetInfo;
        }
    }
}

// flag = true : broadcast except from
// flag = false : broadcast all
void Plume::BroadcastNewBlock(const Block &block,Ipv4Address from,bool flag) {  
    NS_LOG_FUNCTION(this);
    const uint8_t delimiter[] = "#";
    rapidjson::Document d;
    rapidjson::Value value;
    rapidjson::Value parents(rapidjson::kArrayType);
    d.SetObject();

    value = BLOCK;
    d.AddMember("message",value,d.GetAllocator());
    value.SetString(block.m_hash.c_str(),block.m_hash.size(),d.GetAllocator());
    d.AddMember("hash",value,d.GetAllocator());
    value.SetInt(block.m_seq);
    d.AddMember("seq",value,d.GetAllocator());
    value.SetDouble(block.m_timestamp);
    d.AddMember("timestamp",value,d.GetAllocator());
    for (std::vector<std::string>::const_iterator i=block.m_parent.begin();i!=block.m_parent.end();++i) {
        value.SetString((*i).c_str(),(*i).size(),d.GetAllocator());
        parents.PushBack(value,d.GetAllocator());
    }
    d.AddMember("parents",parents,d.GetAllocator());

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    d.Accept(writer);

    for (std::vector<Ipv4Address>::const_iterator i=m_peersAddresses.begin();i!=m_peersAddresses.end();++i) {
        if (*i == from) {
            if (flag) {
                continue;
            }
            else {
                m_peersSockets[*i]->Send(reinterpret_cast<const uint8_t*>(buffer.GetString()),buffer.GetSize(),0);
                m_peersSockets[*i]->Send(delimiter,1,0);
            }
        }
        m_peersSockets[*i]->Send(reinterpret_cast<const uint8_t*>(buffer.GetString()),buffer.GetSize(),0);
        m_peersSockets[*i]->Send(delimiter,1,0);
    }
}

Block Plume::CreateNewBlock(void) {
    NS_LOG_FUNCTION(this);
    // no transactions
    Block block(m_seq);
    std::vector<std::string> tips = FindAllTips();
    block.m_parent = tips;
    block.m_hash = block.CalBlockHash();
    // padding block
    m_localBlocks[block.m_hash] = block;
    return block;
}

std::vector<std::string> Plume::FindAllTips(void) {
    NS_LOG_FUNCTION(this);
    std::vector<std::string> tipHashList;
    for (std::map<std::string,Block>::const_iterator iter=m_localBlocks.begin();iter!=m_localBlocks.end();++iter) {
        if (m_blockHelpers[iter->second.m_hash].m_children.size() == 0) {
            tipHashList.push_back(iter->first);
        }
    }
    return tipHashList;
}

std::vector<std::string> Plume::AddBlockToLocal(Block& block) {
    m_localBlocks[block.m_hash] = block;
    BlockHelper blockHelper = BlockHelper(block,block.m_parent);
    m_blockHelpers[block.m_hash] = blockHelper;

    // update local dag
    std::vector<std::string> unfinedParents;
    for (std::vector<std::string>::const_iterator i=block.m_parent.begin();i!=block.m_parent.end();++i) {
        std::map<std::string,Block>::const_iterator iter = m_localBlocks.find(*i);
        // parent not in dag
        if (iter == m_localBlocks.end()) {
            //TODO timeout
            unfinedParents.push_back(*i);
        }
        // parent in dag : add edge
        else {
            m_blockHelpers[iter->second.m_hash].m_children.push_back(block.m_hash);
        }
    }
    return unfinedParents;
}

void Plume::SendBlocksReq(std::vector<std::string> unfinedParents, Ipv4Address from) {
    NS_LOG_FUNCTION(this);
    const uint8_t delimiter[] = "#";
    rapidjson::Document d;
    rapidjson::Value value;
    rapidjson::Value parents(rapidjson::kArrayType);
    d.SetObject();
    value = GET_BLOCKS_REQ;
    d.AddMember("message",value,d.GetAllocator());
    for (std::vector<std::string>::const_iterator i = unfinedParents.begin(); i != unfinedParents.end(); i++) {
        value.SetString((*i).c_str(),(*i).size(),d.GetAllocator());
        parents.PushBack(value,d.GetAllocator());
    }
    d.AddMember("parents",parents,d.GetAllocator());

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    d.Accept(writer);

    m_peersSockets[from]->Send(reinterpret_cast<const uint8_t*>(buffer.GetString()),buffer.GetSize(),0);
    m_peersSockets[from]->Send(delimiter,1,0);
    NS_LOG_INFO(GetNode()->GetId()<<"send req to "<<from<<buffer.GetString());
}

void Plume::SendBlock(const Block& block,Ipv4Address dst) {
    NS_LOG_FUNCTION(this);
    const uint8_t delimiter[] = "#";
    rapidjson::Document d;
    rapidjson::Value value;
    rapidjson::Value parents(rapidjson::kArrayType);
    d.SetObject();
    value = GET_BLOCKS_RESP;
    d.AddMember("message",value,d.GetAllocator());
    value.SetString(block.m_hash.c_str(),block.m_hash.size(),d.GetAllocator());
    d.AddMember("hash",value,d.GetAllocator());
    value.SetInt(block.m_seq);
    d.AddMember("seq",value,d.GetAllocator());
    value.SetDouble(block.m_timestamp);
    d.AddMember("timestamp",value,d.GetAllocator());
    for (std::vector<std::string>::const_iterator i = block.m_parent.begin(); i != block.m_parent.end(); ++i) {
        value.SetString((*i).c_str(),(*i).size(),d.GetAllocator());
        parents.PushBack(value,d.GetAllocator());
    }
    d.AddMember("parents",parents,d.GetAllocator());

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    d.Accept(writer);

    m_peersSockets[dst]->Send(reinterpret_cast<const uint8_t*>(buffer.GetString()),buffer.GetSize(),0);
    m_peersSockets[dst]->Send(delimiter,1,0);
    NS_LOG_INFO("send block resp finish");
}

void Plume::GetNewBlock(void) {
    Block block = CreateNewBlock();
    BroadcastNewBlock(block,Ipv4Address::GetAny(),false);
}
