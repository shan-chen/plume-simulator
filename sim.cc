#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-nix-vector-helper.h"
#include "ns3/topology-read-module.h"

#include <vector>

#include "plume-simulator/plume.h"

using namespace ns3;

const uint16_t listenPort=8787;

int main(int argc, char const *argv[])
{
    //TODO:mpi-interface : set parallel communication

    std::string format ("Inet");
    std::string input ("src/topology-read/examples/100ms.txt");               //1000
    // std::string input ("src/topology-read/examples/Inet_toposample.txt");  //4000

    CommandLine cmd;
    cmd.AddValue("format", "Format to use for data input [Orbis|Inet|Rocketfuel].", format);
    cmd.AddValue("input", "Name of the input file.", input);
    cmd.Parse(argc, argv);

    // 读取拓扑
    TopologyReaderHelper topoHelp;
    topoHelp.SetFileName(input);
    topoHelp.SetFileType(format);
    Ptr<TopologyReader> topoReader = topoHelp.GetTopologyReader();
    if (topoReader == 0) {
        NS_LOG_ERROR("Problems get topology file. Exit.");
        return -1;
    }

    NodeContainer nodes = topoReader->Read();

    if (topoReader->LinksSize() == 0) {
        NS_LOG_ERROR("Problems reading the topology file. Exit.");
        return -1;
    }

    // 给节点安装协议栈
    NS_LOG_INFO("creating internet stack")
    InternetStackHelper stack;
    stack.Install(nodes);

    // 一条链路为一个对象
    int totalLinks = topoReader->LinksSize();
    NS_LOG_INFO("creating node containers");
    NodeContainer* nc = new NodeContainer[totalLinks];
    std::string* delay=new std::string[totalLinks];

    TopologyReader::ConstLinksIterator iter;
    int i=0;
    for (iter = topoReader->LinksBegin(); iter != topoReader->LinksEnd(); iter++,i++) {
        nc[i] = NodeContainer (iter->GetFromNode(), iter->GetToNode());
        delay[i]=iter->GetAttribute("Weight")+"ms";
    }

    // 安装网络设备
    NS_LOG_INFO ("creating net device containers");
    NetDeviceContainer* ndc = new NetDeviceContainer[totalLinks];
    PointToPointHelper p2p;
    for (int i = 0; i < totalLinks; i++) {
        p2p.SetChannelAttribute("Delay",StringValue(delay[i]));
        p2p.SetDeviceAttribute("DataRate", StringValue ("70Mbps"));
        //Ptr<RateErrorModel> em = CreateObject<RateErrorModel> ();
        //em->SetAttribute("ErrorRate", DoubleValue (0.00001));
        //p2p.SetDeviceAttribute("ReceiveErrorModel", PointerValue (em));
        ndc[i] = p2p.Install(nc[i]);
    }
    delete[] delay;

    // 创建子网
    NS_LOG_INFO("setting ipv4 addresses")
    Ipv4AddressHelper address;
    address.SetBase("10.0.0.0", "255.255.255.252");
    NS_LOG_INFO("creating ipv4 interfaces");
    Ipv4InterfaceContainer* ipInterfaces = new Ipv4InterfaceContainer[totalLinks];
    for (int i = 0; i < totalLinks; i++) {
        ipInterfaces[i] = address.Assign(ndc[i]);
        address.NewNetwork();
    }

    NS_LOG_INFO ("creating ipv4NeighMap");
    std::map<uint32_t, std::vector<Ipv4Address>> ipv4NeighMap;
    i=0;
    for (iter = topoReader->LinksBegin(); iter != topoReader->LinksEnd(); iter++, i++) {
        uint32_t from = iter->GetFromNode()->GetId();
        uint32_t to = iter->GetToNode()->GetId();

        std::map<uint32_t,std::vector<Ipv4Address>>::iterator it_from = ipv4NeighMap.find(from);
        if (it_from == ipv4NeighMap.end()) {
            std::vector<Ipv4Address> v;
            v.push_back(ipInterfaces[i].GetAddress(1));
            ipv4NeighMap.insert(std::make_pair(from,v));
        } else {
            ipv4NeighMap[from].push_back(ipInterfaces[i].GetAddress(1));
        }

        std::map<uint32_t,std::vector<Ipv4Address>>::iterator it_to = ipv4NeighMap.find(to);
        if (it_to == ipv4NeighMap.end()) {
            std::vector<Ipv4Address> v;
            v.push_back(ipInterfaces[i].GetAddress(0));
            ipv4NeighMap.insert(std::make_pair(to,v));
        } else {
            ipv4NeighMap[to].push_back(ipInterfaces[i].GetAddress(0));
        }
    }

    // Ipv4Address::GetAny():"0.0.0.0"
    InetSocketAddress dst = InetSocketAddress(Ipv4Address::GetAny(),listenPort);

    // 安装应用
    NS_LOG_INFO("add application plume")
    NodeContainer::Iterator i;
    int count = 1;
    for (i=nodes.Begin();i!=nodes.End();++i) {
        uint32_t id = (*i)->GetId();
        Ptr<Socket> socket  = Socket::CreateSocket((*i)),TcpSocketFactory::GetTypeId());
        //TODO : set socket buffer size
        //socket->SetAttribute("RcvBufSize", UintegerValue(totalTxBytes));
        socket->Bind(dst);
        socket->Listen();
        socket->SetAcceptCallback (
            MakeNullCallback<bool, Ptr<Socket>, const Address &> (),
            MakeCallback(&Plume::HandleAccept)
        );
        socket->SetRecvCallback(MakeCallback(&Plume::HandleRead));

        Ptr<Plume> plume = CreateObject<Plume>();
        (*i)->SetApplication(plume);

        plume->m_nodeID = id;
        plume->m_seq = count;
        plume->m_socket = socket;
        plume->m_numOfPeers = ipv4NeighMap[id].size();
        plume->m_peersAddresses = ipv4NeighMap[id];

        plume->SetStartTime(Seconds(1));
        //TODO set block time
        Simulator::Schedule(Seconds(count+1),&Plume::GetNewBlock);

        count++;
    }

    // 创建socket连接peers
    for (i=nodes.Begin();i!=nodes.End();++i) {
        uint32_t id = (*i)->GetId();
        std::map<Ipv4Address,Ptr<Socket>> peersSockets;
        std::vector<Ipv4Address> peersAddresses = ipv4NeighMap[id];
        for (std::vector<Ipv4Address>::const_iterator j = peersAddresses.begin(); j != peersAddresses.end(); ++j) {
            peersSockets[*j] = Socket::CreateSocket((*i),TcpSocketFactory::GetTypeId());
            peersSockets[*j]->Connect(InetSocketAddress(*j,listenPort));
        }
        //TODO:再想想怎么搞
        dynamic_cast<Plume*>((*i)->GetApplication(0))->m_peersSockets = peersSockets;
    }

    NS_LOG_INFO("Simulation Start");
    Simulator::Run();
    Simulator::Destroy();

    delete[] ipInterfaces;
    delete[] ndc;
    delete[] nc;

    NS_LOG_INFO ("Simulation is over");

    return 0;
}
