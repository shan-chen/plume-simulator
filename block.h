#ifndef BLOCK_H
#define BLOCK_H

#include <algorithm>

class Block {
public:
    Block();
    virtual ~Block(void);

    bool IsBlockValid(void);

    TODO:区块包含哪些信息:不包括交易;时间戳表示;
    int                 m_ownerId;
    std::string         m_hash;
    std::vector<string> m_parent;
};

#endif