#ifndef BLOCKHELPER_H
#define BLOCKHELPER_H

#include "plume-simulator/block.h"

#include <vector>

using namespace ns3;

class BlockHelper {
public:
    BlockHelper(const Block &block, std::vector<string> parent);
    virtual ~BlockHelper(void);

    void AddChild(string child);

    Block                 m_block;
    std::vector<string>   m_parent;
    std::vector<string>   m_children;

};

#endif