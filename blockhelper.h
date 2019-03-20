#ifndef BLOCKHELPER_H
#define BLOCKHELPER_H

#include "plume-simulator/block.h"

using namespace ns3;

class BlockHelper {
public:
    BlockHelper();
    virtual ~BlockHelper(void);

    Block             block;
    std::vector<Block> m_parent;
    std::vector<Block> m_children;

};

#endif