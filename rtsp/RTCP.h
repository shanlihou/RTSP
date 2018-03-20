#pragma once
#include "RTP.h"
class RTCP :public UDP
{
public:
    RTCP(UINT32 srcPort, const char *dstAddr, UINT32 dstPort);
private:
    RTP rtp;
};