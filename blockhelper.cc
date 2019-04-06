#include "blockhelper.h"
#include "block.h"

using namespace ns3;

BlockHelper::BlockHelper(void) {

}

BlockHelper::BlockHelper(Block &block,std::vector<std::string> parent) {
    m_block = block;
    m_parent = parent;
}

BlockHelper::~BlockHelper(void) {

}

void BlockHelper::AddChild(std::string child) {
    m_children.push_back(child);
}





