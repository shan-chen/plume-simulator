#include "plume-simulator/blockhelper.h"

BlockHelper::BlockHelper(const Block &block,std::vector<string> parent) {
    m_block = block;
    m_parent = parent;
}

BlockHelper::~BlockHelper(void) {

}

void BlockHelper::AddChild(string child) {
    m_children.push_back(child);
}

