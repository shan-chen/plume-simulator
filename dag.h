#ifndef DAG_H
#define DAG_H

#include <algorithm>

#include "plume-simulator/block.h"

class DAG {
public:
    DAG(void);
    virtual ~DAG(void);

    TODO:dag包含信息
    std::map<std::vector<string>,std::vector<string>> m_dag;
};

#endif