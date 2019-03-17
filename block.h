#ifndef BLOCK_H
#define BLOCK_H

#include <algorithm>

class Block {
public:
    Block();
    virtual ~Block(void);

    TODO:区块包含哪些信息
    int                 m_ownerId;
    std::string         m_hash;
    std::vector<string> m_refHashList;
};

#endif