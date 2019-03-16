#include <ctime>
#include <sstream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-nix-vector-helper.h"
#include "ns3/topology-read-module.h"
#include <list>
#include "wifi-example-apps.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("TopologyCreationExperiment");

const uint16_t listenPort=12345;
const int HELLO_SIZE=30;
std::map<uint32_t,std::vector<Ipv4Address>> ipv4NeighMap;

void Init(Ptr<Node> n, Ipv4Address dstaddr);
void HandleAccept (Ptr<Socket>, const Address &);
   
TimestampTag ReqTag;
void SetReqTag();

int main(int argc, char const *argv[])
{
    std::string format ("Inet");
    // std::string input ("src/topology-read/examples/double.txt");          //2
    // std::string input ("src/topology-read/examples/triple.txt");          //3
    // std::string input ("src/topology-read/examples/tree.txt");          //10
    std::string input ("src/topology-read/examples/100ms.txt");        //1000
    // std::string input ("src/topology-read/examples/Inet_toposample.txt");  //4000

    // Set up command line parameters used to control the experiment.
    CommandLine cmd;
    cmd.AddValue("format", "Format to use for data input [Orbis|Inet|Rocketfuel].",
                format);
    cmd.AddValue("input", "Name of the input file.",
                input);
    cmd.Parse(argc, argv);

    // ------------------------------------------------------------
    // -- Read topology data.
    // --------------------------------------------

    // Pick a topology reader based in the requested format.
    TopologyReaderHelper topoHelp;
    topoHelp.SetFileName(input);
    topoHelp.SetFileType(format);
    Ptr<TopologyReader> topoReader = topoHelp.GetTopologyReader();
    if (topoReader == 0) {
        NS_LOG_ERROR("Problems get topology file. Failing.");
        return -1;
    }

    NodeContainer nodes = topoReader->Read();

    if (topoReader->LinksSize() == 0) {
        NS_LOG_ERROR("Problems reading the topology file. Failing.");
        return -1;
    }

    NS_LOG_INFO("creating internet stack")
    InternetStackHelper stack;
    stack.Install(nodes);

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
    delete[] delay;

    // NS_LOG_INFO ("creating net device containers");
    NetDeviceContainer* ndc = new NetDeviceContainer[totalLinks];
    PointToPointHelper p2p;
    for (int i = 0; i < totalLinks; i++) {
        p2p.SetChannelAttribute("Delay",StringValue(delay[i]));
        p2p.SetDeviceAttribute("DataRate", StringValue ("70Mbps"));

        Ptr<RateErrorModel> em = CreateObject<RateErrorModel> ();
        em->SetAttribute("ErrorRate", DoubleValue (0.00001));
        p2p.SetDeviceAttribute("ReceiveErrorModel", PointerValue (em));
        ndc[i] = p2p.Install(nc[i]);
    }

    // creates little subnets, one for each couple of nodes.
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

    // Tpv4Address::GetAny():返回0.0.0.0
    InetSocketAddress dst = InetSocketAddress(Ipv4Address::GetAny(),listenPort);
    // NodeContainer::GetN():返回node总数
    for (uint32_t i = 0; i < nodes.GetN(); i++ ) {
        Ptr<Socket> dstSocket = Socket::CreateSocket(nodes.Get(i),TcpSocketFactory::GetTypeId());
        dstSocket->SetAttribute("RcvBufSize", UintegerValue(totalTxBytes));
        dstSocket->Bind(dst);
        dstSocket->Listen();
        dstSocket->SetAcceptCallback (
            MakeNullCallback<bool, Ptr<Socket>, const Address &> (),
            MakeCallback(&HandleAccept)
        );
    }

    // AnimationInterface anim ("animation.xml");
    // p2p.EnablePcapAll ("tcp-socket-piece-200ms");

    Simulator::Schedule(Seconds(0.01),&SetReqTag);
    for(int i=0;i<count;i++) {
        Simulator::Schedule(Seconds(0.1+0.0001*i),&Init,nodes.Get(0),ipInterfaces[0].GetAddress(1));
    }

    NS_LOG_INFO("Simulation start");
    Simulator::Run();
    Simulator::Destroy();

    delete[] ipInterfaces;
    delete[] ndc;
    delete[] nc;

    NS_LOG_INFO ("Simulation is over");

    return 0;
}

void SetReqTag() {
    ReqTag.SetTimestamp (Simulator::Now ());
}

void HandleAccept (Ptr<Socket> s, const Address& from) {
    NS_LOG_INFO("HandleAccept");
    s->SetRecvCallback (MakeCallback (&dstSocketRecv));
    TODO:
}

void HandleRecv (Ptr<Socket> s) {

}

void  Init( Ptr<Node> n,Ipv4Address dstaddr) {
    TimestampTag initBlockTag;
    initBlockTag.SetTimestamp(Simulator::Now ());
    NS_ASSERT(!initBlockTag.Equal(ReqTag));
    SendHelloPacketWithTag(n,dstaddr,HELLO_SIZE,initBlockTag);
    return;
}

void SendHelloPacketWithTag(Ptr<Node> n,Ipv4Address dst,int size,Tag tag) {
    NS_ASSERT(size>0);
    Ptr<Socket> sendSocket = Socket::CreateSocket(n,TcpSocketFactory::GetTypeId());
    sendSocket->Connect(InetSocketAddress(dst,listenPort)); 
    Ptr<Packet> p = Create<Packet> (reinterpret_cast<const uint8_t*> ("hello"),5,true);
    p->AddByteTag(tag);
    sendSocket->Send(p);
    if(sendSocket) {
        sendSocket->Close();
    }
    return;
}


