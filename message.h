#ifndef MESSAGE_H
#define MESSAGE_H

using namespace ns3;

enum Messages {
    INV,              //0
    GET_HEADERS,      //1
    HEADERS,          //2
    GET_BLOCKS,       //3
    BLOCK,            //4
    GET_DATA,         //5
    NO_MESSAGE,       //6
    EXT_INV,          //7
    EXT_GET_HEADERS,  //8
    EXT_HEADERS,      //9
    EXT_GET_BLOCKS,   //10
    CHUNK,            //11
    EXT_GET_DATA,     //12
};

#endif