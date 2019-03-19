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