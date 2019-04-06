#ifndef BLOCKHELPER_H
#define BLOCKHELPER_H

#include "block.h"

#include <vector>

namespace ns3 {

class BlockHelper {
public:
    BlockHelper(void);
    BlockHelper(Block &block, std::vector<std::string> parent);
    virtual ~BlockHelper(void);

    void AddChild(std::string child);

    Block                      m_block;
    std::vector<std::string>   m_parent;
    std::vector<std::string>   m_children;


};

}
#endif
