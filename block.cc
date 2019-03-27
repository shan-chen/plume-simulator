#include "plume-simulator/block.h"

using namespace ns3;

Block::Block(seq) {
    m_timestamp = Simulator::Now().GetSeconds();
    m_seq = seq;
}

Block::~Block(void) {

}

std::string Block::CalBlockHash(void) {
    //TODO calculate block hash
}
