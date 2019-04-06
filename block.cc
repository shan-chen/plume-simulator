#include "block.h"

#include "ns3/simulator.h"

using namespace ns3;

Block::Block() {

}

Block::Block(int seq) {
    m_timestamp = Simulator::Now().GetSeconds();
    m_seq = seq;
}

Block::~Block(void) {

}

std::string Block::CalBlockHash(void) {
    //TODO calculate block hash
    std::stringstream stream;
    stream << m_seq;
    return stream.str();
}

