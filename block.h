#ifndef BLOCK_H
#define BLOCK_H

#include <time.h>
#include <algorithm>
#include <sstream>

namespace ns3 {

class Block {
public:
    Block(void);
    Block(int seq);
    virtual ~Block(void);

    std::string CalBlockHash(void);
    bool IsBlockValid(void);

    //TODO:区块包含哪些信息:不包括交易;时间戳表示;
    int                 m_seq;
    double              m_timestamp;
    std::string         m_hash;
    std::vector<std::string> m_parent;
};

}
#endif
