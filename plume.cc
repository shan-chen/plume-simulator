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
            TODO:结束符还是
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
                //TODO:1.解析出block数据; 2.处理blockhelper; 3.广播(除from)

                receivedData.erase(0,pos+delimiter.length());
            }

            m_bufferedData[from] = receivedData;
            delete []packetInfo;
        }
    }
}

void Plume::SendMessage(Messages recvType, Messages respType, rapidjson::Document &d,Ptr<Socket> socket) {

}

void Plume::BroadcastNewBlock(const Block &block,Ipv4Address from) {
    for (std::vector<Ipv4Address>::const_iterator i=m_peersAddresses.begin();i!=m_peersAddresses.end();++i) {
        if (*i != from) {
            const uint8_t delimiter[] = "#";
            TODO:发送区块
            m_peersSockets[*i]->Send();
            m_peersSockets[*i]->Send(delimiter,1,0);
        }
    }
}

void Plume::BroadcastNewBlock(const Block &block) {
    for (std::vector<Ipv4Address>::const_iterator i=m_peersAddresses.begin();i!=m_peersAddresses.end();++i) {
        const uint8_t delimiter[] = "#";
        TODO:发送区块
        m_peersSockets[*i]->Send();
        m_peersSockets[*i]->Send(delimiter,1,0);
    }
}

