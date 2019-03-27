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

#include"plume-simulator/plume.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Plume");

NS_OBJECT_ENSURE_REGISTERED (Plume);

TypeId Plume::GetTypeId(void) {
    static TypeId tid = TypeId("ns3::Plume");
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

std::vector<Ipv4Address> Plume::GetPeersAddresses(void) {
    return m_peersAddresses;
}

std::vector<Ptr<Socket>> Plume::GetPeersSocket(void) {
    return m_peersSockets;
}

void Plume::HandleAccept(Ptr<Socket> socket, const Address &from) {
    NS_LOG_FUNCTION(this<<socket<<from);
    socket->SetRecvCallBack(MakeCallBack(&BitcoinNode::HandleRead,this));
}

void Plume::HandleRead(Ptr<Socket> socket) {
    NS_LOG_FUNCTION(this<<socket);
    Ptr<Packet> packet;
    Address from;
    double newBlockReceiveTime = Simulator::Now ().GetSeconds();

    while (packet = socket.RecvFrom(from)) {
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
                    continue
                }

                rapidjson::StringBuffer buffer;
                rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
                d.Accept(writer);
                NS_LOG_INFO("get message"<<buffer.GetString());

                switch (d["message"].GetInt())
                {
                    case BLOCK:
                        {
                            //TODO: 1.
                        }
                    case GET_BLOCK_REQ:
                        {
                            //TODO:
                        }
                    case GET_BLOCK_RESP:
                        {
                            //TODO:
                        }
                }

                receivedData.erase(0,pos+delimiter.length());
            }

            m_bufferedData[from] = receivedData;
            delete []packetInfo;
        }
    }
}

void Plume::SendMessage(Messages recvType, Messages respType, rapidjson::Document &d,Ptr<Socket> socket) {

}

// flag = true : broadcast except from
// flag = false : broadcast all
void Plume::BroadcastNewBlock(const Block &block,Ipv4Address from,bool flag) {
    const uint8_t delimiter[] = "#";
    rapidjson::Document d;
    rapidjson::Value value;
    rapidjson::Value parents(rapidjson::kArrayType);
    std::ostringstream stringStream;
    std::string blockHash = stringStream.str();
    d.SetObject();

    value = BLOCK;
    d.AddMember("message",value,d.GetAllocator());
    value.SetString(block.m_hash);
    d.AddMember("hash",value,d.GetAllocator());
    value.SetInt(block.m_seq);
    d.AddMember("seq",value,d.GetAllocator());
    value.SetDouble(block.m_timestamp);
    for (std::vector<std::string>const_iterator i=block.m_parent.begin();i!=block.m_parent.end();++i) {
        value.SetString(*i);
        parents.PushBack(value,d.GetAllocator());
    }
    d.AddMember("parents",parents,d.GetAllocator());

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    d.Accept(writer);

    const uint8_t delimiter[] = "#";
    for (std::vector<IPv4Address>::const_iterator i=m_peersAddresses.begin();i!=m_peersAddresses.end();++i) {
        if (*i == from) {
            if (flag) {
                continue
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
    // no transactions
    Block block(m_seq);
    std::vector<std::string> tips = FindAllTips();
    block.m_parent = tips;
    block.m_hash = block.CalBlockHash();
    return block;
}

std::vector<std::string> Plume::FindAllTips(void) {
    std::vector<std::string> tipHashList;
    for (map<std::string,Block*>::const_iterator iter=m_localBlocks.begin();iter!=m_localBlocks.end();++iter) {
        if (m_blockHelpers[iter->second]->m_children.size() == 0) {
            tipHashList.push_back(iter->first);
        }
    }
    return tipHashList;
}

void Plume::AddBlockToLocal(const Block& block) {
    m_localBlocks[block.m_hash] = &block;
    blockHelper = BlockHelper::BlockHelper(block,block.m_parent);
    m_blockHelpers[&block] = &blockHelper;

    // update dag
    for (std::vector<std::string>::const_iterator i=block.m_parent.begin();iter!=block.m_parent.end();++i) {
        map<std::string,Block*> const_iterator iter = m_localBlocks.find(*i);
        // parent not in dag
        if (iter == m_localBlocks.end()) {
            // TODO handle parent not in dag
            time_t currentTime = time(nullptr));
        }
        // parent in dag : add edge
        else {
            m_blockHelpers[iter->second]->m_children.push_back(block.m_hash);
        }

    }
}
